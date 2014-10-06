/* the file containing methods pertaining to all things project related written by Davenge */

#include "mud.h"

PROJECT *init_project( void )
{
   PROJECT *project;

   CREATE( project, PROJECT, 1 );
   project->workspaces = AllocList();
   project->tag = init_tag();
   project->tag->type = PROJECT_IDS;
   clear_project( project );
   return project;
}

int free_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   if( project->tag )
      free_tag( project->tag );

   CLEARLIST( project->workspaces, WORKSPACE );
   FreeList( project->workspaces );
   project->workspaces = NULL;

   FREE( project );

   return ret;
}

int clear_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   project->name = strdup( "new_project" );
   project->Public = FALSE;

   return ret;
}

PROJECT *load_project_by_query( const char *query )
{
   PROJECT *project = NULL;
   MYSQL_ROW row;

   if( ( row = db_query_single_row( query ) ) == NULL )
      return NULL;

   if( ( project = init_project() ) == NULL )
      return NULL;

   db_load_project( project, &row );
   load_project_entries( project );
   free( row ); /* this might be leaky... */

   return project;
}

PROJECT *load_project_by_id( int id )
{
   return load_project_by_query( quick_format( "SELECT * FROM `projects` WHERE projectID=%d;", id ) );
}

PROJECT *load_project_by_name( const char *name )
{
   return load_project_by_query( quick_format( "SELECT * FROM `projects` WHERE name='%s';", name ) );
}

int new_project( PROJECT *project )
{
   int ret = RET_SUCCESS;

   if( !project )
   {
      BAD_POINTER( "project" );
      return ret;
   }

   if( !strcmp( project->tag->created_by, "null" ) )
   {
      if( ( ret = new_tag( project->tag, "system" ) ) != RET_SUCCESS )
      {
         bug( "%s: failed to pull new tag from handler.", __FUNCTION__ );
         return ret;
      }
   }

   if( !quick_query( "INSERT INTO projects VALUES( %d, %d, '%s', '%s', '%s', '%s', '%s', '%d' );",
         project->tag->id, project->tag->type, project->tag->created_by, project->tag->created_on,
         project->tag->modified_by, project->tag->modified_on, project->name, (int)project->Public ) )
      return RET_FAILED_OTHER;

   return ret;
}

int new_project_entry( PROJECT *project, ID_TAG *tag )
{
   int ret = RET_SUCCESS;

   if( !project )
   {
      BAD_POINTER( "wSpace" );
      return ret;
   }
   if( !tag )
   {
      BAD_POINTER( "tag" );
      return ret;
   }

   if( !quick_query( "INSERT INTO `project_entries` VALUES( %d, '%c%d' );", project->tag->id, tag_table_characters[tag->type], tag->id ) )
      return RET_FAILED_OTHER;

   return ret;
}

void db_load_project( PROJECT *project, MYSQL_ROW *row )
{
   int counter;

   counter = db_load_tag( project->tag, row );
   project->name = strdup( (*row)[counter++] );
   project->Public = (bool)( atoi( (*row)[counter++] ) );

   return;
}

