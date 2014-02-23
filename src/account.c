/* account.c: methods pertaining to accounts written by Davenge */

#include "mud.h"

/* creation */

ACCOUNT_DATA *init_account( void )
{
   ACCOUNT_DATA *account;

   CREATE( account, ACCOUNT_DATA, 1 );
   clear_account( account );
   account->characters = AllocList();
   account->command_tables = AllocList();
   account->commands = AllocList();
   return account;
}

void clear_account( ACCOUNT_DATA *account )
{
   account->name = "new_account";
   account->password = "new_password";
   account->level = 1;
   account->pagewidth = DEFAULT_PAGEWIDTH;
   return;
}

/* deletion */
void free_account( ACCOUNT_DATA *account )
{
   ITERATOR Iter;
   account->socket = NULL;
   DETACHCONTENTS( account->characters, ENTITY_DATA );
}
