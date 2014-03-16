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
   account->socket = NULL;
   FreeList( account->characters );
   account->characters = NULL;
   FreeList( account->command_tables );
   account->command_tables = NULL;
   FreeList( account->commands );
   account->commands = NULL;
   FREE( account->name );
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
   int ret = RET_SUCCESS;

   if( ( fp = fopen( path, "r" ) ) == NULL )
      return RET_FAILED_BAD_PATH;

   if( account == NULL )
   {
      BAD_POINTER( "account" );
      return ret;
   }

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   if( strcmp( word, "#ACCOUNT" ) )
   {
      BAD_FORMAT( word );
      goto to_return;
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
               if( ( ret = fread_account_base( account, fp ) ) != RET_SUCCESS )
                  goto to_return;
               found = TRUE;
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

int save_account( ACCOUNT_DATA *account )
{
   FILE *fp;
   char aDir[MAX_BUFFER], aFile[MAX_BUFFER];
   int ret = RET_SUCCESS;

   /* create the "file directory" for the account, so we can check if it exists */
   mud_printf( aDir, "../accounts/%s", capitalize( account->name ) );

   if( opendir( aDir ) == NULL )
   {
      if( ( mkdir( aDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) != 0 ) /* was unsuccessful */
      {
         bug( "Unable to create folder for the new account: %s", account->name );
         return RET_FAILED_OTHER;
      }
   }

   /* create the "file name" for the account */
   mud_printf( aFile, "%s/account.afile", aDir );

   /* open the pointer to the file, if we can't, spit out a bug msg and return */
   if( ( fp = fopen( aFile, "w" ) ) == NULL )
   {
      BAD_PATH( aFile );
      return RET_FAILED_BAD_PATH;
   }

   if( ( ret = fwrite_account_base( account, fp ) ) != RET_SUCCESS )
      goto to_return;

   to_return:
      fprintf( fp, "%s\n", FILE_TERMINATOR );
      fclose( fp );
      return ret;
}

int fwrite_account_base( ACCOUNT_DATA *account, FILE *fp )
{
   fprintf( fp, "Name          %s~\n", account->name );
   fprintf( fp, "Password      %s~\n", account->password );
   fprintf( fp, "Pagewidth     %d\n", account->pagewidth );
   fprintf( fp, "Level         %d\n", account->level );
   return RET_SUCCESS;

}

int fread_account_base( ACCOUNT_DATA *account, FILE *fp )
{
   char *word;
   bool found, done = FALSE;
   int ret;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );

   while( !done )
   {
      found = FALSE;

      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ) { goto to_return; }
            break;
         case 'L':
            IREAD( "Level", account->level );
            break;
         case 'N':
            SREAD( "Name", account->name );
            break;
         case 'P':
            IREAD( "PageWidth", account->pagewidth );
            SREAD( "Password", account->password );
            break;
      }
      if( !found )
      {
         BAD_FORMAT( word );
         goto to_return;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   to_return:
      fclose( fp );
      return ret;
}

