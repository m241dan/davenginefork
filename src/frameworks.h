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
ENTITY_FRAMEWORK *init_eFramework( void );
int clear_eFramework( ENTITY_FRAMEWORK *frame );

/* deletion */
int free_eFramework( ENTITY_FRAMEWORK *frame );

/* i/o */
int load_eFramework( ENTITY_FRAMEWORK *frame, const char *name );
int new_eFramework( ENTITY_FRAMEWORK *frame );
void db_load_eFramework( ENTITY_FRAMEWORK *frame, MYSQL_ROW *row );

bool framework_list_has_by_id( LLIST *frameworks, int id );
