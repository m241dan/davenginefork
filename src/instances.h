/* instaces.h the headerfile for allthings instanced written by Davenge */

struct entity_instance
{
   ID_TAG *tag;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;

   LLIST *contents;
   LLIST *specifications;
   ENTITY_FRAMEWORK *framework;
};

ENTITY_INSTANCE *init_eInstance( void );
int clear_eInstance( ENTITY_INSTANCE *eInstance );
int free_eInstance( ENTITY_INSTANCE *eInstance );

ENTITY_INSTANCE *load_eInstance_by_query( const char *query);

ENTITY_INSTANCE *get_instance_by_id( int id );
ENTITY_INSTANCE *get_active_instance_by_id( int id );
ENTITY_INSTANCE *load_eInstance_by_id( int id );

ENTITY_INSTANCE *get_instance_by_name( const char *name );
ENTITY_INSTANCE *get_active_instance_by_name( const char *name );
ENTITY_INSTANCE *load_eInstance_by_name( const char *name );

int new_eInstance( ENTITY_INSTANCE *eInstance );
void db_load_eInstance( ENTITY_INSTANCE *eInstance, MYSQL_ROW *row );

ENTITY_INSTANCE *instance_list_has_by_id( LLIST *instance_list, int id );
ENTITY_INSTANCE *instance_list_has_by_name( LLIST *instance_list, const char *name );

ENTITY_INSTANCE *eInstantiate( ENTITY_FRAMEWORK *frame );

const char *instance_name( ENTITY_INSTANCE *instance );
const char *instance_short_descr( ENTITY_INSTANCE *instance );
const char *instance_long_descr( ENTITY_INSTANCE *instance );
const char *instance_description( ENTITY_INSTANCE *instance );

