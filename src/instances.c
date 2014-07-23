/* the file containing the methods pertaining to all things instanced written by Davenge */

#include "mud.h"

ENTITY_INSTANCE *init_eInstance( void )
{
   ENTITY_INSTANCE *eInstance;

   CREATE( eInstance, ENTITY_INSTANCE, 1 );
   eInstance->contents = AllocList();
   eInstance->specifications = AllocList();
   eInstance->tag = init_tag();
   eInstance->tag->type = ENTITY_INSTANCE_IDS;
   if( clear_eInstance( eInstance ) != RET_SUCCESS )
   {
      free_eInstance( eInstance );
      return NULL;
   }
   return eInstance;
}

int clear_eInstance( ENTITY_INSTANCE *eInstance )
{
   eInstance->name = NULL;
   eInstance->short_descr = NULL;
   eInstance->long_descr = NULL;
   eInstance->description = NULL;
   eInstance->framework = NULL;
   eInstance->socket = NULL;
   eInstance->account = NULL;
   eInstance->contained_by = NULL;
   return RET_SUCCESS;
}

int free_eInstance( ENTITY_INSTANCE *eInstance )
{
   FREE( eInstance->name );
   FREE( eInstance->short_descr );
   FREE( eInstance->long_descr );
   FREE( eInstance->description );
   eInstance->framework = NULL;

   CLEARLIST( eInstance->contents, ENTITY_INSTANCE );
   FreeList( eInstance->contents );
   eInstance->contents = NULL;

   specification_clear_list( eInstance->specifications );
   FreeList( eInstance->specifications );
   eInstance->specifications = NULL;

   eInstance->socket = NULL;
   eInstance->contained_by = NULL;
   eInstance->account = NULL;

   FREE( eInstance );
   return RET_SUCCESS;
}

ENTITY_INSTANCE *init_builder( void )
{
   ENTITY_INSTANCE *builder;

   builder = init_eInstance();
   builder->name = strdup( "Builder" );
   builder->short_descr = strdup( "A builder" );
   builder->long_descr = strdup( "A construct is here building things." );
   builder->description = strdup( "none" );
   builder->live = TRUE;
   builder->builder = TRUE;
   return builder;
}

ENTITY_INSTANCE *load_eInstance_by_query( const char *query )
{
   ENTITY_INSTANCE *instance = NULL;
   MYSQL_ROW row;

   if( !db_query_single_row( &row, query ) )
      return NULL;

   if( ( instance = init_eInstance() ) == NULL )
      return NULL;

   db_load_eInstance( instance, &row );
   load_specifications_to_list( instance->specifications, quick_format( "%d", instance->tag->id ) );

   return instance;
}

ENTITY_INSTANCE *get_instance_by_id( int id )
{
   ENTITY_INSTANCE *eInstance;

   if( ( eInstance = get_active_instance_by_id( id ) ) == NULL )
      if( ( eInstance = load_eInstance_by_id( id ) ) != NULL )
         AttachToList( eInstance, eInstances_list );

   return eInstance;
}

ENTITY_INSTANCE *get_active_instance_by_id( int id )
{
   return instance_list_has_by_id( eInstances_list, id );
}

ENTITY_INSTANCE *load_eInstance_by_id( int id )
{
   return load_eInstance_by_query( quick_format( "SELECT * FROM `%s` WHERE %s=%d;", tag_table_strings[ENTITY_INSTANCE_IDS], tag_table_whereID[ENTITY_INSTANCE_IDS], id ) );
}

ENTITY_INSTANCE *get_instance_by_name( const char *name )
{
   ENTITY_INSTANCE *eInstance;

   if( ( eInstance = get_active_instance_by_name( name ) ) == NULL )
      if( ( eInstance = load_eInstance_by_name( name ) ) != NULL )
         AttachToList( eInstance, eInstances_list );

   return eInstance;
}

ENTITY_INSTANCE *get_active_instance_by_name( const char *name )
{
   return instance_list_has_by_name( eInstances_list, name );
}

ENTITY_INSTANCE *load_eInstance_by_name( const char *name )
{
   ENTITY_INSTANCE *instance;
   ENTITY_FRAMEWORK *frame;

   if( ( instance = load_eInstance_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[ENTITY_INSTANCE_IDS], name ) ) ) == NULL )
   {
      if( ( frame = get_framework_by_name( name ) ) != NULL )
         instance = load_eInstance_by_query( quick_format( "SELECT * FROM '%s' WHERE frameworkID=%d LIMIT 1;", tag_table_strings[ENTITY_INSTANCE_IDS], frame->tag->id ) );
   }
   return instance;
}

