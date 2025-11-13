/*        888                  888                      
 *        888                  888                      
 *        888                  888                      
 *.d8888b 88888b.  8888b.  .d88888 .d88b. 888  888  888.d8888b
 *88K     888 "88b    "88bd88" 888d88""88b888  888  88888k
 *"Y8888b.888  888.d888888888  888888  888888  888  888"Y888b.
 *     X88888  888888  888Y88b 888Y88..88PY88b 888 d88P     X8
 * 88888P'888  888"Y888888 "Y88888 "Y88P"  "Y8888888P" 88888P'
 * 
 *                 888     
 *                 888     
 *                 888     
 *	888d888 .d88b. 88888b.   .d88b. 888d88888888b.  
 *	888P"  d8P  Y8b888 "88bd88""88b888P"  888 "88b 
 *	888    88888888888  888888  888888    888  888 
 *	888    Y8b.    888 d88PY88..88P888    888  888 
 *	888     "Y8888 88888P"  "Y88P" 888    888  888  
 *           Om - Shadows Reborn - v1.0
 *           lastcomm.c - November 13, 2025
 */
/***************************************************************************
 *  Last Command Tracking - Based on code by Erwin S. Andreasen            *
 *  From merc-l mailing list, July 1996                                    *
 *  Tracks the last command executed for crash debugging                   *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "merc.h"
#include "interp.h"

/* Global variable to hold the last input line */
char last_command [MAX_STRING_LENGTH];

/* Write last command to file */
void write_last_command ()
{
    int fd;

    /* Return if no last command - set before normal exit */
    if (!last_command[0])
        return;

    fd = open (LAST_COMMAND_FILE, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

    if (fd < 0)
        return;

    write (fd, last_command, strlen(last_command));
    write (fd, "\n", 1);
    close (fd);
}

void nasty_signal_handler (int no)
{
    write_last_command();
    do_auto_shutdown();
}

/* Call this before starting the game_loop */
void install_other_handlers ()
{
    last_command [0] = '\0';

    if (atexit (write_last_command) != 0)
    {
        perror ("install_other_handlers:atexit");
        exit (1);
    }

    /* should probably check return code here */
    signal (SIGSEGV, nasty_signal_handler);

    /* Possibly other signals could be caught? */
}

/* Find out the last thing that happened before a crash */
/* Add a call to this after booting the database */
void examine_last_command ()
{
    FILE *fp;
    char buf [MAX_STRING_LENGTH];

    fp = fopen (LAST_COMMAND_FILE, "r");
    if (!fp)
        return;

    if (fscanf (fp, "%[^\n]", buf) > 0)
    {
        mud_logf ("Last recorded command before crash: %s", buf);
    }
    fclose (fp);
    unlink (LAST_COMMAND_FILE);
}

/* Auto shutdown function for emergency copyover */
void do_auto_shutdown()
{
   FILE *fp;
   DESCRIPTOR_DATA *d, *d_next;
   char buf [100], buf2[100];
   extern int port, control;
	
   fp = fopen (COPYOVER_FILE, "w");   

   if (!fp)
   {
       for (d = descriptor_list;d != NULL;d = d_next)
       {
           if(d->character)
           {
              do_function( d->character, &do_save, "" );
              send_to_char("Ok I tried but we're crashing anyway sorry!\n\r",d->character);
           }

           d_next=d->next;
           close_socket(d);
       }
         
       exit(1);
   }
	
   do_function( NULL, &do_asave, "changed" );

   sprintf(buf,"\n\rYour mud is crashing attempting a copyover now!\n\r");
   
   for (d = descriptor_list; d ; d = d_next)
   {
	CHAR_DATA * och = CH (d);
	d_next = d->next; /* We delete from the list , so need to save this */
		
	if (!d->character || d->connected > CON_PLAYING) 
	{
	    write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
	    close_socket (d); /* throw'em out */
	}
		
        else
	{
	    fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
	    save_char_obj (och);
	    write_to_descriptor (d->descriptor, buf, 0);
	}
   }
	
   fprintf (fp, "-1\n");
   fclose(fp);
   fclose (fpReserve);
   sprintf (buf, "%d", port);
   sprintf (buf2, "%d", control);
   execl (EXE_FILE, "rom", buf, "copyover", buf2, (char *) NULL);   
   exit(1);
}