void load_project_entries( PROJECT *project )
{
   WORKSPACE *wSpace;
   LLIST *list;
   MYSQL_ROW row;
   ITERATOR Iter;
   int id;

   if( !project )
   {
      bug( "%s: bad project pointer.", __FUNCTION__ );
      return;
   }

   list = AllocList();
   if( !db_query_list_row( list, quick_format( "SELECT entry FROM `project_entries` WHERE projectID=%d", project->tag->id ) ) )
   {
      bug( "%s: could not load the list of rows.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, list );
   while( ( row = (MYSQL_ROW)NextInList( &Iter ) ) != NULL )
   {
      switch( row[0][0] )
      {
         default: continue;
         case 'w':
            id = atoi( row[0]+1 );
            if( ( wSpace = get_workspace_by_id( id ) ) == NULL )
            {
               bug( "%s: bad entry in project_entires %d.", __FUNCTION__, id );
               continue;
            }
            AttachToList( wSpace, project->workspaces );
            free( row );
            break;
      }
   }
   FreeList( list );
   return;
}

void load_project_workspaces_into_olc( PROJECT *project, INCEPTION *olc )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( project->workspaces ) < 1 )
      return;

   AttachIterator( &Iter, project->workspaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      AttachToList( wSpace, olc->wSpaces );
   DetachIterator( &Iter );

   return;
}

void add_workspace_to_project( WORKSPACE *wSpace, PROJECT *project )
{
   if( !wSpace )
   {
      bug( "%s: workspace pointer is NULL.", __FUNCTION__ );
      return;
   }
   if( !project )
   {
      bug( "%s: project pointer is NULL.", __FUNCTION__ );
      return;
   }
   if( workspace_list_has_by_id( project->workspaces, wSpace->tag->id ) )
   {
       bug( "%s: project always has workspace with %d id.", __FUNCTION__, wSpace->tag->id );
       return;
   }

   AttachToList( wSpace, project->workspaces );
   new_project_entry( project, wSpace->tag );
   return;
}

void rem_workspace_from_project( WORKSPACE *wSpace, PROJECT *project )
{
   return;
}

void load_project_into_olc( PROJECT *project, INCEPTION *olc )
{
   if( !olc )
   {
      bug( "%s: trying to load project into NULL olc.", __FUNCTION__ );
      return;
   }
   if( !project )
   {
       bug( "%s: trying load add NULL project into olc.", __FUNCTION__ );
       return;
   }
   olc->project = project;
   load_project_workspaces_into_olc( project, olc );
   return;
}

/* this monster needs factoring */
void export_project( PROJECT *project )
{
   LLIST *framework_list, *instance_list;
   int *workspace_id_table;
   int *framework_id_table;
   int *instance_id_table;
   char directory[MAX_BUFFER];

   /* craft directory for our project */
   mud_printf( directory, "%s", create_project_directory( project ) );

   framework_list = AllocList();
   instance_list = AllocList();

   /* write our grand lists for id tables */
   create_complete_framework_and_instance_list_from_workspace_list( project->workspaces, instance_list, framework_list );

   /* build workspace_id_table first */
   workspace_id_table = build_workspace_id_table( project->workspaces );
   instance_id_table = build_instance_id_table( instance_list );
   framework_id_table = build_framework_id_table( framework_list );

   /* commence writing */
/*  save_workspace_list_export( project->workspaces, &directory, workspace_id_table ); */
   save_instance_list_export( instance_list, directory, instance_id_table, framework_id_table );
/*   save_framework_list_export( framework_list, &directory, framework_id_table ); */

   /* free memory */
   CLEARLIST( framework_list, ENTITY_FRAMEWORK );
   FreeList( framework_list );
   CLEARLIST( instance_list, ENTITY_INSTANCE );
   FreeList( instance_list );
   FREE( workspace_id_table );
   FREE( framework_id_table );
   FREE( instance_id_table );
   return;
}

void save_instance_list_export( LLIST *instance_list, char *directory, int *instance_id_table, int *framework_id_table )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      save_instance_export( directory, instance, instance_id_table, framework_id_table );
}

