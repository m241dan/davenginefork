/* account.c: methods pertaining to accounts written by Davenge */

#include "mud.h"

/* creation */

ACCOUNT_DATA *init_account( void )
{
   ACCOUNT_DATA *account;

   CREATE( account, ACCOUNT_DATA, 1 );
   if( clear_account( account ) != RET_SUCCESS )
   {
      free_account( account );
      return NULL;
   }
   account->command_tables = AllocList();
   account->commands = AllocList();
   return account;
}

int clear_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   account->name = "new_account";
   account->password = "new_password";
   account->level = 1;
   account->pagewidth = DEFAULT_PAGEWIDTH;

   return ret;
}

/* deletion */
int free_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   account->socket = NULL;
   FreeList( account->characters );
   account->characters = NULL;
   FreeList( account->command_tables );
   account->command_tables = NULL;
   FreeList( account->commands );
   account->commands = NULL;
   FREE( account->name );
   FREE( account->command_tables );
   FREE( account->commands );

   return ret;
}

int account_prompt( D_SOCKET *dsock )
{
   BUFFER *buf = buffer_new(MAX_BUFFER);
   int width = dsock->account->pagewidth;

   bprintf( buf, "/%s\\\r\n", print_header( "Account Menu", "-", width - 2 ) );
   bprintf( buf, "| %-*.*s |\r\n", width - 4, width - 4, "Nothing Here Yet" );
   bprintf( buf, "\\%s/\r\n", "End Menu", "-", width -2 );
   bprintf( buf, "What is your choice?: " );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return RET_SUCCESS; 
}
