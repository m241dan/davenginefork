/* the file containing the methods pertaining to all things instanced written by Davenge */

#include "mud.h"

ENTITY_INSTANCE *init_eInstance( void )
{
   ENTITY_INSTANCE *eInstance;
   int x;

   CREATE( eInstance, ENTITY_INSTANCE, 1 );
   eInstance->contents = AllocList();
   for( x = 0; x < MAX_QUICK_SORT; x++ )
     eInstance->contents_sorted[x] = AllocList();
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
   return RET_SUCCESS;
}

int free_eInstance( ENTITY_INSTANCE *eInstance )
{
   int x;

   FREE( eInstance->name );
   FREE( eInstance->short_descr );
   FREE( eInstance->long_descr );
   FREE( eInstance->description );
   eInstance->framework = NULL;

   CLEARLIST( eInstance->contents, ENTITY_INSTANCE );
   FreeList( eInstance->contents );
   eInstance->contents = NULL;

   for( x = 0; x < MAX_QUICK_SORT; x++ )
   {
      CLEARLIST( eInstance->contents_sorted[x], ENTITY_INSTANCE );
      FreeList( eInstance->contents_sorted[x] );
      eInstance->contents_sorted[x] = NULL;
   }

   specification_clear_list( eInstance->specifications );
   FreeList( eInstance->specifications );
   eInstance->specifications = NULL;

   return RET_SUCCESS;
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
   return load_eInstance_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[ENTITY_INSTANCE_IDS], name ) );
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
   int framework_id;
   int counter;

   counter = db_load_tag( eInstance->tag, row );

   if( (*row[counter++]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[counter] );

   if( (*row[counter++]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[counter] );

   if( (*row[counter++]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[counter] );

   if( (*row[counter++]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[counter] );

   framework_id = atoi( (*row)[counter++] );
   if( ( eInstance->framework = get_active_framework_by_id( framework_id ) ) == NULL )
   {
      if( ( eInstance->framework = load_eFramework_by_id( framework_id ) ) == NULL )
         bug( "%s: instance has a NULL framework: ID %d", __FUNCTION__, eInstance->tag->id );
      else
         AttachToList( eInstance->framework, active_frameworks );
   }

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
      if( !strcasecmp( name, eInstance->name ) )
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
