/* the file containing the methods pertaining to all things instanced written by Davenge */

#include "mud.h"

ENTITY_INSTANCE *init_eInstance( void )
{
   ENTITY_INSTANCE *eInstance;
   int x;

   CREATE( eInstance, ENTITY_INSTANCE, 1 );
   eInstance->commands = AllocList();
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
   eInstance->framework = NULL;
   eInstance->socket = NULL;
   eInstance->account = NULL;
   eInstance->contained_by = NULL;
   eInstance->contained_by_id = -1;
   return RET_SUCCESS;
}

int free_eInstance( ENTITY_INSTANCE *eInstance )
{
   int x;

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
   builder->framework = init_eFramework();

   builder->framework->name = strdup( "Builder" );
   builder->framework->short_descr = strdup( "A builder" );
   builder->framework->long_descr = strdup( "A construct is here building things." );
   builder->framework->description = strdup( "none" );
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

   if( !quick_query( "INSERT INTO entity_instances VALUES( %d, %d, '%s', '%s', '%s', '%s', %d, %d );",
         eInstance->tag->id, eInstance->tag->type, eInstance->tag->created_by,
         eInstance->tag->created_on, eInstance->tag->modified_by, eInstance->tag->modified_on,
         eInstance->contained_by ? eInstance->contained_by->tag->id : 0, eInstance->framework->tag->id ) )
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

   eInstance->contained_by_id = atoi( (*row)[counter++] );
   framework_id = atoi( (*row)[counter++] ); /* don't grab containedBY just yet */

   if( ( eInstance->framework = get_framework_by_id( framework_id ) ) == NULL )
      bug( "%s: instance has a NULL framework: ID %d", __FUNCTION__, eInstance->tag->id );

   return;
}

void entity_from_container( ENTITY_INSTANCE *entity )
{
   if( entity->contained_by )
   {
      DetachFromList( entity, entity->contained_by->contents );
      entity->contained_by_id = 0;
   }
   if( !quick_query( "UPDATE `entity_instances` SET containedBy='%d' WHERE entityInstanceId=%d;", entity->contained_by_id, entity->tag->id ) )
      bug( "%s: could not update databaes with %d's new location in the world.", __FUNCTION__, entity->tag->id );
   return;

}

void entity_to_world( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   if( !entity )
      return;

   if( entity->contained_by )
      entity_from_container( entity );

   if( !container )
   {
      entity->contained_by = NULL;
      entity->contained_by_id = 0;
   }
   else
   {
      AttachToList( entity, container->contents );
      entity->contained_by = container;
      entity->contained_by_id = container->tag->id;
   }
   if( !quick_query( "UPDATE `entity_instances` SET containedBy='%d' WHERE entityInstanceId=%d;", entity->contained_by_id, entity->tag->id ) )
      bug( "%s: could not update databaes with %d's new location in the world.", __FUNCTION__, entity->tag->id );
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
   return instance->framework->name;
}
const char *instance_short_descr( ENTITY_INSTANCE *instance )
{
   return instance->framework->short_descr;
}
const char *instance_long_descr( ENTITY_INSTANCE *instance )
{
   return instance->framework->long_descr;
}
const char *instance_description( ENTITY_INSTANCE *instance )
{
   return instance->framework->description;
}

int text_to_entity( ENTITY_INSTANCE *entity, const char *fmt, ... )
{
   va_list va;
   int res;
   char dest[MAX_BUFFER];

   va_start( va, fmt );
   res = vsnprintf( dest, MAX_BUFFER, fmt, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when attempting to format string for message." );
   }

   text_to_buffer( entity->socket, dest );
   return res;
}

int builder_prompt( D_SOCKET *dsock )
{
   int ret = RET_SUCCESS;

   if( !dsock->controlling )
      bug( "%s: socket is controlling nothing...", __FUNCTION__ );

   /* ugly but works for now */

   if( dsock->controlling->contained_by )
      text_to_entity( dsock->controlling, "Builder Mode:(%d)> ", dsock->controlling->contained_by->tag->id );
   else
      text_to_buffer( dsock, "Builder Mode:> " );

   return ret;
}

int show_ent_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
{
   int ret = RET_SUCCESS;

   if( !entity )
   {
      BAD_POINTER( "entity" );
      return ret;
   }
   if( !viewing )
   {
      BAD_POINTER( "viewing" );
      return ret;
   }

   text_to_entity( entity, "%s\r\n", instance_short_descr( viewing ) );
   text_to_entity( entity, "%s\r\n", print_bar( "-", entity->socket->account ? entity->socket->account->pagewidth : 80 ) );
   text_to_entity( entity, "%s\r\n", instance_description( viewing ) );
   text_to_entity( entity, "%s\r\n", print_bar( "-", entity->socket->account ? entity->socket->account->pagewidth : 80 ) );

   return ret;
}

void entity_goto( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_INSTANCE *ent_to_goto;
   char buf[MAX_BUFFER];

   arg = one_arg( arg, buf );

   if( check_selection_type( buf ) != SEL_INSTANCE )
   {
      if( is_number( buf ) )
         ent_to_goto = get_instance_by_id( atoi( buf ) );
      else
         ent_to_goto = get_instance_by_name( buf );
   }
   else
   {
      if( interpret_entity_selection( buf ) )
         ent_to_goto = (ENTITY_INSTANCE *)retrieve_entity_selection();
   }

   if( !ent_to_goto )
   {
      text_to_entity( entity, "No such instance exists.\r\n" );
      return;
   }
   text_to_entity( entity, "You wisk away to the desired instance.\r\n" );
   entity_to_world( entity, ent_to_goto );
   show_ent_to_ent( entity, entity->contained_by );

   return;
}

void entity_instance( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_FRAMEWORK *frame_to_instance;
   ENTITY_INSTANCE *ent_to_instance;
   ENTITY_INSTANCE *new_ent;
   char buf[MAX_BUFFER];

   arg = one_arg( arg, buf );

   if( !interpret_entity_selection( buf ) )
   {
      text_to_entity( entity, "There is a problem with the input selection pointer, please contact the nearest Admin or try again in a few seconds.\r\n" );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         text_to_entity( entity, "There's been a major problem. Contact your nearest admin.\r\n" );
         break;
      case SEL_FRAME:
         frame_to_instance = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         break;
      case SEL_INSTANCE:
         ent_to_instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
         frame_to_instance = ent_to_instance->framework;
         break;
      case SEL_STRING:
         text_to_entity( entity, (char *)retrieve_entity_selection() );
         return;
   }

   if( ( new_ent = eInstantiate( frame_to_instance ) ) == NULL )
   {
      text_to_entity( entity, "There's been a major problem, framework you are trying to instantiate from may not be live.\r\n" );
      return;
   }
   if( new_eInstance( new_ent ) != RET_SUCCESS )
   {
      free_eInstance( new_ent );
      text_to_entity( entity, "Could not add new instance to the database, deleting it from live memory.\r\n" );
      return;
   }

   AttachToList( new_ent, eInstances_list );
   entity_to_world( new_ent, entity->contained_by ? entity->contained_by : NULL );
   text_to_entity( entity, "You create a new instance of %s.\r\n", instance_name( new_ent ) );
   return;
}

void entity_look( void *passed, char *arg )
{
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)passed;
   show_ent_to_ent( instance, instance->contained_by );
   return;
}
