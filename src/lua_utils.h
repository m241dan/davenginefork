/* the headerfile for lua_utils.c written by Davenge */

int luaopen_mud( lua_State *L );
bool prep_stack( const char *file, const char *function );
const char *get_script_path_from_spec( SPECIFICATION *spec );
extern inline const char *get_frame_script_path( ENTITY_FRAMEWORK *frame );
extern inline const char *get_instance_script_path( ENTITY_INSTANCE *instance );
extern inline const char *get_stat_framework_script_path( STAT_FRAMEWORK *fstat );
extern inline const char *get_stat_instance_script_path( STAT_INSTANCE *stat );

void push_instance( ENTITY_INSTANCE *instance, lua_State *L );
void push_framework( ENTITY_FRAMEWORK *frame, lua_State *L );
void push_specification( SPECIFICATION *spec, lua_State *L );

int lua_bug( lua_State *L );
int lua_getGlobalVar( lua_State *L );
int lua_setGlobalVar( lua_State *L );