void save_instance_export( char *pDir, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table )
{
   FILE *fp;
   int new_id;

   bug( "%s: pdir = %s", __FUNCTION__, pDir );

   new_id = get_id_table_position( instance_id_table, instance->tag->id );
   if( ( fp = fopen( quick_format( "%s/%d.instance", pDir, new_id ), "w" ) ) == NULL )
   {
      bug( "%s: Unable to write instance (%d)%s.", instance->tag, instance_name( instance ) );
      return;
   }
   fwrite_instance_export( fp, instance, instance_id_table, framework_id_table );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fwrite_instance_export( FILE *fp, ENTITY_INSTANCE *instance, int *instance_id_table, int *framework_id_table )
{
   fprintf( fp, "#IDTAG\n" );
   fprintf( fp, "ID           %d\n", get_id_table_position( instance_id_table, instance->tag->id ) );
   fprintf( fp, "CreatedOn    %s~\n", instance->tag->created_on );
   fprintf( fp, "CreatedBy    %s~\n", instance->tag->created_by );
   fprintf( fp, "ModifiedOn   %s~\n", instance->tag->modified_on );
   fprintf( fp, "ModifiedBy   %s~\n", instance->tag->modified_by );
   fprintf( fp, "END\n\n" );

   fprintf( fp, "#INSTANCE\n" );
   fprintf( fp, "Level        %d\n", instance->level );

   fwrite_instance_content_list_export( fp, instance->contents, instance_id_table );
   fwrite_specifications( fp, instance->specifications );

   fprintf( fp, "Framework    %d\n", get_id_table_position( framework_id_table, instance->framework->tag->id ) );
   fprintf( fp, "ContainedBy  %d\n", get_id_table_position( instance_id_table, instance->contained_by_id ) );
   fprintf( fp, "END\n\n" );
}

void fwrite_instance_content_list_export( FILE *fp, LLIST *contents, int *instance_id_table )
{
   ENTITY_INSTANCE *content;
   ITERATOR Iter;

   AttachIterator( &Iter, contents );
   while( ( content = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "Content      %d\n", get_id_table_position( instance_id_table, content->tag->id ) );
   DetachIterator( &Iter );

   return;
}

/*
void export_project( PROJECT *project )
{
   LLIST *workspace_list;
   LLIST *framework_list;
   LLIST *instance_list;

   int *workspace_ids;
   int *instance_ids;
   int *framework_ids;

   if( !project )
   {
      bug( "%s: passed a NULL project.", __FUNCTION__ );
      return;
   }

   workspace_list = AllocList();
   framework_list = AllocList();
   instance_list = AllocList();

   copy_all_workspace_and_contents( project, workspace_list, framework_list, instance_list );
   copy_all_instance_frames_into_list_ndi( instance_list, framework_list );

   CREATE( workspace_ids, int, SizeOfList( workspace_list ) );
   CREATE( instance_ids, int, SizeOfList( instance_list ) );
   CREATE( framework_ids, int, SizeOfList( framework_list ) );

   swap_and_track_workspace_ids( workspace_list, workspace_ids );

}
*/

char *create_project_directory( PROJECT *project )
{
   static char pDir[MAX_BUFFER];
   memset( &pDir[0], 0, sizeof( pDir ) );

   mud_printf( pDir, "../projects/%s-%s", project->name, smash_newline( ctime( &current_time ) ) );
   if( opendir( pDir ) == NULL )
   {
      bug( "%s: pDir = %s", __FUNCTION__, pDir );
      if( ( mkdir( pDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) != 0 ) /* was unsuccessful */
      {
         bug( "%s: unable to create directory: %s.", __FUNCTION__, pDir );
         return NULL;
      }
   }
   return pDir;
}

void create_complete_framework_and_instance_list_from_workspace_list( LLIST *workspace_list, LLIST *instance_list, LLIST *framework_list )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      append_instance_lists_ndi( wSpace->instances, instance_list );
      append_framework_lists_ndi( wSpace->frameworks, framework_list );
   }
   DetachIterator( &Iter );

   append_instance_list_content_to_list_recursive_ndi( instance_list, instance_list );
   append_framework_list_content_to_list_recursive_ndi( framework_list, framework_list );
   append_framework_list_inheritance_to_list_recursive_ndi( framework_list, framework_list );
   return;
}

void append_instance_list_content_to_list_recursive_ndi( LLIST *instance_list, LLIST *append_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter, IterTwo;

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
   {
      AttachIterator( &IterTwo, instance->contents );
      while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      {
         if( !instance_list_has_by_id( append_list, instance->tag->id ) )
            AttachToList( instance, append_list );
         append_instance_list_content_to_list_recursive_ndi( instance->contents, append_list );
      }
      DetachIterator( &IterTwo );
   }
   DetachIterator( &Iter );

   return;
}

void append_framework_list_content_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter, IterTwo;

   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      AttachIterator( &IterTwo, frame->fixed_contents );
      while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      {
         if( !framework_list_has_by_id( append_list, frame->tag->id ) )
            AttachToList( frame, append_list );
         append_framework_list_content_to_list_recursive_ndi( frame->fixed_contents, append_list );
      }
      DetachIterator( &IterTwo );
   }
   DetachIterator( &Iter );

   return;
}

