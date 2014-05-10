/* methods pertaining to nannys written by Davege */

#include "mud.h"

/*****************
 * NANNY LIBRARY *
 *****************/

const struct nanny_lib_entry nanny_lib[] = {
   { "login", nanny_login_messages, nanny_login_code, FALSE },
   { "new account", nanny_new_account_messages, nanny_new_account_code, FALSE },
   { NULL, NULL, NULL, FALSE } /* gandalf */
};

/***************
 * NANNY LOGIN *
 ***************/
const char *const nanny_login_messages[] = {
   "What's your account name? ", "Password: ",
   NULL /* gandalf */
};

nanny_fun *const nanny_login_code[] = {
   nanny_login, nanny_password,
   NULL /* gandalf */
};

int nanny_login( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   ACCOUNT_DATA *a_new;

   if( ( ret = load_account( a_new, arg ) ) == RET_DB_NO_ENTRY ) */
   {
      if( !check_name( arg ) )
      {
         text_to_nanny( nanny, "%s is a bad name, please try again: ", arg );
         return ret;
      }

      if( ( a_new = init_account() ) == NULL )
      {
         bug( "%s: could not allocate new account." );
         return ret;
      }
      a_new->name = strdup( arg );
      if( set_nanny_lib_from_name( nanny, "new account" ) != RET_SUCCESS )
      {
         bug( "%s: 'new account' nanny lib missing.", __FUNCTION__ );
         text_to_nanny( nanny, "Something is really messed up." );
         close_socket( nanny->socket, FALSE );
         return RET_FAILED_OTHER;
      }
      change_nanny_state( nanny, 0, TRUE );

   }
   else if( ret = RET_SUCCESS )
   {
      nanny->socket->account = a_new;
      a_new->socket = nanny->socket;
      change_socket_state( nanny->socket, STATE_ACCOUNT );
   }
   else if( ret = RET_FAILED_OTHER )
   {
      text_to_nanny( nanny, "There's been a major error." );
      close_socket( nanny->socket, FALSE );
      return ret;
   }
   return ret;
}

int nanny_password( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   return ret;
}

/*********************
 * NANNY NEW ACCOUNT *
 *********************/
const char *const nanny_new_account_messages[] = {
   "Please enter a password for your account: ", "Repeat the Password: ",
   NULL /* gandalf */
};

nanny_fun *nanny_new_account_code[] = {
   nanny_new_password, nanny_confirm_new_password,
   NULL /* gandalf */
};

int nanny_new_password( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   return ret;
}

int nanny_confirm_new_password( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   return ret;
}
/***********************
 * NANNY SPECIFIC CODE *
 ***********************/

/* creation */
NANNY_DATA *init_nanny( void )
{
   NANNY_DATA *nanny;

   CREATE( nanny, NANNY_DATA, 1 );
   clear_nanny( nanny );
   return nanny;
}

int clear_nanny( NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   nanny->socket = NULL;
   nanny->content = NULL;
   nanny->info = NULL;
   nanny->state = -1;

   return ret;
}

/* deletion */
int free_nanny( NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   clear_nanny( nanny );
   free_nanny( nanny );

   return ret;
}

/* input handling */
int handle_nanny_input( D_SOCKET *dsock, char *arg )
{
   NANNY_DATA *nanny;
   int ret = RET_SUCCESS;

   if( ( nanny = dsock->nanny ) == NULL)
   {
      BAD_POINTER( "nanny" );
      return ret;
   }

   if( !nanny->info )
   {
      BAD_POINTER( "info" );
      return ret;
   }

   if( nanny->state < 0 )
   {
      bug( "%s: BAD STATE %d.", __FUNCTION__, nanny->state );
      return RET_FAILED_OTHER;
   }

   if( !strcmp( arg, "/back" ) )
   {
      if( !nanny->info->back_allowed )
      {
         text_to_socket( dsock, "Going back is not allowed.\r\n" );
         return ret;
      }
      nanny_state_prev( nanny, TRUE );
      return ret;
   }
   (*nanny->info->nanny_code[nanny->state])( nanny, arg );
   return ret;

}

int change_nanny_state( NANNY_DATA *nanny, int state, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state = state;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

int nanny_state_next( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state++;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

int nanny_state_prev( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state--;
   if( message )
      text_to_buffer( nanny->socket, nanny->info->nanny_messages[nanny->state] );
   return ret;
}

/* controlling */
int control_nanny( D_SOCKET *dsock, NANNY_DATA *nanny )
{
   int ret = RET_SUCCESS;

   if( dsock == NULL )
   {
      BAD_POINTER( "dsock" );
      return ret;
   }

   if( nanny == NULL )
   {
      BAD_POINTER( "nanny" );
      return ret;
   }
   dsock->nanny = nanny;
   nanny->socket = dsock;
   return ret;
}

int uncontrol_nanny( D_SOCKET *dsock )
{
   int ret = RET_SUCCESS;

   if( dsock == NULL )
   {
      BAD_POINTER( "dsock" );
      return ret;
   }
   if( dsock->nanny == NULL )
   {
      BAD_POINTER( "dsock->nanny" );
      return ret;
   }
   dsock->nanny->socket = NULL;
   dsock->nanny = NULL;
   return ret;
}

/* communication */
int text_to_nanny( NANNY_DATA *nanny, const char *fmt, ... )
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

   text_to_buffer( nanny->socket, dest );
   return res;
}

int set_nanny_lib_from_name( NANNY_DATA *dest, const char *name )
{
   int x;

   for( x = 0; nanny_lib[x].name != NULL || nanny_lib[x].name[0] != '\0'; x++ )
   {
      if( !strcmp( nanny_lib[x].name, name ) )
      {
         dest->info = &nanny_lib[x];
         return RET_SUCCESS;
      }
   }

   bug( "%s: could not find library entry titled %s.", __FUNCTION__, name );
   return RET_FAILED_NO_LIB_ENTRY;
}

