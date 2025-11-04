/*---
 | Add to merc.h, with the typedefs:
 ---*/
typedef struct recent_data RECENT_DATA;


/*---
 | Add to merc.h, some place with the rest of the structure definitions:
 ---*/
struct recent_data {
   RECENT_DATA *prev;
   RECENT_DATA *next;       /* Thanks to Kyndig for reporting my err. */
   char        name[16];
   time_t      logOn;
   time_t      logOff;
   bool        used;
};


/*---
 | Add to merc.h, with the rest of the globals:
 ---*/
extern RECENT_DATA * recent_first;
extern RECENT_DATA * recent_free;


/*---
 | Add to merc.h, with the rest of the DECLARE_DO_FUN( ):
 ---*/
DECLARE_DO_FUN( do_recent );


/*---
 | Add to merc.h, with the "db.c" prototypes:
 ---*/
bool   recent_create   args( ( sh_int size ) );
bool   recent_add      args( ( CHAR_DATA *ch ) );


/*---
 | Add to merc.h, with the "handler.c" prototypes:
 ---*/
void   timeString      args( ( time_t time, char here[16] ) );


/*---
 | Add to act_info.c, at the bottom, this function:
 ---*/
void do_recent( CHAR_DATA *ch, char *argument ) {
   if( ch == NULL ) {
      return;
   }
   else if( recent_first == NULL || recent_free == NULL ) {
      send_to_char("There have been no recent logoffs.\n\r", ch );
      return;
   }
   else {
      RECENT_DATA *r;
      char buf[MAX_STRING_LENGTH], lo[16], lf[16];

      sprintf( buf, " %-15s | %-15s | %-15s\n\r", "Character",
         "Log on time", "Log off time" );
      send_to_char( buf, ch );
      send_to_char("-----------------------------------------------------\n\r",
         ch );

      for( r = recent_free-&gt;next; r != recent_free &amp;&amp; r != NULL; r = r-&gt;next ){
         if( r-&gt;used ) {
            timeString( r-&gt;logOn , lo );
            timeString( r-&gt;logOff, lf );
            sprintf( buf, " %-15s | %-15s | %-15s\n\r", r-&gt;name, lo, lf );
            send_to_char( buf, ch );
         }
      }
   }
   return;
}


/*---
 | Add to db.c, with the rest of the globals:
 ---*/
RECENT_DATA * recent_first;
RECENT_DATA * recent_free;


/*---
 | Add to db.c, at the bottom, this function:
 ---*/
bool recent_create( sh_int size ) {
   if( size &lt; 1 ) {
      bug( "recent_create: passed a size &lt; 1, size: %d", size );
      return FALSE;
   }
   else {
      RECENT_DATA *r;
      int count;

      if( recent_first == NULL ) {
         recent_first          = alloc_perm( sizeof( *r ) );
         recent_first-&gt;prev    = NULL;
         recent_first-&gt;next    = NULL;
         recent_first-&gt;name[0] = '\0';
         recent_first-&gt;logOn   = time(0);
         recent_first-&gt;logOff  = time(0);
         recent_first-&gt;used    = FALSE;
         recent_free           = recent_first;
      }

      for( count = 0; count &lt; size; count++ ) {
         r          = alloc_perm( sizeof( *r ) );
         r-&gt;prev    = NULL;
         r-&gt;next    = NULL;
         r-&gt;name[0] = '\0';
         r-&gt;logOn   = time(0);
         r-&gt;logOff  = time(0);
         r-&gt;used    = FALSE;

         recent_first-&gt;prev = r; /* look back to new entry */
         r-&gt;next = recent_first; /* look ahead to recent_first */
         recent_first = r;       /* push recent_first, take its place */
      }

      return TRUE;
   }
   return FALSE;
}


/*---
 | Add to db.c, at the bottom, this function:
 ---*/
bool recent_add( CHAR_DATA *ch ) {
   if( ch == NULL ) {
      return FALSE;
   }
   if( recent_free == NULL || recent_first == NULL ) {
      if( !recent_create( 20 ) ) {
         bug("recent_add: call \'recent_create( 20 )\' failed.", 0 );
         return FALSE;
      }
   }

   sprintf( recent_free-&gt;name, ch-&gt;name );
   recent_free-&gt;logOn  = ch-&gt;logon;
   recent_free-&gt;logOff = time(0);
   recent_free-&gt;used   = TRUE;
   recent_free         = recent_free-&gt;prev;

   return TRUE;
}


/*---
 | Add to handler.c, at the bottom, this function:
 ---*/
void timeString( time_t time, char here[16] ) {
   int  count;
   char buf[MAX_STRING_LENGTH], tim[16];

   /* Turn the time into a string. */
   sprintf( buf, ctime( &amp;time ) );

   /* Chop the day and year. */
   for( count = 0; count &lt; 16; count++ ) {
      tim[count] = buf[count+4];
   }

   tim[15] = '\0';
   sprintf( here, tim );
   return;
}


/*---
 | In act_comm.c, in the function "do_quit", find this block of code:
 ---*/
sprintf( log_buf, "%s has quit.", ch-&gt;name );
log_string( log_buf );


/*---
 | Right after that add this line:
 ---*/
recent_add( ch );


/*---
 | Closing comments:
 ---*/

