/* header filefor strings_table.c written by Davenge */

extern const char *const data_types[MAX_MEMORY_TYPE+1];

/* match string from given table */
int match_string_table( const char *string, const char *const string_table[] );
