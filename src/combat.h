/* header file for the combat.c library written by Davenge */

extern LLIST *damage_queue; /* queue of sent damages */

typedef enum
{
   HIT_SUCCESS, HIT_DODGED, HIT_PARRIED, MAX_CH_RET
} ch_ret;

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
ch_ret  melee_attack	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool send_damage ( DAMAGE *dmg );
bool receive_damage ( DAMAGE *dmg );

/* checkers */
bool    does_dodge	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );
bool    does_parry	( ENTITY_INSTANCE *attacker, ENTITY_INSTANCE *victim );

/* monitor */
void damage_monitor ( void );


/* inlines */

/* creation */
extern inline void	free_damage_list( LLIST *damages );

/* utility */
extern inline void    add_damage( DAMAGE *dmg );
extern inline void    rem_damage( DAMAGE *dmg );

/* checkers */
extern inline bool is_dmg_queued( DAMAGE *dmg );
