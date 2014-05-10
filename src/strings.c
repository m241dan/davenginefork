/*
 * This file handles string copy/search/comparison/etc.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/* include main header file */
#include "mud.h"

/*
 * Checks if aStr is a prefix of bStr.
 */
bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}

char *one_arg(char *fStr, char *bStr)
{
  /* skip leading spaces */
  while (isspace(*fStr))
    fStr++; 

  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == ' ')
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* skip past any leftover spaces */
  while (isspace(*fStr))
    fStr++;

  /* return the leftovers */
  return fStr;
}

char *capitalize(char *txt)
{
   static char buf[MAX_BUFFER];
   mud_printf( buf, "%s", downcase( txt ) );
   buf[0] = toupper( buf[0] );
   return buf;
}

char *downcase( char *txt )
{
   static char buf[MAX_BUFFER];
   int size, x;
   memset( &buf[0], 0, sizeof( buf ) );

   if( !txt || txt[0] == '\0' )
      return buf;

   size = strlen( txt );

   for( x = 0; x < size; x++ )
      buf[x] = tolower( txt[x] );
   buf[size] = '\0';

   return buf; 
}

/*  
 * Create a new buffer.
 */
BUFFER *__buffer_new(int size)
{
  BUFFER *buffer;
    
  buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void __buffer_strcat(BUFFER *buffer, const char *text)  
{
  int new_size;
  int text_len;
  char *new_data;
 
  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);
    
  /* Adding empty string ? */ 
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
  { 
    new_size = buffer->size + text_len + 1;
   
    /* Allocate the new buffer */
    new_data = malloc(new_size);
  
    /* Copy the current buffer to the new buffer */
    memcpy(new_data, buffer->data, buffer->len);
    free(buffer->data);
    buffer->data = new_data;  
    buffer->size = new_size;
  }
  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  free(buffer->data);
 
  /* Free buffer */
  free(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{  
  char buf[MAX_BUFFER];
  va_list va;
  int res;
    
  va_start(va, fmt);
  res = vsnprintf(buf, MAX_BUFFER, fmt, va);
  va_end(va);
    
  if (res >= MAX_BUFFER - 1)  
  {
    buf[0] = '\0';
    bug("Overflow when printing string %s", fmt);
  }
  else
    buffer_strcat(buffer, buf);
   
  return res;
}

char *strdup(const char *s)
{
  char *pstr;
  int len;

  len = strlen(s) + 1;
  pstr = (char *) calloc(1, len);
  strcpy(pstr, s);

  return pstr;
}

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}

int mud_printf( char *dest, const char *format, ... )
{
   va_list va;
   int res;

   va_start( va, format );
   res = vsnprintf( dest, MAX_BUFFER, format, va );
   va_end( va );

   if( res >= MAX_BUFFER -1 )
   {
      dest[0] = '\0';
      bug( "Overflow when printing string %s", format );
   }

   return res;
}
/*
char *colorize( const char *to_color )
{
   static char buf[MAX_BUFFER];
   char *ptr;
   memset( &buf[0], 0 sizeof( buf ) );

   ptr = buf;

   while( *to_color )
   {
      
   }


}
*/

const char *print_header( const char *title, const char *pattern, int width )
{
   static char buf[MAX_BUFFER];
   const char *pat_ptr;
   char *buf_ptr;
   int title_len = strlen( title );
   int pattern_len = strlen( pattern );
   int each_sides_pattern_len, side_pattern_remainder, loop_limit, extra, x;

   memset( &buf, 0, sizeof( buf ) );
   buf_ptr = buf;

   each_sides_pattern_len = ( width - title_len - 2 ) / 2; /* minus two for preceeding and appending spaces to the title */
   side_pattern_remainder = each_sides_pattern_len % pattern_len;
   loop_limit = each_sides_pattern_len - side_pattern_remainder;
   extra = title_len % 2;

   pat_ptr = pattern;
   for( x = 0; x < loop_limit; x++ )
   {
      *buf_ptr++ = *pat_ptr++;
      if( ( x + 1 ) % pattern_len == 0 )
         pat_ptr = pattern;
   }

   for( x = -1 ; x < side_pattern_remainder; x++ )
      *buf_ptr++ = ' ';

   for( x = 0; x < title_len; x++ )
      *buf_ptr++ = *title++;

   for( x = -1; x < ( side_pattern_remainder + extra ); x++ )
      *buf_ptr++ = ' ';

   pat_ptr = pattern;
   for( x = 0; x < loop_limit; x++ )
   {
      *buf_ptr++ = *pat_ptr++;
      if( ( x + 1 ) % pattern_len == 0 )
         pat_ptr = pattern;
   }

   buf[strlen( buf )] = '\0';
   return buf;
}

void bprint_commandline( BUFFER *buf, COMMAND *com, int sublevel, int pagewidth )
{
   char symbol[3];
   int subindent, commandspace;

   if( com->can_sub )
   {
      if( com->subcommands )
         snprintf( symbol, 3, "(+)" );
      else
         snprintf( symbol, 3, "(-)" );
   }

   subindent = sublevel * 3;
   commandspace = pagewidth - 8 - subindent;
   if( commandspace < 1 )
   {
      bprintf( buf, "| COMMAND SPACING PROBLEM\r\n" );
      return;
   }
   bprintf( buf, "| %-*.*s%-3.3s %-*.*s |\r\n",
                  subindent, subindent, "   ",
                  com->can_sub ? symbol : "   ",
                  commandspace, commandspace, com->cmd_name );

   return;
}
