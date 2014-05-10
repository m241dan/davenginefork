/* account.h written by Davenge */

struct game_account
{
   D_SOCKET *socket;
   LLIST *characters;
   LLIST *command_tables;
   LLIST *commands;

   int accountID;
   char *name;
   char *password;
   sh_int level;
   sh_int pagewidth;

};

/* creation */
ACCOUNT_DATA *init_account( void );
int clear_account( ACCOUNT_DATA *account );

/* deletion */
int free_account( ACCOUNT_DATA *account );

/* i/o */
int load_account( ACCOUNT_DATA *account, const char *name );
int new_account( ACCOUNT_DATA *account );
int save_account( ACCOUNT_DATA *account );

/* prompt */
int account_prompt( D_SOCKET *dsock );

ACCOUNT_DATA *check_account_reconnect(const char *act_name);

/* setting */
