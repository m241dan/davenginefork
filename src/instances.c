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

   return RET_SUCCESS;
}

ENTITY_INSTANCE *get_instance_by_id( int id )
{
   ENTITY_INSTANCE *eInstance;
   ITERATOR Iter;

   if( SizeOfList( eInstances_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, eInstances_list );
   while( ( eInstance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( eInstance->tag->id == id )
         break;
   DetachIterator( &Iter );

   return eInstance;
}

ENTITY_INSTANCE *load_eInstance_by_id( int id )
{
   ENTITY_INSTANCE *eInstance;
   MYSQL_RES *result;
   MYSQL_ROW row;

   if( !quick_query( "SELECT * FROM entity_instances WHERE entityInstanceID=%d;", id ) )
      return NULL;
   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
      return NULL;
   if( mysql_num_rows( result ) == 0 )
      return NULL;
   if( ( row = mysql_fetch_row( result ) ) == NULL )
   {
      mysql_free_result( result );
      return NULL;
   }
   if( ( eInstance = init_eInstance() ) == NULL )
      return NULL;

   db_load_eInstance( eInstance, &row );
   mysql_free_result( result );
   load_specifications_to_list( eInstance->specifications, quick_format( "%d", id ) );
   return eInstance;
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

   eInstance->tag->id = atoi( (*row)[0] );
   eInstance->tag->type = atoi( (*row)[1] );
   eInstance->tag->created_by = strdup( (*row)[2] );
   eInstance->tag->created_on = strdup( (*row)[3] );
   eInstance->tag->modified_by = strdup( (*row)[4] );
   eInstance->tag->modified_on = strdup( (*row)[5] );

   if( (*row[6]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[6] );

   if( (*row[7]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[7] );

   if( (*row[8]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[8] );

   if( (*row[9]) == NULL )
     eInstance->name = NULL;
   else
     eInstance->name = strdup( (*row)[9] );

   framework_id = atoi( (*row)[10] );
   if( ( eInstance->framework = get_active_framework( framework_id ) ) == NULL )
   {
      if( ( eInstance->framework = load_eFramework( framework_id ) ) == NULL )
         bug( "%s: instance has a NULL framework: ID %d", __FUNCTION__, eInstance->tag->id );
      else
         AttachToList( eInstance->framework, active_frameworks );
   }

   return;
}

ENTITY_INSTANCE *eInstance_list_has_by_id( LLIST *instance_list, int id )
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

ENTITY_INSTANCE *eInstace_list_has_by_name( LLIST *instance_list, const char *name )
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


