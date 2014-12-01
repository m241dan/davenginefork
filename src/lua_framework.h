/* header file for lua_framework.c written by Davenge */

int luaopen_EntityFrameworkLib( lua_State *L );
int EntityFrameworkGC( lua_State *L );

/* lib functions */

int getFramework( lua_State *l );

/* getters */
int getFrameID( lua_State *L );
int getFrameName( lua_State *L );
int getFrameShort( lua_State *L );
int getFrameLong( lua_State *L );
int getFrameDesc( lua_State *L );
int getFrameSpec( lua_State *L );
int getFrameInheritance( lua_State *L );
int getFrameHeight( lua_State *L );
int getFrameWeight( lua_State *L );
int getFrameWidth( lua_State *L );
/* actions */
int luaInherits( lua_State *L );

