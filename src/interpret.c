/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

/****************************************************************************
* ACCOUNT COMMAND TABLE                                                     *
****************************************************************************/
struct typCmd account_commands[] = {
   { "quit", account_quit, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "inception", inception_open, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "chat", account_chat, LEVEL_BASIC, NULL, FALSE, NULL, account_commands },
   { "settings", account_settings, LEVEL_BASIC, NULL, TRUE, NULL, account_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL } /* gandalf */
};

/******************************************************************************
* DESC_FUNC METHODS                                                           *
******************************************************************************/

const char *chat_desc( void *extra )
{
   return " - global chat";
}

const char * settings_desc( void *extra )
{
   return " - settings, pagewidth, chat stuff, etc";
}
/*****************************************************************************
* SETTINGS SUB COMMANDS TABLE                                                *
*****************************************************************************/

struct typCmd settings_sub_commands[] = {
   { "pagewidth", set_pagewidth, LEVEL_BASIC, NULL, FALSE, pagewidth_desc, settings_sub_commands },
   { "chat_as", account_chatas, LEVEL_BASIC, NULL, FALSE, chatas_desc, settings_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/******************************************************************************
* DESC_FUNC METHODS                                                           *
******************************************************************************/

const char *pagewidth_desc( void *extra )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)extra;
   static char buf[MAX_BUFFER];
   memset( &buf, 0, sizeof( buf ) );

   if( !extra )
   {
      bug( "%s: passed a bad account pointer.", __FUNCTION__ );
      return "";
   }

   mud_printf( buf, "#R: #B%d#n", account->pagewidth );
   return buf;
}

const char *chatas_desc( void *extra )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)extra;
   static char buf[MAX_BUFFER];
   memset( &buf, 0, sizeof( buf ) );

   if( !extra )
   {
     bug( "%s: passed a bad account pointer.", __FUNCTION__ );
     return "";
   }

   mud_printf( buf, ": %s", account->chatting_as[0] != ' ' ? account->chatting_as : "none" );
   return buf;
}

/*******************************************************************************
* INCEPTION OLC COMMAND TABLE                                                  *
* NAME, CMD_FUNC, LVL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE             *
*******************************************************************************/
struct typCmd olc_commands[] = {
   { "quit", olc_quit, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "using", olc_using, LEVEL_BASIC, NULL, FALSE, NULL, olc_commands },
   { "workspace", olc_workspace, LEVEL_BASIC, NULL, TRUE, NULL, olc_commands },
   { "file", olc_file, LEVEL_BASIC, NULL, TRUE, NULL, olc_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};

/*******************************************************************************
* WORKSPACE SUB COMMANDS TABLE                                                 *
* NAME, CMD_FUNC, LEVEL, SUB_COMMANDS, CAN_SUB, DESC_FUN, FROM_TABLE           *
*******************************************************************************/
struct typCmd workspace_sub_commands[] = {
   { "wload", workspace_load, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { "wnew", workspace_new, LEVEL_BASIC, NULL, FALSE, NULL, workspace_sub_commands },
   { '\0', NULL, 0, NULL, FALSE, NULL }
};


int account_handle_cmd( ACCOUNT_DATA *account, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( account->commands, command ) ) == NULL )
      text_to_account( account, "No such command.\r\n" );
   else
      execute_command( account, com, account, arg );

   return ret;
}

int olc_handle_cmd( INCEPTION *olc, char *arg )
{
   COMMAND *com;
   char command[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !olc )
   {
      BAD_POINTER( "olc" );
      return ret;
   }

   arg = one_arg( arg, command );

   if( ( com = find_loaded_command( olc->commands, command ) ) == NULL )
      text_to_olc( olc, "No such command.\r\n" );
   else
      execute_command( olc->account, com, olc, arg );

   return ret;
}

void execute_command( ACCOUNT_DATA *account, COMMAND *com, void *passed, char *arg )
{
      account->executing_command = com;
      (*com->cmd_funct)( passed, arg );
      FREE( account->last_command );
      account->last_command = strdup( account->executing_command->cmd_name );
      account->executing_command = NULL;
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
   command->desc_func = to_copy->desc_func;
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
