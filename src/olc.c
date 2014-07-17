/* olc.c: methods pertainingto the OLC written by Davenge */

#include "mud.h"

INCEPTION *init_olc( void )
{
   INCEPTION *olc;

   CREATE( olc, INCEPTION, 1 );
   olc->commands = AllocList();
   olc->wSpaces = AllocList();
   if( clear_olc( olc ) != RET_SUCCESS )
   {
      free_olc( olc );
      return NULL;
   }
   return olc;
}

int clear_olc( INCEPTION *olc )
{
   int ret = RET_SUCCESS;
   olc->using_workspace = NULL;
   olc->editing = NULL;
   if( olc->editor_commands )
   {
      free_command_list( olc->editor_commands );
      olc->editor_commands = NULL;
   }
   return ret;
}

int free_olc( INCEPTION *olc )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;
   int ret = RET_SUCCESS;

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      unuse_workspace( wSpace, olc->account );
   DetachIterator( &Iter );

   CLEARLIST( olc->wSpaces, WORKSPACE );
   FreeList( olc->wSpaces );
   olc->wSpaces = NULL;

   free_command_list( olc->commands );
   FreeList( olc->commands );
   olc->commands = NULL;

   olc->account = NULL;
   olc->using_workspace = NULL;

   if( olc->editor_commands )
   {
      free_command_list( olc->editor_commands );
      olc->editor_commands = NULL;
   }
   FREE( olc );

   return ret;
}

WORKSPACE *init_workspace( void )
{
   WORKSPACE *wSpace;

   CREATE( wSpace, WORKSPACE, 1 );
   wSpace->tag = init_tag();
   wSpace->tag->type = WORKSPACE_IDS;
   wSpace->frameworks = AllocList();
   wSpace->instances = AllocList();
   wSpace->who_using = AllocList();
   if( clear_workspace( wSpace ) != RET_SUCCESS )
   {
      free_workspace( wSpace );
      return NULL;
   }
   return wSpace;

}

int clear_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   FREE( wSpace->name );
   wSpace->name = strdup( "new workspace" );
   FREE( wSpace->description );
   wSpace->description = strdup( "blank description" );
   wSpace->Public = FALSE;

   /* this needs to eventually clear lists */

   return ret;
}

int free_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( wSpace->tag )
      free_tag( wSpace->tag );

   CLEARLIST( wSpace->frameworks, ENTITY_FRAMEWORK );
   FreeList( wSpace->frameworks );
   wSpace->frameworks = NULL;
   FreeList( wSpace->instances );
   wSpace->instances = NULL;
   CLEARLIST( wSpace->who_using, ACCOUNT_DATA );
   FreeList( wSpace->who_using );
   wSpace->who_using = NULL;


   FREE( wSpace->name );
   FREE( wSpace->description );
   FREE( wSpace );

   return ret;
}

WORKSPACE *get_active_workspace( const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !name || name[0] == '\0' )
      return NULL;
   if( SizeOfList( active_wSpaces ) < 1 )
      return NULL;

   AttachIterator( &Iter, active_wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( name, wSpace->name ) )
         break;
   DetachIterator( &Iter );

   return wSpace;
}

WORKSPACE *load_workspace( const char *name )
{
   WORKSPACE *wSpace;
   MYSQL_RES *result;
   MYSQL_ROW row;

   if( !name || name[0] == '\0' )
      return NULL;
   if( !quick_query( "SELECT * FROM workspaces WHERE name='%s';", name ) )
      return NULL;
   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
      return NULL;
   if( mysql_num_rows( result ) < 1 )
      return NULL;
   if( ( row = mysql_fetch_row( result ) ) == NULL )
   {
      mysql_free_result( result );
      return NULL;
   }

   wSpace = init_workspace();
   db_load_workspace( wSpace, &row );
   load_workspace_entries( wSpace );
   return wSpace;
}

void db_load_workspace( WORKSPACE *wSpace, MYSQL_ROW *row )
{
   wSpace->tag->id = atoi( (*row)[0] );
   wSpace->tag->type = atoi( (*row)[1] );
   wSpace->tag->created_by = strdup( (*row)[2] );
   wSpace->tag->created_on = strdup( (*row)[3] );
   wSpace->tag->modified_by = strdup( (*row)[4] );
   wSpace->tag->modified_on = strdup( (*row)[5] );
   wSpace->name = strdup( (*row)[6] );
   wSpace->description = strdup( (*row)[7] );
   wSpace->Public = (bool)atoi( (*row)[8] );
   return;
}

