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
   /* bools */
   { "isLoaded", isLoaded },
   { "isLive", isLive },
   { "isBuilder", isBuilder },
   { "hasItemInInventoryFramework", hasItemInInventoryFramework },
   /* actions */
   { "interp", luaEntityInstanceInterp },
   { "teleport", luaEntityInstanceTeleport },
   { NULL, NULL } /* gandalf */
};

const struct luaL_Reg EntityInstanceLib_f[] = {
   { "getInstance", getInstance },
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

int getName( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, instance_name( instance ) );
   return 1;
}

int getShort( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, instance_short_descr( instance ) );
   return 1;
}

int getLong( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, instance_long_descr( instance ) );
   return 1;
}

int getDesc( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushstring( L, instance_description( instance ) );
   return 1;
}

int getID( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, -1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushnumber( L, instance->tag->id );
   return 1;
}

int getLevel( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

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
   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   switch( lua_type( L, 2 ) )
   {
      default:
         bug( "%s: bad argument passed", __FUNCTION__ );
         lua_pushnil( L );
         return 1;
      case LUA_TNUMBER:
         if( ( item = get_instance_by_id( lua_tonumber( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
      case LUA_TSTRING:
         if( ( item = get_instance_by_name( lua_tostring( L, 2 ) ) ) == NULL )
         {
            lua_pushnil( L );
            return 1;
         }
         break;
   }

   push_instance( item, L );
   return 1;
}

int isLoaded( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushboolean( L, (int)instance->loaded );
   return 1;
}

int isLive( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushboolean( L, (int)instance->live );
   return 1;
}

int isBuilder( lua_State *L )
{
   ENTITY_INSTANCE *instance;

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushnil( L );
      return 1;
   }

   lua_pushboolean( L, (int)instance->builder );
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

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushboolean( L, 0 );
      return 1;
   }

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

   if( ( instance = *(ENTITY_INSTANCE**)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      return 0;
   }

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

   if( ( instance = *(ENTITY_INSTANCE **)luaL_checkudata( L, 1, "EntityInstance.meta" ) ) == NULL )
   {
      bug( "%s: passed non-instance argument.", __FUNCTION__ );
      lua_pushboolean( L, 0 );
      return 1;
   }

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

