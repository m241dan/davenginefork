/* the file containg functions pertainign to methods minus lua stuff written by Davenge */

#include "mud.h"

STAT_FRAMEWORK *init_stat_framework( void )
{
   STAT_FRAMEWORK *fstat;

   CREATE( fstat, STAT_FRAMEWORK, 1 );
   fstat->tag = init_tag();
   fstat->tag->type = ENTITY_STAT_FRAMEWORK_IDS;
   fstat->name = strdup( "null" );
   return fstat;
}

void free_stat_framework( STAT_FRAMEWORK *fstat )
{
   FREE( fstat->name );
   free_tag( fstat->tag );
   FREE( fstat );
   return;
}
void new_stat_framework( STAT_FRAMEWORK *fstat )
{
   if( !fstat )
   {
      bug( "%s: passed a NULL fstat.", __FUNCTION__ );
      return;
   }
   if( !strcmp( fstat->tag->created_by, "null" ) )
   {
      if( new_tag( fstat->tag, "system" ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return;
      }
   }

   if( !quick_query( "INSERT INTO `stat_frameworks` VALUES ( '%d', '%d', '%s', '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d' );",
      fstat->tag->id, fstat->tag->type, fstat->tag->created_by, fstat->tag->created_on, fstat->tag->modified_by, fstat->tag->modified_on,
      fstat->name, fstat->softcap, fstat->hardcap, fstat->softfloor, fstat->hardfloor ) )
      bug( "%s: could not add to database %s.", __FUNCTION__, fstat->name );

   return;
}

inline void new_stat_on_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame )
{
   quick_query( "INSERT INTO `entity_framework_stats` VALUES ( '%d', '%d' );", frame->tag->id, fstat->tag->id );
}

inline void add_stat_to_frame( STAT_FRAMEWORK *fstat, ENTITY_FRAMEWORK *frame )
{
   AttachToList( fstat, frame->stats );
   if( !strcmp( frame->tag->created_by, "null" ) )
      return;
   new_stat_on_frame( fstat, frame );
}

void db_load_stat_framework( STAT_FRAMEWORK *fstat, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( fstat->tag, row );
   fstat->name = strdup( (*row)[counter++] );
   fstat->softcap = atoi( (*row)[counter++] );
   fstat->hardcap = atoi( (*row)[counter++] );
   fstat->softfloor = atoi( (*row)[counter++] );
   fstat->hardfloor = atoi( (*row)[counter++] );
   return;
}

void load_framework_stats( ENTITY_FRAMEWORK *frame )
{
   STAT_FRAMEWORK *fstat;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   if( !frame )
      return;

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT statFrameworkID FROM `entity_framework_stats` WHERE entityFrameworkID=%d;", frame->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      fstat = get_stat_framework_by_id( atoi( row[0] ) );
      AttachToList( fstat, frame->stats );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}

STAT_INSTANCE *init_stat( void )
{
   STAT_INSTANCE *stat;

   CREATE( stat, STAT_INSTANCE, 1 );
   stat->owner = NULL;
   stat->framework = NULL;
   return stat;
}

void free_stat( STAT_INSTANCE *stat )
{
   stat->owner = NULL;
   stat->framework = NULL;
   FREE( stat );
   return;
}

void new_stat_instance( STAT_INSTANCE *stat )
{
   if( !stat )
   {
      bug( "%s: passed a NULL stat.", __FUNCTION__ );
      return;
   }
   if( !stat->owner )
   {
      bug( "%s: this stat has no owner.", __FUNCTION__ );
      return;
   }
   if( !stat->framework )
   {
      bug( "%s: this stat has no framework.", __FUNCTION__ );
      return;
   }
   if( !quick_query( "INSERT INTO `entity_stats` VALUES ( '%d', '%d', '%d', '%d' );",
      stat->framework->tag->id, stat->owner->tag->id, stat->perm_stat, stat->mod_stat ) )
      bug( "%s: could not save stat into database.", __FUNCTION__ );

   return;
}

void db_load_stat_instance( STAT_INSTANCE *stat, MYSQL_ROW *row )
{
   int counter = 0;

   stat->framework = get_stat_framework_by_id( atoi( (*row)[counter++] ) );
   stat->owner = get_instance_by_id( atoi( (*row)[counter++] ) );
   stat->perm_stat = atoi( (*row)[counter++] );
   stat->mod_stat = atoi( (*row)[counter++] );
   return;
}

