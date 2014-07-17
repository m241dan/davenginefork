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

ENTITY_FRAMEWORK *load_eFramework_by_query( const char *fmt_query, ... )
{
   ENTITY_FRAMEWORK *frame = NULL;
   MYSQL_RES *result;
   MYSQL_ROW row;
   char query[MAX_BUFFER];
   va_list va;

   va_start( va, fmt_query );
   vsnprintf( query, MAX_BUFFER, fmt_query, va );
   va_end( va );

   if( !quick_query( "SELECT * FROM entity_frameworks WHERE %s;", query ) )
      return NULL;
   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
      return NULL;
   if( mysql_num_rows( result ) == 0 )
      goto thereturn;
   if( ( row = mysql_fetch_row( result ) ) == NULL )
      goto thereturn;
   if( ( frame= init_eFramework() ) == NULL )
      goto thereturn;

   db_load_eFramework( frame, &row );
   load_specifications_to_list( frame->specifications, quick_format( "f%d", frame->tag->id ) );

   thereturn:
      mysql_free_result( result );
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
   return load_eFramework_by_query( "%s=%d", tag_table_whereID[ENTITY_FRAMEWORK_IDS], id );
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
   return load_eFramework_by_query( "name='%s' LIMIT 1", name );
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
   frame->tag->id = atoi( (*row)[0] );
   frame->tag->type = atoi( (*row)[1] );
   frame->tag->created_by = strdup( (*row)[2] );
   frame->tag->created_on = strdup( (*row)[3] );
   frame->tag->modified_by = strdup( (*row)[4] );
   frame->tag->modified_on = strdup( (*row)[5] );
   frame->name = strdup( (*row)[6] );
   frame->short_descr = strdup( (*row)[7] );
   frame->long_descr = strdup( (*row)[8] );
   frame->description = strdup( (*row)[9] );
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
   if( !frame->tag )
      return FALSE;
   if( !strcmp( frame->tag->created_by, "null" ) )
      return FALSE;

   return TRUE;
}
