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

void export_project( PROJECT *project )
{
   LLIST *workspace_list;
   LLIST *framework_list;
   LLIST *instance_list;

   if( !project )
   {
      bug( "%s: passed a NULL project.", __FUNCTION__ );
      return;
   }

   workspace_list = AllocList();
   framework_list = AllocList();
   instance_list = AllocList();

   copy_all_workspace_and_contents( project, workspace_list, framework_list, instance_list );

}

void copy_all_workspace_and_contents( PROJECT *project, LLIST *workspace_list, LLIST *framework_list, LLIST *instance_list )
{
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
