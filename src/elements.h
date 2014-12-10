/* header file for the elements library writte by Davenge */

#define COMP_OWNER_NONE     0
#define COMP_OWNER_ELEMENT  1
#define COMP_OWNER_FRAME    2
#define COMP_OWNER_INSTANCE 3

extern LLIST element_frameworks;

struct element_framework
{
   char *name;
   LLIST *strong_against;
   LLIST *weak_against;
   LLIST *composition;
};

struct composition
{
   void *owner;
   int ownertype;
   ELEMENT_FRAMEWORK *frame;
   int amount;
};

struct element_info
{
   ENTITY_INSTANCE *owner;
   ELEMENT_FRAMEWORK *frame;
   int pen;
   int res;
   int potency;
};

/* creation | deletion */
ELEMENT_FRAMEWORK *init_element_frame( void );
void free_element_frame( ELEMENT_FRAMEWORK *element );

ELEMENT_INFO *init_element_info( void );
void free_element_info( ELEMENT_INFO *element );

COMPOSITION *init_composition( void );
void free_composition( COMPOSITION *comp );

/* utility */
void load_elements_table( void );
void reload_elements_table( void );
ELEMENT_FRAMEWORK *get_element_framework( const char *name );
