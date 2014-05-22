/*
 * This file contains all sorts of utility functions used
 * all sorts of places in the code.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

/*
 * Check to see if a given name is
 * legal, returning FALSE if it
 * fails our high standards...
 */
bool check_name(const char *name)
{
  int size, i;

  if ((size = strlen(name)) < 3 || size > 12)
    return FALSE;

  for (i = 0 ;i < size; i++)
    if (!isalpha(name[i])) return FALSE;

  return TRUE;
}

void communicate(void)
{
   return;
}

/*
 * Loading of help files, areas, etc, at boot time.
 */
void load_muddata(bool fCopyOver)
{  
  load_helps();

}

char *get_time()
{
  static char buf[16];
  char *strtime;
  int i;

  strtime = ctime(&current_time);
  for (i = 0; i < 15; i++)   
    buf[i] = strtime[i + 4];
  buf[15] = '\0';

  return buf;
}

bool check_sql( void )
{
   if( sql_handle == NULL )
   {
      bug( "%s has found that the SQL handle is NULL.", __FUNCTION__ );
      return FALSE;
    }

   return TRUE;
}

void report_sql_error( MYSQL *con )
{
   bug( "%s: %s", __FUNCTION__, mysql_error( con ) );
   return;
}

bool quick_query( const char *format, ... )
{
   char query[MAX_BUFFER];
   va_list va;
   int res;

   va_start( va, format );
   res = vsnprintf( query, MAX_BUFFER, format, va );
   va_end( va );

   if( res >= MAX_BUFFER )
   {
      bug( "%s: received a long or unusable format: %s", __FUNCTION__, format );
      return FALSE;
   }

   if( !check_sql() )
      return FALSE;

   if( mysql_query( sql_handle, query ) )
   {
      report_sql_error( sql_handle );
      return FALSE;
   }

   return TRUE;
}

