/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

struct typCmd account_commands[] = {
   { "settings", account_settings, LEVEL_BASIC, NULL, TRUE },
   { "quit", account_quit, LEVEL_BASIC, NULL, FALSE },
   { "", NULL, 0 } /* gandalf */
};

struct typCmd settings_sub_commands[] = {
   { "pagewidth", set_pagewidth, LEVEL_BASIC, NULL, FALSE },
   { "", NULL, 0 }
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

int load_account_commands( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;
   int x;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }
   if( !account->commands )
   {
      BAD_POINTER( "account->commands" );
      return ret;
   }
   if( SizeOfList( account->commands ) > 0 )
      free_command_list( account->commands );

   for( x = 0; account_commands[x].cmd_name[0] != '\0'; x++ )
   {
      if( account_commands[x].level > account->level )
         continue;
      else
      {
         COMMAND *com;
         CREATE( com, COMMAND, 1 );

         if( copy_command( &account_commands[x], com ) != RET_SUCCESS )
            continue;

         AttachToList( com, account->commands );
      }
   }
   return ret;
}

int copy_command( COMMAND *to_copy, COMMAND *command )
{
   int ret = RET_SUCCESS;

   if( !to_copy )
   {
      BAD_POINTER( "to_copy");
      return ret;
   }

   if( !command )
   {
      BAD_POINTER( "command" );
      return ret;
   }

   command->cmd_name = to_copy->cmd_name;
   command->cmd_funct = to_copy->cmd_funct;
   command->sub_commands = NULL;
   command->can_sub = to_copy->can_sub;

   return ret;
}

int free_command_list( LLIST *com_list )
{
   ITERATOR Iter;
   COMMAND *com;
   int ret = RET_SUCCESS;

   if( !com_list )
   {
      BAD_POINTER( "com_list" );
      return ret;
   }

   AttachToList( &Iter, com_list );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
      free_command( com );
   DetachIterator( &Iter );

   return ret;
}

int free_command( COMMAND *command )
{
   int ret = RET_SUCCESS;
   command->cmd_name = NULL;
   command->cmd_funct = NULL;
   if( command->sub_commands )
      free_command_list( command->sub_commands );

   FreeList( command->sub_commands );

   return ret;
}
