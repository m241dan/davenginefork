/* account.c: methods pertaining to accounts written by Davenge */

#include "mud.h"

/* creation */

ACCOUNT_DATA *init_account( void )
{
   ACCOUNT_DATA *account;

   CREATE( account, ACCOUNT_DATA, 1 );
   clear_account( account );
   account->command_tables = AllocList();
   account->commands = AllocList();
   return account;
}

void clear_account( ACCOUNT_DATA *account )
{
   account->name = "new_account";
   account->password = "new_password";
   account->level = 1;
   account->pagewidth = DEFAULT_PAGEWIDTH;
   return;
}

/* deletion */
void free_account( ACCOUNT_DATA *account )
{
   ITERATOR Iter;
   account->socket = NULL;
   FreeList( account->characters );
   account->characters = NULL;
   FreeList( account->command_tables );
   account->command_tables = NULL;
   FreeList( account->commands );
   account->commands = NULL;
   FREE( account->namne );
   FREE( account->command_tables );
   FREE( account->commands );
   return;
}

/* i/o */
int load_account_file( const char *path, ACCOUNT_DATA *account )
{
   FILE *fp;
   char *word;
   bool found, done = FALSE;
   int ret = IO_SUCCESS;

   if( ( fp = fopen( path, "r" ) ) == NULL )
      return IO_FAILED_BAD_PATH;

   if( account == NULL )
      return IO_FAILED_NULL_DESTINATION;

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   if( strcmp( word, "#ACCOUNT" ) )
   {
      BAD_FORMAT( word );
      goto to_return:
   }

   while( !done )
   {
      found = FALSE;
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) )
               goto to_return;
            break;
         case 'A':
            if( !strcmp( word, "#ACCOUNT" ) )
            {
               if( ( ret = fread_account_base( account, fp ) ) != IO_SUCCESS )
                  goto to_return:
               found = TRUE;
               break;
            }
            break;
         case 'C':
            if( !strcmp( word, "#CHAR_SHEET" ) )
            {
               if( ( cSheet = fread_char_sheet( fp ) ) != NULL )
               {
                  found = TRUE;
                  AttachToList( cSheet, account->characters );
               }
               break;
            }
            break;
      }
      if( !found )
      {
         BAD_FORMAT( word );
	 goto to_return;
      }
      if( !done )
         word = ( feof ( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   }
   to_return:
      fclose( fp );
      return ret;
}

