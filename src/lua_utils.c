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

const char *get_script_path_from_spec( SPECIFICATION *spec )
{
   int id;

   if( spec->owner[0] == 'f' )
   {
      id = atoi( spec->owner +1 );
      return get_frame_script_path( get_framework_by_id( id ) );
   }
   id = atoi( spec->owner );
   return get_instance_script_path( get_instance_by_id( id ) );
}

const char *get_frame_script_path( ENTITY_FRAMEWORK *frame )
{
   return quick_format( "../scripts/frames/%d.lua", frame->tag->id );
}

const char *get_instance_script_path( ENTITY_INSTANCE *instance )
{
   return quick_format( "../scripts/instances/%d.lua", instance->tag->id );
}

void push_instance( ENTITY_INSTANCE *instance, lua_State *L )
{
   ENTITY_INSTANCE **box;

   if( !instance )
   {
       bug( "%s: trying to push a NULL instance.", __FUNCTION__ );
       lua_pushnil( L );
       return;
   }

   if( !strcmp( instance->tag->created_by, "null" ) )
   {
      bug( "%s: bad instance trying to be pushed, created_by null", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (ENTITY_INSTANCE **)lua_newuserdata( L, sizeof( ENTITY_INSTANCE * ) );
   luaL_getmetatable( L, "EntityInstance.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: EntityInstance.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 ); /* pop meta */
      lua_pop( L, -1 ); /* pop box */
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = instance;
   return;
}