void append_framework_list_inheritance_to_list_recursive_ndi( LLIST *framework_list, LLIST *append_list )
{
   ENTITY_FRAMEWORK *frame;
   ITERATOR Iter;

   AttachIterator( &Iter, framework_list );
   while( ( frame = (ENTITY_FRAMEWORK *)NextInList( &Iter ) ) != NULL )
   {
      while( ( frame = frame->inherits ) != NULL )
         if( !framework_list_has_by_id( append_list, frame->tag->id ) )
            AttachToList( frame, append_list );
   }
   DetachIterator( &Iter );

   return;
}

void copy_all_workspace_and_contents( PROJECT *project, LLIST *workspace_list, LLIST *framework_list, LLIST *instance_list )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( !project )
   {
      bug( "%s: passed a NULL project.", __FUNCTION__ );
      return;
   }
   if( !workspace_list )
   {
      bug( "%s: passed a NULL workspace_list.", __FUNCTION__ );
      return;
   }
   if( !framework_list )
   {
      bug( "%s: passed a NULL framework_list.", __FUNCTION__ );
      return;
   }
   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }

   copy_workspaces_into_list( project->workspaces, workspace_list, FALSE, FALSE );

   /* instances first then frameworks per workspace, this could be factored later */
   AttachIterator( &Iter, workspace_list );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
   {
      copy_instance_list_ndi( wSpace->instances, instance_list );
      copy_framework_list_ndi( wSpace->frameworks, framework_list);
   }
   DetachIterator( &Iter );

   return;
}

void copy_all_instance_frames_into_list_ndi( LLIST *instance_list, LLIST *frame_list )
{
   ENTITY_INSTANCE *instance;
   ITERATOR Iter;

   if( !instance_list )
   {
      bug( "%s: passed a NULL instance_list.", __FUNCTION__ );
      return;
   }

   if( !frame_list )
   {
      bug( "%s: passed a NULL frame_list.", __FUNCTION__ );
      return;
   }

   AttachIterator( &Iter, instance_list );
   while( ( instance = (ENTITY_INSTANCE *)NextInList( &Iter ) ) != NULL )
      copy_framework_ndi( instance->framework, frame_list );
   DetachIterator( &Iter );

   return;
}

void project_newProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "How about a name for that new project?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( SizeOfList( olc->wSpaces ) > 0 )
   {
      text_to_olc( olc, "You can't have any workspaces loaded when creating a new project.\r\n" );
      olc_short_prompt( olc );
      return;
   }

   project = init_project();
   if( new_tag( project->tag, olc->account->name ) != RET_SUCCESS )
   {
      text_to_olc( olc, "You could not get a new tag for your project, therefore, it was not created.\r\n" );
      free_project( project );
      olc_short_prompt( olc );
      return;
   }
   FREE( project->name );
   project->name = strdup( format_string_for_sql( arg ) );
   new_project( project );
   load_project_into_olc( project, olc );
   text_to_olc( olc, "You start a new project named: %s.\r\n", arg );
   return;
}

void project_openProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      text_to_olc( olc, "Open what project?\r\n" );
      olc_short_prompt( olc );
      return;
   }

   if( ( project = load_project_by_name( arg ) ) == NULL )
   {
      text_to_olc( olc, "There is no project by the name %s.\r\n", arg );
      olc_short_prompt( olc );
      return;
   }

   load_project_into_olc( project, olc );
   text_to_olc( olc, "You open the %s project and load it into your olc.\r\n", project->name );
   return;
}

void project_exportProject( void *passed, char *arg )
{
   INCEPTION *olc = (INCEPTION *)passed;
   PROJECT *project;

   if( !arg || arg[0] == '\0' )
   {
      if( !olc->project )
      {
         text_to_olc( olc, "Export what project?\r\n" );
         olc_short_prompt( olc );
         return;
      }
      project = olc->project;
   }
   else
   {
      if( ( project = load_project_by_name( arg ) ) == NULL )
      {
         text_to_olc( olc, "There is no project by the name %s.\r\n", arg );
         olc_short_prompt( olc );
         return;
      }
   }

   export_project( project );
   text_to_olc( olc, "You export %s.\r\n", project->name );
   olc_short_prompt( olc );
   return;
}
