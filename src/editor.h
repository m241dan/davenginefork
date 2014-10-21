/* editor.h written by Davenge */

void editor_global_return( void *passed, char *arg );
void editor_switch( void *passed, char *arg );
int free_editor( INCEPTION *olc );

int init_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame );
void boot_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame );

int editor_eFramework_prompt( D_SOCKET *dsock );
const char *return_framework_strings( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_framework_fixed_content( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_fixed_content_list( LLIST *fixed_list, const char *border, int width, bool inherited );
const char *return_framework_specs_and_stats( ENTITY_FRAMEWORK *frame, const char *border, int width );

void eFramework_name( void *passed, char *arg );
void eFramework_short( void *passed, char *arg );
void eFramework_long( void *passed, char *arg );
void eFramework_description( void *passed, char *arg );
void eFramework_addSpec( void *passed, char *arg );
void eFramework_done( void *passed, char *arg );
void eFramework_save( void *passed, char *arg );
void eFramework_addContent( void *passed, char *arg );
void eFramework_script( void *passed, char *arg );

int init_project_editor( INCEPTION *olc, PROJECT *project );
void boot_project_editor( INCEPTION *olc, PROJECT *project );

int editor_project_prompt( D_SOCKET *dsock );
const char *return_project_workspaces_string( PROJECT *project, const char *border, int width );

void project_name( void *passed, char *arg );
void project_public( void *passed, char *arg );
void project_done( void *passed, char *arg );

int init_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace );
void boot_workspace_editor( INCEPTION *olc, WORKSPACE *wSpace );

int editor_workspace_prompt( D_SOCKET *dsock );

void workspace_name( void *passed, char *arg );
void workspace_description( void *passed, char *arg );
void workspace_public( void *passed, char *arg );
void workspace_done( void *passed, char *arg );

int init_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance );
void boot_instance_editor( INCEPTION *olc, ENTITY_INSTANCE *instance );

int editor_instance_prompt( D_SOCKET *dsock );
const char *return_instance_contents_string( ENTITY_INSTANCE *instance, const char *border, int width );
const char *return_instance_spec_and_stats( ENTITY_INSTANCE *intance, const char *border, int width );

void instance_load( void *passed, char *arg );
void instance_live( void *passed, char *arg );
void instance_level( void *passed, char *arg );
void instance_addcontent( void *passed, char *arg );
void instance_addspec( void *passed, char *arg );
void instance_done( void *passed, char *arg );
