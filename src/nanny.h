/* header file for nanny.c written by Davenge */

extern const struct nanny_lib_entry *const nanny_lib[];

struct nanny_data
{
   D_SOCKET *socket;
   void *content;
   const struct nanny_lib_entry *info;
   int state;
};

struct nanny_lib_entry
{
   char *name;
   const int **const nanny_code;
   const char **const nanny_messages;
};

/* creation */
NANNY_DATA *init_nanny( void );
int clear_nanny( NANNY_DATA *nanny );

/* deletion */
int free_nanny( NANNY_DATA *nanny );

/* input handling */
int handle_nanny_input( D_SOCKET *dsock, char *arg );

/* state handling */
int change_nanny_state( NANNY_DATA *nanny, int state, bool message );
int nanny_state_next( NANNY_DATA *nanny, bool message );
int nanny_state_prev( NANNY_DATA *nanny, bool message );

/* controlling */
int control_nanny( D_SOCKET *dsock, NANNY_DATA *nanny );
int uncontrol_nanny( D_SOCKET *dsock );

/* login */
nanny_fun nanny_login;
nanny_fun nanny_password;

/* new account */
nanny_fun nanny_new_password;
nanny_fun nanny_confirm_new_password;
