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
   return RET_SUCCESS;
}

int free_eInstance( ENTITY_INSTANCE *eInstance )
{
   int x;

   eInstance->framework = NULL;

   for( x = 0; x < MAX_QUICK_SORT; x++ )
   {
      CLEARLIST( eInstance->contents_sorted[x], ENTITY_INSTANCE );
      FreeList( eInstance->contents_sorted[x] );
      eInstance->contents_sorted[x] = NULL;
   }

   clear_ent_contents( eInstance );
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

int clear_ent_contents( ENTITY_INSTANCE *eInstance )
{
   ENTITY_INSTANCE *to_free;
   ITERATOR Iter;

   AttachIterator( &Iter, eInstance->contents );
   while( ( to_free = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      free_eInstance( to_free );
   DetachIterator( &Iter );

   return RET_SUCCESS;
}

ENTITY_INSTANCE *init_builder( void )
{
   ENTITY_INSTANCE *builder;

   builder = init_eInstance();
   builder->tag->id = -69;
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

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( instance = init_eInstance() ) == NULL )
      return NULL;

   db_load_eInstance( instance, &row );
   load_specifications_to_list( instance->specifications, quick_format( "%d", instance->tag->id ) );
   free( row );

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
   ENTITY_INSTANCE *instance = NULL;
   ENTITY_FRAMEWORK *frame;

   if( ( frame = get_framework_by_name( name ) ) != NULL )
      instance = load_eInstance_by_query( quick_format( "SELECT * FROM '%s' WHERE frameworkID=%d LIMIT 1;", tag_table_strings[ENTITY_INSTANCE_IDS], frame->tag->id ) );
   return instance;
}

ENTITY_INSTANCE *full_load_eFramework( ENTITY_FRAMEWORK *frame )
{
   ENTITY_INSTANCE *instance;
   instance = eInstantiate( frame );
   new_eInstance( instance );
   full_load_instance( instance );
   return instance;
}

void full_load_workspace( WORKSPACE *wSpace )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   AttachIterator( &Iter, wSpace->instances );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      full_load_instance( instance );
   DetachIterator( &Iter );

   return;
}

/* could be factored */
void full_load_instance( ENTITY_INSTANCE *instance )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance_to_contain;
   MYSQL_ROW row;
   LLIST *list;
   ITERATOR Iter;
   int id_to_load;

   if( !instance->loaded )
   {
      set_to_loaded( instance );
      if( SizeOfList( instance->framework->fixed_contents ) > 0 )
      {
         AttachIterator( &Iter, instance->framework->fixed_contents );
         while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
         {
            instance_to_contain = full_load_eFramework( frame );
            entity_to_world( instance_to_contain, instance );
         }
         DetachIterator( &Iter );
      }
      return;
   }

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT content_instanceID FROM `entity_instance_possessions` WHERE %s=%d;", tag_table_whereID[ENTITY_INSTANCE_IDS], instance->tag->id ) ) )
   {
      FreeList( list );
      bug( "%s: could not load contents, bad list.", __FUNCTION__ );
      return;
   }
   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      id_to_load = atoi( (row)[0] );
      if( ( instance_to_contain = get_instance_by_id( id_to_load ) ) == NULL )
      {
         bug( "%s: trying to load an instance that doesnt exist: ID %d", __FUNCTION__, id_to_load );
         continue;
      }
      full_load_instance( instance_to_contain );
      if( !instance_list_has_by_id( instance->contents, id_to_load ) )
         entity_to_contents( instance_to_contain, instance );
   }
   DetachIterator( &Iter );
   FreeList( list );
   return;
}

