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
            text_to_entity( (ENTITY_INSTANCE *)timer->owner, "%s\r\n", timer->end_message );
         break;
   }
   free_timer( timer );
}

/* setters */
void set_melee_timer( ENTITY_INSTANCE *instance, bool message )
{
   TIMER *timer;

   if( CHECK_MELEE( instance ) != 0 )
      return;

   timer = init_timer();
   timer->duration = BASE_MELEE_DELAY;
   timer->owner = instance;
   timer->owner_type = TIMER_INSTANCE;
   timer->type = TT_COOLDOWN;
   timer->key = strdup( MELEE_KEY );
   if( message )
      timer->end_message = strdup( MELEE_CD_MSG );
   start_timer( timer );
}

/* monitor */
void timer_monitor( void )
{
   TIMER *timer;
   ITERATOR Iter;

   AttachIterator( &Iter, timer_queue );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
      if( ( timer->duration -= .25 ) == 0 )
         end_timer( timer );
   DetachIterator( &Iter );
}

inline double check_timer( const char *key )
{
   TIMER *timer;
   ITERATOR Iter;
   double time = 0;
   AttachIterator( &Iter, timer_queue );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( timer->key, key ) )
      {
         time = timer->duration;
         break;
      }
   DetachIterator( &Iter );
   return time;
}

inline double check_timer_instance( ENTITY_INSTANCE *instance, const char *key )
{
   TIMER *timer;
   ITERATOR Iter;
   double time = 0;
   AttachIterator( &Iter, instance->timers );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
       if( !strcmp( timer->key, key ) )
       {
          time = timer->duration;
          break;
       }
   DetachIterator( &Iter );
   return time;
}

