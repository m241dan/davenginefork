/* the headerfile for lua_utils.c written by Davenge */

bool prep_stack( const char *file, const char *function );
const char *get_script_path_from_spec( SPECIFICATION *spec );
const char *get_frame_script_path( ENTITY_FRAMEWORK *frame );
const char *get_instance_script_path( ENTITY_INSTANCE *instance );

void push_instance( ENTITY_INSTANCE *instance, lua_State *L );