void unuse_workspace( WORKSPACE *wSpace, ACCOUNT_DATA *account )
{
   if( account->olc->using_workspace == wSpace )
      account->olc->using_workspace = NULL;
   DetachFromList( account, wSpace->who_using );
   if( SizeOfList( wSpace->who_using ) < 1 )
   {
      DetachFromList( wSpace, active_wSpaces );
      free_workspace( wSpace );
   }
   else
   {
      ACCOUNT_DATA *other_accounts_using;
      ITERATOR Iter;

      AttachIterator( &Iter, wSpace->who_using );
      while( ( other_accounts_using = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
         text_to_account( other_accounts_using, "%s is no longer using %s workspace.\r\n", account->name, wSpace->name );
      DetachIterator( &Iter );
   }
   return;
}

void inception_open( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   text_to_account( account, "Opening Inception OLC...\r\n\r\n" );
   if( !account->olc )
   {
      account->olc = init_olc();
      account->olc->account = account;
   }
   change_socket_state( account->socket, STATE_OLC );
   return;
}

int olc_prompt( D_SOCKET *dsock )
{
   ENTITY_FRAMEWORK *frame;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   ACCOUNT_DATA *account = dsock->account;
   INCEPTION *olc;
   WORKSPACE *wSpace;
   ITERATOR Iter, IterF, IterI;
   char tempstring[MAX_BUFFER];
   int ret = RET_SUCCESS;
   int space_after_pipes, max_list, max_frameworks, max_instances, x, style;
   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   if( ( olc = account->olc ) == NULL )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   space_after_pipes = account->pagewidth - 2;

   bprintf( buf, "/%s\\\r\n", print_header( "Inception OLC", "-", space_after_pipes ) );
   mud_printf( tempstring, " You have %d workspaces loaded.", SizeOfList( olc->wSpaces ) );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   if( SizeOfList( olc->wSpaces ) > 0 )
   {
      AttachIterator( &Iter, olc->wSpaces );
      while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      {
         mud_printf( tempstring, "- %s", wSpace->name );
         bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
      }
      DetachIterator( &Iter );
   }
   if( olc->using_workspace )
   {
      max_frameworks = SizeOfList( olc->using_workspace->frameworks );
      max_instances = SizeOfList( olc->using_workspace->instances );
      max_list = UMAX( SizeOfList( olc->using_workspace->frameworks ), SizeOfList( olc->using_workspace->instances ) );

      if( max_list == max_frameworks )
         style = 1;
      else
         style = 2;

      mud_printf( tempstring, "%s Workspace", olc->using_workspace->name );
      bprintf( buf, "|%s|\r\n", print_header( tempstring, "-", space_after_pipes ) );


      switch( style )
      {
         case 1:
            bprintf( buf, "|%s|", print_header( "Frameworks", " ", ( space_after_pipes - 1 ) / 2 ) );
            bprintf( buf, " %s|\r\n", print_header( "Instances", " ", ( space_after_pipes - 1 ) / 2 ) );
            break;
         case 2:
            bprintf( buf, "|%s|", print_header( "Instances", " ", ( space_after_pipes - 1 ) / 2 ) );
            bprintf( buf, " %s|\r\n", print_header( "Frameworks", " ", ( space_after_pipes - 1 ) / 2 ) );
            break;
      }
      bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );

      AttachIterator( &IterF, olc->using_workspace->frameworks );
      for( x = 0; x < max_list; x++ )
      {
         frame = (ENTITY_FRAMEWORK *)NextInList( &IterF );
         switch( style )
         {
            case 1:
               mud_printf( tempstring, "%s", frame ? frame->name : " " );
               bprintf( buf, "|%s|", print_header( tempstring, " ", ( space_after_pipes - 1 ) / 2 ) );
               bprintf( buf, " %s|\r\n", print_header( "  ", " ", ( space_after_pipes - 1 ) / 2 ) );
               break;
            case 2:
               bprintf( buf, "|%s|", print_header( "  ", " ", ( space_after_pipes - 1 ) / 2 ) );
               mud_printf( tempstring, "%s", frame ? frame->name : " " );
               bprintf( buf, " %s|\r\n", print_header( tempstring, " ", ( space_after_pipes - 1 ) / 2 ) );
               break;
         }
      }
      DetachIterator( &IterF );
   }
   bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );
   print_commands( dsock->account->olc, dsock->account->olc->commands, buf, 0, account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_header( "Version 0.1", "-", space_after_pipes ) );
   text_to_buffer( dsock, buf->data );

   buffer_free( buf );
   return ret;
}

