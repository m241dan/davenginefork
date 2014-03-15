/* methods pertaining to nannys written by Davege */

#include "mud.h"

/*****************
 * NANNY LIBRARY *
 *****************/
const nanny_lib_entry *const nanny_lib[] = {
   { "login", nanny_login_messages, nanny_login_code },
   { "new account", nanny_new_account_messages, nanny_new_account_code },
   { NULL, NULL, NULL } /* gandalf */
};

/***************
 * NANNY LOGIN *
 ***************/
const char *const nanny_login_messages[] = {
   "What's your account name?", "Password:",
   NULL /* gandalf */
};

const nanny_fun *const nanny_code[] = {
   nanny_login, nanny_password,
   NULL /* gandalf */
};

/*********************
 * NANNY NEW ACCOUNT *
 *********************/
const char *const nanny_new_account_messages[] = {
   "Please enter a password for your account: ", "Repeat the Password:",
   NULL /* gandalf */
};

const nanny_fun *const nanny_new_account_code[] = {
   nanny_new_password, nanny_confirm_new_password,
   NULL /* gandalf */
};

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
   nanny->info = NULL
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

   if( nanny->state < 0 || nanny->state >= MAX_NANNY_STATE )
   {
      bug( "%s: BAD STATE %d.", __FUNCTION__, nanny->state );
      return RET_FAILED_OTHER;
   }

   if( !strcmp( arg, "/back" ) )
   {
      nanny_state_prev( nanny, TRUE );
      return ret;
   }
   (*nanny->info.nanny_code[nanny->state]) nanny, arg );
   return ret;

}

int change_nanny_state( NANNY_DATA *nanny, int state, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state = state;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
   return ret;
}

int nanny_state_next( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state++;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
   return ret;
}

int nanny_state_prev( NANNY_DATA *nanny, bool message )
{
   int ret = RET_SUCCESS;
   nanny->state--;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
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
   if( dsock->nanny = NULL )
   {
      BAD_POINTER( "dsock->nanny" );
      return ret;
   }
   dsock->nanny->socket = NULL;
   dsock->nanny = NULL;
   return ret;
}

int nanny_login( NANNY_DATA *nanny, char *arg )
{
   int ret = RET_SUCCESS;
   return ret;
}
