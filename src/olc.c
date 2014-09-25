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
   olc->project = NULL;
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
   CLEARLIST( wSpace->instances, ENTITY_INSTANCE );
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

WORKSPACE *load_workspace_by_query( const char *query )
{
   WORKSPACE *wSpace = NULL;
   MYSQL_ROW row;


   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( wSpace = init_workspace() ) == NULL )
      return NULL;

   db_load_workspace( wSpace, &row );
   load_workspace_entries( wSpace );
   free( row );
   return wSpace;
}

WORKSPACE *get_workspace_by_id( int id )
{
   WORKSPACE *wSpace;

   if( ( wSpace = get_active_workspace_by_id( id ) ) == NULL )
      if( ( wSpace = load_workspace_by_id( id ) ) != NULL )
         AttachToList( wSpace, active_wSpaces );

   return wSpace;
}

WORKSPACE *get_active_workspace_by_id( int id )
{
   return workspace_list_has_by_id( active_wSpaces, id );
}

WORKSPACE *load_workspace_by_id( int id )
{
   return load_workspace_by_query( quick_format( "SELECT * FROM `%s` WHERE %s=%d;", tag_table_strings[WORKSPACE_IDS], tag_table_whereID[WORKSPACE_IDS], id ) );
}

WORKSPACE *get_workspace_by_name( const char *name )
{
   WORKSPACE *wSpace;

   if( ( wSpace = get_active_workspace_by_name( name ) ) == NULL )
      if( ( wSpace = load_workspace_by_name( name ) ) != NULL )
         AttachToList( wSpace, active_wSpaces );

   return wSpace;
}

WORKSPACE *get_active_workspace_by_name( const char *name )
{
   return workspace_list_has_by_name( active_wSpaces, name );
}

WORKSPACE *load_workspace_by_name( const char *name )
{
   return load_workspace_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[WORKSPACE_IDS], name ) );
}

void db_load_workspace( WORKSPACE *wSpace, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( wSpace->tag, row );

   wSpace->name = strdup( (*row)[counter++] );
   wSpace->description = strdup( (*row)[counter++] );
   wSpace->Public = (bool)(atoi( (*row)[counter++] ));
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

WORKSPACE *workspace_list_has_by_name( LLIST *workspace_list, const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !name || name[0] == '\0' )
      return NULL;
   if( !workspace_list || SizeOfList( workspace_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( wSpace->name, name ) )
          break;
   DetachIterator( &Iter );

   return wSpace;
}

WORKSPACE *workspace_list_has_by_id( LLIST *workspace_list, int id )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( id < 0 )
      return NULL;
   if( !workspace_list || SizeOfList( workspace_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( wSpace->tag->id == id )
         break;
   DetachIterator( &Iter );

   return wSpace;
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
   ENTITY_INSTANCE *instance;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   ACCOUNT_DATA *account = dsock->account;
   INCEPTION *olc;
   WORKSPACE *wSpace;
   ITERATOR Iter, IterF, IterI;
   char tempstring[MAX_BUFFER];
   int ret = RET_SUCCESS;
   int space_after_pipes, max_list, max_frameworks, x, style;
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

   if( dsock->bust_prompt == SHORT_PROMPT )
   {
      if( olc->using_workspace )
         text_to_olc( olc, "\r\nUsing: %s> ", olc->using_workspace->name );
      else
         text_to_olc( olc, "\r\nInception OLC> " );
      return ret;
   }

   space_after_pipes = account->pagewidth - 2;

   if( !olc->project )
      bprintf( buf, "/%s\\\r\n", print_header( "Inception OLC", "-", space_after_pipes ) );
   else
      bprintf( buf, "/%s\\\r\n", print_header( quick_format( "Inception OLC - Project: %s", olc->project->name ), "-", space_after_pipes ) );

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
      AttachIterator( &IterI, olc->using_workspace->instances );
      for( x = 0; x < max_list; x++ )
      {
         frame = (ENTITY_FRAMEWORK *)NextInList( &IterF );
         instance = (ENTITY_INSTANCE *)NextInList( &IterI );
         switch( style )
         {
            case 1:
               bprintf( buf, "|%s|", fit_string_to_space( quick_format( " %-7d: %s", frame->tag->id, frame->name ), ( space_after_pipes - 1 ) / 2 ) );
               if( !instance )
                  bprintf( buf, " %s|\r\n", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );
               else
                  bprintf( buf, " %s|\r\n", fit_string_to_space( quick_format( " %-7d: %s", instance->tag->id, instance_name( instance ) ), ( space_after_pipes - 1 ) / 2 ) );
               break;
            case 2:
               bprintf( buf, "|%s|", fit_string_to_space( quick_format( " %-7d: %s", instance->tag->id, instance_name( instance ) ) , ( space_after_pipes - 1 ) / 2 ) );
               if( !frame )
                  bprintf( buf, " %s|\r\n", print_header( " ", " ", ( space_after_pipes - 1 ) / 2 ) );
               else
                  bprintf( buf, " %s|\r\n", fit_string_to_space( quick_format( " %-7d: %s", frame->tag->id, frame->name ), ( space_after_pipes - 1 ) / 2 ) );
               break;
         }
      }
      DetachIterator( &IterF );
      DetachIterator( &IterI );
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

int add_frame_to_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !frame )
   {
      BAD_POINTER( "frame" );
      return ret;
   }
   if( framework_list_has_by_id( wSpace->frameworks, frame->tag->id ) )
      return RET_LIST_HAS;

   AttachToList( frame, wSpace->frameworks );
   new_workspace_entry( wSpace, frame->tag );
   return ret;
}

int add_instance_to_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace )
{
   int ret = RET_SUCCESS;

   if( !wSpace )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !instance )
   {
      BAD_POINTER( "instance" );
      return ret;
   }
   if( instance_list_has_by_id( wSpace->instances, instance->tag->id ) )
      return RET_LIST_HAS;

   AttachToList( instance, wSpace->instances );
   new_workspace_entry( wSpace, instance->tag );
   return ret;
}

