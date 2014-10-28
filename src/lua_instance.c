/* methods pertaining to lua boxed entity instances written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityInstanceLib_m[] = {
   /* getters */
   { "getName", getName },
   { "getShort", getShort },
   { "getLong", getLong },
   { "getDesc", getDesc },
   { "getID", getID },
   { "getLevel", getID },
   { "getItemFromInventory", getItemFromInventory },
   { "getSpec", getSpec },
   { "getFramework", getInstanceFramework },
   { "getContainer", getContainer },
   { "getVar", getVar },

   /* setters */
   { "setVar", setVar },
   { "addSpec", addSpec },
   /* bools */
   { "isLoaded", isLoaded },
   { "isLive", isLive },
   { "isBuilder", isBuilder },
   { "hasItemInInventoryFramework", hasItemInInventoryFramework },
   { "isSameRoom", isSameRoom },
   /* actions */
   { "interp", luaEntityInstanceInterp },
   { "to", luaEntityInstanceTeleport },
   { "echo", luaEcho },
   { "echoAt", luaEchoAt },
   { "echoAround", luaEchoAround },
   { NULL, NULL } /* gandalf */
};

const struct luaL_Reg EntityInstanceLib_f[] = {
   { "getInstance", getInstance },
   { "new", luaNewInstance },
   { NULL, NULL } /* gandalf */
};

int luaopen_EntityInstanceLib( lua_State *L )
{
   luaL_newmetatable( L, "EntityInstance.meta" );

   lua_pushvalue( L, -1 );
   lua_setfield( L, -2, "__index" );

   lua_pushcfunction( L, EntityInstanceGC );
   lua_setfield( L, -2, "__gc" );

   luaL_setfuncs( L, EntityInstanceLib_m, 0 );

   luaL_newlib( L, EntityInstanceLib_f );
   return 1;
}

int EntityInstanceGC( lua_State *L )
{
   ENTITY_INSTANCE **instance;
   instance = (ENTITY_INSTANCE **)lua_touserdata( L, -1 );
   *instance = NULL;
   return 0;
}


int getInstance( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   switch( lua_type( L, -1 ) )
   {
      default:
         bug( "%s: passed non-string/integer.", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TSTRING:
         instance = get_instance_by_name( lua_tostring( L, -1 ) );
         break;
      case LUA_TNUMBER:
         instance = get_instance_by_id( lua_tonumber( L, -1 ) );
         break;
   }

   push_instance( instance, L );
   return 1;
}

int luaNewInstance( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_FRAMEWORK *frame;

   if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, -1, "EntityFramework.meta" ) ) == NULL )
   {
      bug( "%s: bad meta table.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   instance = eInstantiate( frame );
   new_eInstance( instance );
   push_instance( instance, L );
   return 1;
}

int getName( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );

   lua_pushstring( L, instance_name( instance ) );
   return 1;
}

int getShort( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_short_descr( instance ) );
   return 1;
}

int getLong( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_long_descr( instance ) );
   return 1;
}

int getDesc( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushstring( L, instance_description( instance ) );
   return 1;
}

int getID( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, instance->tag->id );
   return 1;
}

int getLevel( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   lua_pushnumber( L, instance->level );
   return 1;
}

