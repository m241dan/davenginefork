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

int new_specification( SPECIFICATION *spec );
int add_spec_to_framework( SPECIFICATION *spec, ENTITY_FRAMEWORK *frame );
int load_specifications_to_list( LLIST *spec_list, const char *owner );
int db_load_spec( SPECIFICATION *spec, MYSQL_ROW *row );
