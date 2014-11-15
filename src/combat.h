/* header file for the combat.c library written by Davenge */

extern LLIST *damage_queue; /* queue of sent damages */

typedef enum
{
   HIT_UNKNOWN = -1, HIT_SUCCESS, HIT_DODGED, HIT_PARRIED, HIT_MISSED, MAX_CBT_RET
} cbt_ret;

typedef enum
{
   DMG_UNKNOWN = -1, DMG_MELEE, MAX_DMG_TYPE
} DMG_SRC;

struct damage_data
{
   ENTITY_INSTANCE *attacker;
   ENTITY_INSTANCE *victim;
   void *dmg_src;
   DMG_SRC type;
   int amount;
   bool crit;
   sh_int duration; /* total time this damage stays alive in pulses */
   sh_int frequency; /* when ( pcounter == frequency ) do damage */
   sh_int pcounter; /* inc every iteration, when ( pcounter == frequnecy ) pcounter = 0 */
   /* bitvector composition */
   DAMAGE *additional_dmg;
};

/* creation */
DAMAGE *init_damage	( void );
void    free_damage	( DAMAGE *dmg );

/* actions */
void	prep_melee_atk	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
void    prep_melee_dmg	( DAMAGE *dmg );
bool	receive_damage	( DAMAGE *dmg );

cbt_ret melee_attack	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool	send_damage	( DAMAGE *dmg );

/* checkers */
bool	does_check	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, const char *does );

/* monitor */
void damage_monitor ( void );
void handle_damage( DAMAGE *dmg );
void combat_message ( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim, DAMAGE *dmg, cbt_ret status );

/* inlines */

/* creation */
extern inline void	free_damage_list( LLIST *damages );

/* utility */
extern inline void    add_damage( DAMAGE *dmg );
extern inline void    rem_damage( DAMAGE *dmg );

/* checkers */
extern inline bool is_dmg_queued( DAMAGE *dmg );
