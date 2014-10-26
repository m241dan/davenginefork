/* header file for target.c written by Davenge */

struct target_data
{
   void *target;
   TARGET_TYPE type;
};

#define NO_TARGET( instance ) ( (instance->target->type) == -1 ? FALSE : TRUE )

TARGET_DATA *init_target( void );
void free_target( TARGET_DATA *target );

extern inline void set_target_none( TARGET_DATA *target );
extern inline void set_target_f( TARGET_DATA *target, ENTITY_FRAMEWORK *frame );
extern inline void set_target_i( TARGET_DATA *target, ENTITY_INSTANCE *instance );
