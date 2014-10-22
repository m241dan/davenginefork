/* header file for lua_instance.c written by Davenge */

int luaopen_EntityInstanceLib( lua_State *L );
int EntityInstanceGC( lua_State *L );

/* lib functions */

int getInstance( lua_State *L );

/* meta methods */

/* getters */
int getName( lua_State *L );
int getShort( lua_State *L );
int getLong( lua_State *L );
int getDesc( lua_State *L );
int getID( lua_State *L );
int getLevel( lua_State *L);
int getItemFromInventory( lua_State *L );

/* bools */
int isLoaded( lua_State *L );
int isLive( lua_State *L );
int isBuilder( lua_State *L );

/* actions */
int luaEntityInstanceInterp( lua_State *L );

/* later */
int getStat( lua_State *L );
int getSpec( lua_State *L );
int getFramework( lua_State *L );