int add_workspace_to_olc( WORKSPACE *wSpace, INCEPTION *olc )
{
   ACCOUNT_DATA *account;
   ITERATOR Iter;
   char who_using[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( SizeOfList( wSpace->who_using ) > 0 )
   {
      memset( &who_using[0], 0, sizeof( who_using ) );
      AttachIterator( &Iter, wSpace->who_using );
      while( ( account = (ACCOUNT_DATA *)NextInList( &Iter ) ) != NULL )
      {
         if( who_using[0] != '\0' )
            strcat( who_using, ", " );
         strcat( who_using, account->name );
      }
      DetachIterator( &Iter );
      text_to_olc( olc, " These users(%s) are already using this workspace.", who_using );
   }
   text_to_olc( olc, "\r\n" );
   AttachToList( wSpace,  olc->wSpaces );
   AttachToList( olc->account, wSpace->who_using );

   if( olc->project && !workspace_list_has_by_id( olc->project->workspaces, wSpace->tag->id ) )
      add_workspace_to_project( wSpace, olc->project );

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
   if( !quick_query( "INSERT INTO workspace_entries VALUES ( %d, '%c%d' );", wSpace->tag->id, tag_table_characters[tag->type], tag->id ) )
      return RET_FAILED_OTHER;

   return ret;
}

int load_workspace_entries( WORKSPACE *wSpace )
{
   ENTITY_INSTANCE *instance;
   ENTITY_FRAMEWORK *frame;
   MYSQL_RES *result;
   MYSQL_ROW row;
   int ret = RET_SUCCESS;
   int id;

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
   {
      mysql_free_result( result );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      switch( row[0][0] )
      {
         default: continue;
         case 'f':
            id = atoi( row[0]+1 );
            if( ( frame = get_framework_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in workspace_entries %d,", __FUNCTION__, id );
               continue;
            }
            AttachToList( frame, wSpace->frameworks );
            break;
         case 'i':
            id = atoi( row[0]+1 );
            if( ( instance = get_instance_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in workspace_entries %d,", __FUNCTION__, id );
               continue;
            }
            AttachToList( instance, wSpace->instances );
            break;
      }
   }
   mysql_free_result( result );
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
   olc->account->socket->bust_prompt = NO_PROMPT;
   return;
}

void olc_short_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = SHORT_PROMPT;
}

void olc_show_prompt( INCEPTION *olc )
{
   if( !olc->account || !olc->account->socket )
      return;
   olc->account->socket->bust_prompt = NORMAL_PROMPT;
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


WORKSPACE *copy_workspace( WORKSPACE *wSpace )
{
   WORKSPACE *wSpace_copy;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   if( !wSpace )
   {
      bug( "%s: passed a NULL wSpace.", __FUNCTION__ );
      return NULL;
   }

   CREATE( wSpace_copy, WORKSPACE, 1 );
   wSpace_copy->tag = copy_tag( wSpace->tag );

   wSpace_copy->name = strdup( wSpace->name );
   wSpace_copy->description = strdup( wSpace->description );

   wSpace_copy->Public = wSpace->Public;
   wSpace_copy->hide_frameworks = wSpace->hide_frameworks;
   wSpace_copy->hide_instances = wSpace->hide_instances;

   wSpace_copy->frameworks = AllocList();
   AttachIterator( &Iter, wSpace->frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      AttachToList( frame, wSpace_copy->frameworks );
   DetachIterator( &Iter );

   wSpace_copy->instances = AllocList();
   AttachIterator( &Iter, wSpace->instances );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      AttachToList( instance, wSpace_copy->instances );
   DetachIterator( &Iter );

   return wSpace_copy;
}

LLIST *copy_workspace_list( LLIST *wSpaces, bool copy_content )
{
   LLIST *list;
   WORKSPACE *wSpace_copy;
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !wSpaces || SizeOfList( wSpaces ) < 1 )
   {
       bug( "%s: was passed an empty or NULL wSpaces.", __FUNCTION__ );
       return NULL;
   }

   list = AllocList();
   AttachIterator( &Iter, wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if( copy_content )
      {
         wSpace_copy = copy_workspace( wSpace );
         AttachToList( wSpace_copy, list );
         continue;
      }
      AttachToList( wSpace, list );
   }
   DetachIterator( &Iter );

   return list;
}

void copy_workspaces_into_list( LLIST *wSpaces, LLIST *copy_into_list, bool copy_content )
{
   WORKSPACE *wSpace;
   WORKSPACE *wSpace_copy;
   ITERATOR Iter;

   if( !wSpaces )
   {
      bug( "%s: was passed a NULL wSpaces.", __FUNCTION__ );
      return;
   }
   if( !copy_into_list )
   {
      bug( "%s: was passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if( copy_content )
      {
         wSpace_copy = copy_workspace( wSpace );
         AttachToList( wSpace_copy, copy_into_list );
         continue;
      }
      AttachToList( wSpace, copy_into_list );
   }
   DetachIterator( &Iter );

   return;
}

void olc_file( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   COMMAND *file_command;

   if( ( file_command = olc->account->executing_command ) == NULL )
      return;

   if( file_command->sub_commands )
   {
      free_command_list( file_command->sub_commands );
      FreeList( file_command->sub_commands );
      file_command->sub_commands = NULL;
      text_to_olc( olc, "File Menu Closed.\r\n" );
   }
   else
   {
      file_command->sub_commands = AllocList();
      load_commands( file_command->sub_commands, file_sub_commands, olc->account->level );
      text_to_olc( olc, "File Menu Opened.\r\n" );
   }
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
   return;
}

void workspace_new( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "You need to enter a name for your new workspace.\r\n" );
      return;
   }

   if( ( wSpace = get_workspace_by_name( arg ) ) )
   {
      text_to_olc( olc, "A workspace with that name already exists.\r\n" );
      return;
   }

   wSpace = init_workspace();
   if( new_tag( wSpace->tag, olc->account->name ) != RET_SUCCESS )
   {
      text_to_olc( olc, "You could not get a new tag for your workspace, therefore, it was not created.\r\n" );
      free_workspace( wSpace );
      olc_short_prompt( olc );
      return;
   }
   FREE( wSpace->name );
   wSpace->name = strdup( arg );
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
   return;
}

void workspace_load( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   WORKSPACE *wSpace;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   char buf[MAX_BUFFER];
   bool found = FALSE;
   int x;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "What's the name of the workspace you want to load?\r\n" );
      return;
   }

   arg = one_arg( arg, buf );

   /* workspace messaging, needs factoring */

   AttachIterator( &Iter, active_wSpaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      if(  ( x = strcasecmp( buf, wSpace->name ) ) == 0 || x == -110 )
      {
         if( workspace_list_has_name( olc->wSpaces, wSpace->name ) )
         {
            text_to_olc( olc, "You already have workspace %s loaded.\r\n", wSpace->name );
            found = TRUE;
            continue;
         }
         found = TRUE;
         if( !wSpace->Public && strcmp( wSpace->tag->created_by, olc->account->name ) )
            text_to_olc( olc, "The workspace %s is private and you did not create it.\r\n", wSpace->name );
         else
         {
            text_to_olc( olc, "Workspace %s loaded into your OLC.", wSpace->name );
            add_workspace_to_olc( wSpace, olc );
         }
      }
   }
   DetachIterator( &Iter );

   /* needs factoring */

   list = AllocList();
   if( db_query_list_row( list, quick_format( "SELECT * FROM workspaces WHERE name LIKE '%s%%';", buf ) ) )
   {
      AttachIterator( &Iter, list );
      while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
      {
         wSpace = init_workspace();
         db_load_workspace( wSpace, &row );
         if( workspace_list_has_name( active_wSpaces, wSpace->name ) )
         {
            free_workspace( wSpace );
            continue;
         }
         load_workspace_entries( wSpace );
         found = TRUE;
         AttachToList( wSpace, active_wSpaces );
         text_to_olc( olc, "Workspace %s loaded from database.", wSpace->name );
         add_workspace_to_olc( wSpace, olc );
      }
      DetachIterator( &Iter );
   }
   FreeList( list );

   if( !found )
      text_to_olc( olc, "No workspaces with that name exist.\r\n" );
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
      olc_short_prompt( olc );
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
   ENTITY_INSTANCE *instance;
   char buf[MAX_BUFFER];
   int ret;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Grab what?\r\n" );
      return;
   }

   if( !olc->using_workspace )
   {
      text_to_olc( olc, "You have to be using a workspace to grab.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg( arg, buf );

      if( !interpret_entity_selection( buf ) )
         continue;

      switch( input_selection_typing )
      {
         default: continue;
         case SEL_FRAME:
            frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            if( ( ret = add_frame_to_workspace( frame, olc->using_workspace ) ) == RET_SUCCESS )
               text_to_olc( olc, "Framework %d: %s loaded into %s workspace.\r\n", frame->tag->id, frame->name, olc->using_workspace->name );
            else if( ret == RET_LIST_HAS )
            {
               text_to_olc( olc, "This workspace already has the framework.\r\n" );
               olc_short_prompt( olc );
            }
            break;
         case SEL_INSTANCE:
            instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
            if( ( ret = add_instance_to_workspace( instance, olc->using_workspace ) ) == RET_SUCCESS )
               text_to_olc( olc, "Instance %d: %s loaded into %s workspace.\r\n", instance->tag->id, instance_name( instance ), olc->using_workspace->name );
            else if( ret == RET_LIST_HAS )
            {
               text_to_olc( olc, "this workspace already has that instance.\r\n" );
               olc_short_prompt( olc );
            }
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection() );
            olc_short_prompt( olc );
            break;
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
   init_editor( olc, NULL );
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}

void framework_edit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *to_edit;

   if( olc->editing && ( arg && arg[0] != '\0' ) )
   {
      text_to_olc( olc, "You alraedy have something loaded in your editor, type editor with no arguments to load it and complete it.\r\n" );
      return;
   }

   if( ( to_edit = olc_edit_selection( olc, arg ) ) == NULL ) /* handles its own messaging */
      return;

   init_editor( olc, to_edit );
   text_to_olc( olc, "Editing Frame...\r\n" );
   change_socket_state( olc->account->socket, olc->editing_state );
   return;
}

