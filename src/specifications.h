/* specifications.h is a header file written by Davenge */

extern const char *const spec_table[];

struct typSpec
{
   short type;
   int value;
   char *owner;
};

SPECIFICATION *init_specification( void );
int clear_specification( SPECIFICATION *spec );
int free_specification( SPECIFICATION *spec );
int specification_clear_list( LLIST *spec_list );

int new_specification( SPECIFICATION *spec );
int add_spec_to_framework( SPECIFICATION *spec, ENTITY_FRAMEWORK *frame );
int add_spec_to_instance( SPECIFICATION *spec, ENTITY_INSTANCE *instance );
int load_specifications_to_list( LLIST *spec_list, const char *owner );
int db_load_spec( SPECIFICATION *spec, MYSQL_ROW *row );

SPECIFICATION *spec_list_has_by_type( LLIST *spec_list, int type );
SPECIFICATION *spec_list_has_by_name( LLIST *spec_list, const char *name );
SPECIFICATION *has_spec( ENTITY_INSTANCE *entity, const char *spec_name );
SPECIFICATION *frame_has_spec( ENTITY_FRAMEWORK *frame, const char *spec_name );
int get_spec_value( ENTITY_INSTANCE *entity, const char *spec_name );
