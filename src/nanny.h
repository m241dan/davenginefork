/* header file for nanny.c written by Davenge */

extern const char *const nanny_messages[MAX_NANNY_TYPE][MAX_NANNY_STATE];
extern const nanny_fun *const nanny_code[MAX_NANNY_TYPE[MAX_NANNY_STATE];

struct nanny_data
{
   D_SOCKET *socket;
   void *content;
   nanny_fun *input_handler;

   int type;
   int state;
};

/* creation */
NANNY_DATA *init_nanny( void );
void clear_nanny( NANNY_DATA *nanny );

/* deletion */
void free_nanny( NANNY_DATA *nanny );

/* input handling */
void handle_nanny_input( D_SOCKET *dsock, char *arg );

/* state handling */
void change_nanny_state( NANNY_DATA *nanny, int state, bool message );
void nanny_state_next( NANNY_DATA *nanny, bool message );
void nanny_state_prev( NANNY_DATA *nanny, bool message );

/* controlling */
int control_nanny( D_SOCKET *dsock, NANNY_DATA *nanny );
int uncontrol_nanny( D_SOCLET *dsock );

/* login */
NANNY_FUN nanny_login;
NANNY_FUN nanny_password;

/* new account */
NANNY_FUN nanny_new_password;
NANNY_FUN nanny_confirm_new_password;