void framework_iedit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *to_edit;
   ENTITY_FRAMEWORK *inherited_to_edit;

   if( olc->editing && ( arg && arg[0] != '\0' ) )
   {
      text_to_olc( olc, "You alrady have something loaded in your editor, type editor with no arguments to load it and complete it.\r\n" );
      return;
   }

   if( ( to_edit = olc_edit_selection( olc, arg ) ) == NULL ) /* handles its own messaging */
      return;

   if( ( inherited_to_edit = create_inherited_framework( to_edit ) ) == NULL ) /* does its own setting and databasing */
   {
      text_to_olc( olc, "Something has gone wrong tryin gto create an inherited frame.\r\n" );
      return;
   }

   init_editor( olc, inherited_to_edit );
   change_socket_state( olc->account->socket, olc->editing_state );
   text_to_olc( olc, "You begin to edit %s.\r\n", chase_name( inherited_to_edit ) );
   return;
}

void olc_instantiate( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   if( check_selection_type( arg ) != SEL_FRAME )
   {
      text_to_olc( olc, "Use proper selection typing for a Framework.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         text_to_olc( olc, "There's been a major problem. Contact your nearest admin.\r\n" );
         olc_short_prompt( olc );
         return;
      case SEL_FRAME:
         frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
      case SEL_STRING:
         text_to_olc( olc, (char *)retrieve_entity_selection() );
   }

   if( ( instance = eInstantiate( frame ) ) == NULL )
   {
      text_to_olc( olc, "There's been a major problem, framework you are trying to instantiate from may not be live.\r\n" );
      return;
   }

   if( new_eInstance( instance ) != RET_SUCCESS )
   {
      free_eInstance( instance );
      text_to_olc( olc, "Could not add new instance to database, deleting it from live memory.\r\n" );
      return;
   }

   AttachToList( instance, eInstances_list );
   if( olc->using_workspace )
      add_instance_to_workspace( instance, olc->using_workspace );

   text_to_olc( olc, "You create a new instance using the %s framework, its ID is %d.\r\n", frame->name, instance->tag->id );
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
      olc_short_prompt( olc );
   }
   else
   {
      olc->using_workspace = wSpace;
      text_to_olc( olc, "You are now using %s.\r\n", wSpace->name );
   }
   return;
}

