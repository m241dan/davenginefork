/* the actual functions for lua utility written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityVariablesLib_f[] = {
  { "getGlobal", lua_getGlobalVar },
  { "setGlobal", lua_setGlobalVar },
  { NULL, NULL }
};

int luaopen_mud( lua_State *L )
{
   luaL_newlib( L, EntityVariablesLib_f );
   return 1;
}

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

void push_framework( ENTITY_FRAMEWORK *frame, lua_State *L )
{
   ENTITY_FRAMEWORK **box;

   if( !frame )
   {
      bug( "%s: trying to push a NULL frame.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      bug( "%s: bad framework trying to be pushed, created_by null", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (ENTITY_FRAMEWORK **)lua_newuserdata( L, sizeof( ENTITY_FRAMEWORK * ) );
   luaL_getmetatable( L, "EntityFramework.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: EntityFramework.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = frame;
   return;
}

void push_specification( SPECIFICATION *spec, lua_State *L )
{
   SPECIFICATION **box;

   if( !spec )
   {
      bug( "%s: trying to push a NULL spec.", __FUNCTION__ );
      lua_pushnil( L );
      return;
   }

   box = (SPECIFICATION **)lua_newuserdata( L, sizeof( SPECIFICATION * ) );
   luaL_getmetatable( L, "Specification.meta" );
   if( lua_isnil( L, -1 ) )
   {
      bug( "%s: Specification.meta is missing.", __FUNCTION__ );
      lua_pop( L, -1 );
      lua_pop( L, -1 );
      lua_pushnil( L );
      return;
   }
   lua_setmetatable( L, -2 );

   *box = spec;
   return;
}

int lua_getGlobalVar( lua_State *L )
{
   EVAR *var;
   const char *var_name;

   if( ( var_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var = get_global_var( var_name ) ) == NULL )
   {
      bug( "%s: no global var named %s.", __FUNCTION__, var_name );
      lua_pushnil( L );
      return 1;
   }
   switch( var->type )
   {
      default: lua_pushnil( L ); return 1;
      case VAR_INT:
         lua_pushnumber( L, atoi( var->value ) );
         return 1;
      case VAR_STR:
         lua_pushstring( L, var->value );
         return 1;
   }
   return 0;
}

int lua_setGlobalVar( lua_State *L )
{
   EVAR *var;
   const char *var_name;

   if( ( var_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no variable name passed.", __FUNCTION__ );
      return 0;
   }

   var = get_global_var( var_name );

   switch( lua_type( L, -1 ) )
   {
      default: bug( "%s: bad value passed.", __FUNCTION__ ); return 0;
      case LUA_TNUMBER:
         if( !var )
         {
            var = new_int_var( var_name, lua_tonumber( L, -1 ) );
            new_global_var( var );
            return 0;
         }
         if( var->type != VAR_INT )
            update_var_type( var, VAR_INT );
         update_var_value( var, itos( lua_tonumber( L, -1 ) ) );
         return 0;
      case LUA_TSTRING:
         if( !var )
         {
            var = new_str_var( var_name, lua_tostring( L, -1 ) );
            new_global_var( var );
            return 0;
         }
         if( var->type != VAR_STR )
            update_var_type( var, VAR_STR );
         update_var_value( var, lua_tostring( L, -1 ) );
         return 0;
   }
   return 0;
}
