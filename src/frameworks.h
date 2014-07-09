/* frameworks.h written by Davenge */

struct entity_framework
{
   ID_TAG *tag;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;

   LLIST *contents;
   LLIST *specifications;
};

/* creation */
ENTITY_FRAMEWORK *init_framework( void );
int clear_eFramework( ENTITY_FRAMEWORK *frame );

/* deletion */
int free_eFramework( ENTITY_FRAMEWORK *frame );

/* i/o */
int load_eFramework( ENTITY_FRAMEWORK *frame, const char *name );
int new_eFramework( ENTITY_FRAMEWORK *frame );

