/* editor.c: methods pertaining to the Editors written by Davenge */

#include "mud.h"

int init_eFramework_editor( INCEPTION *olc, ENTITY_FRAMEWORK *frame )
{
   int ret = RET_SUCCESS;

   if( !frame )
   {
      CREATE( olc->editing, ENTITY_FRAMEWORK, 1 );
      olc->editing = init_eFramework();
   }
   else
      olc->editing = frame;

   olc->editing_state = STATE_EFRAME_EDITOR;
   text_to_olc( olc, "Opening the Framework Editor...\r\n" );
   olc->editor_commands = AllocList();

   return ret;
}

int free_editor( INCEPTION *olc )
{
   int ret = RET_SUCCESS;

   olc->editing = NULL;
   free_command_list( olc->editor_commands );
   FreeList( olc->editor_commands );
   olc->editor_commands = NULL;
   olc->editing_state = olc->account->socket->prev_state;

   return ret;
}

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

   if( frame->inherits )
      strcat( tempstring, quick_format( " | Inherits from %s ID: %d", chase_name( frame->inherits ), frame->inherits->tag->id ) );

   bprintf( buf, "/%s\\\r\n", print_header( tempstring, "-", space_after_pipes ) );

   bprintf( buf, "%s", return_framework_strings( frame, "|~|", dsock->account->pagewidth ) );

   if( SizeOfList( frame->specifications ) > 0 || inherited_frame_has_any_spec( frame ) )
      bprintf( buf, "%s", return_framework_specs_and_stats( frame, "|", dsock->account->pagewidth ) );

   if( SizeOfList( frame->fixed_contents ) > 0 || inherited_frame_has_any_fixed_possession( frame ) )
      bprintf( buf, "%s", return_framework_fixed_content( frame, "|", dsock->account->pagewidth ) );

   bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );

   print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_bar( "-", space_after_pipes ) );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return ret;
}

const char *return_framework_strings( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;


   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Name : %s%s", chase_name( frame ), !strcmp( frame->name, "__inherited__" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   strcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Short : %s%s", chase_short_descr( frame ), !strcmp( frame->short_descr, "__inherited__" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   strcat( buf, tempstring );


   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Long : %s%s", chase_long_descr( frame ), !strcmp( frame->long_descr, "__inherited__" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   strcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space(
      quick_format( " Desc : %s%s", chase_description( frame ), !strcmp( frame->description, "__inherited__" ) ? " ( inherited )" : "" ),
      space_after_border ),
      border );

   strcat( buf, tempstring );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_framework_specs_and_stats( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   strcat( buf, tempstring );

   space_after_border = width - ( strlen( border ) * 3 );

   mud_printf( tempstring, "%s%s", border, print_header( "Specifications", " ", space_after_border / 2 ) );
   strcat( buf, tempstring );

   strcat( buf, border );

   mud_printf( tempstring, " %s%s\r\n", print_header( "Stats", " ", space_after_border / 2 ), border );
   strcat( buf, tempstring );

   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   strcat( buf, tempstring );

   strcat( buf, return_spec_and_stat_list( frame->specifications, border, width, FALSE ) );
   while( ( frame = frame->inherits ) != NULL )
      strcat( buf, return_spec_and_stat_list( frame->specifications, border, width, TRUE ) );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_spec_and_stat_list( LLIST *spec_list, const char *border, int width, bool inherited )
{
   SPECIFICATION *spec;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];

   memset( &buf[0], 0, sizeof( buf ) );

   AttachIterator( &Iter, spec_list );
   while( ( spec = (SPECIFICATION *)NextInList( &Iter ) ) != NULL )
      strcat( buf, return_spec_and_stat( spec, border, width, inherited ) );
   DetachIterator( &Iter );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_spec_and_stat( SPECIFICATION *spec, const char *border, int width, bool inherited )
{
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s", border,
   fit_string_to_space(
   quick_format( " %s : %d%s", spec_table[spec->type], spec->value, inherited ? " ( inherited )" : "" ), ( space_after_border / 2 ) - 1 ) );
   strcat( buf, tempstring );

   strcat( buf, border );

   mud_printf( tempstring, " %s%s\r\n",
   print_header( " ", " ", ( space_after_border / 2 ) - 1 ),
   border );
   strcat( buf, tempstring );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_framework_fixed_content( ENTITY_FRAMEWORK *frame, const char *border, int width )
{
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   strcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_header( "Fixed Possessions", " ", space_after_border ), border );
   strcat( buf, tempstring );

   mud_printf( tempstring, "%s%s%s\r\n", border, print_bar( "-", space_after_border ), border );
   strcat( buf, tempstring );

   strcat( buf, return_fixed_content_list( frame->fixed_contents, border, width, FALSE ) );
   while( ( frame = frame->inherits ) != NULL )
      strcat( buf, return_fixed_content_list( frame->fixed_contents, border, width, TRUE ) );

   buf[strlen( buf )] = '\0';
   return buf;
}

