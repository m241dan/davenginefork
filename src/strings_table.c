#include "mud.h"

const char *const data_types[MAX_MEMORY_TYPE+1] = {
   "int", "char", "D_SOCKET",
   '\0' /* gandalf */
};

/* match from from given table */
int match_string_table( const char *string, const char *const string_table[] )
{
   int x;

   for( x = 0; string_table[x] != '\0'; x++ )
      if( !strcmp( string, string_table[x] ) )
         return x;

   return -1;
}
