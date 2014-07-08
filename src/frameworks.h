/* frameworks.h written by Davenge */

struct entity_framework
{
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;

   LLIST *contents;
   LLIST *specifications;
};
