/* olc.c: methods pertainingto the OLC written by Davenge */

#include "mud.h"

INCEPTION *init_olc( void )
{
   INCEPTION *olc;

   CREATE( olc, INCEPTION, 1 );
   if( clear_olc( olc ) != RET_SUCCESS )
   {
      free_olc( olc );
      return NULL;
   }
   olc->commands = AllocList();
   olc->wSpaces = AllocList();
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

void inception_open( void *passed, char *arg )
{
   ACCOUNT_DATA *account = (ACCOUNT_DATA *)passed;

   text_to_account( account, "Opening Inception OLC...\r\n\r\n" );
   account->olc = init_olc();
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
   int center;

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

   bprintf( buf, "/%s\\\r\n", print_header( "Inception OLC", "-", account->pagewidth - 2 ) );
   mud_printf( tempstring, " You have %d workspaces loaded.", SizeOfList( olc->wSpaces ) );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, 78 ) );
   center = ( account->pagewidth - 7 ) / 2;
   bprintf( buf, "| %s  |", center_string( "Frameworks", center ) );
   bprintf( buf, " %s |\r\n", center_string( "Instances", center ) );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
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

