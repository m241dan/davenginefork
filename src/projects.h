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

void save_workspace_list_export( LLIST *workspace_list, char *directory, int *workspace_id_table, int *instance_id_table, int *framework_id_table );
void save_workspace_export( char *pDir, WORKSPACE *wSpace, int *workspace_id_table, int *instance_id_table, int *framework_id_table );
void fwrite_workspace_export( FILE *fp, WORKSPACE *wSpace, int *workspace_id_table, int *instance_id_table, int *framework_id_table );
void fwrite_workspace_entries_export( FILE *fp, WORKSPACE *wSpace, int *instance_id_table, int *framework_id_table );

void save_instance_list_export( LLIST *instance_list, char *directory, int *instance_id_table, int *framework_id_table );
void save_instance_export( char *pDir, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table );
void fwrite_instance_export( FILE *fp, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table );
void fwrite_instance_content_list_export( FILE *fp, LLIST *contents, int *instance_id_table );

void save_framework_list_export( LLIST *instance_list, char *directory, int *framework_id_table );
void save_framework_export( char *pDir, ENTITY_FRAMEWORK *frame, int *framework_id_table );
void fwrite_framework_export( FILE *fp, ENTITY_FRAMEWORK *wSpace, int *framework_id_table );
void fwrite_framework_content_list_export( FILE *fp, LLIST *contents, int *framework_id_table );

char *create_project_directory( PROJECT *project );
void create_complete_framework_and_instance_list_from_workspace_list( LLIST *workspace_list, LLIST *instance_list, LLIST *framework_list );
void append_instance_list_content_to_list_recursive_ndi( LLIST *instance_list, LLIST *append_list );
void append_framework_list_content_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list );
void append_framework_list_inheritance_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list );
void copy_all_workspace_and_contents( PROJECT *project, LLIST *workspace_list, LLIST *framework_list, LLIST *instance_list );
void copy_all_instance_frames_into_list_ndi( LLIST *instance_list, LLIST *frame_list );

void project_newProject( void *passed, char *arg );
void project_openProject( void *passed, char *arg );
void project_exportProject( void *passed, char *arg );
