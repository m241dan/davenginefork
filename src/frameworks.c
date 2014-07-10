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

int new_eFramework( ENTITY_FRAMEWORK *frame )
{
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

bool framework_list_has_by_id( LLIST *frameworks, int id )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   if( !frameworks )
      return FALSE;

   if( SizeOfList( frameworks ) < 1 )
      return FALSE;

   AttachIterator( &Iter, frameworks );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( frame->tag->id == id )
         break;
   DetachIterator( &Iter );

   if( frame )
      return TRUE;
   return FALSE;
}
