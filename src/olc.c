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

int load_workspaces( void )
{
   WORKSPACE *wSpace;
   MYSQL_RES *result;
   MYSQL_ROW row;

   int ret = RET_SUCCESS;

   if( !quick_query( "SELECT * FROM workspaces;" ) )
      return RET_FAILED_OTHER;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
      return RET_DB_NO_ENTRY;

   if( !active_wSpaces )
   {
      BAD_POINTER( "active_wSpaces" );
      return ret;
   }

   while( ( row = mysql_fetch_row( result ) ) )
   {
      wSpace = init_workspace();
      db_load_workspace( wSpace, &row );
      AttachToList( wSpace, active_wSpaces );
   }
   mysql_free_result( result );
   return ret;
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
   BUFFER *buf = buffer_new( MAX_BUFFER );
   ACCOUNT_DATA *account = dsock->account;
   INCEPTION *olc;
   WORKSPACE *wSpace;
   ITERATOR Iter;
   char tempstring[MAX_BUFFER];
   int ret = RET_SUCCESS;
   int center, space_after_pipes;
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
      mud_printf( tempstring, "%s Workspace", olc->using_workspace->name );
      bprintf( buf, "|%s|\r\n", print_header( tempstring, "-", space_after_pipes ) );
      center = ( account->pagewidth - 7 ) / 2;
      bprintf( buf, "| %s  |", print_header( "Frameworks", " ", center ) );
      bprintf( buf, " %s |\r\n", center_string( "Instances", center ) );
      if( SizeOfList( olc->using_workspace->frameworks ) < 1 )
         bprintf( buf, "| %s  |", center_string( "(empty)", center ) );
      /* else print the first entry in frameworks list */
      if( SizeOfList( olc->using_workspace->instances )  < 1 )
         bprintf( buf, " %s |\r\n", center_string( "(empty)", center ) );
      /* else ibid for instances */

      /* print the reminder of the contents */
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
   return res;
}

void olc_no_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = FALSE;
   return;
}

bool workspace_list_has_name( LLIST *wSpaces, const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

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
   }
   else
   {
      workspace_command->sub_commands = AllocList();
      load_commands( workspace_command->sub_commands, workspace_sub_commands, olc->account->level );
      text_to_olc( olc, "Workspace Commands Menu Opened.\r\n" );
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
   text_to_olc( olc, "New Workspace Created.\r\n" );
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
               found = TRUE;
               AttachToList( wSpace, active_wSpaces );
               AttachToList( wSpace, olc->wSpaces );
               AttachToList( olc->account, wSpace->who_using );
               text_to_olc( olc, "Workspace %s loaded from database.\r\n", wSpace->name );
            }
         }

   if( !found )
   {
      text_to_olc( olc, "No workspaces with that name exist.\r\n" );
   }
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

   unuse_workspace( wSpace, olc->account );
   text_to_olc( olc, "Workspace unloaded.\r\n" );
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
   }
   else
   {
      frameworks_command->sub_commands = AllocList();
      load_commands( frameworks_command->sub_commands, frameworks_sub_commands, olc->account->level );
      text_to_olc( olc, "Frameworks Commands Menu Opened.\r\n" );
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
   olc->editor_commands = AllocList();
   change_socket_state( olc->account->socket, olc->editing_state );
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
      olc->using_workspace = NULL;
      return;
   }

   AttachIterator( &Iter, olc->wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( arg, wSpace->name ) )
         break;
   DetachIterator( &Iter );

   if( !wSpace )
   {
      text_to_olc( olc, "You have no such workspace loaded. Remember, be specific!\r\n" );
      olc_no_prompt( olc );
   }
   else
   {
      olc->using_workspace = wSpace;
      text_to_olc( olc, "Using %s.\r\n", wSpace->name );
   }
   return;
}

void olc_quit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "You close the Inception OLC.\r\n" );
   change_socket_state( olc->account->socket, STATE_ACCOUNT );
   return;
}

