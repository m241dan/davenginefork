/*
 * This is the main headerfile
 */

#ifndef MUD_H
#define MUD_H

#include <zlib.h>
#include <pthread.h>
#include <arpa/telnet.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <mysql.h>

#include "llist.h"
#include "stack.h"

/************************
 * Standard definitions *
 ************************/

/* define TRUE and FALSE */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#define eTHIN   0
#define eBOLD   1

/* A few globals */
#define PULSES_PER_SECOND     4                   /* must divide 1000 : 4, 5 or 8 works */
#define MAX_FRAMEWORK_NSL   255                   /* max for non-text string sql entrees */
#define MAX_BUFFER         1024                   /* seems like a decent amount         */
#define MAX_OUTPUT         4096                   /* well shoot me if it isn't enough   */
#define MAX_HELP_ENTRY     4096                   /* roughly 40 lines of blocktext      */
#define MUDPORT            6500                   /* just set whatever port you want    */
#define FILE_TERMINATOR    "EOF"                  /* end of file marker                 */
#define COPYOVER_FILE      "../txt/copyover.dat"  /* tempfile to store copyover data    */
#define EXE_FILE           "../src/SocketMud"     /* the name of the mud binary         */
#define DB_NAME            "muddb"
#define DB_ADDR            "localhost"
#define DB_LOGIN           "m241dan"
#define DB_PASSWORD        "Grc937!"


/* Connection states */
typedef enum
{
   STATE_NANNY, STATE_ACCOUNT, STATE_OLC, STATE_EFRAME_EDITOR, STATE_PLAYING, STATE_CLOSED, MAX_STATE
} socket_states;

/* Thread states - please do not change the order of these states    */
#define TSTATE_LOOKUP          0  /* Socket is in host_lookup        */
#define TSTATE_DONE            1  /* The lookup is done.             */
#define TSTATE_WAIT            2  /* Closed while in thread.         */
#define TSTATE_CLOSED          3  /* Closed, ready to be recycled.   */

/* player levels */
#define LEVEL_GUEST            1  /* Dead players and actual guests  */
#define LEVEL_PLAYER           2  /* Almost everyone is this level   */
#define LEVEL_ADMIN            3  /* Any admin without shell access  */
#define LEVEL_GOD              4  /* Any admin with shell access     */

typedef enum
{
   LEVEL_BASIC, MAX_ACCOUNT_LEVEL
} account_levels;

typedef enum
{
   ACCOUNT_IDS, WORKSPACE_IDS, ENTITY_FRAMEWORK_IDS, MAX_ID_HANDLER
} id_handler_types;

typedef enum
{
   CHAT_LEVEL, MAX_COMM_LEVEL
} global_chat_levels;

/* Communication Ranges */
#define COMM_LOCAL             0  /* same room only                  */
#define COMM_LOG              10  /* admins only                     */

/* account globals */
#define DEFAULT_PAGEWIDTH     80
#define MAX_CHARACTER          3

typedef enum
{
   TYPE_INT, TYPE_CHAR, TYPE_SOCKET, MAX_MEMORY_TYPE
} memory_types;

typedef enum
{
   RET_SUCCESS, RET_FAILED_BAD_PATH, RET_FAILED_BAD_FORMAT, RET_FAILED_NULL_POINTER,
   RET_FAILED_NO_LIB_ENTRY,
   RET_NO_SQL, RET_DB_NO_ENTRY,
   RET_FAILED_OTHER, MAX_RET_CODES
} ret_codes;

/*****************
 * NANNY INDEXES *
 *****************/
typedef enum
{
   NANNY_LOGIN, NANNY_NEW_ACCOUNT, MAX_NANNY
} nanny_indexes;

/* define simple types */
typedef  unsigned char     bool;
typedef  short int         sh_int;
typedef  unsigned long int VALUE;


extern const unsigned char do_echo       [];
extern const unsigned char dont_echo       [];

/******************************
 * End of standard definitons *
 ******************************/

/***********************
 * Defintion of Macros *
 ***********************/
#define BAD_POINTER( pointer )						\
do									\
{									\
   bug( "%s: BAD POINTER %s.", __FUNCTION__, (pointer) );		\
   ret = RET_FAILED_NULL_POINTER;					\
} while(0)

#define BAD_PATH( path )						\
do									\
{									\
   bug( "%s: BAD PATH %s.", __FUNCTION__, (path) );			\
   ret = RET_FAILED_BAD_PATH;						\
} while(0)

