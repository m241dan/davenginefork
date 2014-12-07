/* timers.c containers the functions of the timer library written by Davenge */

#include "mud.h"

/* creation */
TIMER *init_timer( void )
{
   TIMER *timer;

   CREATE( timer, TIMER, 1 );
   timer->owner_type = TIMER_NO_OWNER;
   timer->active = FALSE;
   timer->key = strdup( "null" );
   AttachToList( timer, paused_timer_queue );
   return timer;
}

void free_timer( TIMER *timer )
{
   if( timer->active )
      DetachFromList( timer, timer_queue );
   else
      DetachFromList( timer, paused_timer_queue );

   if( timer->owner_type != TIMER_NO_OWNER )
      unown_timer( timer );
   FREE( timer->key );
   FREE( timer->update_message );
   FREE( timer->end_message );
   FREE( timer );
}

void start_timer( TIMER *timer )
{
   if( timer->owner_type == TIMER_NO_OWNER )
   {
       bug( "%s: cannot start an unowned timer.", __FUNCTION__ );
       return;
   }
   if( !timer->duration )
   {
      bug( "%s: cannot start a timer with 0 duration.", __FUNCTION__ );
      return;
   }
   if( !timer->key || timer->key[0] == '\0' || !strcmp( timer->key, "null" ) )
   {
      bug( "%s: cannot start a timer with no key.", __FUNCTION__ );
      return;
   }
   if( timer->timer_type == TT_UNKNOWN )
   {
      bug( "%s: cannot start a timer with no timer_type.", __FUNCTION__ );
      return;
   }

   if( timer->active )
      return;
   else
      DetachFromList( timer, paused_timer_queue );

   timer->active = TRUE;

   AttachToList( timer, timer_queue );
}

void pause_timer( TIMER *timer )
{
   if( !timer->active )
      return;
   timer->active = FALSE;
   DetachFromList( timer, timer_queue );
   AttachToList( timer, paused_timer_queue );
}

void end_timer( TIMER *timer )
{
   if( !timer->active )
   {
      bug( "%s: cannot end an inactive timer.", __FUNCTION__ );
      return;
   }

   switch( timer->owner_type )
   {
      default: bug( "%s: bad owner.", __FUNCTION__ ); break;
      case TIMER_MUD:
         delete_timer( timer );
         break;
      case TIMER_INSTANCE:
         if( timer->end_message && timer->end_message[0] != '\0' )
         {
            text_to_entity( (ENTITY_INSTANCE *)timer->owner, "%s\r\n", timer->end_message );
            ((ENTITY_INSTANCE *)timer->owner)->socket->bust_prompt = TRUE;
         }
         delete_timer( timer );
         break;
      case TIMER_DAMAGE:
      {
         DAMAGE *dmg = (DAMAGE *)timer->owner;
         if( timer->end_message && timer->end_message[0] != '\0' )
         {
            if( dmg->attacker )
               text_to_entity( dmg->attacker, "%s\r\n", timer->end_message );
            if( dmg->victim )
               text_to_entity( dmg->victim, "%s\r\n", timer->end_message );
         }
         free_damage( dmg ); /* free dmg takes care of the timer for us */
         return;
      }

   }
   free_timer( timer );
}

void new_timer( TIMER *timer )
{
   time_t now;
   int id = -1;
   int expire_time;

   if( timer->owner_type == TIMER_INSTANCE )
      id = ((ENTITY_INSTANCE *)timer->owner)->tag->id;

   time(&now);
   expire_time = now + ( timer->duration * 4 );

   if( !quick_query( "INSERT INTO `timers` VALUES( '%d', '%d', '%s', '%d', '%d', '%d', '%s', '%s', '%d', '%d' );",
      id, timer->owner_type, timer->key, expire_time, timer->frequency, timer->counter, timer->update_message, timer->end_message, timer->timer_type, timer->active ) )
      bug( "%s: could not add timer to the database.", __FUNCTION__ );
}

void load_instance_timers( ENTITY_INSTANCE *instance )
{
   TIMER *timer;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT * FROM `timers` WHERE owner_type=1 && owner=%d;", instance->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      timer = init_timer();
      db_load_timer( timer, &row );
      loaded_timer_check( timer );
   }
   DetachIterator( &Iter );

}

void db_load_timer( TIMER *timer, MYSQL_ROW *row )
{
   int id, duration, counter = 0;
   time_t now, expiration;

   id = atoi( (*row)[counter++] );
   timer->owner_type = atoi( (*row)[counter++] );
   switch( timer->owner_type )
   {
      default: break;
      case TIMER_INSTANCE:
         timer->owner = get_instance_by_id( id );
         break;
   }
   time(&now);
   expiration = atoi( (*row)[counter++] );
   if( ( duration = expiration - now ) <= 0 )
      timer->duration = 0;
   else
      duration *= 4;
   timer->frequency = atoi( (*row)[counter++] );
   timer->counter = atoi( (*row)[counter++] );
   timer->update_message = strdup( (*row)[counter++] );
   timer->end_message = strdup( (*row)[counter++] );
   timer->timer_type = atoi( (*row)[counter++] );
   timer->active = (bool)atoi( (*row)[counter++] );
}

void loaded_timer_check( TIMER *timer )
{
   if( timer->duration < 1 )
   {
      end_timer( timer );
      return;
   }
   own_timer( timer );
   if( timer->active )
      start_timer( timer );
   else
      pause_timer( timer );
   return;
}