void set_to_loaded( ENTITY_INSTANCE *instance )
{
   instance->loaded = TRUE;
   if( !quick_query( "UPDATE `entity_instances` SET loaded=1 WHERE %s=%d;", tag_table_whereID[ENTITY_INSTANCE_IDS], instance->tag->id ) )
      bug( "%s: could not set entity to loaded: ID %d", __FUNCTION__, instance->tag->id );
   return;
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

   if( !quick_query( "INSERT INTO entity_instances VALUES( %d, %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d );",
         eInstance->tag->id, eInstance->tag->type, eInstance->tag->created_by,
         eInstance->tag->created_on, eInstance->tag->modified_by, eInstance->tag->modified_on,
         eInstance->contained_by ? eInstance->contained_by->tag->id : -1, eInstance->framework->tag->id,
         (int)eInstance->live, (int)eInstance->loaded ) )
      return RET_FAILED_OTHER;

   AttachIterator( &Iter, eInstance->specifications );
   while( ( spec = (SPECIFICATION *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( spec->owner, "%d", eInstance->tag->id );
      new_specification( spec );
   }
   DetachIterator( &Iter );

   AttachToList( eInstance, eInstances_list );
   return ret;
}

void db_load_eInstance( ENTITY_INSTANCE *eInstance, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( eInstance->tag, row );

   eInstance->contained_by = get_instance_by_id( atoi( (*row)[counter++] ) );
   eInstance->framework = get_framework_by_id( atoi( (*row)[counter++] ) );
   eInstance->live = atoi( (*row)[counter++] );
   eInstance->loaded = atoi( (*row)[counter++] );

   return;
}

void entity_from_container( ENTITY_INSTANCE *entity )
{
   if( entity->contained_by )
   {
      if( !quick_query( "UPDATE `entity_instances` SET containedBy=-1 WHERE entityInstanceId=%d;", entity->tag->id ) )
         bug( "%s: could not update entity's containedBy variable for %d.", __FUNCTION__, entity->tag->id );
      if( !entity->builder )
         if( !quick_query( "DELETE FROM `entity_instance_possessions` WHERE entityInstanceID=%d AND content_instanceID=%d;", entity->contained_by->tag->id, entity->tag->id ) )
            bug( "%s: could not delete from database possession entry for %d.", __FUNCTION__, entity->contained_by->tag->id );
      detach_entity_from_contents( entity, entity->contained_by );
   }
   return;

}

void entity_to_world( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   if( !entity )
      return;

   if( entity->contained_by )
      entity_from_container( entity );

   entity_to_contents( entity, container );
   return;
}

void entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   if( !container )
   {
      entity->contained_by = NULL;
      return;
   }
   attach_entity_to_contents( entity, container );
   if( !entity->builder )
      if( !quick_query( "INSERT INTO `entity_instance_possessions` VALUES ( %d, %d );", entity->contained_by->tag->id, entity->tag->id ) )
         bug( "%s: could not insert into database with %d's new location in the world.", __FUNCTION__, entity->tag->id );
   return;
}

void attach_entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   AttachToList( entity, container->contents );
   entity_to_contents_quick_sort( entity, container );
   entity->contained_by = container;
}

void detach_entity_from_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   DetachFromList( entity, container->contents );
   entity_from_contents_quick_sort( entity, container );
   entity->contained_by = NULL;
}

void entity_to_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   int x;

   for( x = 0; x < MAX_QUICK_SORT; x++ )
      if( has_spec( entity, spec_table[x] ) )
         AttachToList( entity, container->contents_sorted[x] );

   return;
}

void entity_from_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container )
{
   int x;

   for( x = 0; x < MAX_QUICK_SORT; x++ )
      if( has_spec( entity, spec_table[x] ) )
         DetachFromList( entity, container->contents_sorted[x] );

   return;
}

ENTITY_INSTANCE *copy_instance( ENTITY_INSTANCE *instance, bool copy_id, bool copy_contents, bool copy_specs, bool copy_frame )
{
   ENTITY_INSTANCE *instance_copy;
   int x;

   if( !instance )
   {
      bug( "%s: passed a NULL instance.", __FUNCTION__ );
      return NULL;
   }

   CREATE( instance_copy, ENTITY_INSTANCE, 1 );
   instance_copy->level = instance->level;
   for( x = 0; x < MAX_QUICK_SORT; x++ )
     instance_copy->contents_sorted[x] = AllocList();


   if( copy_id )
      instance_copy->tag = copy_tag( instance->tag );
   else
      instance_copy->tag = init_tag();

   if( copy_contents )
      instance_copy->contents = copy_instance_list( instance->contents, TRUE, TRUE, TRUE, TRUE );
   else
      instance_copy->contents = AllocList();

   if( copy_specs )
      instance_copy->specifications = copy_specification_list( instance->specifications, TRUE );
   else
      instance_copy->specifications = AllocList();

   if( copy_frame )
      instance_copy->framework = copy_framework( instance->framework, TRUE, TRUE, TRUE, TRUE );
   else
      instance_copy->framework = instance->framework;

   return instance_copy;
}

