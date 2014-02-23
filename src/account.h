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


}

/* creation */
ACCOUNT_DATA *init_account( void );
void clear_account( ACCOUNT_DATA *account );

/* deletion */
void free_account( ACCOUNT_DATA *account );