int new_workspace( WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !strcmp( wSpace->tag->created_on, "null" ) )
   {
      if( ( ret = new_tag( wSpace->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: called to new_tag failed giving back the returned code.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO workspaces VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', %d );",
              wSpace->tag->id, wSpace->tag->type, wSpace->tag->created_by,
              wSpace->tag->created_on, wSpace->tag->modified_by, wSpace->tag->modified_on,
              wSpace->name, wSpace->description, (int)wSpace->Public ) )
      return RET_FAILED_OTHER;

   return ret;
}

int new_workspace_entry( WORKSPACE *wSpace, ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }
   if( !quick_query( "INSERT INTO workspace_entries VALUES ( %d, '%s%d' );", wSpace->tag->id, tag->type == ENTITY_FRAMEWORK_IDS ? "f" : "", tag->id ) )
      return RET_FAILED_OTHER;

   return ret;
}

int load_workspace_entries( WORKSPACE *wSpace )
{
   ENTITY_FRAMEWORK *frame;
   MYSQL_RES *result;
   MYSQL_ROW row;
   int ret = RET_SUCCESS;
   int framework_id;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }

   if( !quick_query( "SELECT entry FROM workspace_entries WHERE workspaceID=%d;", wSpace->tag->id ) )
      return RET_FAILED_OTHER;
   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
     return RET_FAILED_OTHER;
   if( mysql_num_rows( result ) < 1 )
      return RET_DB_NO_ENTRY;
   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      if( row[0][0] == 'f' )
      {
         framework_id = atoi( row[0]+1 );
         if( ( frame = get_framework_by_id( framework_id ) ) == NULL )
         {
            bug( "%s: bad entry in workspace_entries %d,", __FUNCTION__, framework_id );
            continue;
         }
         AttachToList( frame, wSpace->frameworks );
      }
   /* else
      {
         instance stuff
      } */
   }
   return ret;
}


int text_to_olc( INCEPTION *olc, const char *fmt, ... )
{
   va_list va;
   int res;
   char dest[MAX_BUFFER];

   va_start( va, fmt );
   res = vsnprintf( dest, MAX_BUFFER, fmt, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when attempting to format string for message." );
   }

   text_to_buffer( olc->account->socket, dest );
   olc_no_prompt( olc );
   return res;
}

void olc_no_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = FALSE;
   return;
}

void olc_show_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = TRUE;
   return;
}

bool workspace_list_has_name( LLIST *wSpaces, const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !name || name[0] == '\0' )
      return FALSE;

   if( !wSpaces || SizeOfList( wSpaces ) < 1 )
      return FALSE;

   AttachIterator( &Iter, wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( wSpace->name, name ) )
         break;
   DetachIterator( &Iter );

   if( wSpace )
      return TRUE;
   return FALSE;
}


void olc_file( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "You used the file command.\r\n" );
   return;
}

void olc_workspace( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *workspace_command;

   if( ( workspace_command = olc->account->executing_command ) == NULL )
      return;

   if( workspace_command->sub_commands )
   {
      free_command_list( workspace_command->sub_commands );
      FreeList( workspace_command->sub_commands );
      workspace_command->sub_commands = NULL;
      text_to_olc( olc, "Workspace Commands Menu Closed.\r\n" );
      olc_show_prompt( olc );
   }
   else
   {
      workspace_command->sub_commands = AllocList();
      load_commands( workspace_command->sub_commands, workspace_sub_commands, olc->account->level );
      text_to_olc( olc, "Workspace Commands Menu Opened.\r\n" );
      olc_show_prompt( olc );
   }
}

void workspace_new( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   MYSQL_RES *result;
   char buf[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "You need to enter a name for your new workspace.\r\n" );
      return;
   }

   arg = one_arg( arg, buf );

   if( !quick_query( "SELECT * FROM workspaces WHERE name='%s';", buf ) )
      return;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return;
   }

   if( mysql_num_rows( result ) != 0 )
   {
      text_to_olc( olc, "A workspace with that name already exists.\r\n" );
      mysql_free_result( result );
      return;
   }

   wSpace = init_workspace();
   if( new_tag( wSpace->tag, olc->account->name ) != RET_SUCCESS )
   {
      text_to_olc( olc, "You could not get a new tag for your workspace, therefore, it was not created.\r\n" );
      free_workspace( wSpace );
      return;
   }
   wSpace->name = strdup( buf );
   if( new_workspace( wSpace ) != RET_SUCCESS )
   {
      text_to_olc( olc, "Your new workspace could not be saved to the database it will not be created.\r\n" );
      free_workspace( wSpace );
      return;
   }
   AttachToList( wSpace, olc->wSpaces );
   AttachToList( wSpace, active_wSpaces );
   AttachToList( olc->account, wSpace->who_using );
   text_to_olc( olc, "A new workspace %s has been created and loaded.\r\n", wSpace->name );
   mysql_free_result( result );
   return;
}

