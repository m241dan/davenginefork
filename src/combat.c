/* combat.c is a library of combat specific functions written by Davenge */

#include "mud.h"

ch_ret melee_attack( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim )
{
   return HIT_SUCCESS;
}

bool dodge_dodge( ENTITY_INSTANCE *attack, ENTITY_INSTANCE *victim )
{
   return FALSE;
}

bool does_parry( ENTITY_INSTANCE *attack, ENTITY_INSTANCE *victim )
{
   return FALSE;
}

