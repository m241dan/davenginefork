/* editor.c: methods pertaining to the Editors written by Davenge */

#include "mud.h"

int editor_eFramework_prompt( D_SOCKET *dsock )
{
   ENTITY_FRAMEWORK *frame;
   BUFFER *buf = buffer_new( MAX_BUFFER );
   char tempstring[MAX_BUFFER];
   int space_after_pipes;
   int ret = RET_SUCCESS;

   if( !dsock->account || !dsock->account->olc )
   {
      text_to_socket( dsock, "You have a NULL account or NULL olc...\r\n" );
      return ret;
   }

   if( ( frame = (ENTITY_FRAMEWORK *)dsock->account->olc->editing ) == NULL )
   {
      bug( "%s: being called with nothing to edit.", __FUNCTION__ );
      return ret;
   }

   space_after_pipes = dsock->account->pagewidth - 2;

   if( !strcmp( frame->tag->created_by, "null" ) )
      mud_printf( tempstring, "Potential Framework ID: %d", get_potential_id( frame->tag->type ) );
   else
      mud_printf( tempstring, "Framework ID: %d", frame->tag->id );
   bprintf( buf, "/%s\\\r\n", print_header( tempstring, "-", space_after_pipes ) );
   mud_printf( tempstring, " Name : %s", frame->name );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Short: %s", frame->short_descr );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Long : %s", frame->long_descr );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Desc : %s", frame->description );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );
   bprintf( buf, "|%s|", print_header( "Specifications Here", " ", ( space_after_pipes - 1 ) / 2 ) );
   bprintf( buf, " %s|\r\n", print_header( "Stats Here", " ", ( space_after_pipes - 1 ) / 2 ) );
   bprintf( buf, "|%s|", print_bar( "-", space_after_pipes ) );
   print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_bar( "-", space_after_pipes ) );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return ret;
}

void eFramework_name( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK *)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   FREE( frame->name );
   frame->name = strdup( arg );
   text_to_olc( olc, "Name changed.\r\n" );

   if( live_frame( frame ) )
   {
      quick_query( "UPDATE entity_frameworks SET name='%s' WHERE entityFrameworkID=%d;", frame->name, frame->tag->id );
      update_tag( frame->tag, olc->account->name );
   }

   return;
}

void eFramework_short( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   FREE( frame->short_descr );
   frame->short_descr = strdup( arg );
   text_to_olc( olc, "Short description changed.\r\n" );

   if( live_frame( frame ) )
   {
      quick_query( "UPDATE entity_frameworks SET short_descr='%s' WHERE entityFrameworkID=%d;", frame->short_descr, frame->tag->id );
      update_tag( frame->tag, olc->account->name );
   }

   return;
}

void eFramework_long( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( strlen( arg ) > MAX_FRAMEWORK_NSL )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   FREE( frame->long_descr );
   frame->long_descr = strdup( arg );
   text_to_olc( olc, "Long description changed.\r\n" );

   if( live_frame( frame ) )
   {
      quick_query( "UPDATE entity_frameworks SET long_descr='%s' WHERE entityFrameworkID=%d;", frame->long_descr, frame->tag->id );
      update_tag( frame->tag, olc->account->name );
   }

   return;
}

void eFramework_description( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame;

   if( strlen( arg ) > MAX_BUFFER )
   {
      text_to_olc( olc, "%s is too long.\r\n", arg );
      return;
   }
   if( ( frame = (ENTITY_FRAMEWORK*)olc->editing ) == NULL )
   {
      text_to_olc( olc, "You aren't actually editing anything...\r\n" );
      change_socket_state( olc->account->socket, STATE_OLC );
      return;
   }

   FREE( frame->description );
   frame->description = strdup( arg );
   text_to_olc( olc, "Long description changed.\r\n" );

   if( live_frame( frame ) )
   {
      quick_query( "UPDATE entity_frameworks SET description='%s' WHERE entityFrameworkID=%d;", frame->description, frame->tag->id );
      update_tag( frame->tag, olc->account->name );
   }

   return;
}

void eFramework_done( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;

   if( !strcmp( frame->tag->created_by, "null" ) )
   {
      new_tag( frame->tag, olc->account->name );
      new_eFramework( frame );
      AttachToList( frame, active_frameworks );
   }

   FREE( olc->editing );
   free_command_list( olc->editor_commands );
   FreeList( olc->editor_commands );
   olc->editor_commands = NULL;
   olc->editing_state = STATE_OLC;
   change_socket_state( olc->account->socket, STATE_OLC );
   text_to_olc( olc, "Exiting Entity Framework Editor.\r\n" ); 

   return;
}
