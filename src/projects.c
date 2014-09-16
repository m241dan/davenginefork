/* the file containing methods pertaining to all things project related written by Davenge */

#include "mud.h"

PROJECT *init_project( void )
{
   PROJECT *project;

   CREATE( project, PROJECT, 1 );
   project->workspaces = AllocList();
   project->tag = init_tag();
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
   MYSQL_ROW *row;

   row = malloc( sizeof( MYSQL_ROW ) );

   if( !db_query_single_row( row, query ) )
      return NULL;

   if( ( project = init_project() ) == NULL )
      return NULL;

   db_load_project( project, row );
   load_workspace_entires( wSpace );
   free( row );
   if( *row )
      free( *row );

   return project;
}
