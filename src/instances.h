/* instaces.h the headerfile for allthings instanced written by Davenge */

struct entity_instance
{
   ID_TAG *tag;
   bool loaded;
   bool live;
   bool builder;
   sh_int level;

   LLIST *contents;
   LLIST *contents_sorted[MAX_QUICK_SORT];
   LLIST *specifications;

   ENTITY_FRAMEWORK *framework;

   ENTITY_INSTANCE *contained_by;
   int contained_by_id; /* loading considerations */

   D_SOCKET *socket;
   ACCOUNT_DATA *account;
   LLIST *commands;
};

ENTITY_INSTANCE *init_eInstance( void );
int clear_eInstance( ENTITY_INSTANCE *eInstance );
int free_eInstance( ENTITY_INSTANCE *eInstance );
int clear_ent_contents( ENTITY_INSTANCE *eInstance );


ENTITY_INSTANCE *init_builder( void );

ENTITY_INSTANCE *load_eInstance_by_query( const char *query );

ENTITY_INSTANCE *get_instance_by_id( int id );
ENTITY_INSTANCE *get_active_instance_by_id( int id );
ENTITY_INSTANCE *load_eInstance_by_id( int id );

ENTITY_INSTANCE *get_instance_by_name( const char *name );
ENTITY_INSTANCE *get_active_instance_by_name( const char *name );
ENTITY_INSTANCE *load_eInstance_by_name( const char *name );

ENTITY_INSTANCE *full_load_eFramework( ENTITY_FRAMEWORK *frame );
void full_load_instance( ENTITY_INSTANCE *instance );

int new_eInstance( ENTITY_INSTANCE *eInstance );
void db_load_eInstance( ENTITY_INSTANCE *eInstance, MYSQL_ROW *row );
void entity_from_container( ENTITY_INSTANCE *entity );
void entity_to_world( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void attach_entity_to_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void detach_entity_from_contents( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_to_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );
void entity_from_contents_quick_sort( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *container );

ENTITY_INSTANCE *instance_list_has_by_id( LLIST *instance_list, int id );
ENTITY_INSTANCE *instance_list_has_by_name( LLIST *instance_list, const char *name );
ENTITY_INSTANCE *instance_list_has_by_short_prefix( LLIST *instance_list, const char *name );

ENTITY_INSTANCE *eInstantiate( ENTITY_FRAMEWORK *frame );

ENTITY_INSTANCE *create_room_instance( const char *name );
ENTITY_INSTANCE *create_exit_instance( const char *name, int dir );
ENTITY_INSTANCE *create_mobile_instance( const char *name );

void move_create( ENTITY_INSTANCE *entity, ENTITY_FRAMEWORK *exit_frame, char *arg );
bool should_move_create( ENTITY_INSTANCE *entity, char *arg );


const char *instance_name( ENTITY_INSTANCE *instance );
const char *instance_short_descr( ENTITY_INSTANCE *instance );
const char *instance_long_descr( ENTITY_INSTANCE *instance );
const char *instance_description( ENTITY_INSTANCE *instance );

int text_to_entity( ENTITY_INSTANCE *entity, const char *fmt, ... );
int builder_prompt( D_SOCKET *dsock );
int show_ent_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_contents_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_exits_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_mobiles_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_objects_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int show_ent_rooms_to_ent( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *viewing );
int move_entity( ENTITY_INSTANCE *entity, ENTITY_INSTANCE *move_to );

void entity_goto( void *passed, char *arg );
void entity_instance( void *passed, char *arg );
void entity_look( void *passed, char *arg );
void entity_inventory( void *passed, char *arg );
void entity_drop( void *passed, char *arg );
void entity_get( void *passed, char *arg );
void entity_quit( void *passed, char *arg );
void entity_create( void *passed, char *arg );
void entity_edit( void *passed, char *arg );
void entity_iedit( void *passed, char *arg );
void entity_load( void *passed, char *arg );
void entity_north( void *passed, char *arg );
void entity_south( void *passed, char *arg );
void entity_west( void *passed, char *arg );
void entity_east( void *passed, char *arg );
void entity_up( void *passed, char *arg );
void entity_down( void *passed, char *arg );
void entity_chat( void *passed, char *arg );
