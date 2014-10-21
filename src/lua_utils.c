/* the actual functions for lua utility written by Davenge */

#include "mud.h"

bool prep_stack( const char *file, const char *function )
{
   int ret;

   if( !lua_handle )
   {
      bug( "%s: the lua stack isn't initialized", __FUNCTION__ );
      return FALSE;
   }

   lua_pushnil( lua_handle );
   lua_setglobal( lua_handle, function );

   if( ( ret = luaL_loadfile( lua_handle, file ) ) != 0 )
   {
      if( ret != LUA_ERRFILE )
         bug( "%s: %s: %s\n\r", __FUNCTION__, function, lua_tostring( lua_handle, -1 ) );
      lua_pop( lua_handle, 1 );
      return FALSE;
   }

   if( ( ret = lua_pcall( lua_handle, 0, 0, 0 ) ) != 0 )
   {
      bug( "%s: %s %s\r\n", __FUNCTION__, function, lua_tostring( lua_handle, -1 ) );
      lua_pop( lua_handle, 1 );
      return FALSE;
   }

   lua_getglobal( lua_handle, function );
   if( lua_isnil( lua_handle, -1 ) )
   {
      lua_pop( lua_handle, -1 );
      return FALSE;
   }
   return TRUE;
}
