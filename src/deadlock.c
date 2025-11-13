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
 *           deadlock.c - November 13, 2025
 */
/***************************************************************************
 *  Deadlock Detection - Based on code by Erwin S. Andreasen               *
 *  From merc-l mailing list, July 1996                                    *
 *  Detects if the MUD has frozen and aborts the process                   *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "merc.h"

#define BOOT_DB_ABORT_THRESHOLD          15
#define RUNNING_ABORT_THRESHOLD           5
#define ALARM_FREQUENCY                  20

/* Interval in pulses after which to abort */
int    abort_threshold = BOOT_DB_ABORT_THRESHOLD;
bool_t disable_timer_abort = FALSE;
int    last_checkpoint;

/* find number of CPU seconds spent in user mode so far */
int get_user_seconds ()
{
    struct rusage rus;
    getrusage (RUSAGE_SELF, &rus);
    return rus.ru_utime.tv_sec;
}

/* Update the checkpoint */
void alarm_update ()
{
    last_checkpoint = get_user_seconds();
    if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
    {
        abort_threshold = RUNNING_ABORT_THRESHOLD;
        fprintf (stderr, "Used %d user CPU seconds.\n", last_checkpoint);
    }
}

/* Set the virtual (CPU time) timer to the standard setting, ALARM_FREQUENCY */
void reset_itimer ()
{
    struct itimerval itimer;
    itimer.it_interval.tv_usec = 0; /* milliseconds */
    itimer.it_interval.tv_sec  = ALARM_FREQUENCY;
    itimer.it_value.tv_usec = 0;
    itimer.it_value.tv_sec = ALARM_FREQUENCY;

    /* start the timer - in that many CPU seconds, alarm_handler will be called */    
    if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
    {
        perror ("reset_itimer:setitimer");
        exit (1);
    }
}

const char * szFrozenMessage = "Alarm_handler: Not checkpointed recently, aborting!\n";

/* Signal handler for alarm - suggested for use in MUDs by Fusion */
void alarm_handler (int signo)
{
    int usage_now = get_user_seconds();

    /* Has there gone abort_threshold CPU seconds without alarm_update? */
    if (!disable_timer_abort && (usage_now - last_checkpoint > abort_threshold ))
    {
        /* For the log file */
        write(STDERR_FILENO, szFrozenMessage, strlen(szFrozenMessage)+1);

        signal (signo, SIG_DFL);
        abort();
    }
    
    /* The timer resets to the values specified in it_interval automatically */
}

/* Install signal alarm handler */
void init_alarm_handler()
{
    struct sigaction sa;
    
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART; /* Restart interrupted system calls */
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGVTALRM, &sa, NULL) < 0) /* setup handler for virtual timer */
    {
        perror ("init_alarm_handler:sigaction");
        exit (1);
    }
    last_checkpoint = get_user_seconds();
    reset_itimer(); /* start timer */
}

