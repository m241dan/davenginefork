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
   dmg->duration = -1;
   dmg->frequency = -1;
   dmg->pcounter = -1;
   return dmg;
}

void free_damage( DAMAGE *dmg )
{
   rem_damage( dmg );
   dmg->attacker = NULL;
   dmg->victim = NULL;
   dmg->dmg_src = NULL;
   FREE( dmg );
   return;
}


/* actions */
ch_ret melee_attack( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   return HIT_SUCCESS;
}

bool send_damage( DAMAGE *dmg )
{
   return FALSE;
}

bool receive_damage( DAMAGE *dmg )
{
   return FALSE;
}


/* checkers */
bool dodge_dodge( ENTITY_INSTANCE *attack, ENTITY_INSTANCE *victim )
{
   return FALSE;
}

bool does_parry( ENTITY_INSTANCE *attack, ENTITY_INSTANCE *victim )
{
   return FALSE;
}

/* monitor */
void damage_monitor( void )
{
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
