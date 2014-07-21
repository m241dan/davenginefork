/* frameworks.c: methods pertaining to frameworks written by Davenge */

#include "mud.h"

ENTITY_FRAMEWORK *init_eFramework( void )
{
   ENTITY_FRAMEWORK *frame;

   CREATE( frame, ENTITY_FRAMEWORK, 1 );
   frame->tag = init_tag();
   frame->tag->type = ENTITY_FRAMEWORK_IDS;
   frame->contents = AllocList();
   frame->specifications = AllocList();
   if( clear_eFramework( frame ) != RET_SUCCESS )
   {
      bug( "could not clear memory allocated by %s", __FUNCTION__ );
      free_eFramework( frame );
      return NULL;
   }
   return frame;
}

int clear_eFramework( ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   FREE( frame->name );
   frame->name = strdup( "new_frame" );
   FREE( frame->short_descr );
   frame->short_descr = strdup( "a new frame" );
   FREE( frame->long_descr );
   frame->long_descr = strdup( "a new frame is here" );
   FREE( frame->description );
   frame->description = strdup( "none" );

   return ret;
}

int free_eFramework( ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   if( frame->tag )
      free_tag( frame->tag );

   FreeList( frame->contents );
   frame->contents = NULL;
   FreeList( frame->specifications );
   frame->specifications = NULL;

   FREE( frame->name );
   FREE( frame->short_descr );
   FREE( frame->long_descr );
   FREE( frame->description );
   FREE( frame );

   return ret;
}

ENTITY_FRAMEWORK *load_eFramework_by_query( const char *query )
{
   ENTITY_FRAMEWORK *frame = NULL;
   MYSQL_ROW row;

   if( !db_query_single_row( &row, query ) )
      return NULL;

   if( ( frame = init_eFramework() ) == NULL )
      return NULL;

   db_load_eFramework( frame, &row );
   load_specifications_to_list( frame->specifications, quick_format( "f%d", frame->tag->id ) );
   return frame;
}

ENTITY_FRAMEWORK *get_framework_by_id( int id )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = get_active_framework_by_id( id ) ) == NULL )
      if( ( frame = load_eFramework_by_id( id ) ) != NULL )
         AttachToList( frame, active_frameworks );

   return frame;
}

ENTITY_FRAMEWORK *get_active_framework_by_id( int id )
{
   return framework_list_has_by_id( active_frameworks, id );
}

ENTITY_FRAMEWORK *load_eFramework_by_id( int id )
{
   return load_eFramework_by_query( quick_format( "SELECT * FROM `%s` WHERE %s=%d;", tag_table_strings[ENTITY_FRAMEWORK_IDS], tag_table_whereID[ENTITY_FRAMEWORK_IDS], id ) );
}

ENTITY_FRAMEWORK *get_framework_by_name( const char *name )
{
   ENTITY_FRAMEWORK *frame;

   if( ( frame = get_active_framework_by_name( name ) ) == NULL )
      if( ( frame = load_eFramework_by_name( name ) ) != NULL )
         AttachToList( frame, active_frameworks );

   return frame;
}

ENTITY_FRAMEWORK *get_active_framework_by_name( const char *name )
{
   return framework_list_has_by_name( active_frameworks, name );
}

ENTITY_FRAMEWORK *load_eFramework_by_name( const char *name )
{
   return load_eFramework_by_query( quick_format( "SELECT * FROM `%s` WHERE name='%s' LIMIT 1;", tag_table_strings[ENTITY_FRAMEWORK_IDS], name ) );
}

int new_eFramework( ENTITY_FRAMEWORK *frame )
{
   SPECIFICATION *spec;
   ITERATOR Iter;
   int ret = RET_SUCCESS;

   if( !frame )
   {
      BAD_POINTER( "frame" );
      return ret;
   }

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      if( ( ret = new_tag( frame->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO entity_frameworks VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' );",
         frame->tag->id, frame->tag->type, frame->tag->created_by,
         frame->tag->created_on, frame->tag->modified_by, frame->tag->modified_on,
         frame->name, frame->short_descr, frame->long_descr, frame->description ) )
      return RET_FAILED_OTHER;

   AttachIterator( &Iter, frame->specifications );
   while( ( spec = (SPECIFICATION *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( spec->owner, "f%d", frame->tag->id );
      new_specification( spec );
   }
   DetachIterator( &Iter );

   return ret;
}

void db_load_eFramework( ENTITY_FRAMEWORK *frame, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( frame->tag, row );
   frame->name = strdup( (*row)[counter++] );
   frame->short_descr = strdup( (*row)[counter++] );
   frame->long_descr = strdup( (*row)[counter++] );
   frame->description = strdup( (*row)[counter++] );
   return;
}

ENTITY_FRAMEWORK *framework_list_has_by_id( LLIST *frameworks, int id )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frameworks )
      return NULL;
   if( SizeOfList( frameworks ) < 1 )
      return NULL;

   AttachIterator( &Iter, frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( frame->tag->id == id )
         break;
   DetachIterator( &Iter );

   return frame;
}

ENTITY_FRAMEWORK *framework_list_has_by_name( LLIST *frameworks, const char *name )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frameworks )
      return NULL;
   if( SizeOfList( frameworks ) < 1 )
      return NULL;

   AttachIterator( &Iter, frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( frame->name, name ) )
         break;
   DetachIterator( &Iter );

   return frame;
}

bool live_frame( ENTITY_FRAMEWORK *frame )
{
   if( !frame )
      return FALSE;
   if( !frame->tag )
      return FALSE;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return FALSE;

   return TRUE;
}
