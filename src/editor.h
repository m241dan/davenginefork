/* editor.h written by Davenge */

int init_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame );
int free_editor( INCEPTION *olc );

int editor_eFramework_prompt( D_SOCKET *dsock );
const char *return_framework_strings( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_framework_specs_and_stats( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_spec_and_stat_list( LLIST *spec_list, const char *border, int width, bool inherited );
const char *return_spec_and_stat( SPECIFICATION *spec, const char *border, int width, bool inherited );
const char *return_framework_fixed_content( ENTITY_FRAMEWORK *frame, const char *border, int width );
const char *return_fixed_content_list( LLIST *fixed_list, const char *border, int width, bool inherited );

void eFramework_name( void *passed, char *arg );
void eFramework_short( void *passed, char *arg );
void eFramework_long( void *passed, char *arg );
void eFramework_description( void *passed, char *arg );
void eFramework_addSpec( void *passed, char *arg );
void eFramework_done( void *passed, char *arg );
void eFramework_addContent( void *passed, char *arg );

int init_project_editor( INCEPTION *olc, PROJECT *project );

int editor_project_prompt( D_SOCKET *dsock );


