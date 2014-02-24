/* account.h written by Davenge */


struct game_account
{
   D_SOCKET *socket;
   LIST *characters;
   LIST *command_tables;
   LIST *commands;

   char *name;
   char *password;
   sh_int level;
   sh_int pagewidth;


};

/* creation */
ACCOUNT_DATA *init_account( void );
void clear_account( ACCOUNT_DATA *account );

/* deletion */
void free_account( ACCOUNT_DATA *account );

/* i/o */
int load_account_file( const char *path, ACCOUNT_DATA *account );
int save_account( ACCOUNT_DATA *account );
int fwrite_account_base( ACCOUNT_DATA *account, FILE *fp );
int fread_account_base( ACCOUNT_DATA *account, FILE *fp );

/* setting */
void set_account( ACCOUNT_DATA *account, VALUE value, int type );