#define BAD_FORMAT( word ) 						\
do									\
{									\
   bug( "%s: BAD FORMAT %s.", __FUNCTION__, (word) );					\
   ret = RET_FAILED_BAD_FORMAT;						\
} while(0)

#define DETACHCONTENTS( LLIST, type )					\
do									\
{									\
   (type) *to_detach;							\		\
   AttachIterator( &Iter, (LLIST) );					\
   while( ( to_detach = (type *)NextInLLIST( &Iter ) ) != NULL )		\
      DetachFromLLIST( to_detach, (LLIST) )				\
   DetachIterator( &Iter );						\
} while(0)

#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }									\
} while(0)

#define FREE(point)                      \
do                                          \
{                                           \
   if( (point) )                            \
   {                                        \
      free( (void*) (point) );              \
      (point) = NULL;                       \
   }                                        \
} while(0)

#define CLEARLIST( list, type )						\
do									\
{									\
   type  *cell; 							\
   ITERATOR clearlist_iter;						\
   AttachIterator( &clearlist_iter, (list) );				\
   while( ( cell = (type *)NextInList( &clearlist_iter ) ) != NULL )	\
      DetachFromList( cell, list );					\
   DetachIterator( &clearlist_iter );					\
} while (0)								\

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) < (b) ? (b) : (a))
#define IS_ADMIN(dMob)          ((dMob->level) > LEVEL_PLAYER ? TRUE : FALSE)
#define IREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    int sValue = fread_number(fp);    \
    sPtr = sValue;                    \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
#define SREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    sPtr = fread_string(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}

/***********************
 * End of Macros       *
 ***********************/

/******************************
 * New structures             *
 ******************************/

/* type defintions */
typedef struct  dSocket       D_SOCKET;
typedef struct  help_data     HELP_DATA;
typedef struct  lookup_data   LOOKUP_DATA;
typedef struct  event_data    EVENT_DATA;
typedef struct  game_account  ACCOUNT_DATA;
typedef struct  nanny_data    NANNY_DATA;
typedef struct  typCmd        COMMAND;
typedef int     nanny_fun( NANNY_DATA *nanny, char *arg );
typedef const struct nanny_lib_entry NANNY_LIB_ENTRY;
typedef struct  id_handler    ID_HANDLER;
typedef struct  id_tag        ID_TAG;
typedef struct  entity_framework ENTITY_FRAMEWORK;
typedef struct  inception_olc  INCEPTION;
typedef struct  workspace      WORKSPACE;
typedef struct  typSpec        SPECIFICATION;

/* the actual structures */
struct dSocket
{
  NANNY_DATA    * nanny;
  LLIST          * events;
  char          * hostname;
  char            inbuf[MAX_BUFFER];
  char            outbuf[MAX_OUTPUT];
  char            next_command[MAX_BUFFER];
  bool            bust_prompt;
  sh_int          lookup_status;
  sh_int          state;
  sh_int          control;
  sh_int          top_output;
  unsigned char   compressing;                 /* MCCP support */
  z_stream      * out_compress;                /* MCCP support */
  unsigned char * out_compress_buf;            /* MCCP support */

   ACCOUNT_DATA *account;
};

struct help_data
{
  time_t          load_time;
  char          * keyword;
  char          * text;
};

struct lookup_data
{
  D_SOCKET       * dsock;   /* the socket we wish to do a hostlookup on */
  char           * buf;     /* the buffer it should be stored in        */
};

struct typCmd
{
   char      * cmd_name;
   void     (* cmd_funct)(void *passed, char *arg);
   sh_int      level;
   LLIST     *sub_commands;
   bool      can_sub;
   const char *(*desc_func)( void *extra );
   COMMAND   *from_table;
};

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;

/* here we include external structure headers */
#include "event.h"
#include "account.h"
#include "strings_table.h"
#include "nanny.h"
#include "interpret.h"
#include "id-handler.h"
#include "communication.h"
#include "olc.h"
#include "frameworks.h"
#include "editor.h"
#include "specifications.h"

/******************************
 * End of new structures      *
 ******************************/

/***************************
 * Global Variables        *
 ***************************/

