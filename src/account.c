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
   account->idtag = init_tag();
   account->characters = AllocList();
   account->command_tables = AllocList();
   account->commands = AllocList();
   return account;
}

int clear_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   account->socket = NULL;
   account->name = strdup( "new_account" );
   account->password = strdup( "new_password" );
   account->level = 1;
   account->pagewidth = DEFAULT_PAGEWIDTH;
   account->last_command = strdup( "none" );
   account->executing_command = NULL;

   return ret;
}

/* deletion */
int free_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   if( account->idtag )
      free_tag( account->idtag );
   account->socket = NULL;
   FreeList( account->characters );
   account->characters = NULL;
   free_command_list( account->commands );
   FreeList( account->commands );
   account->commands = NULL;
   FREE( account->name );
   FREE( account->password );
   FREE( account->command_tables );
   FREE( account->commands );
   FREE( account->last_command );

   return ret;
}

/* i/o */
int load_account( ACCOUNT_DATA *account, const char *name )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   int num_row;

   if( !quick_query( "SELECT * FROM accounts WHERE name='%s';", name ) )
      return RET_FAILED_OTHER;

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_NO_SQL;
   }

   num_row = mysql_num_rows( result );
   if( num_row > 1 )
   {
      bug( "%s: Query for account %s turned up more than one entry...", __FUNCTION__, name );
      mysql_free_result( result );
      return RET_FAILED_OTHER;
   }
   else if( num_row == 0 )
   {
      mysql_free_result( result );
      return RET_DB_NO_ENTRY;
   }

   row = mysql_fetch_row( result );
   account->idtag->id = atoi( row[0] );
   account->idtag->type = atoi( row[1] );
   account->idtag->created_by = strdup( row[2] );
   account->idtag->created_on = strdup( row[3] );
   account->idtag->modified_by = strdup( row[4] );
   account->idtag->modified_on = strdup( row[5] );
   account->name = strdup( row[6] );
   account->password = strdup( row[7] );
   account->level = atoi( row[8] );
   account->pagewidth = atoi( row[9] );


   mysql_free_result( result );
   return RET_SUCCESS;
}

int new_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   account->idtag->type = ACCOUNT_IDS; /* set type before trying to get new tag */
   if( ( ret = new_tag( account->idtag, "system" ) ) != RET_SUCCESS )
   {
      bug( "%s: could not set parameters for a new ID tag.", __FUNCTION__ );
      return ret;
   }

   if( !quick_query( "INSERT INTO accounts VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d );",
              account->idtag->id, account->idtag->type, account->idtag->created_by,
              account->idtag->created_on, account->idtag->modified_by, account->idtag->modified_on,
              account->name, account->password, account->level, account->pagewidth ) )
      return RET_FAILED_OTHER;

   return ret;
}

int save_account( ACCOUNT_DATA *account )
{
   int ret = RET_SUCCESS;

   if( !account )
   {
      BAD_POINTER( "account" );
      return ret;
   }
   return ret;
}
int account_prompt( D_SOCKET *dsock )
{
   BUFFER *buf = buffer_new(MAX_BUFFER);
   int width = dsock->account->pagewidth;

   bprintf( buf, "/%s\\\r\n", print_header( "Account Menu", "-", width - 2 ) );
   print_commands( dsock->account->commands, buf, 0, dsock->account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_header( "End Menu", "-", width -2 ) );
   bprintf( buf, "What is your choice?: " );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return RET_SUCCESS; 
}

ACCOUNT_DATA *check_account_reconnect(const char *act_name)
{
  ACCOUNT_DATA *account;
  ITERATOR Iter;

  AttachIterator(&Iter, account_list);
  while ((account = (ACCOUNT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(account->name, act_name))
    {
      if (account->socket)
        close_socket(account->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return account;
}

int text_to_account( ACCOUNT_DATA *account, const char *fmt, ... )
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

   text_to_buffer( account->socket, dest );
   return res;
}


void account_quit( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   if( account->socket->state != STATE_ACCOUNT )
      return;

   DetachFromList( account, account_list );

   text_to_socket( account->socket, "Quitting...\r\n" );
   close_socket( account->socket, FALSE );
   return;
}

void account_settings( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;
   COMMAND *settings_command;

   if( ( settings_command = account->executing_command ) == NULL )
      return;

   if( settings_command->sub_commands )
   {
      free_command_list( settings_command->sub_commands );
      FreeList( settings_command->sub_commands );
      settings_command->sub_commands = NULL;
   }
   else
   {
      settings_command->sub_commands = AllocList();
      load_commands( settings_command->sub_commands, settings_sub_commands, account->level );
   }
   text_to_account( account, "Settings Opened.\r\n" );
   return;
}

void set_pagewidth( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;
   int value;

   if( !account )
   {
      bug( "%s: Account passed is a bad pointer.", __FUNCTION__ );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      text_to_account( account, "Current Pagewidth: %d\r\n", account->pagewidth );
      return;
   }

   if( !is_number( arg ) )
   {
      text_to_account( account, "Pagewidth can only take a number.\r\n" );
      return;
   }

   if( ( value = atoi( arg ) ) < 40 )
   {
      text_to_account( account, "Pagewidths have an absolute minimum of 40, for sanity reasons.\r\n" );
      return;
   }

   account->pagewidth = value;

   quick_query( "UPDATE `accounts` SET pagewidth='%d' WHERE accountID='%d';", value, account->idtag->id );
   update_tag( account->idtag, "%s-pagewidthCommand", account->name );

   text_to_account( account, "Pagewidth set to %d.\r\n", value );
   return;
}

void account_chat( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_account( account, "Chat what?\r\n" );
      return;
   }

   communicate( CHAT_LEVEL, account, arg );
   account->socket->bust_prompt = FALSE;
   text_to_account( account, "What is your choice? " );
   return;
}