int getItemFromInventory( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *item;

   int top = lua_gettop( L );

   if( top != 2 )
   {
      bug( "%s: passed improper amount of arguments.", __FUNCTION__, top );
      lua_pushnil( L );
      return 1;
   }

   DAVLUACM_INSTANCE_NIL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: bad argument passed", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TNUMBER:
         if( ( item = instance_list_has_by_id( instance->contents, lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( item = instance_list_has_by_name_regex( instance->contents, lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
   }
   push_instance( item, L );
   return 1;
}

int getSpec( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   SPECIFICATION *spec;
   const char *spectype;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( ( spectype = luaL_checkstring( L, 2 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( spec = has_spec( instance, spectype ) ) == NULL )
   {
      bug( "%s: no such spec %s.", __FUNCTION__, spectype );
      lua_pushnil( L );
      return 1;
   }
   push_specification( spec, L );
   return 1;
}

int getInstanceFramework( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   push_framework( instance->framework, L );
   return 1;
}

int getContainer( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NIL( instance, L );
   if( instance->contained_by )
      push_instance( instance->contained_by, L );
   else
      lua_pushnil( L );
   return 1;
}

int getVar( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   EVAR *var;
   const char *var_name;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( instance->tag->id == -69 )
   {
      bug( "%s: builders have no vars", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var_name = luaL_checkstring( L, -1 ) ) == NULL )
   {
      bug( "%s: no string passed.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   if( ( var = get_entity_var( instance, var_name ) ) == NULL )
   {
      bug( "%s: entity %s has no var %s.", __FUNCTION__, instance_short_descr( instance ), var_name );
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

int setVar( lua_State *L )
{
   EVAR *var;
   ENTITY_INSTANCE *instance;
   const char *var_name;

   DAVLUACM_INSTANCE_NIL( instance, L );

   if( instance->tag->id == -69 )
   {
      bug( "%s: builders have no vars", __FUNCTION__ );
      return 0;
   }

   if( ( var_name = luaL_checkstring( L, -2 ) ) == NULL )
   {
      bug( "%s: no variable name passed.", __FUNCTION__ );
      return 0;
   }

   var = get_entity_var( instance, var_name );

   switch( lua_type( L, -1 ) )
   {
      default: bug( "%s: bad value passed.", __FUNCTION__ ); return 0;
      case LUA_TNUMBER:
         if( !var )
         {
            var = new_int_var( var_name, lua_tonumber( L, -1 ) );
            new_entity_var( instance, var );
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
         update_var_value( var, lua_tostring( L, -2 ) );
         return 0;
   }
   return 0;
}

int addSpec( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   SPECIFICATION *spec;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( instance->tag->id == -69 )
   {
      bug( "%s: don't spec builders plz", __FUNCTION__ );
      return 0;
   }

   if( ( spec = *(SPECIFICATION **)luaL_checkudata( L, 2, "Specification.meta" ) ) == NULL )
   {
      bug( "%s: no spec passed.", __FUNCTION__ );
      return 0;
   }

   if( strcmp( spec->owner, "null" ) )
   {
      bug( "%s: cannot add a spec that already has an owner.", __FUNCTION__ );
      return 0;
   }
   add_spec_to_instance( spec, instance );
   return 0;
}

int isLoaded( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->loaded );
   return 1;
}

int isLive( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->live );
   return 1;
}

int isBuilder( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );
   lua_pushboolean( L, (int)instance->builder );
   return 1;
}

int isSameRoom( lua_State *L )
{
   ENTITY_INSTANCE *instance, *oth_instance;

   DAVLUACM_INSTANCE_BOOL( instance, L );

   if( ( oth_instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: argument passed is not of EntityInstance.meta", __FUNCTION__ );
      lua_pushboolean( L, 0 );
      return 1;
   }

   if( instance->contained_by == oth_instance->contained_by )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

int hasItemInInventoryFramework( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *item;
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;
   int top = lua_gettop( L );
   bool found = FALSE;

   if( top != 2 )
   {
      bug( "%s: bad number of arguments passed %d", __FUNCTION__, top );
      lua_pushboolean( L, 0 );
      return 1;
   }

   DAVLUACM_INSTANCE_BOOL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: passed bad argument.", __FUNCTION__ );
         lua_pushboolean( L, 0 );
         return 1;
      case LUA_TUSERDATA:
         if( ( frame = *(ENTITY_FRAMEWORK **)luaL_checkudata( L, 2, "EntityFramework.meta" ) ) == NULL )
         {
            if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
            {
               bug( "%s: bad userdata passed.", __FUNCTION__ );
               lua_pushboolean( L, 0 );
               return 1;
            }
            frame = instance->framework;
         }
         break;
      case LUA_TNUMBER:
         if( ( frame = get_framework_by_id( lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( frame = get_framework_by_name( lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
   }

   AttachIterator( &Iter, instance->contents );
   while( ( item = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( item->framework == frame )
      {
         found = TRUE;
         break;
      }
   DetachIterator( &Iter );

   if( found )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );

   return 1; 
}

/* actions */
int luaEntityInstanceInterp( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   char  *order;

   DAVLUACM_INSTANCE_NONE( instance, L );

   if( lua_type( L, 2 ) != LUA_TSTRING )
   {
      bug( "%s: non-string input passed.\r\n", __FUNCTION__ );
      return 0;
   }

   order = strdup( lua_tostring( L, 2 ) );

   entity_handle_cmd( instance, order );
   FREE( order );
   return 0;
}

int luaEntityInstanceTeleport( lua_State *L )
{
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *destination;
   int top = lua_gettop( L );

   if( top != 2 )
   {
      bug( "%s: improper amount of arguments passed %d.", __FUNCTION__, top );
      lua_pushboolean( L, 0 );
      return 1;
   }

   DAVLUACM_INSTANCE_BOOL( instance, L );

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: bad destination passed", __FUNCTION__ );
         lua_pushboolean( L, 0 );
         return 1;
      case LUA_TUSERDATA:
         if( ( destination = *(ENTITY_INSTANCE **)luaL_checkudata( L, 2, "EntityInstance.meta" ) ) == NULL )
         {
            bug( "%s: passed non-entity instance userdata", __FUNCTION__ );
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( destination = get_instance_by_name( lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
      case LUA_TNUMBER:
         if( ( destination = get_instance_by_id( lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushboolean( L, 0 );
            return 1;
         }
         break;
   }

   entity_to_world( instance, destination );

   lua_pushboolean( L, 1 );
   return 1;
}

int luaEcho( lua_State *L )
{
   ENTITY_INSTANCE *room;

   DAVLUACM_INSTANCE_NONE( room, L );
   echo_to_room( room, lua_tostring( L, 2 ) );
   return 0;
}

int luaEchoAt( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   DAVLUACM_INSTANCE_NONE( instance, L );
   text_to_entity( instance, lua_tostring( L, 2 ) );
   return 0;
}

int luaEchoAround( lua_State *L )
{
   ENTITY_INSTANCE *room, *instance;
   int x, max = lua_gettop( L );

   DAVLUACM_INSTANCE_NONE( room, L );
   for( x = 1; x < ( max - 1 ); x++ )
   {
      if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, x, "EntityInstance.meta" ) ) == NULL )
         continue;
      text_to_entity( instance, lua_tostring( L, -1 ) );
   }
   return 0;
}
