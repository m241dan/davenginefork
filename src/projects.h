/* project.h the headerfile for all things project related written by Davenge */

struct project
{
   ID_TAG *tag;
   char *name;
   bool Public;

   LLIST *workspaces;
};

PROJECT *init_project( void );
int free_project( PROJECT *project );
int clear_project( PROJECT *project );

PROJECT *load_project_by_query( const char *query );
PROJECT *load_project_by_id( int id );
PROJECT *load_project_by_name( const char *name );

int new_project( PROJECT *project );
int new_project_entry( PROJECT *project, ID_TAG *tag );
void db_load_project( PROJECT *project, MYSQL_ROW *row );
void load_project_entries( PROJECT *project );
void load_project_workspaces_into_olc( PROJECT *project, INCEPTION *olc );
void add_workspace_to_project( WORKSPACE *wSpace, PROJECT *project );
void rem_workspace_from_project( WORKSPACE *wSpace, PROJECT *project );
void load_project_into_olc( PROJECT *project, INCEPTION *olc );

void export_project( PROJECT *project );

void project_newProject( void *passed, char *arg );
void project_openProject( void *passed, char *arg );
