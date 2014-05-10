/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

const struct typCmd account_commands[] = {
   { "settings", account_settings, LEVEL_BASIC, NULL, TRUE },
   { "quit", account_quit, LEVEL_BASIC, NULL, FALSE },
   { "", NULL, 0 } /* gandalf */
};

int account_handle_cmd( ACCOUNT_DATA *account, char *arg )
{
   ITERATOR Iter;
   COMMAND *com;
   char command[MAX_BUFFER];
   bool found_cmd = FALSE;
   int ret = RET_SUCCESS;

   arg = one_arg( arg, command );

   AttachIterator( &Iter, account->commands );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
      if( is_prefix( com->cmd_name, command ) )
      {
         (*com->cmd_funct)( account, arg );
         found_cmd = TRUE;
         break;
      }
   DetachIterator( &Iter );
   if( !found_cmd )
      text_to_account( account, "No such command.\r\n" );
   return ret;
}
