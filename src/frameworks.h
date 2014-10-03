/* frameworks.h written by Davenge */

struct entity_framework
{
   ID_TAG *tag;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;

   LLIST *fixed_contents; /* frameworks structs */
   LLIST *specifications;

   ENTITY_FRAMEWORK *inherits;

};

/* creation */
ENTITY_FRAMEWORK *init_eFramework( void );
int clear_eFramework( ENTITY_FRAMEWORK *frame );
int set_to_inherited( ENTITY_FRAMEWORK *frame );

/* deletion */
int free_eFramework( ENTITY_FRAMEWORK *frame );

/* i/o */
ENTITY_FRAMEWORK *load_eFramework_by_query( const char *fmt_query );

ENTITY_FRAMEWORK *get_framework_by_id( int id );
ENTITY_FRAMEWORK *get_active_framework_by_id( int id );
ENTITY_FRAMEWORK *load_eFramework_by_id( int id );

ENTITY_FRAMEWORK *get_framework_by_name( const char *name );
ENTITY_FRAMEWORK *get_active_framework_by_name( const char *name );
ENTITY_FRAMEWORK *load_eFramework_by_name( const char *name );

int new_eFramework( ENTITY_FRAMEWORK *frame );
void db_load_eFramework( ENTITY_FRAMEWORK *frame, MYSQL_ROW *row );
int load_fixed_possessions_to_list( LLIST *fixed_contents, int id );

ENTITY_FRAMEWORK *copy_framework( ENTITY_FRAMEWORK *frame, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance );
LLIST *copy_framework_list( LLIST *frameworks, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance );
void copy_frameworks_into_list( LLIST *frame_list, LLIST *copy_into_list, bool copy_id, bool copy_content, bool copy_specs, bool copy_inheritance );
ENTITY_FRAMEWORK *copy_framework_ndi( ENTITY_FRAMEWORK *frame, LLIST *frame_list );
void copy_framework_list_ndi( LLIST *frame_list, LLIST *copy_into_list );

ENTITY_FRAMEWORK *framework_list_has_by_id( LLIST *frameworks, int id );
ENTITY_FRAMEWORK *framework_list_has_by_name( LLIST *frameworks, const char *name );
bool live_frame( ENTITY_FRAMEWORK *frame );

ENTITY_FRAMEWORK *create_room_framework( const char *name );
ENTITY_FRAMEWORK *create_exit_framework( const char *name, int dir );
ENTITY_FRAMEWORK *create_mobile_framework( const char *name );
ENTITY_FRAMEWORK *create_inherited_framework( ENTITY_FRAMEWORK *inherit_from );
ENTITY_FRAMEWORK *entity_edit_selection( ENTITY_INSTANCE *entity, const char *arg );
ENTITY_FRAMEWORK *olc_edit_selection( INCEPTION *olc, const char *arg );

const char *chase_name( ENTITY_FRAMEWORK *frame );
const char *chase_short_descr( ENTITY_FRAMEWORK *frame );
const char *chase_long_descr( ENTITY_FRAMEWORK *frame );
const char *chase_description( ENTITY_FRAMEWORK *frame );

void add_frame_to_fixed_contents( ENTITY_FRAMEWORK *frame_to_add, ENTITY_FRAMEWORK *container );
void rem_frame_from_fixed_contents( ENTITY_FRAMEWORK *frame_to_rem, ENTITY_FRAMEWORK *container );
