/*
 * This file contains the dynamic help system.
 * If you wish to update a help file, simply edit
 * the entry in ../help/ and the mud will load the
 * new version next time someone tries to access
 * that help file.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h> 

/* include main header file */
#include "mud.h"

LLIST     *  help_list = NULL;   /* the linked list of help files     */
char     *  greeting;           /* the welcome greeting              */
char     *  motd;               /* the MOTD help file                */

/*
 * Loads all the helpfiles found in ../help/
 */
void load_helps()
{
  HELP_DATA *new_help;
  char buf[MAX_BUFFER];
  char *s;
  DIR *directory;
  struct dirent *entry;

  log_string("Load_helps: getting all help files.");

  help_list = AllocList();

  directory = opendir("../help/");
  for (entry = readdir(directory); entry; entry = readdir(directory))
  {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    snprintf(buf, MAX_BUFFER, "../help/%s", entry->d_name);
    s = read_help_entry(buf);

    if (s == NULL)
    {
      bug("load_helps: Helpfile %s does not exist.", buf);
      continue;
    }

    if ((new_help = malloc(sizeof(*new_help))) == NULL)
    {
      bug("Load_helps: Cannot allocate memory.");
      abort();
    }

    new_help->keyword    =  strdup(entry->d_name);
    new_help->text       =  strdup(s);
    new_help->load_time  =  time(NULL);
    AttachToList(new_help, help_list);

    if (!strcasecmp("GREETING", new_help->keyword))
      greeting = new_help->text;
    else if (!strcasecmp("MOTD", new_help->keyword))
      motd = new_help->text;
  }
  closedir(directory);
}

