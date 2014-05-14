/* id-handlers.c written by Davenge */

#include "mud.h"

ID_HANDLER *handlers[MAX_ID_HANDLER];

ID_HANDLER *init_handler( void )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   handler->recycled_ids = AllocList();
   if( clear_handler( handler ) != RET_SUCCESS )
   {
      free_handler( handler );
      return NULL;
   }
   return handler;
}

int clear_handler( ID_HANDLER *handler )
{
   ITERATOR Iter;
   int *rec_id;
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
   handler->can_recycle = FALSE;

   if( SizeOfList( handler->recycled_ids ) > 0 )
   {
      AttachIterator( &Iter, handler->recycled_ids );
      while( ( rec_id = (int *)NextInList( &Iter ) ) != NULL )
         FREE( rec_id );
      DetachIterator( &Iter );
   }
   return ret;
}

int free_handler( ID_HANDLER *handler )
{
   int ret = RET_SUCCESS;
   FREE( handler->name );
   if( SizeOfList( handler->recycled_ids ) > 0 )
      clear_handler( handler );
   FreeList( handler->recycled_ids );
   handler->recycled_ids = NULL;
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

int delete_tag( ID_TAG *tag )
{
   int ret = RET_SUCCESS;
   return ret;
}
int load_id_handlers( void )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   ID_HANDLER *handler;
   char query[MAX_BUFFER];
   int ret = RET_SUCCESS;

   mud_printf( query, "SELECT * FROM `id-handlers`;" );

   if( mysql_query( sql_handle, query) )
   {
      report_sql_error( sql_handle );
      return RET_FAILED_OTHER;
   }

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      if( ( handler = init_handler() ) == NULL )
      {
         BAD_POINTER( "handler" );
         return ret;
      }

      handler->type = atoi( row[0] );
      handler->name = strdup( row[1] );
      handler->top_id = atoi( row[2] );
      handler->can_recycle = (bool)atoi( row[3] );
      if( handlers[handler->type] != NULL )
      {
         bug( "%s: two handlers have identitical IDs", __FUNCTION__ );
         free_handler( handler );
         return RET_FAILED_OTHER;
      }
      handlers[handler->type] = handler;
   }

   mysql_free_result( result );
   return ret;
}

int load_recycled_ids( void )
{
   MYSQL_RES *result;
   MYSQL_ROW row;
   char query[MAX_BUFFER];
   int ret = RET_SUCCESS;

   mud_printf( query, "SELECT * FROM `id-recycled`;" );

   if( mysql_query( sql_handle, query ) )
   {
     report_sql_error( sql_handle );
     return RET_FAILED_OTHER;
   }

   if( ( result = mysql_store_result( sql_handle ) ) == NULL )
   {
      report_sql_error( sql_handle );
      return RET_DB_NO_ENTRY;
   }

   while( ( row = mysql_fetch_row( result ) ) != NULL )
   {
      int type;
      int *id;
      CREATE( id, int, 1 );
      type = atoi( row[0] );
      *id = atoi( row[1] );
      AttachToList( id, handlers[type]->recycled_ids );
   }

   mysql_free_result( result );
   return ret;
}

int get_new_id( int type )
{
   ID_HANDLER *handler;
   int *id;     /* have to use a pointer to remove from list, cannot use locally allocated memory */
   int rec_id; /* must use this integer as storage because the recycled id memory will need to be deleted before return */


   if( ( handler = handlers[type] ) == NULL )
   {
      bug( "%s: %d is a bad handler type.", __FUNCTION__, type );
      return -1;
   }

   if( handler->can_recycle && SizeOfList( handler->recycled_ids ) > 0 )
   {
      ITERATOR Iter;

      AttachIterator( &Iter, handler->recycled_ids );
      if( ( id = (int *)NextInList( &Iter ) ) == NULL )
      {
         bug( "%s: could not get id from recycled list of handler %s.", __FUNCTION__, handler->name);
         return -1;
      }
      DetachFromList( id, handler->recycled_ids );
      rec_id = *id;
      FREE( id );
      DetachIterator( &Iter );
      return rec_id;
   }

   return handler->top_id++;
}
