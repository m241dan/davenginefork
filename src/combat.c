/* combat.c is a library of combat specific functions written by Davenge */

#include "mud.h"

/* creation */
DAMAGE *init_damage( void )
{
   DAMAGE *dmg;

   CREATE( dmg, DAMAGE, 1 );
   dmg->attacker = NULL;
   dmg->victim = NULL;
   dmg->dmg_src = NULL;
   dmg->type = DMG_UNKNOWN;
   dmg->amount = 0;
   dmg->timer = init_timer();
   set_timer_owner( dmg->timer, dmg, TIMER_DAMAGE );
   dmg->timer->key = strdup( "dmg key" );
   return dmg;
}

void free_damage( DAMAGE *dmg )
{
   if( is_dmg_queued( dmg ) )
      rem_damage( dmg );
   free_timer( dmg->timer );
   dmg->attacker = NULL;
   dmg->victim = NULL;
   dmg->dmg_src = NULL;
   FREE( dmg );
   return;
}


/* actions */
void prep_melee_atk( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   SPECIFICATION *spec;
   DAMAGE *dmg;
   const char *path;
   int top = lua_gettop( lua_handle );

   if( ( spec = has_spec( attacker, "prepMeleeTimer" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "prepMeleeTimer" );
   push_instance( attacker, lua_handle );

   dmg = init_damage();
   set_dmg_attacker( dmg, attacker );
   set_dmg_victim( dmg, victim );
   set_dmg_src( dmg, attacker, DMG_MELEE );

   push_timer( dmg->timer, lua_handle );
   if( lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) )
   {
      bug( "%s: failed to call onMeleeAttack script with path %s.", __FUNCTION__, path );
      lua_settop( lua_handle, top );
      return;
   }
   lua_settop( lua_handle, top );
   send_damage( dmg );
   return;
}

void prep_melee_dmg( DAMAGE *dmg )
{
   SPECIFICATION *spec;
   const char *path;
   int top = lua_gettop( lua_handle );

   if( ( spec = has_spec( dmg->attacker, "prepMeleeDamage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "prepMeleeDamage" );
   push_instance( dmg->attacker, lua_handle );
   push_damage( dmg, lua_handle );
   if( lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) )
   {
      bug( "%s: %s.", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
      dmg->amount = 0;
   }
   lua_settop( lua_handle, top );
   return;
}

bool receive_damage( DAMAGE *dmg )
{
   SPECIFICATION *spec;
   const char *path;
   int top = lua_gettop( lua_handle );

   if( ( spec = has_spec( dmg->victim, "onReceiveDamage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "onReceiveDamage" );
   push_instance( dmg->victim, lua_handle );
   push_damage( dmg, lua_handle );
   if( lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) )
   {
      bug( "%s: %s.", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
      dmg->amount = 0;
   }
   lua_settop( lua_handle, top );
   return TRUE;
}


cbt_ret melee_attack( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   if( DODGE_ON && does_check( attacker, victim, "dodgeChance" ) )
      return HIT_DODGED;

   if( PARRY_ON && does_check( attacker, victim, "parryChance" ) )
      return HIT_PARRIED;

   if( MISS_ON && does_check( attacker, victim, "missChance" ) )
      return HIT_MISSED;

   return HIT_SUCCESS;
}

bool send_damage( DAMAGE *dmg )
{
   /* safeties */
   if( !dmg->attacker->primary_dmg_received_stat && !dmg->attacker->builder )
   {
      bug( "%s: %s trying to do damage at the utility level but it cannot receive any in return.", __FUNCTION__, instance_name( dmg->attacker ) );
   }
   if( !dmg->victim->primary_dmg_received_stat )
   {
      bug( "%s: %s trying to do damage at the utility level to something that can't take damage.", __FUNCTION__, instance_name( dmg->attacker ) );
      return FALSE;
   }
   add_damage( dmg );
   return TRUE;
}

/* checkers */
bool does_check( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, const char *does )
{
   SPECIFICATION *spec;
   const char *path;
   int chance, top = lua_gettop( lua_handle );

   if( ( spec = has_spec( victim, does ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, does );
   push_instance( attacker, lua_handle );
   push_instance( victim, lua_handle );
   if( lua_pcall( lua_handle, 2, LUA_MULTRET, 0 ) )
   {
      bug( "%s: failed to call does_check script %s path: %s", __FUNCTION__, does, path );
      lua_settop( lua_handle, top );
      return FALSE;
   }

   if( lua_type( lua_handle, -1 ) != LUA_TNUMBER )
   {
      bug( "%s: bad value returned by lua.", __FUNCTION__ );
      lua_settop( lua_handle, top );
      return FALSE;
   }
   chance = lua_tonumber( lua_handle, -1 );
   if( number_percent() < chance )
      return TRUE;
   return FALSE;
}

/* utility */

const char *compose_dmg_key( DAMAGE *dmg )
{
   static char buf[MAX_BUFFER];
   memset( &buf[0], 0, sizeof( buf ) );

   if( dmg->attacker )
      strcat( buf, quick_format( "%s's ", instance_name( dmg->attacker ) ) );

   if( dmg->type != DMG_UNKNOWN )
   {
      if( dmg->type == DMG_MELEE )
      {
         strcat( buf, "melee " );
         if( dmg->dmg_src != dmg->attacker )
            strcat( buf, quick_format( "with %s", instance_name( (ENTITY_INSTANCE *)dmg->dmg_src ) ) );
       }
   }

   if( dmg->victim )
      strcat( buf, quick_format( "against %s", instance_name( dmg->victim ) ) );

   buf[strlen( buf )] = '\0';
   return buf;
}

void handle_damage( DAMAGE *dmg )
{
   cbt_ret status;
   switch( dmg->type )
   {
      default: return;
      case DMG_MELEE:
         status = melee_attack( dmg->attacker, dmg->victim );
         break;
   }
   if( status != HIT_SUCCESS )
   {
      dmg->amount = 0;
      combat_message( dmg->attacker, dmg->victim, dmg, status );
      return;
   }

   /* run the necessary lua scripts */
   if( dmg->type == DMG_MELEE )
      prep_melee_dmg( dmg );

   /* damage is received universally via lua, can be global script or local */
   receive_damage( dmg );

   if( dmg->amount )
      do_damage( dmg->victim, dmg );

   /* this is just a test message */
   if( !get_stat_current( dmg->victim->primary_dmg_received_stat ) )
      text_to_entity( dmg->attacker, "You killed %s.\r\n", instance_short_descr( dmg->victim ) );

   /* actual combat messaging */
   combat_message( dmg->attacker, dmg->victim, dmg, status );
}

void combat_message( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, DAMAGE *dmg, cbt_ret status )
{
   SPECIFICATION *spec;
   const char *path;
   char msg_attacker[MAX_BUFFER], msg_victim[MAX_BUFFER], msg_room[MAX_BUFFER];
   int top = lua_gettop( lua_handle );

   if( ( spec = has_spec( attacker, "combatMessage" ) ) != NULL && spec->value > 0 )
      path = get_script_path_from_spec( spec );
   else
      path = "../scripts/settings/combat.lua";

   prep_stack( path, "combatMessage" );
   push_instance( attacker, lua_handle );
   push_instance( victim, lua_handle );
   push_damage( dmg, lua_handle );
   lua_pushnumber( lua_handle, (int)status );
   if( lua_pcall( lua_handle, 4, LUA_MULTRET, 0 ) )
   {
      bug( "%s: could not get messages for combatMessage at path: %s", __FUNCTION__, path );
      bug( "%s: error: %s", __FUNCTION__, lua_tostring( lua_handle, -1 ) );
      lua_settop( lua_handle, top );
      return;
   }

   mud_printf( msg_attacker, "%s\r\n", lua_tostring( lua_handle, -3 ) );
   mud_printf( msg_victim, "%s\r\n", lua_tostring( lua_handle, -2 ) );
   mud_printf( msg_room, "%s\r\n", lua_tostring( lua_handle, -1 ) );
   lua_settop( lua_handle, top );

   text_to_entity( attacker, msg_attacker );
   text_to_entity( victim, msg_victim );
   if( attacker->contained_by == victim->contained_by )
      text_around_entity( attacker->contained_by, 2, msg_room, attacker, victim );
   else
   {
      text_around_entity( attacker->contained_by, 1, msg_room, attacker );
      text_around_entity( victim->contained_by, 1, msg_room, victim );
   }
   return;
}

/* inlines */

/* creation */
inline void free_damage_list( LLIST *damages )
{
   DAMAGE *dmg;
   ITERATOR Iter;
   AttachIterator( &Iter, damages );
   while( ( dmg = (DAMAGE *)NextInList( &Iter ) ) != NULL )
   {
      free_damage( dmg );
      DetachFromList( dmg, damages );
   }
   DetachIterator( &Iter );
}

/* utility */
inline void add_damage( DAMAGE *dmg )
{
   if( !dmg->attacker || !dmg->victim )
   {
      bug( "%s: could not add dmg, bad attacker or victim.", __FUNCTION__ );
      return;
   }
   AttachToList( dmg, damage_queue );
   AttachToList( dmg, dmg->attacker->damages_sent );
   AttachToList( dmg, dmg->victim->damages_received );
}

inline void rem_damage( DAMAGE *dmg )
{
   DetachFromList( dmg, damage_queue );
   if( !dmg->attacker || !dmg->victim )
   {
      bug( "%s: could not rem dmg, bad attacker or victim.", __FUNCTION__ );
      return;
   }
   DetachFromList( dmg, dmg->attacker->damages_sent );
   DetachFromList( dmg, dmg->victim->damages_received );
}

/* checker */
inline bool is_dmg_queued( DAMAGE *dmg )
{
   DAMAGE *dmg_q;
   ITERATOR Iter;
   AttachIterator( &Iter, damage_queue );
   while( ( dmg_q = (DAMAGE *)NextInList( &Iter ) ) != NULL )
      if( dmg == dmg_q )
         break;
   DetachIterator( &Iter );
   if( dmg_q ) return TRUE;
   return FALSE;

}

/* setters */

inline void set_dmg_attacker( DAMAGE *dmg, ENTITY_INSTANCE *attacker )
{
   dmg->attacker = attacker;
   if( dmg->type == DMG_UNKNOWN ) return;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_victim( DAMAGE *dmg, ENTITY_INSTANCE *victim )
{
   dmg->victim = victim;
   if( dmg->type == DMG_UNKNOWN ) return;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_type( DAMAGE *dmg, DMG_SRC type )
{
   dmg->type = type;
   FREE( dmg->timer->key );
   dmg->timer->key = strdup( compose_dmg_key( dmg ) );
}

inline void set_dmg_src( DAMAGE *dmg, void *dmg_src, DMG_SRC type )
{
   dmg->dmg_src = dmg_src;
   set_dmg_type( dmg, type );
}
