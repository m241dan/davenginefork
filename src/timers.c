/* timers.c containers the functions of the timer library written by Davenge */

#include "mud.h"

/* creation */
TIMER *init_timer( void )
{
   TIMER *timer;

   CREATE( timer, TIMER, 1 );
   timer->owner_type = TIMER_NO_OWNER;
   return timer;
}

void free_timer( TIMER *timer )
{
   switch( timer->owner_type )
   {
      default: timer->owner = NULL; break;
      case TIMER_INSTANCE:
         timer->owner = NULL;
         break;
   }
   FREE( timer->key );
   FREE( timer->end_message );
   FREE( timer );
}

void start_timer( TIMER *timer )
{
   if( SizeOfList( paused_timer_queue ) > 0 )
      DetachFromList( timer, paused_timer_queue );

   AttachToList( timer, timer_queue );
   switch( timer->owner_type )
   {
      default: bug( "%s: bad owner.", __FUNCTION__ ); break;
      case TIMER_MUD:
         break;
      case TIMER_INSTANCE:
         AttachToList( timer, ((ENTITY_INSTANCE *)timer->owner)->timers );
         break;
   }
}

void pause_timer( TIMER *timer )
{
   DetachFromList( timer, timer_queue );
   AttachToList( timer, paused_timer_queue );
}

void end_timer( TIMER *timer )
{
   DetachFromList( timer, timer_queue );
   switch( timer->owner_type )
   {
      default: bug( "%s: bad owner.", __FUNCTION__ ); break;
      case TIMER_MUD:
         break;
      case TIMER_INSTANCE:
         DetachFromList( timer, ((ENTITY_INSTANCE *)timer->owner)->timers );
         if( timer->end_message && timer->end_message[0] != '\0' )
            text_to_entity( (ENTITY_INSTANCE *)timer->owner, timer->end_message );
         break;
   }
   free_timer( timer );
}

void timer_monitor( void )
{
   TIMER *timer;
   ITERATOR Iter;

   AttachToList( &Iter, timer_queue );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
      if( ( timer->duration -= .25 ) == 0 )
         end_timer( timer );
   DetachIterator( &Iter );
}
