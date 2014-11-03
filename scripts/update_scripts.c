#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_BUFFER 16384
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif
typedef  unsigned char     bool;

typedef enum
{
   TEMP_FUNC, SCRIPT_FUNC, PREFIX_TEST
} WRITE;

typedef struct lua_function
{
   char *header;
   char *body;
} LUA_FUNCTION;

typedef struct lua_function * LUA_FUNCTION_ARRAY;

char *fread_file( FILE *fp );
char *one_arg_delim( char *fStr, char *bStr, char delim );
bool until_function( char *str );
bool until_end( char *str );
void *update_frameworks( void *arg );
void *update_instances( void *arg );
void *update_stats( void *arg );
bool is_prefix(const char *aStr, const char *bStr);
int count_lua_functions( char *str );
LUA_FUNCTION_ARRAY get_functions( char *str );
pthread_t tid[3];



int main( void )
{

   pthread_create( &(tid[0] ), NULL, &update_frameworks, NULL );
   pthread_create( &(tid[1] ), NULL, &update_instances, NULL );
   pthread_create( &(tid[2] ), NULL, &update_stats, NULL );

   pthread_join( tid[0], NULL );
   pthread_join( tid[1], NULL );
   pthread_join( tid[2], NULL );
   return 0;
}

void *update_frameworks( void *arg )
{
   DIR *directory;
   struct dirent *entry;
   FILE *template_fp;
   FILE *script_fp;
   char template_buf[MAX_BUFFER];
   char script_buf[MAX_BUFFER];
   char *Template;
   char *script;
   WRITE write;

   if( ( template_fp = fopen( "templates/frame.lua", "r" ) ) == NULL )
      return NULL;
   memset( &template_buf[0], 0, sizeof( template_buf ) );
   strcat( template_buf, fread_file( template_fp ) );
   fclose( template_fp );

   directory = opendir( "frames/" );
   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      Template = template_buf;
      char buf[255], line_one[510], line_two[510];
      if( !strcmp( entry->d_name, "." ) || !strcmp( entry->d_name, ".." ) )
         continue;
      snprintf( buf, 255, "frames/%s", entry->d_name );
      if( ( script_fp = fopen( buf, "r" ) ) == NULL )
         continue;

      memset( &script_buf[0], 0, sizeof( script_buf ) );
      script = script_buf;
      strcat( script_buf, fread_file( script_fp ) );
      fclose( script_fp );

      if( ( script_fp = fopen( buf, "w" ) ) == NULL )
         continue;

      write = TEMP_FUNC;
      while( 1 )
      {
         if( !script || script[0] == '\0' )
         {
            fprintf( script_fp, "%s\n", Template );
            break;
         }
         if( !Template || Template[0] == '\0' )
         {
            fprintf( script_fp, "%s\n", script );
            break;
         }
         switch( write )
         {
            case TEMP_FUNC:
               Template = one_arg_delim( Template, line_two, '\n' );
               while( !until_function( line_two ) )
               {
                  fprintf( script_fp, "%s\n", line_two );
                  Template = one_arg_delim( Template, line_two, '\n' );
               }
               write = SCRIPT_FUNC;
               continue;
            case SCRIPT_FUNC:
               script = one_arg_delim( script, line_one, '\n' );
               while( !until_function( line_one ) )
               {
                  fprintf( script_fp, "%s\n", line_one );
                  script = one_arg_delim( script, line_one, '\n' );
               }
               write = PREFIX_TEST;
               continue;
            case PREFIX_TEST:
               printf( "running prefix test\nline_two = %s | line_one = %s\n", line_two, line_one );
               if( is_prefix( line_two, line_one ) )
               {
                  while( !until_end( line_two ) )
                  {
                     fprintf( script_fp, "%s\n", line_two );
                     Template = one_arg_delim( Template, line_two, '\n' );
                  }
                  while( !until_end( line_one ) )
                  {
                     script = one_arg_delim( script, line_one, '\n' );
                     fprintf( script_fp, "%s\n", line_one );
                  }
                  write = TEMP_FUNC;
                  continue;
               }
               else
               {
                  do
                  {
                     fprintf( script_fp, "%s\n", line_two );
                     script = one_arg_delim( script, line_two, '\n' );
                  } while( !until_function( line_two ) );
               }
         }
      }
      fclose( script_fp );
   }
   closedir( directory );
   return NULL;
}

void *update_instances( void *arg )
{
   return NULL;
}

void *update_stats( void *arg )
{
   return NULL;
}

bool until_function( char *str )
{
   if( is_prefix( str, "function" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

bool until_end( char *str )
{
   if( is_prefix( str, "end" ) || str[0] == '\0' )
      return TRUE;
   return FALSE;
}

char *fread_file( FILE *fp )
{
   static char buf[MAX_BUFFER];
   int c, count = 0;

   memset( &buf[0], 0, sizeof( buf ) );

   while( ( c = getc( fp ) ) != EOF )
   {
      buf[count++] = c;
      if( count >= ( MAX_BUFFER - 1 ) )
         break;
   }

   buf[strlen( buf )] = '\0';
   return buf;

}
char *one_arg_delim( char *fStr, char *bStr, char delim )
{
  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == delim)
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* return the leftovers */
  return fStr;
}

bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if( *bStr == '\0' )
       break;
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}