void free_stat_list( LLIST *list )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( stat, list );
      free_stat( stat );
   }
   DetachIterator( &Iter );
   return;
}

void stat_instantiate( ENTITY_INSTANCE *owner, STAT_FRAMEWORK *fstat )
{
   STAT_INSTANCE *stat;

   stat = init_stat();
   stat->framework = fstat;
   stat->owner = owner;
   AttachToList( stat, owner->stats );
   if( !strcmp( owner->tag->created_by, "null" ) )
      return;
   new_stat_instance( stat );
}

void load_entity_stats( ENTITY_INSTANCE *entity )
{
   STAT_INSTANCE *stat;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;

   list = AllocList();

   if( !db_query_list_row( list, quick_format( "SELECT * FROM `entity_stats` WHERE owner=%d;", entity->tag->id ) ) )
   {
      FreeList( list );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      CREATE( stat, STAT_INSTANCE, 1 );
      db_load_stat_instance( stat, &row );
      AttachToList( stat, entity->stats );
   }
   DetachIterator( &Iter );
   FreeList( list );

   return;
}

void instantiate_entity_stats_from_framework( ENTITY_INSTANCE *entity )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   if( !entity->framework )
      return;

   AttachIterator( &Iter, entity->framework->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      stat_instantiate( entity, fstat );
   DetachIterator( &Iter );

   return;
}

void clear_stat_list( LLIST *list )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      free_stat( stat );
   DetachIterator( &Iter );
}

STAT_FRAMEWORK *get_stat_framework_by_query( const char *query )
{
   STAT_FRAMEWORK *fstat;
   MYSQL_ROW row;

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   CREATE( fstat, STAT_FRAMEWORK, 1 );
   CREATE( fstat->tag, ID_TAG, 1 );
   db_load_stat_framework( fstat, &row );
   return fstat;
}
inline STAT_FRAMEWORK *get_stat_framework_by_id( int id )
{
   STAT_FRAMEWORK *fstat;
   if( ( fstat = get_active_stat_framework_by_id( id ) ) == NULL )
      if( ( fstat = load_stat_framework_by_id( id ) ) != NULL )
         AttachToList( fstat, stat_frameworks );
   return fstat;
}

STAT_FRAMEWORK *get_active_stat_framework_by_id( int id )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, stat_frameworks );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( fstat->tag->id == id )
         break;
   DetachIterator( &Iter );
   return fstat;
}

inline STAT_FRAMEWORK *load_stat_framework_by_id( int id )
{
   return get_stat_framework_by_query( quick_format( "SELECT * FROM `stat_frameworks` WHERE statFrameworkID=%d;", id ) );
}
inline STAT_FRAMEWORK *get_stat_framework_by_name( const char *name )
{
   STAT_FRAMEWORK *fstat;
   if( ( fstat = get_active_stat_framework_by_name( name ) ) == NULL )
      if( ( fstat = load_stat_framework_by_name( name ) ) != NULL )
         AttachToList( fstat, stat_frameworks );
   return fstat;
}

STAT_FRAMEWORK *get_active_stat_framework_by_name( const char *name )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, stat_frameworks );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( fstat->name, name ) )
         break;
   DetachIterator( &Iter );
   return fstat;
}

inline STAT_FRAMEWORK *load_stat_framework_by_name( const char *name )
{
   return get_stat_framework_by_query( quick_format( "SELECT * FROM `stat_frameworks` WHERE name=%s;", name ) );
}

STAT_FRAMEWORK *get_stat_from_framework_by_id( ENTITY_FRAMEWORK *frame, int id, int *spec_from )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, frame->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( fstat->tag->id == id )
         break;
   DetachIterator( &Iter );

   if( fstat )
      return fstat;

   if( frame->inherits )
   {
      *spec_from = 1;
      return get_stat_from_framework_by_id( frame->inherits, id, spec_from );
   }
   return NULL;
}

