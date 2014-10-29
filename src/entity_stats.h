/* the header file for all methods in entity_stats.c written by Davenge */

extern LLIST *stat_frameworks;

struct stat_framework
{
   ID_TAG *tag;
   char *name;
   int softcap;
   int hardcap;
   int softfloor;
   int hardfloor;

};

struct state_instance
{
   ENTITY_INSTANCE *owner;
   STAT_FRAMEWORK *framework;
   int perm_stat;
   int mod_stat;
}

STAT_FRAMEWORK *init_stat_framework( void );
void *free_stat_framework( STAT_FRAMEWORK *fstat );

STAT_INSTANCE *init_stat( void );
void *free_stat( STAT_INSTANCE *stat );


