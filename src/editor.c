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

   bprintf( buf, "/%s\\\r\n", print_bar( "-", space_after_pipes ) );
   mud_printf( tempstring, " Name : %s", frame->name );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Short: %s", frame->short_descr );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Long : %s", frame->long_descr );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   mud_printf( tempstring, " Desc : %s", frame->description );
   bprintf( buf, "|%s|\r\n", fit_string_to_space( tempstring, space_after_pipes ) );
   bprintf( buf, "|%s|\r\n", print_bar( "-", space_after_pipes ) );
   print_commands( dsock->account->olc, dsock->account->olc->editor_commands, buf, 0, dsock->account->pagewidth );
   bprintf( buf, "\\%s/\r\n", print_bar( "-", space_after_pipes ) );

   text_to_buffer( dsock, buf->data );


   buffer_free( buf );
   return ret;
}

void eFramework_name( void *passed, char *arg )
{
   return;
}

void eFramework_short( void *passed, char *arg )
{
   return;
}

void eFramework_long( void *passed, char *arg )
{
   return;
}

void eFramework_description( void *passed, char *arg )
{
   return;
}

void eFramework_done( void *passed, char *arg )
{
   return;
}
