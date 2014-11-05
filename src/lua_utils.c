/* the actual functions for lua utility written by Davenge */

#include "mud.h"

const struct luaL_Reg EntityVariablesLib_f[] = {
  { "bug",       lua_bug },
  { "getGlobal", lua_getGlobalVar },
  { "setGlobal", lua_setGlobalVar },
  { NULL, NULL }
};

int count_lua_functions( char *str )
{
   char buf[510];
   int count = 0;

   while( str && str[0] != '\0' )
   {
      str = one_arg_delim( str, buf, '\n' );
      if( is_prefix( buf, "function" ) )
         count++;
   }
   return count;
}


LUA_FUNCTION_ARRAY get_functions( char *str )
{
   LUA_FUNCTION_ARRAY func_array;
   int size, x;

   size = count_lua_functions( str );
   if( size == 0 )
   {
      printf( "%s: no lua functions in str:\n%s\n\n", __FUNCTION__, str );
      return NULL;
   }

   func_array = (LUA_FUNCTION_ARRAY)calloc( size + 1, sizeof( LUA_FUNCTION * ) );
   func_array[size] = NULL;

   for( x = 0; x < size; x++ )
      func_array[x] = get_lua_func( &str );

   return func_array;
}

LUA_FUNCTION *get_lua_func( char **str )
{
   typedef enum
   {
      NOISE, HEADER, BODY
   } MODE;

   LUA_FUNCTION *func;
   char *box = *str;
   char line[510], noise[MAX_BUFFER], header[MAX_BUFFER], body[MAX_BUFFER];
   MODE mode = NOISE;

   func = (LUA_FUNCTION *)malloc( sizeof( LUA_FUNCTION ) );
   func->noise = NULL;
   func->header = NULL;
   func->body = NULL;
   func->wrote = FALSE;

   memset( &noise[0], 0, sizeof( noise ) );
   memset( &header[0], 0, sizeof( header ) );
   memset( &body[0], 0, sizeof( body ) );

   while( box && box[0] != '\0' )
   {
      box = one_arg_delim( box, line, '\n' );
      if( line[0] == '\0' )
         continue;
      switch( mode )
      {
         case NOISE:
            if( !until_function( line ) )
            {
               strcat( noise, line );
               strcat( noise, "\n" );
               break;
            }
            mode = HEADER;
         case HEADER:
            strcat( header, line );
            strcat( header, "\n" );
            mode = BODY;
            break;
         case BODY:
            if( until_end( line ) )
            {
               box = one_arg_delim( box, line, '\n' );
               goto exit;
            }
            strcat( body, line );
            strcat( body, "\n" );
            break;
      }
   }
   exit:
   noise[strlen( noise )] = '\0';
   header[strlen( header )] = '\0';
   body[strlen( body )] = '\0';
   func->noise = strdup( noise );
   func->header = strdup( header );
   func->body = strdup( body );
   *str = box;
   return func;

}

void free_lua_func( LUA_FUNCTION *func )
{
   free( func->noise );
   free( func->header );
   free( func->body );
   free( func );
}

void free_lua_func_array( LUA_FUNCTION_ARRAY func_array )
{
   int x;

   for( x = 0; func_array[x] != NULL; x++ )
      free_lua_func( func_array[x] );
   free( func_array );
}

bool until_function( char *str )
{
   if( is_prefix( str, "function" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

bool until_end( char *str )
{
   if( is_prefix( str, "end" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

void free_lua_args( LLIST *list  )
{
   void *content;
   ITERATOR Iter;

   if( SizeOfList( list ) == 0 )
      return;

   AttachIterator( &Iter, list );
   while( ( content = NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( content, list );
      FREE( content );
   }
   DetachIterator( &Iter );
   return;
}

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

inline const char *get_frame_script_path( ENTITY_FRAMEWORK *frame )
{
   return quick_format( "../scripts/frames/%d.lua", frame->tag->id );
}

inline const char *get_instance_script_path( ENTITY_INSTANCE *instance )
{
   return quick_format( "../scripts/instances/%d.lua", instance->tag->id );
}

inline const char *get_stat_framework_script_path( STAT_FRAMEWORK *fstat )
{
   return quick_format( "../scripts/stats/%d.lua", fstat->tag->id );
}

inline const char *get_stat_instance_script_path( STAT_INSTANCE *stat )
{
   return quick_format( "../scripts/stats/%d.lua", stat->framework->tag->id );
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

int lua_bug( lua_State *L )
{
   switch( lua_type( L, -1 ) )
   {
      case LUA_TNUMBER:
         bug( "%s: Number Error %d.", __FUNCTION__, lua_tonumber( L, -1 ) );
         break;
      case LUA_TSTRING:
         bug( "%s: %s.", __FUNCTION__, lua_tostring( L, -1 ) );
         break;
   }
   return 0;
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

bool autowrite_init( ENTITY_INSTANCE *instance )
{
   typedef enum
   {
      FIND_INIT, FIND_END, FINISH
   } MODE;
   ENTITY_FRAMEWORK *frame = instance->framework;
   STAT_INSTANCE *stat;
   FILE *fp;
   char *script;
   ITERATOR Iter;
   char script_buf[MAX_BUFFER * 4], line[MAX_BUFFER];
   MODE mode;


   if( ( fp = open_f_script( frame, "r" ) ) == NULL )
   {
      bug( "%s: could not open the frameworks script.\r\n", __FUNCTION__ );
      return FALSE;
   }

   script = script_buf;
   snprintf( script_buf, ( MAX_BUFFER * 4 ), "%s", fread_file( fp ) );
   fclose( fp );

   if( ( fp = open_f_script( frame, "w" ) ) == NULL )
   {
      bug( "%s: could not open the framework's script to write.\r\n", __FUNCTION__ );
      return FALSE;
   }

   mode = FIND_INIT;
   while( script && script[0] != '\0' )
   {
      script = one_arg_delim_literal( script, line, '\n' );
      if( mode == FIND_INIT  )
      {
         fprintf( fp, "%s\n", line );
         if( !strcmp( line, AUTOWRITE_INIT ) )
         {
            AttachIterator( &Iter, instance->stats );
            while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
               fprintf( fp, "   instance:setStatPerm( \"%s\", %d )\n", stat->framework->name, stat->perm_stat );
            DetachIterator( &Iter );
            mode = FIND_END;
         }
         continue;
      }
      else if( mode == FIND_END )
      {
         if( !strcmp( line, ENDAUTOWRITE_INIT ) )
         {
            fprintf( fp, "%s\n", line );
            mode = FINISH;
            continue;
         }
         else
            continue;
      }
      else if( mode == FINISH )
      {
         fprintf( fp, "%s\n", line );
         fprintf( fp, "%s\n", script );
         break;
      }
   }
   fclose( fp );
   return TRUE;
}
