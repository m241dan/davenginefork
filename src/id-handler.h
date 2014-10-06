/* id-handlers.h written by Davenge */

struct id_handler
{
   int type;
   char *name;
   int top_id;
   bool can_recycle;
   LLIST *recycled_ids;
};

struct id_tag
{
   int type;
   int id;
   char *created_by;
   char *created_on;
   char *modified_by;
   char *modified_on;
};

extern ID_HANDLER *handlers[MAX_ID_HANDLER];

ID_HANDLER *init_handler( void );
int clear_handler( ID_HANDLER *handler );
int free_handler( ID_HANDLER *handler );

ID_TAG *init_tag( void );
int clear_tag( ID_TAG *tag );
int free_tag( ID_TAG *tag );
int delete_tag( ID_TAG *tag );
int new_tag( ID_TAG *tag, const char *creator );
int db_load_tag( ID_TAG *tag, MYSQL_ROW *row );
int update_tag( ID_TAG *tag, const char *effector, ... );

int load_id_handlers( void );
int load_recycled_ids( void );
int get_new_id( int type );
int get_potential_id( int type );

ID_TAG *copy_tag( ID_TAG *tag );

int *build_workspace_id_table( LLIST *workspace_list );
int *build_instance_id_table( LLIST *instance_list );
int *build_framework_id_table( LLIST *framework_list );
int get_id_table_position( int *table, int id );
void print_table( int *table );
