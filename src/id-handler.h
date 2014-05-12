/* id-handlers.h written by Davenge */

struct id_handler
{
   int type;
   char *name;
   int top_id;
   char *recycled_ids;
   bool can_recycle;
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
int recycle_tag( ID_TAG *tag );

int load_id_handlers( void );
int get_new_id( int type );
