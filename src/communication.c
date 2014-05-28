/* communcation.c: methods pertaining to global communication written by Davenge */

#include "mud.h"

int communicate( int level, const char *speaker, const char *message )
{
   D_SOCKET *socket;
   ITERATOR Iter;
   char comm_tag[MAX_BUFFER];
   int ret = RET_SUCCESS;

   if( !message || message[0] == '\0' )
   {
      bug( "%s: received a blank message.", __FUNCTION__ );
      return 0;
   }

   if( !speaker || speaker[0] == '\0' )
   {
      bug( "%s: received a blank speaker.", __FUNCTION__ );
   }
   switch( level )
   {
      default:
         bug( "%s: unknown global communication level: %d", __FUNCTION__, level );
         return RET_FAILED_OTHER;
      case CHAT_LEVEL:
         AttachIterator( &Iter, dsock_list );
         while( ( socket = NextInList( &Iter ) ) != NULL )
         {
            
         }
   }
   return ret;
}
