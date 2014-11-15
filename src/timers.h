/* header file for the timers.c library written by Davenge */

extern LLIST *timer_queue;
extern LLIST *paused_timer_queue;

typedef enum
{
   TIMER_NO_OWNER = -1, TIMER_MUD, TIMER_INSTANCE, MAX_TIMER_OWNER
} TIMER_OWNER_TYPES;

typedef enum
{
   TT_UNKNOWN, TT_COOLDOWN, MAX_TT
} TIMER_TYPES;

struct timer
{
   void *owner;
   char owner_type;
   char *key;
   double duration;
   char *end_message;
   char type;
};

/* creation */
TIMER *init_timer( void );
void free_timer( TIMER *timer );

void start_timer( TIMER *timer );
void pause_timer( TIMER *timer );
void end_timer( TIMER *timer );

/* setter */
void set_melee_timer( ENTITY_INSTANCE *instance, bool message );

/* monitor */
void timer_monitor( void );

/* inlines */
/* checkers */
extern inline double check_timer( const char *key );
extern inline double check_timer_instance( ENTITY_INSTANCE *instance, const char *key );
#define MELEE_KEY "melee attack"
#define MELEE_CD_MSG "You may attack again."
#define CHECK_MELEE( instance ) ( check_timer_instance( (instance), MELEE_KEY ) )