LLIST *copy_instance_list( LLIST *instances, bool copy_id, bool copy_contents, bool copy_specs, bool copy_frame )
{
   LLIST *list;

   if( !instances )
   {
      bug( "%s: passed a NULL instances.", __FUNCTION__ );
      return NULL;
   }

   list = AllocList();
   copy_instances_into_list( instances, list, copy_id, copy_contents, copy_specs, copy_frame );

   return list;
}
void copy_instances_into_list( LLIST *instance_list, LLIST *copy_into_list, bool copy_id, bool copy_contents, bool copy_specs, bool copy_frame )
{
   ENTITY_INSTANCE *instance, *instance_copy;
   ITERATOR Iter;

   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }

   if( !copy_into_list )
   {
      bug( "%s: passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( copy_id || copy_contents || copy_specs || copy_frame )
      {
         instance_copy = copy_instance( instance, copy_id, copy_contents, copy_specs, copy_frame );
         AttachToList( instance_copy, copy_into_list );
         continue;
      }
      AttachToList( instance, copy_into_list );
   }
   DetachIterator( &Iter );
   return;
}

ENTITY_INSTANCE *copy_instance_ndi( ENTITY_INSTANCE *instance, LLIST *instance_list ) /* no duplicate ids */
{
   ENTITY_INSTANCE *instance_copy, *instance_content, *instance_content_copy;
   ITERATOR Iter;
   int x;

   if( !instance )
   {
      bug( "%s: passed a NULL instance.", __FUNCTION__ );
      return NULL;
   }

   if( ( instance_copy = instance_list_has_by_id( instance_list, instance->tag->id ) ) != NULL )
      return instance_copy;

   CREATE( instance_copy, ENTITY_INSTANCE, 1 );
   instance_copy->contents = AllocList();
   for( x = 0; x < MAX_QUICK_SORT; x++ )
     instance_copy->contents_sorted[x] = AllocList();
   instance_copy->specifications = AllocList();

   instance_copy->tag = copy_tag( instance->tag );
   instance_copy->level = instance->level;

   AttachIterator( &Iter, instance->contents ); /* I hate having to iterate in this function but it's so specific  */
   while( ( instance_content = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      instance_content_copy = copy_instance_ndi( instance_content, instance_list );
      AttachToList( instance_content_copy, instance_copy->contents );
   }
   DetachIterator( &Iter );

   copy_instance_list_ndi( instance->contents, instance_copy->contents );
   copy_specifications_into_list( instance->specifications, instance_copy->specifications, TRUE );
   instance_copy->framework = instance->framework;
   AttachToList( instance_copy, instance_list );

   return instance_copy;
}

void copy_instance_list_ndi( LLIST *instance_list, LLIST *copy_into_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }
   if( !copy_into_list )
   {
      bug( "%s: passed a NULL copy_into_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      copy_instance_ndi( instance, copy_into_list );
   DetachIterator( &Iter );

   return;
}

void append_instance_lists_ndi( LLIST *instance_list, LLIST *append_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   if( !instance_list )
   {
      bug( "%s: passed NULL instance_list.", __FUNCTION__ );
      return;
   }
   if( !append_list )
   {
      bug( "%s: passed a NULL append_list,", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( instance_list_has_by_id( append_list, instance->tag->id ) )
         continue;
      AttachToList( instance, append_list );
   }
   DetachIterator( &Iter );

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

   if( !name || name[0] == '\0' )
      return NULL;
   if( !instance_list || SizeOfList( instance_list ) < 1 )
      return NULL;

   AttachIterator( &Iter, instance_list );
   while( ( eInstance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( !strcasecmp( name, instance_name( eInstance ) ) )
         break;
   DetachIterator( &Iter );

   return eInstance;
}

ENTITY_INSTANCE *instance_list_has_by_short_prefix( LLIST *instance_list, const char *name )
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
      if( is_prefix( name, instance_short_descr( eInstance ) ) )
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

ENTITY_INSTANCE *create_room_instance( const char *name )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   frame = create_room_framework( name );
   instance = eInstantiate( frame );
   new_eInstance( instance );

   return instance;

}

ENTITY_INSTANCE *create_exit_instance( const char *name, int dir )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   SPECIFICATION *spec;

   frame = create_exit_framework( name, 0 );
   instance = eInstantiate( frame );

   if( dir > 0 )
   {
      spec = init_specification();
      spec->type = SPEC_ISEXIT;
      spec->value = dir;
      add_spec_to_instance( spec, instance );
   }
   new_eInstance( instance );

   return instance;
}

ENTITY_INSTANCE *create_mobile_instance( const char *name  )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   frame = create_mobile_framework( name );
   instance = eInstantiate( frame );
   new_eInstance( instance );

   return instance;

}

/* factor me PLEASE */
void move_create( ENTITY_INSTANCE *entity, ENTITY_FRAMEWORK *exit_frame, char *arg )
{
   ENTITY_FRAMEWORK *room_frame;
   ENTITY_INSTANCE *room_instance;
   ENTITY_INSTANCE *exit_instance;
   ENTITY_INSTANCE *mirrored_exit_instance;
   SPECIFICATION *spec;
   char buf[MAX_BUFFER];
   int mirror_id;
   int count = 0;
   bool mirror = TRUE;
   bool new_room = FALSE;
   bool have_frame = FALSE;

   while( arg[0] != '\0' )
   {
      count++;
      arg = one_arg( arg, buf );
      if( !strcasecmp( buf, "nomirror" ) )
      {
         mirror = FALSE;
         continue;
      }

      if( !interpret_entity_selection( buf ) )
      {
         text_to_entity( entity, STD_SELECTION_ERRMSG_PTR_USED );
         return;
      }

      switch( input_selection_typing )
      {
         default: clear_entity_selection(); break;
         case SEL_FRAME:
            have_frame = TRUE;
            room_frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            break;
         case SEL_INSTANCE:
            have_frame = TRUE;
            new_room = TRUE;
            room_instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
            break;
         case SEL_STRING:
            text_to_entity( entity, (char *)retrieve_entity_selection() );
            break;
      }
      if( count == 2 )
         break;
   }

   /* create room */
   if( !have_frame )
   {
      room_frame = init_eFramework();
      new_eFramework( room_frame );
   }

   if( !new_room )
   {
      room_instance = eInstantiate( room_frame );
      new_eInstance( room_instance );
   }
   entity_to_world( room_instance, NULL );
   /* create exit */
   exit_instance = eInstantiate( exit_frame );
   new_eInstance( exit_instance );

   spec = init_specification();
   spec->type = SPEC_ISEXIT;
   spec->value = room_instance->tag->id;
   mud_printf( spec->owner, "%d", exit_instance->tag->id );
   new_specification( spec );
   add_spec_to_instance( spec, exit_instance );

   entity_to_world( exit_instance, entity->contained_by );
   text_to_entity( entity, "You create a new room(%d) to the %s.\r\n", room_instance->tag->id, instance_name( exit_instance ) );

   /* create mirrored exit */
   if( mirror )
   {
      if( ( mirror_id = get_spec_value( exit_instance, "MirrorExit" ) ) == -1 )
      {
         text_to_entity( entity, "That exit has no mirror value...\r\n" );
         return;
      }

      mirrored_exit_instance = eInstantiate( get_framework_by_id( mirror_id ) );
      new_eInstance( mirrored_exit_instance );

      spec = init_specification();
      spec->type = SPEC_ISEXIT;
      spec->value = entity->contained_by->tag->id;
      mud_printf( spec->owner, "%d", mirrored_exit_instance->tag->id );
      new_specification( spec );
      add_spec_to_instance( spec, mirrored_exit_instance );

      entity_to_world( mirrored_exit_instance, room_instance );
   }
   text_to_entity( entity, "\r\n\r\n" );
   move_entity( entity, exit_instance );
   return;
}

bool should_move_create( ENTITY_INSTANCE *entity, char *arg )
{
   ENTITY_INSTANCE *exit;

   if( !entity->contained_by )
   {
      text_to_entity( entity, "You are float around meaninglessly in a world of nothingness.\r\n" );
      return FALSE;
   }

   if( ( exit = instance_list_has_by_short_prefix( entity->contained_by->contents_sorted[SPEC_ISEXIT], arg ) ) != NULL )
   {
      move_entity( entity, exit );
      return FALSE;
   }
   return TRUE;
}

const char *instance_name( ENTITY_INSTANCE *instance )
{
   return instance->framework ? chase_name( instance->framework ) : "null";
}

const char *instance_short_descr( ENTITY_INSTANCE *instance )
{
   return instance->framework ? chase_short_descr( instance->framework ) : "null";
}

const char *instance_long_descr( ENTITY_INSTANCE *instance )
{
   return instance->framework ? chase_long_descr( instance->framework ) : "null";
}

const char *instance_description( ENTITY_INSTANCE *instance )
{
   return instance->framework ? chase_description( instance->framework ) : "null";
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
   INCEPTION *olc;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   int ret = RET_SUCCESS;

   if( !dsock->controlling )
      bug( "%s: socket is controlling nothing...", __FUNCTION__ );

   olc = dsock->controlling->account->olc;
   if( olc->using_workspace )
      bprintf( buf, "UW: \"%s\" :\r\n", olc->using_workspace->name );

   if( dsock->controlling->contained_by )
      bprintf( buf, "Location:(%d):> ", dsock->controlling->contained_by->tag->id );
   else
      bprintf( buf, "Location: \"TheEther\" :> " );

   text_to_buffer( dsock, buf->data );
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

   if( entity->builder )
      text_to_entity( entity, "%s - ID:%d\r\n", instance_short_descr( viewing ), viewing->tag->id );
   else
      text_to_entity( entity, "%s\r\n", instance_short_descr( viewing ) );
   text_to_entity( entity, "%s\r\n", print_bar( "-", entity->socket->account ? entity->socket->account->pagewidth : 80 ) );
   text_to_entity( entity, "%s\r\n", instance_description( viewing ) );
   text_to_entity( entity, "%s\r\n", print_bar( "-", entity->socket->account ? entity->socket->account->pagewidth : 80 ) );

   return ret;
}

int show_ent_contents_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
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

   if( SizeOfList( viewing->contents_sorted[SPEC_ISROOM] ) > 0 )
      show_ent_rooms_to_ent( entity, viewing );
   if( SizeOfList( viewing->contents_sorted[SPEC_ISEXIT] ) > 0 )
      show_ent_exits_to_ent( entity, viewing );
   else if( has_spec( viewing, "IsRoom" ) )
   {
      text_to_entity( entity, "Exits:\r\n" );
      text_to_entity( entity, "  None\r\n\r\n" );
   }
   if( SizeOfList( viewing->contents_sorted[SPEC_ISMOB] ) > 0 )
      show_ent_mobiles_to_ent( entity, viewing );
   if( SizeOfList( viewing->contents_sorted[SPEC_ISOBJECT] ) > 0 )
      show_ent_objects_to_ent( entity, viewing );
   return ret;
}

int show_ent_exits_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
{
   ENTITY_INSTANCE *exit, *exit_to;
   ITERATOR Iter;
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

   text_to_entity( entity, "Exits:\r\n" );

   AttachIterator( &Iter, viewing->contents_sorted[SPEC_ISEXIT] );
   while( ( exit = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( entity == exit )
         continue;
      exit_to = get_active_instance_by_id( get_spec_value( exit, "IsExit" ) );
      if( entity == exit || entity == exit_to )
         continue;
      text_to_entity( entity, "%s - %s\r\n", instance_short_descr( exit ),  exit_to ? instance_short_descr( exit_to ) : "Nowhere" );
   }
   DetachIterator( &Iter );
   text_to_entity( entity, "\r\n" );
   return ret;
}

int show_ent_mobiles_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
{
   ENTITY_INSTANCE *mob;
   ITERATOR Iter;
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

   AttachIterator( &Iter, viewing->contents_sorted[SPEC_ISMOB] );
   while( ( mob = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( entity == mob )
         continue;
      if( entity->builder )
         text_to_entity( entity, "(ID:%d) %s.\r\n", mob->tag->id, instance_long_descr( mob ) );
      else
         text_to_entity( entity, "%s.\r\n", instance_long_descr( mob ) );
   }
   DetachIterator( &Iter );
   return ret;
}

int show_ent_objects_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
{
   ENTITY_INSTANCE *obj;
   ITERATOR Iter;
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

   AttachIterator( &Iter, viewing->contents_sorted[SPEC_ISOBJECT] );
   while( ( obj = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( entity == obj )
         continue;
      if( instance_list_has_by_id( viewing->contents_sorted[SPEC_ISMOB], obj->tag->id ) )
         continue;
      if( entity->builder )
         text_to_entity( entity, "(ID:%d) %s\n", obj->tag->id, instance_long_descr( obj ) );
      else
         text_to_entity( entity, "%s\n", instance_long_descr( obj ) );
   }
   DetachIterator( &Iter );
   return ret;

}

int show_ent_rooms_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing )
{
   ENTITY_INSTANCE *room;
   ITERATOR Iter;
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
   text_to_entity( entity, "Rooms:\r\n" );
   AttachIterator( &Iter, viewing->contents_sorted[SPEC_ISROOM] );
   while( ( room = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      if( entity == room )
         continue;
      if( entity->builder )
         text_to_entity( entity, "(ID:%d) %s - %s.\r\n", room->tag->id, instance_short_descr( room ), instance_long_descr( room ) );
      else
         text_to_entity( entity, "%s - %s.\r\n", instance_short_descr( room ), instance_long_descr( room ) );
   }
   DetachIterator( &Iter );
   return ret;
}

int move_entity( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *exit )
{
   int ret = RET_SUCCESS;
   ENTITY_INSTANCE *move_to;

   if( get_spec_value( entity, "CanMove" ) < 1 && !entity->builder )
   {
      text_to_entity( entity, "You cannot move.\r\n" );
      return ret;
   }

   if( ( move_to = get_active_instance_by_id( get_spec_value( exit, "IsExit" ) ) ) == NULL )
   {
      text_to_entity( entity, "That exit goes to nowhere.\r\n" );
      return ret;
   }

   entity_to_world( entity, move_to );
   text_to_entity( entity, "You move to the %s.\r\n", instance_short_descr( exit ) );
   entity_look( entity, "" );
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
      {
         switch( input_selection_typing )
         {
            default: break;
            case SEL_INSTANCE:
               ent_to_goto = (ENTITY_INSTANCE *)retrieve_entity_selection();
               break;
            case SEL_STRING:
               text_to_entity( entity, (char *)retrieve_entity_selection() );
               return;
         }
      }
   }

   if( !ent_to_goto )
   {
      text_to_entity( entity, "No such instance exists.\r\n" );
      return;
   }
   text_to_entity( entity, "You wisk away to the desired instance.\r\n" );
   entity_to_world( entity, ent_to_goto );
   entity_look( entity, "" );
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
      text_to_entity( entity, STD_SELECTION_ERRMSG_PTR_USED );
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

   entity_to_world( new_ent, entity );
   text_to_entity( entity, "You create a new instance of %s. It has been placed in your inventory.\r\n", instance_name( new_ent ) );
   return;
}

void entity_look( void *passed, char *arg )
{
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)passed;
   show_ent_to_ent( instance, instance->contained_by );
   show_ent_contents_to_ent( instance, instance->contained_by );
   text_to_entity( instance, "\r\n" );
   return;
}

void entity_inventory( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   if( SizeOfList( entity->contents ) == 0 )
   {
      text_to_entity( entity, "Your inventory is empty.\r\n" );
      return;
   }
   show_ent_contents_to_ent( entity, entity );
   return;
}

void entity_drop( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_INSTANCE *to_drop;
   char buf[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_entity( entity, "Drop what?\r\n" );
      return;
   }

   while( arg[0] != '\0' )
   {
      arg = one_arg( arg, buf );

      if( ( to_drop = instance_list_has_by_name( entity->contents, buf ) ) == NULL )
      {
         text_to_entity( entity, "You do not have %s to drop.\r\n", buf );
         continue;
      }
      if( !entity->builder )
      {
         if( get_spec_value( to_drop, "NoDrop" ) > 0 )
         {
            text_to_entity( entity, "You cannot drop %s.\r\n", instance_short_descr( to_drop ) );
            continue;
         }
      }
      text_to_entity( entity, "You drop %s.\r\n", instance_short_descr( to_drop ) );
      entity_to_world( to_drop, entity->contained_by );
   }
   return;
}

void entity_get( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_INSTANCE *to_get;
   char buf[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      text_to_entity( entity, "Get what?\r\n" );
      return;
   }

   while( arg[0] != '\0' )
   {
      arg = one_arg( arg, buf );

      if( !entity->contained_by )
         puts( "it's null" );
      if( ( to_get = instance_list_has_by_name( entity->contained_by->contents, buf ) ) == NULL )
      {
         text_to_entity( entity, "You do not see %s to get.\r\n", buf );
         continue;
      }
      if( !entity->builder )
      {
         if( get_spec_value( to_get, "CanGet" ) > 0 )
         {
            text_to_entity( entity, "You cannot get %s.\r\n", instance_short_descr( to_get ) );
            continue;
         }
      }
      text_to_entity( entity, "You get %s.\r\n", instance_short_descr( to_get ) );
      entity_to_world( to_get, entity );
   }
   return;
}

void entity_quit( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   text_to_entity( entity, "You quit builder-mode.\r\n" );
   change_socket_state( entity->socket, STATE_OLC );
   socket_uncontrol_entity( entity );
   entity_to_world( entity, NULL );
   free_eInstance( entity );
   return;
}

void entity_create( void *passed, char *arg )
{
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   INCEPTION *olc = (INCEPTION *)entity->account->olc;
   char buf[MAX_BUFFER];
   int value;

   if( !arg || arg[0] == '\0' )
   {
      if( olc->editing)
      {
         text_to_entity( entity, "There's already something loaded in your editor, resume work on that first.\r\n" );
         return;
      }

      init_editor( olc, NULL );
      change_socket_state( entity->socket, olc->editing_state );
      return;
   }
   arg = one_arg( arg, buf );

   if( !strcasecmp( buf, "room" ) )
   {
      if( arg[0] != '\0' )
         frame = create_room_framework( arg );
      else
         frame = create_room_framework( NULL );

      init_editor( olc, frame );
      change_socket_state( entity->socket, olc->editing_state );
      return;
   }
   if( !strcasecmp( buf, "mob" ) || !strcasecmp( buf, "mobile" ) )
   {
      if( arg[0] != '\0' )
         frame = create_mobile_framework( arg );
      else
         frame = create_mobile_framework( NULL );

      init_editor( olc, frame );
      change_socket_state( entity->socket, olc->editing_state );
      return;
   }
   if( !strcasecmp( buf, "exit" ) )
   {
      if( arg[0] == '\0' )
      {
         frame = create_exit_framework( NULL, 0 );
         init_editor( olc, frame );
         change_socket_state( entity->socket, olc->editing_state );
         return;
      }
      arg = one_arg( arg, buf );
      if( arg[0] == '\0' || !is_number( arg ) )
      {
         frame = create_exit_framework( quick_format( "%s%s", buf, arg ), 0 );
         init_editor( olc, frame );
         change_socket_state( entity->socket, olc->editing_state );
         return;
      }

      value = atoi( arg );
      instance = create_exit_instance( buf, value );
      entity_to_world( instance, entity->contained_by );
      text_to_entity( entity, "You create a new exit: %s.\r\n", instance_name( instance ) );
      entity_look( entity, "" );
      return;
   }
   return;
}

void entity_edit( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_FRAMEWORK *to_edit;
   INCEPTION *olc;

   if( !entity->socket->account )
   {
      text_to_entity( entity, "You somehow have no account, run away now!\r\n" );
      return;
   }
   /* holy-ugly but my brain is not working right atm */
   if( !entity->socket->account->olc )
   {
      entity->socket->account->olc = init_olc();
      entity->socket->account->olc->account = entity->socket->account;
   }
   olc = entity->socket->account->olc;

   if( olc->editing )
   {
      text_to_entity( entity, "You already have something in your editor, resume to resolve that first.\r\n" );
      return;
   }

   if( ( to_edit = entity_edit_selection( entity, arg ) ) == NULL )
      return;

   init_editor( olc, to_edit );
   change_socket_state( entity->socket, olc->editing_state );
   text_to_entity( entity, "You begin to edit %s.\r\n", chase_name( to_edit ) );
   return;
}

void entity_iedit( void *passed, char *arg ) /* inheritance edit, not instance edit */
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_FRAMEWORK *to_edit;
   ENTITY_FRAMEWORK *inherited_to_edit;
   INCEPTION *olc;

   if( ( olc = entity->socket->account->olc ) == NULL )
   {
      text_to_entity( entity, "You don't have an olc initiated...\r\n" );
      return;
   }

   if( olc->editing )
   {
      text_to_entity( entity, "You already have something in your editor, resume to resolve that first.\r\n" );
      return;
   }

   if( ( to_edit = entity_edit_selection( entity, arg ) ) == NULL ) /* entity_edit_selection handles its own messaging */
      return;

   if( ( inherited_to_edit = create_inherited_framework( to_edit ) ) == NULL ) /* does its own setting and databasing */
   {
      text_to_entity( entity, "Something has gone wrong trying to create an inherited frame.\r\n" );
      return;
   }

   init_editor( olc, inherited_to_edit );
   change_socket_state( entity->socket, olc->editing_state );
   text_to_entity( entity, "You begin to edit %s.\r\n", chase_name( inherited_to_edit ) );
   return;
}

void entity_load( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   ENTITY_FRAMEWORK *frame;
   ENTITY_INSTANCE *instance;

   if( !arg || arg[0] == '\0' )
   {
      if( entity->contained_by )
      {
         text_to_entity( entity, "Loading the entity you are within.\r\n" );
         full_load_instance( entity->contained_by );
         return;
      }
      text_to_entity( entity, "Load what?\r\n" );
      return;
   }

   /* search inventory and room contents */
   if( check_selection_type( arg ) == SEL_NULL )
   {
      if( ( instance = instance_list_has_by_name( entity->contents, arg ) ) == NULL
         && ( !entity->contained_by || ( instance = instance_list_has_by_name( entity->contained_by->contents, arg ) ) == NULL ) )
      {
         text_to_entity( entity, "You don't have %s in your inventory and its not in this room.\r\n", arg );
         return;
      }
      text_to_entity( entity, "Loading %s.\r\n", instance_name( instance ) );
      full_load_instance( instance );
      return;
   }

   if( !interpret_entity_selection( arg ) )
   {
      text_to_entity( entity, STD_SELECTION_ERRMSG_PTR_USED );
      return;
   }

   switch( input_selection_typing )
   {
      default:
         clear_entity_selection();
         text_to_entity( entity, "Invalid selection type, frames and instances only.\r\n" );
         return;
      case SEL_FRAME:
         frame = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
         instance = full_load_eFramework( frame );
         entity_to_world( instance, entity );
         break;
      case SEL_INSTANCE:
         instance = (ENTITY_INSTANCE *)retrieve_entity_selection();
         full_load_instance( instance );
         break;
      case SEL_STRING:
         text_to_entity( entity, (char *)retrieve_entity_selection() );
         return;
   }

   text_to_entity( entity, "You completely load %s.\r\n", instance_name( instance ) );
   return;
}

void entity_north( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "north" ) )
      return;
   move_create( entity, get_framework_by_id( 0 ), arg );
   return;
}