void workspace_load( void *passed, char *arg )
{
   ACCOUNT_DATA *account;
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   MYSQL_RES *result;
   MYSQL_ROW row;
   ITERATOR Iter, IterTwo;
   char buf[MAX_BUFFER];
   char who_using[MAX_BUFFER];
   bool found = FALSE;
   int x;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "What's the name of the workspace you want to load?\r\n" );
      return;
   }

   arg = one_arg( arg, buf );

   AttachIterator( &Iter, active_wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if(  ( x = strcasecmp( buf, wSpace->name ) ) == 0 || x == -110 )
      {
         found = TRUE;
         if( !wSpace->Public && strcmp( wSpace->tag->created_by, olc->account->name ) )
            text_to_olc( olc, "The workspace %s is private and you did not create it.\r\n", wSpace->name );
         else
         {
            text_to_olc( olc, "Workspace %s loaded into your OLC.", wSpace->name );
            if( SizeOfList( wSpace->who_using ) > 0 )
            {
               memset( &who_using[0], 0, sizeof( who_using ) );
               AttachIterator( &IterTwo, wSpace->who_using );
               while( ( account = (ACCOUNT_DATA *)NextInList( &IterTwo ) ) != NULL )
               {
                  if( who_using[0] != '\0' )
                     strcat( who_using, ", " );
                  strcat( who_using, account->name );
               }
               DetachIterator( &IterTwo );
               text_to_olc( olc, " These users(%s) are already using this workspace.", who_using );
            }
            text_to_olc( olc, "\r\n" );
            AttachToList( wSpace, olc->wSpaces );
            AttachToList( olc->account, wSpace->who_using );
         }
      }
   }
   DetachIterator( &Iter );

   if( quick_query( "SELECT * FROM workspaces WHERE name LIKE '%s%%';", buf ) )
      if( ( result = mysql_store_result( sql_handle ) ) != NULL )
         if( mysql_num_rows( result ) > 0 )
         {
            while( ( row = mysql_fetch_row( result ) ) != NULL )
            {
               wSpace = init_workspace();
               db_load_workspace( wSpace, &row );
               if( workspace_list_has_name( olc->wSpaces, wSpace->name ) )
               {
                  free_workspace( wSpace );
                  continue;
               }
               load_workspace_entries( wSpace );
               found = TRUE;
               AttachToList( wSpace, active_wSpaces );
               AttachToList( wSpace, olc->wSpaces );
               AttachToList( olc->account, wSpace->who_using );
               text_to_olc( olc, "Workspace %s loaded from database.\r\n", wSpace->name );
            }
         }

   if( !found )
      text_to_olc( olc, "No workspaces with that name exist.\r\n" );
   mysql_free_result( result );
   return;
}

void workspace_unload( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( olc->wSpaces ) < 1 )
   {
      text_to_olc( olc, "You have no workspaces to unload.\r\n" );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Unload which workspace?\r\n" );
      return;
   }

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( arg, wSpace->name ) )
         break;
   DetachIterator( &Iter );

   if( !wSpace )
   {
      text_to_olc( olc, "You have no such workspace loaded.\r\n" );
      return;
   }

   DetachFromList( wSpace, olc->wSpaces );
   unuse_workspace( wSpace, olc->account );
   text_to_olc( olc, "Workspace unloaded.\r\n" );
   return;
}