int new_eInstance( ENTITY_INSTANCE *eInstance )
{
   SPECIFICATION *spec;
   ITERATOR Iter;
   int ret = RET_SUCCESS;

   if( !eInstance )
   {
      BAD_POINTER( "eInstance" );
      return ret;
   }

   if( !strcmp( eInstance->tag->created_by, "null" ) )
   {
      if( ( ret = new_tag( eInstance->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return RET_FAILED_OTHER;
      }
   }

   if( !quick_query( "INSERT INTO entity_instances VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d );",
         eInstance->tag->id, eInstance->tag->type, eInstance->tag->created_by,
         eInstance->tag->created_on, eInstance->tag->modified_by, eInstance->tag->modified_on,
         eInstance->name, eInstance->short_descr, eInstance->long_descr,
         eInstance->description, eInstance->framework->tag->id ) )
      return RET_FAILED_OTHER;

   AttachIterator( &Iter, eInstance->specifications );
   while( ( spec = (SPECIFICATION *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( spec->owner, "%d", eInstance->tag->id );
      new_specification( spec );
   }
   DetachIterator( &Iter );

   return ret;
}

void db_load_eInstance( ENTITY_INSTANCE *eInstance, MYSQL_ROW *row )
{
   char wtf_buf[MAX_BUFFER];
   int framework_id;
   int counter;

   counter = db_load_tag( eInstance->tag, row );

   if( !strcmp( (*row)[counter++], "(null)" ) )
      eInstance->name = NULL;
   else
      eInstance->name = strdup( (*row)[counter-1] );

   if( !strcmp( (*row)[counter++], "(null)" ) )
      eInstance->short_descr = NULL;
   else
      eInstance->short_descr = strdup( (*row)[counter-1] );

   if( !strcmp( (*row)[counter++], "(null)" ) )
      eInstance->long_descr = NULL;
   else
      eInstance->long_descr = strdup( (*row)[counter-1] );

   if( !strcmp( (*row)[counter++], "(null)" ) )
      eInstance->description = NULL;
   else
      eInstance->description = strdup( (*row)[counter-1] );

   strcpy( wtf_buf, (*row)[counter++] );
   framework_id = atoi( wtf_buf );

   if( ( eInstance->framework = get_framework_by_id( framework_id ) ) == NULL )
      bug( "%s: instance has a NULL framework: ID %d", __FUNCTION__, eInstance->tag->id );

   return;
}

ENTITY_INSTANCE *instance_list_has_by_id( LLIST *instance_list, int id )
{
   ENTITY_INSTANCE *eInstance;
   ITERATOR Iter;

   if( !instance_list )
      return NULL;
   if( SizeOfList( instance_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, instance_list );
   while( ( eInstance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( eInstance->tag->id == id )
         break;
   DetachIterator( &Iter );

   return eInstance;
}

ENTITY_INSTANCE *instance_list_has_by_name( LLIST *instance_list, const char *name )
{
   ENTITY_INSTANCE *eInstance;
   ITERATOR Iter;

   if( !instance_list )
      return NULL;
   if( SizeOfList( instance_list ) < 1 )
      return NULL;
   if( !name || name[0] == '\0' )
      return NULL;

   AttachIterator( &Iter, instance_list );
   while( ( eInstance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( name, instance_name( eInstance ) ) )
         break;
   DetachIterator( &Iter );

   return eInstance;
}


ENTITY_INSTANCE *eInstantiate( ENTITY_FRAMEWORK *frame )
{
   ENTITY_INSTANCE *eInstance;

   if( !live_frame( frame ) )
      return NULL;

   eInstance = init_eInstance();
   eInstance->framework = frame;
   return eInstance;
}

const char *instance_name( ENTITY_INSTANCE *instance )
{
   return instance->name ? instance->name : instance->framework->name;
}
const char *instance_short_descr( ENTITY_INSTANCE *instance )
{
   return instance->short_descr ? instance->short_descr : instance->framework->short_descr;
}
const char *instance_long_descr( ENTITY_INSTANCE *instance )
{
   return instance->long_descr ? instance->long_descr : instance->framework->long_descr;
}
const char *instance_description( ENTITY_INSTANCE *instance )
{
   return instance->description ? instance->description : instance->framework->description;
}

int builder_prompt( D_SOCKET *dsock )
{
   int ret = RET_SUCCESS;

   return ret;
}