void entity_south( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "south" ) )
      return;
   move_create( entity, get_framework_by_id( 1 ), arg );
   return;
}

void entity_east( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "east" ) )
      return;
   move_create( entity, get_framework_by_id( 2 ), arg );
   return;
}

void entity_west( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "west" ) )
      return;
   move_create( entity, get_framework_by_id( 3 ), arg );
   return;
}

void entity_up( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "up" ) )
      return;
   move_create( entity, get_framework_by_id( 4 ), arg );
   return;
}

void entity_down( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !should_move_create( entity, "down" ) )
      return;
   move_create( entity, get_framework_by_id( 5 ), arg );
   return;
}

void entity_chat( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;

   if( !arg || arg[0] == '\0' )
   {
      text_to_entity( entity, "Chat what?\r\n" );
      return;
   }

   communicate( CHAT_LEVEL, entity->socket->account, arg );
   entity->socket->bust_prompt = NO_PROMPT;
   text_to_entity( entity, ":> " );
   return;
}

void entity_grab( void *passed, char *arg )
{
   ENTITY_INSTANCE *entity = (ENTITY_INSTANCE *)passed;
   INCEPTION *olc;

   if( !entity->account || !entity->account->olc )
   {
      text_to_entity( entity, "You are missing an account or an olc.\r\n" );
      return;
   }
   olc = entity->account->olc;

   if( !olc->using_workspace )
   {
      text_to_entity( entity, "You are not using a workspace.\r\n" );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      if( !entity->contained_by )
      {
         text_to_entity( entity, "You are not being contained by anything, you can't grab.\r\n" );
         return;
      }
      add_instance_to_workspace( entity->contained_by, olc->using_workspace );
      text_to_entity( entity, "You grab %s what is containing you and add it to %s.\r\n", instance_short_descr( entity->contained_by ), olc->using_workspace->name );
      return;
   }
   return;
}