void workspace_grab( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   char buf[MAX_BUFFER];
   int search_id = -1;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Grab what?\r\n" );
      return;
   }

   if( !olc->using_workspace )
   {
      text_to_olc( olc, "You have to be using a workspace to grab.\r\n" );
      return;
   }

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg( arg, buf );

      if( buf[0] == 'f' )
      {
         if( !is_number( buf+1 ) )
         {
            text_to_olc( olc, "%s is an inproperly formatted ID.\r\n", buf );
            continue;
         }
         search_id = atoi( buf+1 );
         if( framework_list_has_by_id( olc->using_workspace->frameworks, search_id ) )
         {
            text_to_olc( olc, "You already have that framework with an ID of %d grabbed into this workspace.\r\n", search_id );
            continue;
         }
         if( SizeOfList( active_frameworks ) > 0 )
         {
            if( ( frame = get_active_framework_by_id( search_id ) ) != NULL )
            {
               AttachToList( frame, olc->using_workspace->frameworks );
               new_workspace_entry( olc->using_workspace, frame->tag );
               text_to_olc( olc, "Framework %d: %s loaded into %s workspace.\r\n", frame->tag->id, frame->name, olc->using_workspace->name );
               continue;
            }
         }
         if( !frame )
         {
            if( ( frame = load_eFramework_by_id( search_id ) ) == NULL )
            {
               text_to_olc( olc, "No framework with an ID of %d exists.\r\n", search_id );
               continue;
            }
            AttachToList( frame, olc->using_workspace->frameworks );
            AttachToList( frame, active_frameworks );
            new_workspace_entry( olc->using_workspace, frame->tag );
            text_to_olc( olc, "Framework %d: %s loaded into %s workspace.\r\n", frame->tag->id, frame->name, olc->using_workspace->name );
         }
      }
   }
   return;

}
void workspace_ungrab( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Ungrab what?\r\n" );
      return;
   }
   if( !olc->using_workspace )
   {
      text_to_olc( olc, "You can't ungrab anything from a workspace you aren't using.\r\n" );
      return;
   }
   text_to_olc( olc, "I(the ungrab command) don't work yet...\r\n" );
   return;
}

void olc_frameworks( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *frameworks_command;

   if( ( frameworks_command = olc->account->executing_command ) == NULL )
      return;

   if( frameworks_command->sub_commands )
   {
      free_command_list( frameworks_command->sub_commands );
      FreeList( frameworks_command->sub_commands );
      frameworks_command->sub_commands = NULL;
      text_to_olc( olc, "Frameworks Commands Menu Closed.\r\n" );
      olc_show_prompt( olc );
   }
   else
   {
      frameworks_command->sub_commands = AllocList();
      load_commands( frameworks_command->sub_commands, frameworks_sub_commands, olc->account->level );
      text_to_olc( olc, "Frameworks Commands Menu Opened.\r\n" );
      olc_show_prompt( olc );
   }
}

void framework_create( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   if( olc->editing )
   {
      text_to_olc( olc, "There's already something loaded in your editor, finish that first.\r\n" );
      change_socket_state( olc->account->socket, olc->editing_state );
      return;
   }

   CREATE( olc->editing, ENTITY_FRAMEWORK, 1 );
   olc->editing = init_eFramework();
   olc->editing_state = STATE_EFRAME_EDITOR;
   text_to_olc( olc, "Creating a new Entity Framework.\r\n" );
   olc_show_prompt( olc );
   olc->editor_commands = AllocList();
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}

void olc_instantiate( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   int framework_id;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Instantiate what?\r\n" );
      return;
   }

   if( arg[0] != 'f' )
   {
      text_to_olc( olc, "%s is an inproper format. Please use: 'f<id>' or 'f_<name>'\r\n", arg );
      return;
   }
   arg++;

   if( arg[0] != '_' )
   {
      if( !is_number( arg ) )
      {
         text_to_olc( olc, "Bad format. You must enter an ID unless you seperate with an '_'.\r\n" );
         return;
      }
      framework_id = atoi( arg );
   }
   else
   {
   }


   return;
}

void olc_using( void *passed, char *arg )
{
   INCEPTION * olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( olc->wSpaces ) < 1 )
   {
      text_to_olc( olc, "You have no workspaces loaded.\r\n" );
      return;
   }

   if( !strcasecmp( arg, "none" ) )
   {
      text_to_olc( olc, "You are no longer using any workspace.\r\n" );
      olc_show_prompt( olc );
      olc->using_workspace = NULL;
      return;
   }

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( arg, wSpace->name ) )
         break;
   DetachIterator( &Iter );

   if( !wSpace )
      text_to_olc( olc, "You have no such workspace loaded. Remember, be specific!\r\n" );
   else
   {
      olc->using_workspace = wSpace;
      text_to_olc( olc, "You are now using %s.\r\n", wSpace->name );
      olc_show_prompt( olc );
   }
   return;
}

void olc_show( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "Your current Inception looks like this.\r\n" );
   olc_show_prompt( olc );

}
void olc_quit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "You close the Inception OLC.\r\n" );
   olc_show_prompt( olc );
   change_socket_state( olc->account->socket, STATE_ACCOUNT );
   return;
}

