#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

/* event_game_tick is just to show how to make global events
 * which can be used to update the game.
 */

bool event_game_tick(EVENT_DATA *event)
{
   ITERATOR Iter;
   ENTITY_INSTANCE *instance;

   AttachIterator(&Iter, eInstances_list);
   while ((instance = (ENTITY_INSTANCE *) NextInList(&Iter)) != NULL)
      text_to_entity( instance, "Tick! The event queue is working.\r\n" );
   DetachIterator(&Iter);

   event = alloc_event();
   event->fun = &event_game_tick;
   event->type = EVENT_GAME_TICK;
   add_event_game(event, 10 * 60 * PULSES_PER_SECOND);

   return FALSE;
}

/*
bool event_mobile_save(EVENT_DATA *event)
{
  D_MOBILE *dMob;

  * Check to see if there is an owner of this event.
   * If there is no owner, we return TRUE, because
   * it's the safest - and post a bug message.
   *
  if ((dMob = event->owner.dMob) == NULL)
  {
    bug("event_mobile_save: no owner.");
    return TRUE;
  }

  * save the actual player file *
  save_player(dMob);

  * enqueue a new event to save the pfile in 2 minutes *
  event = alloc_event();
  event->fun = &event_mobile_save;
  event->type = EVENT_MOBILE_SAVE;
  add_event_mobile(event, dMob, 2 * 60 * PULSES_PER_SECOND);

  return FALSE;
}
*/
bool event_socket_idle(EVENT_DATA *event)
{
  D_SOCKET *dSock;

  /* Check to see if there is an owner of this event.
   * If there is no owner, we return TRUE, because
   * it's the safest - and post a bug message.
   */
  if ((dSock = (D_SOCKET *)event->owner) == NULL)
  {
    bug("event_socket_idle: no owner.");
    return TRUE;
  }

  /* tell the socket that it has idled out, and close it */
  text_to_socket(dSock, "You have idled out...\n\n\r");
  close_socket(dSock, FALSE);

  /* since we closed the socket, all events owned
   * by that socket has been dequeued, and we need
   * to return TRUE, so the caller knows this.
   */
  return TRUE;
}

bool event_instance_lua_callback( EVENT_DATA *event )
{
   ENTITY_INSTANCE *instance = (ENTITY_INSTANCE *)event->owner;
   ENTITY_INSTANCE *arg_entity;
   void *content;
   ITERATOR Iter;
   int counter = 0;

   prep_stack( get_frame_script_path( instance->framework ), event->argument );
   if( SizeOfList( event->lua_args ) > 0 )
   {
      AttachIterator( &Iter, event->lua_args );
      while( ( content = NextInList( &Iter ) ) != NULL )
      {
         switch( tolower( event->lua_cypher[counter++] ) )
         {
            case 's':
               lua_pushstring( lua_handle, (const char *)content );
               break;
            case 'n':
               lua_pushnumber( lua_handle, *((int *)content) );
               break;
            case 'i':
               if( ( arg_entity = get_active_instance_by_id( *((int *)content ) ) ) == NULL )
               {
                  bug( "%s: instance with ID:%d is no longer active.", __FUNCTION__, *((int *)content ) );
                  lua_pushnil( lua_handle );
                  break;
               }
               push_instance( arg_entity, lua_handle );
               break;
         }
      }
      DetachIterator( &Iter );
   }
   lua_pcall( lua_handle, strlen( event->lua_cypher ), LUA_MULTRET, 0 );

   return FALSE;
}
