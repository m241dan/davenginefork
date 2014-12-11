/* the elements library written by Davenge */

#include "mud.h"

/* creation | deletion */
ELEMENT_FRAMEWORK *init_element_frame( void )
{
   ELEMENT_FRAMEWORK *frame;
   CREATE( frame, ELEMENT_FRAMEWORK, 1 );
   frame->name = strdup( "null" );
   frame->strong_against = AllocList();
   frame->weak_against = AllocList();
   frame->composition = AllocList();
   return frame;
}

ELEMENT_INFO *init_element_info( void )
{
   ELEMENT_INFO *element;
   CREATE( element, ELEMENT_INFO, 1 );
   element->owner = NULL;
   element->frame = NULL;
   return element;
}

void free_element_info( ELEMENT_INFO *element )
{
   element->owner = NULL;
   element->frame = NULL;
   FREE( element );
}

COMPOSITION *init_composition( void )
{
   COMPOSITION *comp;
   CREATE( comp, COMPOSITION, 1 );
   comp->frame = NULL;
   return comp;
}

void free_composition( COMPOSITION *comp )
{
   comp->owner = NULL;
   comp->frame = NULL;
   FREE( comp );
}

/* utility */

