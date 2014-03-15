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

void clear_nanny( NANNY_DATA *nanny )
{
   nanny->socket = NULL;
   nanny->content = NULL;
   nanny->input_handler = NULL;

   nanny->type = -1;
   nanny->state = -1;
}

/* deletion */
void free_nanny( NANNY_DATA *nanny )
{
   clear_nanny( nanny );
   free_nanny( nanny );
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

   if( nanny->type < 0 || nanny->type >= MAX_NANNY_TYPE )
   {
      bug( "%s: BAD TYPE %d.", __FUCNTION__, nanny->type );
      return RET_FAILED_OTHER;
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

   (*nanny_code[nanny->type][nanny->state])( nanny, arg );
   return ret;

}

void change_nanny_state( NANNY_DATA *nanny, int state, bool message )
{
   nanny->state = state;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
   return;
}

void nanny_state_next( NANNY_DATA *nanny, bool message )
{
   nanny->state++;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
   return;
}

void nanny_state_prev( NANNY_DATA *nanny, bool message )
{
   nanny->state--;
   if( message )
      text_to_nanny( nanny, nanny_messages[nanny->type][nanny->state] );
   return;
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

void nanny_login( NANNY_DATA *nanny, char *arg )
{

}
