/* account.h written by Davenge */

struct game_account
{
   D_SOCKET *socket;
   LLIST *characters;
   LLIST *command_tables;
   LLIST *commands;

   COMMAND *executing_command;
   char *last_command;

   ID_TAG *idtag;
   char *name;
   char *password;
   sh_int level;
   sh_int pagewidth;

   char *chatting_as;
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

/* utility */
ACCOUNT_DATA *check_account_reconnect(const char *act_name);
int text_to_account( ACCOUNT_DATA *account, const char *fmt, ... );
int account_prompt( D_SOCKET *dsock );

/* commands */
void account_quit( void *passed, char *arg );
void account_settings( void *passed, char *arg );
void account_chat( void *passed, char *arg );

/* setting */
void set_pagewidth( void *passed, char *arg );
void account_chatas( void *passed, char *arg );
