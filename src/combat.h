/* header file for the combat.c library written by Davenge */

typedef enum
{
   HIT_SUCCESS, HIT_DODGED, HIT_PARRIED, MAX_CH_RET
} ch_ret;

typedef enum
{
   DMG_MELEE, MAX_DMG_TYPE
} DMG_SRC;

struct damage_data
{
   ENTITY_INSTANCE *attacker;
   ENTITY_INSTANCE *victim;
   void *dmg_src;
   DMG_SRC type;
   int amount;
   sh_int duration; /* total time this damage stays alive in pulses */
   sh_int occur_on_pulse; /* when ( pulse_counter == occur_on_pulse ) do damage */
   sh_int pulse_counter; /* inc every iteration, when ( pulse_counter == occur_on_pulse ) pulse_counter = 0 */
   /* bitvector composition */
};

ch_ret  melee_attack	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool    does_dodge	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool    does_parry	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );

bool	send_damage	( DAMAGE *dmg );
bool	receive_damage	( DAMAGE *dmg );
void    damage_monitor	( void );