void olc_builder( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_INSTANCE *builder;

   if( ( builder = init_builder() ) == NULL )
   {
      text_to_olc( olc, "Could not allocate memory for a Builder... that's really bad.\r\n" );
      olc_short_prompt( olc );
      return;
   }
   builder->account = olc->account;
   text_to_olc( olc, "You enter builder mode.\r\n" );
   socket_control_entity( olc->account->socket, builder );
   change_socket_state( olc->account->socket, STATE_BUILDER );
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
   change_socket_state( olc->account->socket, STATE_ACCOUNT );
   return;
}

void olc_load( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Load what?\r\n" );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
      olc_short_prompt( olc );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_olc( olc, "Invalid selection type, frames and instances only.\r\n" );
         return;
      case SEL_FRAME:
         frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         instance = full_load_eFramework( frame );
         break;
      case SEL_INSTANCE:
         instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
         full_load_instance( instance );
         break;
      case SEL_STRING:
         text_to_olc( olc, (char *)retrieve_entity_selection() );
         return;
   }

   if( olc->using_workspace )
      add_instance_to_workspace( instance, olc->using_workspace );

   text_to_olc( olc, "You completely load %s.\r\n", instance_name( instance ) );
   return;
}

void olc_chat( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Chat what?\r\n" );
      return;
   }

   communicate( CHAT_LEVEL, olc->account, arg );
   olc->account->socket->bust_prompt = NO_PROMPT;
   text_to_olc( olc, ":> " );
   return;
}
