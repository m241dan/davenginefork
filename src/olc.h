/* olc.h written by Davenge */

struct inception_olc
{
   ACCOUNT_DATA *account;
   LLIST *commands;
   LLIST *editor_commands;
   LLIST *wSpaces;
   PROJECT *project;
   WORKSPACE *using_workspace;
   void *editing;
   int editing_state;
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

   LLIST *who_using;
};

INCEPTION *init_olc( void );
int free_olc( INCEPTION *olc );
int clear_olc( INCEPTION *olc );

WORKSPACE *init_workspace( void );
int free_workspace( WORKSPACE *wSpace );
int clear_workspace( WORKSPACE *wSpace );
WORKSPACE *load_workspace_by_query( const char *query );
WORKSPACE *get_workspace_by_id( int id );
WORKSPACE *get_active_workspace_by_id( int id );
WORKSPACE *load_workspace_by_id( int id );
WORKSPACE *get_workspace_by_name( const char *name );
WORKSPACE *get_active_workspace_by_name( const char *name );
WORKSPACE *load_workspace_by_name( const char *name );
void db_load_workspace( WORKSPACE *wSpace, MYSQL_ROW *row );
void unuse_workspace( WORKSPACE *wSpace, ACCOUNT_DATA *account );

WORKSPACE *workspace_list_has_by_name( LLIST *workspace_list, const char *name );
WORKSPACE *workspace_list_has_by_id( LLIST *workspace_list, int id );

void inception_open( void *passed, char *arg );
int olc_prompt( D_SOCKET *dsock );
int text_to_olc( INCEPTION *olc, const char *fmt, ... );
void olc_no_prompt( INCEPTION *olc );
void olc_short_prompt( INCEPTION *olc );
void olc_show_prompt( INCEPTION *olc );
int new_workspace( WORKSPACE *wSpace );
int add_frame_to_workspace( ENTITY_FRAMEWORK *frame, WORKSPACE *wSpace );
int add_instance_to_workspace( ENTITY_INSTANCE *instance, WORKSPACE *wSpace );
int add_workspace_to_olc( WORKSPACE *wSpace, INCEPTION *olc );
int new_workspace_entry( WORKSPACE *wSpace, ID_TAG *tag );
int load_workspace_entries( WORKSPACE *wSpace );
bool workspace_list_has_name( LLIST *wSpaces, const char *name );

void olc_file( void *passed, char *arg );
void olc_workspace( void *passed, char *arg );
void workspace_new( void *passed, char *arg );
void workspace_load( void *passed, char *arg );
void workspace_unload( void *passed, char *arg );
void workspace_grab( void *passed, char *arg );
void olc_frameworks( void *passed, char *arg );
void framework_create( void *passed, char *arg );
void framework_edit( void *passed, char *arg );
void framework_iedit( void *passed, char *arg );
void olc_instantiate( void *passed, char *arg );
void olc_using( void *passed, char *arg );
void olc_builder( void *passed, char *arg );
void olc_show( void *passed, char *arg );
void olc_quit( void *passed, char *arg );
void olc_load( void *passed, char *arg );
void olc_chat( void *passed, char *arg );
