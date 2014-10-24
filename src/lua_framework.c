/* methods pertaining to lua boxed entity frameworks written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityFrameworkLib_m[] = {
   { "getID", getFrameID },
   { "getName", getFrameName },
   { "getShort", getFrameShort },
   { "getLong", getFrameLong },
   { "getDesc", getFrameDesc },
   { "getSpec", getFrameSpec },
   { "getInheritance", getFrameInheritance },
   { NULL, NULL }
};

const struct luaL_Reg EntityFrameworkLib_f[] = {
   { "getFramework", getFramework },
   { NULL, NULL }
};

int luaopen_EntityFrameworkLib( lua_State *L )
{
   luaL_newmetatable( L, "EntityFramework.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, EntityFrameworkGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, EntityFrameworkLib_m, 0 );

   luaL_newlib( L, EntityFrameworkLib_f );
   return 1;
}

int EntityFrameworkGC( lua_State *L )
{
   ENTITY_FRAMEWORK **frame;
   frame = (ENTITY_FRAMEWORK **)lua_touserdata( L, -1 );
   *frame = NULL;
   return 0;

}

/* lib functions */

int getFramework( lua_State *L )
{
   ENTITY_FRAMEWORK *framework;
   ENTITY_INSTANCE *instance;

   switch( lua_type( L, -1 ) )
   {
      default:
         bug( "5s: passed a bad argument.", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TUSERDATA:
         if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
         {
            bug( "%s: passed non-instance argument.", __FUNCTION__ );
            lua_pushnil( L );
            return 1;
         }
         framework = instance->framework;
         break;
      case LUA_TNUMBER:
         if( ( framework = get_framework_by_id( lua_tonumber( L, -1 ) ) ) == NULL )
         {
            bug( "%s: no frame with the ID %d", __FUNCTION__, lua_tonumber( L, -1 ) );
            lua_pushnil( L );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( framework = get_framework_by_name( lua_tostring( L, -1 ) ) ) == NULL )
         {
            bug( "%s: no frame with the name %s", __FUNCTION__, lua_tonumber( L, -1 ) );
            lua_pushnil( L );
            return 1;
         }
         break;
   }
   push_framework( framework, L );
   return 1;
}

/* getters */
int getFrameID( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   lua_pushnumber( L, frame->tag->id );
   return 1;
}

int getFrameName( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   lua_pushstring( L, chase_name( frame ) );
   return 1;
}

int getFrameShort( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   lua_pushstring( L, chase_short_descr( frame ) );
   return 1;
}

int getFrameLong( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   lua_pushstring( L, chase_long_descr( frame ) );
   return 1;
}

int getFrameDesc( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   lua_pushstring( L, chase_description( frame ) );
   return 1;
}

int getFrameSpec( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;
   SPECIFICATION *spec;
   const char *spectype;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spectype = luaL_checkstring( L, 2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spec = frame_has_spec( frame, spectype ) ) == NULL )
   {
      bug( "%s: no such spec %s.", __FUNCTION__, spectype );
      lua_pushnil( L );
      return 1;
   }
   push_specification( spec, L );
   return 1;
}

int getFrameInheritance( lua_State *L )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: EntityFramework.meta metatable is broken.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }
   push_framework( frame->inherits, L );
   return 1;
}




