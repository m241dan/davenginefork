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
   olc->displaying_workspace = NULL;
   return ret;
}

int free_olc( INCEPTION *olc )
{
   int ret = RET_SUCCESS;

   olc->account = NULL;
   FreeList( olc->wSpaces );
   olc->wSpaces = NULL;
   FreeList( olc->commands );
   olc->commands = NULL;
   olc->displaying_workspace = NULL;
   FREE( olc );

   return ret;
}

WORKSPACE *init_workspace( void )
{
   WORKSPACE *wSpace;

   CREATE( wSpace, WORKSPACE, 1 );
   wSpace->tag = init_tag();
   wSpace->frameworks = AllocList();
   wSpace->instances = AllocList();
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

   FreeList( wSpace->frameworks );
   wSpace->frameworks = NULL;
   FreeList( wSpace->instances );
   wSpace->instances = NULL;

   FREE( wSpace->name );
   FREE( wSpace->description );
   FREE( wSpace );

   return ret;
}

void inception_open( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   text_to_account( account, "Opening Inception OLC...\r\n\r\n" );
   account->olc = init_olc();
   account->olc->account = account;
   change_socket_state( account->socket, STATE_OLC );
   return;
}

int olc_prompt( D_SOCKET *dsock )
{
   BUFFER *buf = buffer_new( MAX_BUFFER );
   ACCOUNT_DATA *account = dsock->account;
   INCEPTION *olc;
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
   bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );
   if( olc->displaying_workspace )
   { 
      center = ( account->pagewidth - 7 ) / 2;
      bprintf( buf, "| %s  |", center_string( "Frameworks", center ) );
      bprintf( buf, "| %s  |", print_header( "Frameworks", " ", center ) );
      bprintf( buf, " %s |\r\n", center_string( "Instances", center ) );
      if( SizeOfList( olc->displaying_workspace->frameworks ) < 1 )
         bprintf( buf, "| %s |", center_string( "(empty)", center ) );
      /* else print the first entry in frameworks list */
      if( SizeOfList( olc->displaying_workspace->instances )  < 1 )
         bprintf( buf, "| %s |\r\n", center_string( "(empty)", center ) );
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

   if( wSpace->tag->type == -1 )
   {
      wSpace->tag->type = WORKSPACE_IDS;
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
   char buf[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "You need to enter a name for your new workspace.\r\n" );
      return;
   }

   arg = one_arg( arg, buf );
   wSpace = init_workspace();
   wSpace->tag->type = WORKSPACE_IDS;
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
   text_to_olc( olc, "New Workspace Created.\r\n" );
   return;


}

void olc_quit( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;

   text_to_olc( olc, "You close the Inception OLC.\r\n" );
   change_socket_state( olc->account->socket, STATE_ACCOUNT );
   olc->account->olc = NULL;
   free_olc( olc );

   return;
}
