/* methods pertaining to targets written by Davenge */

#include "mud.h"

TARGET_DATA *init_target( void )
{
   TARGET_DATA *target;

   CREATE( target, TARGET_DATA, 1 );
   target->target = NULL;
   target->type = -1;
   return target;
}

void free_target( TARGET_DATA *target )
{
   target->target = NULL;
   FREE( target );
}

inline void set_target_f( TARGET_DATA *target, ENTITY_FRAMEWORK *frame )
{
   target->target = frame;
   target->type = TARGET_FRAMEWORK;
}

inline void set_target_i( TARGET_DATA *target, ENTITY_INSTANCE *instance )
{
   target->target = instance;
   target->type = TARGET_INSTANCE;
}