void delete_timer( TIMER *timer )
{
   int owner = -1;
   switch( timer->owner_type )
   {
      default: break;
      case TIMER_INSTANCE:
         owner = ((ENTITY_INSTANCE *)timer->owner)->tag->id;
         break;
   }

   if( !quick_query( "DELETE FROM `timers` WHERE owner_type=%d AND owner=%d AND key='%s';", timer->owner_type, owner, timer->key ) )
      bug( "%s: could not delete timer from database.", __FUNCTION__ );
}

/* setters */
void set_melee_timer( ENTITY_INSTANCE *instance, bool message )
{
   TIMER *timer;

   if( CHECK_MELEE( instance ) != 0 )
      return;

   timer = init_timer();
   timer->duration = get_auto_cd( instance );
   set_timer_owner( timer, instance, TIMER_INSTANCE );
   timer->timer_type = TT_COOLDOWN;
   timer->key = strdup( MELEE_KEY );
   if( message )
      timer->end_message = strdup( MELEE_CD_MSG );
   start_timer( timer );
}

void own_timer( TIMER *timer )
{
   if( timer->owner_type == TIMER_NO_OWNER )
   {
      bug( "%s: cannot own an unknown type timer", __FUNCTION__ );
      return;
   }
   switch( timer->owner_type )
   {
      default: bug( "%s: bad owner type.", __FUNCTION__ ); return;
      case TIMER_MUD:
         new_timer( timer );
      case TIMER_DAMAGE:
         break;
      case TIMER_INSTANCE:
         AttachToList( timer, ((ENTITY_INSTANCE *)timer->owner)->timers );
         new_timer( timer );
         break;
   }
   return;
}

void unown_timer( TIMER *timer )
{
   switch( timer->owner_type )
   {
      default: bug( "%s: bad owner type.", __FUNCTION__ ); return;
      case TIMER_NO_OWNER:
         return;
      case TIMER_MUD:
         break;
      case TIMER_INSTANCE:
         DetachFromList( timer, ((ENTITY_INSTANCE *)timer->owner)->timers );
         break;
      case TIMER_DAMAGE:
         break;
   }
   timer->owner = NULL;
   pause_timer( timer );
   timer->owner_type = TIMER_NO_OWNER;
   return;
}

/* monitor */
void timer_monitor( void )
{
   TIMER *timer;
   ITERATOR Iter;

   AttachIterator( &Iter, timer_queue );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
   {
      if( timer->frequency == ++timer->counter )
      {
         switch( timer->owner_type )
         {
            default: bug( "%s: bad owner timer.", __FUNCTION__ ); break;
            case TIMER_NO_OWNER:
               bug( "%s: timer_no_owner somehow in the active queue.", __FUNCTION__ );
               break;
            case TIMER_MUD:
               if( timer->update_message && timer->update_message[0] != '\0' );
                  /* echo echo echo */
               break;
            case TIMER_INSTANCE:
               text_to_entity( (ENTITY_INSTANCE *)timer->owner, timer->update_message );
               break;
            case TIMER_DAMAGE:
            {
               DAMAGE *dmg = (DAMAGE *)timer->owner;
               if( dmg->attacker )
                  text_to_entity( dmg->attacker, timer->update_message );
               if( dmg->victim )
                  text_to_entity( dmg->victim, timer->update_message );
               handle_damage( dmg );
               break;
            }
         }
         timer->counter = 0;
      }
      if( ( timer->duration -= 1 ) == 0 )
         end_timer( timer );
   }
   DetachIterator( &Iter );
}

/* inlines */

/* setters */
inline void set_timer_owner( TIMER *timer, void *owner, TIMER_OWNER_TYPES type )
{
   if( timer->owner_type != TIMER_NO_OWNER )
      unown_timer( timer );

   timer->owner = owner;
   timer->owner_type = type;
   own_timer( timer );
   return;
}

/* getters */

inline TIMER *get_timer( const char *key )
{
   TIMER *timer;
   if( ( timer = get_active_timer( key ) ) == NULL )
      timer = get_inactive_timer( key );
   return timer;
}

inline TIMER *get_mud_timer( const char *key )
{
   TIMER *timer;
   if( ( timer = get_timer_from_list_by_key_and_type( key, TIMER_MUD, timer_queue ) ) == NULL )
      timer = get_timer_from_list_by_key_and_type( key, TIMER_MUD, paused_timer_queue );
   return timer;
}

inline TIMER *get_timer_from_list_by_key( const char *key, LLIST *list )
{
   TIMER *timer;
   ITERATOR Iter;
   AttachIterator( &Iter, list );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( timer->key, key ) )
         break;
   DetachIterator( &Iter );
   return timer;
}

inline TIMER *get_timer_from_list_by_key_and_type( const char *key, TIMER_OWNER_TYPES type, LLIST *list )
{
   TIMER *timer;
   ITERATOR Iter;
   AttachIterator( &Iter, list );
   while( ( timer = (TIMER *)NextInList( &Iter ) ) != NULL )
      if( timer->timer_type == type && !strcmp( timer->key, key ) )
         break;
   DetachIterator( &Iter );
   return timer;
}

inline int check_timer( const char *key )
{
   TIMER *timer;
   ITERATOR Iter;
   int time = 0;
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

inline int check_timer_instance( ENTITY_INSTANCE *instance, const char *key )
{
   TIMER *timer;
   ITERATOR Iter;
   int time = 0;
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