STAT_FRAMEWORK *get_stat_from_framework_by_name( ENTITY_FRAMEWORK *frame, const char *name, int *spec_from )
{
   STAT_FRAMEWORK *fstat;
   ITERATOR Iter;

   AttachIterator( &Iter, frame->stats );
   while( ( fstat = (STAT_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( fstat->name, name ) )
         break;
   DetachIterator( &Iter );

   if( fstat )
      return fstat;

   if( frame->inherits )
   {
      *spec_from = 1;
      return get_stat_from_framework_by_name( frame->inherits, name, spec_from );
   }
   return NULL;
}

STAT_INSTANCE  *get_stat_from_instance_by_id( ENTITY_INSTANCE *entity, int id )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, entity->stats );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( stat->framework->tag->id == id )
         break;
   DetachIterator( &Iter );

   return stat;
}
STAT_INSTANCE  *get_stat_from_instance_by_name( ENTITY_INSTANCE *entity, const char *name )
{
   STAT_INSTANCE *stat;
   ITERATOR Iter;

   AttachIterator( &Iter, entity->stats );
   while( ( stat = (STAT_INSTANCE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( stat->framework->name, name ) )
         break;
   DetachIterator( &Iter );

   return stat;
}

inline void set_softcap( STAT_FRAMEWORK *fstat, int value )
{
   fstat->softcap = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET softcap=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_hardcap( STAT_FRAMEWORK *fstat, int value )
{
   fstat->hardcap = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET hardcap=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_softfloor( STAT_FRAMEWORK *fstat, int value )
{
   fstat->softfloor = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET softfloor=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}

inline void set_hardfloor( STAT_FRAMEWORK *fstat, int value )
{
   fstat->hardfloor = value;
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET hardfloor=%d WHERE statFrameworkID=%d;", value, fstat->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}
inline void set_name( STAT_FRAMEWORK *fstat, const char *name )
{
   FREE( fstat->name );
   fstat->name = strdup( name );
   if( !strcmp( fstat->tag->created_by, "null" ) ) return;
   if( !quick_query( "UPDATE `stat_frameworks` SET name='%s' WHERE statFrameworkID=%d;", name, fstat->tag->id ) )
      bug( "%s: could not update database with new name.", __FUNCTION__ );
}

inline void set_perm_stat( STAT_INSTANCE *stat, int value )
{
   stat->perm_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET perm_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update database with new value.", __FUNCTION__ );
}
inline void set_mod_stat( STAT_INSTANCE *stat, int value )
{
   stat->mod_stat = value;
   if( !quick_query( "UPDATE `entity_stats` SET mod_stat=%d WHERE statFrameworkID=%d AND owner=%d;", value, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update databaes with new value.", __FUNCTION__ );
}
inline void set_stat_owner( STAT_INSTANCE *stat, ENTITY_INSTANCE *owner )
{
   if( owner != stat->owner ) DetachFromList( stat, stat->owner->stats );
   if( !quick_query( "UPDATE `entity_stats` SET owner=%d WHERE statFrameworkID=%d and owner=%d;", owner->tag->id, stat->framework->tag->id, stat->owner->tag->id ) )
      bug( "%s: could not update databse with new owner.", __FUNCTION__ );
   stat->owner = owner;
}

FILE *open_s_script( STAT_FRAMEWORK *fstat, const char *permissions )
{
   FILE *script;
   script = fopen( get_stat_framework_script_path( fstat ), permissions );
   return script;
}

bool s_script_exists( STAT_FRAMEWORK *fstat )
{
   FILE *script;

   if( !strcmp( fstat->tag->created_by, "null" ) )
      return FALSE;

   if( ( script = fopen( quick_format( "../scripts/stats/%d.lua", fstat->tag->id ), "r" ) ) == NULL )
      return FALSE;

   fclose( script );
   return TRUE;

}

void init_s_script( STAT_FRAMEWORK *fstat, bool force )
{
   FILE *temp, *dest;

   if( s_script_exists( fstat ) && !force )
      return;

   if( ( temp = fopen( "../scripts/templates/stat.lua", "r" ) ) == NULL )
   {
      bug( "%s: could not open the template.", __FUNCTION__ );
      return;
   }

   if( ( dest = fopen( quick_format( "../scripts/stats/%d.lua", fstat->tag->id ), "w" ) ) == NULL )
   {
      bug( "5s: could not open the script.", __FUNCTION__ );
      return;
   }
   copy_flat_file( dest, temp );
   fclose( dest );
   fclose( temp );
   return;
}

const char *print_s_script( STAT_FRAMEWORK *fstat )
{
   const char *buf;
   FILE *fp;

   if( !s_script_exists( fstat ) )
      return "This framework has no script.";

   if( ( fp = open_s_script( fstat, "r" ) ) == NULL )
      return "There was a pretty bug error.";

   buf = fread_file( fp );
   fclose( fp );
   return buf;
}





