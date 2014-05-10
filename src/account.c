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

/* i/o */
int load_account( ACCOUNT_DATA *account, const char *name )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   int num_row;
   char query[MAX_BUFFER];

   if( !check_sql() )
      return RET_NO_SQL;

   mud_printf( query, "SELECT * FROM accounts WHERE name='%s';", name );

   if( mysql_query( sql_handle, query ) )
   {
      report_sql_error( sql_handle );
      return RET_NO_SQL;
   }

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
   account->accountID = atoi( row[0] );
   account->name = strdup( row[1] );
   account->password = strdup( row[2] );
   account->level = atoi( row[3] );
   account->pagewidth = atoi( row[4] );


   mysql_free_result( result );
   return RET_SUCCESS;
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
