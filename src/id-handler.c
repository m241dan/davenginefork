/* id-handlers.c written by Davenge */

#include "mud.h"

ID_HANDLER handlers[MAX_ID_HANDLER];

ID_HANDLER *init_handler( void )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   if( clear_handler( handler ) != RET_SUCCESS )
   {
      free_handler( handler );
      return NULL;
   }

   return handler;
}

int clear_handler( ID_HANDLER *handler )
{
   int ret = RET_SUCCESS;

   if( !handler )
   {
      BAD_POINTER( "handler" );
      return ret;
   }

   handler->type = -1;
   FREE( handler->name );
   handler->name = strdup( "new handler" );
   handler->top_id = 0;
   FREE( handler->recycled_ids );
   handler->recycled_ids = NULL;
   handler->can_recycle = FALSE;

   return ret;
}

int free_handler( ID_HANDLER *handler )
{
   int ret = RET_SUCCESS;
   FREE( handler->name );
   FREE( handler->recycled_ids );
   FREE( handler );
   return ret;
}

ID_TAG *init_tag( void )
{
   ID_TAG *tag;

   CREATE( tag, ID_TAG, 1 );
   if( clear_tag( tag ) != RET_SUCCESS )
   {
      free_tag( tag );
      return NULL;
   }
   return tag;
}

int clear_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }

   tag->type = -1;
   tag->id = -1;
   FREE( tag->created_by );
   FREE( tag->created_on );
   FREE( tag->modified_by );
   FREE( tag->modified_on );
   tag->created_by = strdup( "system" );
   tag->created_on = ctime( &current_time );
   tag->modified_by = strdup( "system" );
   tag->modified_on = ctime( &current_time );

   return ret;
}

int free_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   FREE( tag->created_on );
   FREE( tag->created_by );
   FREE( tag->modified_by );
   FREE( tag->modified_on);
   FREE( tag );
   return ret;
}