extern  STACK        *  dsock_free;       /* the socket free LLIST               */
extern  LLIST        *  dsock_list;       /* the linked LLIST of active sockets  */
extern  LLIST        *  account_list;     /* the linked List of active accounts */
extern  LLIST        *  active_wSpaces;   /* a linked list for active work spaces */
extern  LLIST	     *	active_OLCs;	  /* a linked list of active OLCs */
extern  LLIST        *  active_frameworks; /* a linked list of active frameworks */
extern  LLIST        *  help_list;        /* the linked LLIST of help files      */
extern  MYSQL        *  sql_handle;       /* global connection to sql database */
extern  const struct    typCmd tabCmd[];  /* the command table                  */
extern  bool            shut_down;        /* used for shutdown                  */
extern  char        *   greeting;         /* the welcome greeting               */
extern  char        *   motd;             /* the MOTD help file                 */
extern  int             control;          /* boot control socket thingy         */
extern  time_t          current_time;     /* let's cut down on calls to time()  */

/***************************
 * End of Global Variables *
 ***************************/

/***********************
 *    MCCP support     *
 ***********************/

extern const unsigned char compress_will[];
extern const unsigned char compress_will2[];

#define TELOPT_COMPRESS       85
#define TELOPT_COMPRESS2      86
#define COMPRESS_BUF_SIZE   8192

/***********************
 * End of MCCP support *
 ***********************/

/***********************************
 * Prototype function declerations *
 ***********************************/

/* more compact */
#define  D_S         D_SOCKET

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

char  *crypt                  ( const char *key, const char *salt );

/*
 * socket.c
 */
int   init_socket             ( void );
bool  new_socket              ( int sock );
void  close_socket            ( D_S *dsock, bool reconnect );
bool  read_from_socket        ( D_S *dsock );
bool  text_to_socket          ( D_S *dsock, const char *txt );  /* sends the output directly */
void  text_to_buffer          ( D_S *dsock, const char *txt );  /* buffers the output        */
void  next_cmd_from_buffer    ( D_S *dsock );
bool  flush_output            ( D_S *dsock );
void  handle_new_connections  ( D_S *dsock, char *arg );
void  clear_socket            ( D_S *sock_new, int sock );
void  recycle_sockets         ( void );
void *lookup_address          ( void *arg );

/*
 * interpret.c
 */
void  handle_cmd_input        ( D_S *dsock, char *arg );

/*
 * io.c
 */
void    log_string            ( const char *txt, ... );
void    bug                   ( const char *txt, ... );
time_t  last_modified         ( char *helpfile );
char   *read_help_entry       ( const char *helpfile );     /* pointer         */
char   *fread_line            ( FILE *fp );                 /* pointer         */
char   *fread_string          ( FILE *fp );                 /* allocated data  */
char   *fread_word            ( FILE *fp );                 /* pointer         */
int     fread_number          ( FILE *fp );                 /* just an integer */

/*
 * socket.c
 */
int change_socket_state( D_SOCKET *dsock, int state );

/* 
 * strings.c
 */
char   *one_arg               ( char *fStr, char *bStr );
char   *strdup                ( const char *s );
int     strcasecmp            ( const char *s1, const char *s2 );
bool    is_prefix             ( const char *aStr, const char *bStr );
char   *capitalize            ( char *txt );
char   *downcase              ( char *txt );
BUFFER *__buffer_new          ( int size );
void    __buffer_strcat       ( BUFFER *buffer, const char *text );
void    buffer_free           ( BUFFER *buffer );
void    buffer_clear          ( BUFFER *buffer );
int     bprintf               ( BUFFER *buffer, char *fmt, ... );
int     mud_printf            ( char *dest, const char *format, ... );
const char *print_header( const char *title, const char *pattern, int width );
void bprint_commandline( void *extra, BUFFER *buf, COMMAND *com, int sublevel, int pagewidth );
void print_commands( void *extra, LLIST *commands, BUFFER *buf, int sublevel, int pagewidth );
char *strip_cr( const char *str );
char *strip_nl( const char *str );
const char *handle_pagewidth( int width, const char *txt );
bool is_number( const char *arg );
char *smash_color( const char *str );
int color_count( const char *str );
void add_spaces( char *str, int amount );
void add_lead_space( char *str, int amount );
char *center_string( const char *to_center, int length );
char *fit_string_to_space( const char *orig, int space );
char *print_bar( const char *pattern, int width );
/*
 * help.c
 */
void  load_helps              ( void );

/*
 * utils.c
 */
bool  check_name              ( const char *name );
void  load_muddata            ( bool fCopyOver );
char *get_time                ( void );
bool check_sql( void );
void report_sql_error( MYSQL *con );
bool quick_query( const char *format, ...);
/*
 * mccp.c
 */
bool  compressStart           ( D_S *dsock, unsigned char teleopt );
bool  compressEnd             ( D_S *dsock, unsigned char teleopt, bool forced );

/*******************************
 * End of prototype declartion *
 *******************************/

#endif  /* MUD_H */
