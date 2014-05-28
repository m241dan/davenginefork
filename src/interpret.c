/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

struct typCmd account_commands[] = {
   { "quit", account_quit, LEVEL_BASIC, NULL, FALSE, account_commands },
   { "settings", account_settings, LEVEL_BASIC, NULL, TRUE, account_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL } /* gandalf */
};

struct typCmd settings_sub_commands[] = {
   { "pagewidth", set_pagewidth, LEVEL_BASIC, NULL, FALSE, settings_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

int account_handle_cmd( ACCOUNT_DATA *account, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_PONTER( "account" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( account->commands, command ) ) == NULL )
      text_to_account( account, "No such command.\r\n" );
   else
   {
      account->executing_command = com;
      (*com->cmd_funct)( account, arg );
      FREE( account->last_command );
      account->last_command = strdup( account->executing_command->cmd_name );
      account->executing_command = NULL;
   }
   return ret;
}

COMMAND *find_loaded_command( LLIST *loaded_list, const char *command )
{
   ITERATOR Iter;
   COMMAND *com;

   AttachIterator( &Iter, loaded_list );
   while( ( com = (COMMAND *)NextInList( &Iter ) ) != NULL )
   {
      if( is_prefix( com->cmd_name, command ) )
         break;
      if( com->sub_commands && ( com = find_loaded_command( com->sub_commands, command ) ) != NULL )
         break;
   }

   DetachIterator( &Iter );
   return com;
}

int load_commands( LLIST *command_list, COMMAND command_table[], int level_compare )
{
   int ret = RET_SUCCESS;
   int x;

   if( !command_list )
   {
      BAD_POINTER( "command_list" );
      return ret;
   }

   for( x = 0; command_table[x].cmd_name != '\0'; x++ )
   {
      if( command_table[x].level > level_compare )
         continue;
      else
      {
         COMMAND *com;
         CREATE( com, COMMAND, 1 );

         if( copy_command( &command_table[x], com ) != RET_SUCCESS )
            continue;

         AttachToList( com, command_list );
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
   command->from_table = to_copy->from_table;
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

   AttachIterator( &Iter, com_list );
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
   {
      free_command_list( command->sub_commands );
      FreeList( command->sub_commands );
   }
   command->from_table = NULL;
   FREE( command );
   return ret;
}