const char *return_fixed_content_list( LLIST *fixed_list, const char *border, int width, bool inherited )
{
   ENTITY_FRAMEWORK *fixed_content;
   ITERATOR Iter;
   static char buf[MAX_BUFFER];
   char tempstring[MAX_BUFFER];
   int space_after_border;

   memset( &buf[0], 0, sizeof( buf ) );
   space_after_border = width - ( strlen( border ) * 2 );

   AttachIterator( &Iter, fixed_list );
   while( ( fixed_content = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      mud_printf( tempstring, "%s%s%s\r\n", border,
      fit_string_to_space( quick_format( "(%-7d) %s, %s%s", fixed_content->tag->id, chase_name( fixed_content ), chase_short_descr( fixed_content ), inherited ? " (inherited)" : "" ), space_after_border ), border );
      strcat( buf, tempstring );
   }
   DetachIterator( &Iter );

   buf[strlen( buf )] = '\0';
   return buf;
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

void eFramework_addSpec( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   SPECIFICATION *spec;
   char spec_arg[MAX_BUFFER];
   int spec_type, spec_value;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Valid Specs: %s.\r\n", print_string_table( spec_table ) );
      return;
   }

   arg = one_arg( arg, spec_arg );

   if( ( spec_type = match_string_table_no_case( spec_arg, spec_table ) ) == -1 )
   {
      text_to_olc( olc, "Invalid Spec Type.\r\n" );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
     text_to_olc( olc, "Spec Value Defaulting to 1.\r\n" );
     spec_value = 1;
   }
   else if( !is_number( arg ) )
   {
      text_to_olc( olc, "Spec Value must be a number.\r\n" );
      return;
   }
   else
      spec_value = atoi( arg );

   if( ( spec = spec_list_has_by_type( frame->specifications, spec_type ) ) != NULL )
      text_to_olc( olc, "Override current %s specification who's value was %d.\r\n", spec_table[spec->type], spec->value );
   else
      spec = init_specification();

   spec->type = spec_type;
   spec->value = spec_value;
   add_spec_to_framework( spec, frame );
   text_to_olc( olc, "%s added to %s with the value of %s.\r\n", spec_table[spec_type], frame->name, itos( spec->value ) );
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
      if( olc->using_workspace )
         add_frame_to_workspace( frame, olc->using_workspace );
   }

   free_editor( olc );
   change_socket_state( olc->account->socket, olc->account->socket->prev_state );
   text_to_olc( olc, "Exiting Entity Framework Editor.\r\n" );
   olc_show_prompt( olc );
   return;
}

void eFramework_addContent( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   ENTITY_FRAMEWORK *frame = (ENTITY_FRAMEWORK *)olc->editing;
   ENTITY_FRAMEWORK *frame_to_add;
   int value;

   if( check_selection_type( arg ) != SEL_FRAME )
   {
      if( is_number( arg ) )
      {
         value = atoi( arg );
         if( ( frame_to_add = get_framework_by_id( value ) ) == NULL )
         {
            text_to_olc( olc, "There's no framework with the ID of %d.\r\n", value );
            return;
         }
      }
      else if( ( frame_to_add =  get_framework_by_name( arg ) ) == NULL )
      {
         text_to_olc( olc, "There's no framework with the name %s.\r\n", arg );
         return;
      }
   }
   else
   {
      if( !interpret_entity_selection( arg ) )
      {
         text_to_olc( olc, STD_SELECTION_ERRMSG_PTR_USED );
         return;
      }
      switch( input_selection_typing )
      {
         default: return;
         case SEL_FRAME:
            frame_to_add = (ENTITY_FRAMEWORK *)retrieve_entity_selection();
            break;
         case SEL_STRING:
            text_to_olc( olc, (char *)retrieve_entity_selection() );
            return;
      }
   }
   add_frame_to_fixed_contents( frame_to_add, frame );
   text_to_olc( olc, "%s added to %s's fixed contents.\r\n", chase_name( frame_to_add ), chase_name( frame ) );
   return;
}
