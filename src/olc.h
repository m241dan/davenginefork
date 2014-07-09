/* olc.h written by Davenge */

struct inception_olc
{
   ACCOUNT_DATA *account;
   LLIST *commands;
   LLIST *wSpaces;
   WORKSPACE *using_workspace;
};

struct workspace
{
   ID_TAG *tag;
   char *name;
   char *description;
   bool Public;

   LLIST *frameworks;
   bool hide_frameworks;

   LLIST *instances;
   bool hide_instances;
};

INCEPTION *init_olc( void );
int free_olc( INCEPTION *olc );
int clear_olc( INCEPTION *olc );

WORKSPACE *init_workspace( void );
int free_workspace( WORKSPACE *wSpace );
int clear_workspace( WORKSPACE *wSpace );
int load_workspaces( void );

void inception_open( void *passed, char *arg );
int olc_prompt( D_SOCKET *dsock );
int text_to_olc( INCEPTION *olc, const char *fmt, ... );
void olc_no_prompt( INCEPTION *olc );

void olc_file( void *passed, char *arg );
void olc_workspace( void *passed, char *arg );
void workspace_new( void *passed, char *arg );
void workspace_load( void *passed, char *arg );
void olc_frameworks( void *passed, char *arg );
void framework_create( void *passed, char *arg );
void olc_using( void *passed, char *arg );
void olc_quit( void *passed, char *arg );
