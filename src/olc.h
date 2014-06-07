/* olc.h written by Davenge */

struct inception_olc
{
   ACCOUNT_DATA *account;
   LLIST *commands;
   LLIST *wSpaces;
   WORKSPACE *displaying_workspace;
};

struct workspace
{
   LLIST *frameworks;
   bool hide_frameworks;

   LLIST *instances;
   bool hide_instances;
};

INCEPTION *init_olc( void );
int free_olc( INCEPTION *olc );
int clear_olc( INCEPTION *olc );

void inception_open( void *passed, char *arg );
int olc_prompt( D_SOCKET *dsock );
int text_to_olc( INCEPTION *olc, const char *fmt, ... );
