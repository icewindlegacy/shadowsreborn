/*        888                  888                      
 *        888                  888                      
 *        888                  888                      
 *.d8888b 88888b.  8888b.  .d88888 .d88b. 888  888  888.d8888b
 *88K     888 "88b    "88bd88" 888d88""88b888  888  88888k
 *"Y8888b.888  888.d888888888  888888  888888  888  888"Y888b.
 *     X88888  888888  888Y88b 888Y88..88PY88b 888 d88P     X8
 * 88888P'888  888"Y888888 "Y88888 "Y88P"  "Y8888888P" 88888P'
 * 
 *                       888     
 *                       888     
 *                       888     
 *	888d888 .d88b. 88888b.   .d88b. 888d88888888b.  
 *	888P"  d8P  Y8b888 "88bd88""88b888P"  888 "88b 
 *	888    88888888888  888888  888888    888  888 
 *	888    Y8b.    888 d88PY88..88P888    888  888 
 *	888     "Y8888 88888P"  "Y88P" 888    888  888  
 *           Om - Shadows Reborn - v1.0
 *           clan.c - November 3, 2025
 */            
/* **************************************************************
  December 27th, 1997
  Gothar's Clan Systems Version 1.0
  1997 Copyright <* FREEWARE *>
  If you use this code:
     1.Keep all the credits in the code.
     2.Use the help entry for bank
     3.Send a bug report,any comments or ideas
       Ian McCormick (aka Gothar)
       mcco0055@algonquinc.on.ca

Here is a listing of what the code does:
 1. Allows imms to give leadership to players
 2. Leaders of clans (not imms) guild players
 3. Information listing of the clans

************************************************************** */
/* N.B: READ this code before you ADD anything!!
 *      This is clan.c plus a patch file for other files.
 */
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "clan.h"

#define OUTCAST		2
#define INDEP		0
 
/* for clans */
/* 
 * CLAN NAME   CLAN NUMBER 
 * Independent  clan = 0
 * Rom          clan = 1
 * Outcast      clan = 2
 * Loner        clan = 3
 * Hades        clan = 4
 * Light        clan = 5
 */
struct clan_type clan_table[MAX_CLAN];
 

void do_cleader( CHAR_DATA *ch, char *argument )
{
  /* 
   * Gothar's Clan system 
   * Immortal command to create clan leaders
   */
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* Lists all possible clans */
    if (arg1[0] == '\0'|| arg2[0] == '\0')
    {
        send_to_char( "The Shadows Reborn Clan list:\n\r",ch);
        /* 
         * Won't print the independent
         */
        for(clan = 0; clan < MAX_CLAN; clan++)
	{
            if(!clan_table[clan].independent)
            {
               sprintf( buf, "  %s\n\r", clan_table[clan].name);
               send_to_char( buf, ch);
            }
        }
        send_to_char( "\n\rSyntax: cleader <clan name> <char>\n\r",ch);
        send_to_char( "If <clan name> is 'none' clan leadership has been removed.\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
        send_to_char( "No character by that name exists.\n\r", ch );
        return;
    }
    if(!str_cmp(arg1,"none"))
    {
	 sprintf( buf,"You remove the burden of leadership from %s.\n\r",victim->name);
         send_to_char( buf, ch);
	 sprintf( buf, "You aren't a leader of %s clan anymore!\n\r",
                      clan_table[victim->clan].name);
         send_to_char( buf, victim);
	 victim->pcdata->leader = -1; /* NULL leadership of clan */ 
 	 return;
    }
    if((clan = clan_lookup(arg1)) == INDEP)
    {
	send_to_char("No such clan exists.\n\r",ch);
	return;
    }
    if(clan_table[clan].independent)
    {
       send_to_char("Outcast and Loner aren't clans!\n\r",ch);
       return;
    }
    else  /* is a TRUE clan */
    {
        if(ch->pcdata->leader == -1)
        {
	   sprintf(buf,"%s is now a leader of %s.\n\r",victim->name,clan_table[clan].name);
	   send_to_char(buf,ch);
	   sprintf(buf,"You are now a leader of %s.\n\r",clan_table[clan].name);
	   send_to_char(buf,victim);
	}
	else
	{
	   send_to_char( "They are already hold leadership of a clan.\n\r",ch);
	   return;
	}   
    }
    victim->pcdata->leader = clan; /* SET leadership of clan */
    victim->clan = clan; /* make member of the clan */
}

void do_clanlist( CHAR_DATA *ch, char *argument )
{
  /* 
   * Gothar's Clan system 
   * Command to see a listing of the clans
   */
    char buf[MAX_STRING_LENGTH];
    int clan;

    send_to_char("'The Clans of Shadows Reborn'\n\r",ch);
    send_to_char(" Clan Name       Min Level \n\r",ch);
    send_to_char("---------------------------\n\r",ch);
    for(clan = 0; clan < MAX_CLAN; clan++)
    {
       sprintf( buf, "%-17s    %-5d\n\r",
                clan == INDEP ? "Independent" : clan_table[clan].name,
                clan_table[clan].min_level);
       send_to_char( buf, ch);
    }
    return;
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    /*
     * Unchanged just moved for convenience
     * to keep all clan stuff together.
     *  Gothar -1997.
     */
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (!is_clan(ch) || clan_table[ch->clan].independent)
    {
	send_to_char("You aren't in a clan.\n\r",ch);
	return;
    }
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCLAN))
      {
        send_to_char("Clan channel is now ON\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOCLAN);
      }
      else
      {
        send_to_char("Clan channel is now OFF\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCLAN);
      }
      return;
    }
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
       send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
       return;
    }
    REMOVE_BIT(ch->comm,COMM_NOCLAN);
    sprintf( buf, "You tell the clan '%s'\n\r", argument );
    send_to_char( buf, ch );
    sprintf( buf, "$n tells the clan '%s'", argument );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING 
             && d->character != ch 
             && is_same_clan(ch,d->character) 
             && !IS_SET(d->character->comm,COMM_NOCLAN) 
             && !IS_SET(d->character->comm,COMM_QUIET) )
        {
            act_new("$n tell the clan '$t'",ch,argument,d->character,TO_VICT,POS_DEAD);
        }
    }
    return;
}



void do_cguild( CHAR_DATA *ch, char *argument )
{
  /* 
   * Gothar's Clan system 
   * Clan Leader command to guild clan members
   */
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;

    argument = one_argument( argument, arg );
    clan = ch->clan; /* setting guildee to leader's clan*/
    if(!IS_CLAN(ch))
    {
       /* not in a clan can't use the cguild command */
       sprintf( buf,"Sorry %s, you aren't in a clan.\n\r",ch->name);
       send_to_char( buf,ch);
       return;
    }
    if(arg[0] == '\0')
    {
       /* the syntax for the cguild command */
       send_to_char( "Syntax: cguild <char>\n\r",ch);
       send_to_char( "Type 'help guild' for more information.\n\r",ch);
       send_to_char( "Also see 'help outcast'.\n\r",ch);
       return;
    }
    if( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "No character by that name exists.\n\r", ch );
        return;
    }
       /* Remove Outcast flag */
    if((IS_IMMORTAL(ch) && IS_CLAN(victim)) && (victim->clan == OUTCAST))
    {
        sprintf(buf,"You remove the outcast flag from %s\n\r",victim->name);
        send_to_char(buf,ch);
        send_to_char("Your shame as an Outcast has ended.\n\r", victim);
        victim->clan = INDEP;
        return;
    }
    if(ch->pcdata->leader != clan)
    {
       sprintf(buf,"You aren't a leader of %s clan.\n\r",clan_table[clan].name);
       send_to_char(buf,ch);
       return;
    }
    if(victim->clan == OUTCAST ) /* OUTCAST group */
    {
	 sprintf( buf, "%s is an outcast from the clans.\n\r"
                     "They can't join %s.\n\r",
                     victim->name, clan_table[clan].name);
       send_to_char( buf, ch);
	 sprintf( buf, "You are an outcast!  You can't join a clan."
                     "\n\r Type 'help outcast' for more information.\n\r");
       send_to_char( buf, victim);
       return;
    }
    else if(victim->clan == clan)
    {  
       /* leader outcasts victim */
	 send_to_char("They are now outcast from the clan.\n\r",ch);
	 sprintf( buf, "You have been outcast from %s clan!\n\r",
                      clan_table[clan].name);
       send_to_char( buf, victim);
	 victim->clan = OUTCAST; /* OUTCAST GROUP */
       send_to_char( "Type 'help outcast' for more information.\n\r",ch);
 	 return;
    }
    else if( clan_table[victim->clan].name[0] == '\0')
    {
       if (victim->level >= clan_table[clan].min_level)
       {
          /* victim joins the clan */
          sprintf(buf,"They are now a proud member of %s.\n\r",clan_table[clan].name);
          send_to_char(buf,ch);
          sprintf(buf,"You are now a proud member of %s.\n\r",clan_table[clan].name);
          send_to_char(buf,victim);
          victim->clan = clan;
       }   
       else
       {
          /*  victim's level not high 
           *  enough to join the clan 
           */
          sprintf(buf,"%s's level is too low to join the clan.\n\r"
                      "The requirement was %d.\n\r",
                      victim->name,clan_table[clan].min_level);
          send_to_char(buf,ch);
          sprintf(buf,"Your level is too low to join %s.\n\r"
                      "The requirement was %d.\n\r",
                      clan_table[clan].name,
                      clan_table[clan].min_level);
          send_to_char(buf,victim);
       }
    }
    else
    {
       sprintf( buf, "%s is a member of %s.\n\r",
                     victim->name, clan_table[clan].name);
       send_to_char( buf, ch);
       send_to_char( "You are already in a clan.\n\r", victim);
       return;
    }
    
}


/* returns clan number */
int clan_lookup (const char *name)
{
   int clan;

   for ( clan = 0; clan < MAX_CLAN; clan++)
   {
	if (clan_table[clan].name != NULL && clan_table[clan].name[0] != '\0'
	&&  LOWER(name[0]) == LOWER(clan_table[clan].name[0])
	&&  !str_prefix( name,clan_table[clan].name))
	    return clan;
   }

   return 1; /* none found -return independent */
}

void do_roster( CHAR_DATA *ch, char *argument )
{
    /*
     * Clan Leader command to see roster of clan members
     */
    CHAR_DATA *member;
    int clan;
    int count = 0;
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    
    if (IS_NPC(ch))
        return;
    
    /* Check if character is a clan leader */
    if (ch->pcdata->leader < 0 || ch->pcdata->leader >= MAX_CLAN)
    {
        send_to_char ("You are not a leader of any clan.\n\r", ch);
        return;
    }
    
    clan = ch->pcdata->leader;
    
    /* Verify leader is in the clan they lead */
    if (ch->clan != clan)
    {
        sprintf(buf, "DEBUG: Your clan field is %d, but you're leader of clan %d (%s).\n\r",
                ch->clan, clan, clan_table[clan].name ? clan_table[clan].name : "Unknown");
        send_to_char(buf, ch);
        send_to_char ("You are not a member of the clan you lead.\n\r", ch);
        send_to_char ("Use 'induct <yourself>' to fix this.\n\r", ch);
        return;
    }
    
    if (clan_table[clan].independent)
    {
        send_to_char ("Independent clans don't have rosters.\n\r", ch);
        return;
    }
    
    output = new_buf();
    
    sprintf(buf, "Roster of %s\n\r", 
            clan_table[clan].name ? clan_table[clan].name : "Unknown Clan");
    add_buf(output, buf);
    add_buf(output, "----------------------------------------\n\r");
    
    /* Iterate through all characters */
    for (member = char_list; member != NULL; member = member->next)
    {
        /* Skip NPCs */
        if (IS_NPC(member))
            continue;
        
        /* Only show members of this clan */
        if (member->clan != clan)
            continue;
        
        count++;
        
        /* Format: Name [Level] [Class] [Status] */
        sprintf(buf, "%-15s [%3d] %-12s",
                member->name,
                member->level,
                IS_NPC(member) ? "mobile" : class_table[member->class].name);
        
        /* Add online/offline status */
        if (member->desc && member->desc->connected == CON_PLAYING)
        {
            /* Online - show location if can see */
            if (member->in_room && can_see(ch, member))
            {
                if (member->in_room->name)
                    sprintf(buf + strlen(buf), " - %s", member->in_room->name);
            }
            else
            {
                strcat(buf, " - Online");
            }
        }
        else
        {
            strcat(buf, " - Offline");
        }
        
        /* Mark leader if they are a leader */
        if (member->pcdata && member->pcdata->leader == clan)
        {
            strcat(buf, " [Leader]");
        }
        
        strcat(buf, "\n\r");
        add_buf(output, buf);
    }
    
    if (count == 0)
    {
        add_buf(output, "No members found.\n\r");
    }
    else
    {
        sprintf(buf, "\n\rTotal members: %d\n\r", count);
        add_buf(output, buf);
    }
    
    page_to_char(buf_string(output), ch);
    free_buf(output);
    return;
}

/* Save clans to file */
void save_clans (void)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int i;
    
    sprintf (buf, "%sclans.lst", CLAN_DIR);
    
    if (!(fp = fopen (buf, "w")))
    {
        bugf ("Could not open file %s to save clans.", buf);
        return;
    }
    
    for (i = 0; i < MAX_CLAN; i++)
    {
        fprintf (fp, "#Clan %d\n", i);
        fprintf (fp, "Name: %s\n", clan_table[i].name ? clan_table[i].name : "");
        fprintf (fp, "Who: %s\n", clan_table[i].who_name ? clan_table[i].who_name : "");
        fprintf (fp, "Deathroom: %d\n", clan_table[i].deathroom);
        fprintf (fp, "Recall: %d\n", clan_table[i].recall);
        fprintf (fp, "Independent: %s\n", clan_table[i].independent ? "true" : "false");
        fprintf (fp, "MinLevel: %d\n", clan_table[i].min_level);
        fprintf (fp, "\n");
    }
    
    fclose (fp);
    return;
}

/* Load clans from file */
void load_clans (void)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char line[MAX_STRING_LENGTH];
    int i;
    int clan_num = -1;
    char name_buf[MAX_INPUT_LENGTH];
    char who_buf[MAX_INPUT_LENGTH];
    int deathroom = 0;
    int recall = 0;
    bool independent = FALSE;
    int min_level = 0;
    
    /* Initialize all clans to empty */
    for (i = 0; i < MAX_CLAN; i++)
    {
        clan_table[i].name = NULL;
        clan_table[i].who_name = NULL;
        clan_table[i].deathroom = ROOM_VNUM_MORGUE;
        clan_table[i].recall = ROOM_VNUM_TEMPLE;
        clan_table[i].independent = FALSE;  /* Default to FALSE, set TRUE only for clan 0 */
        clan_table[i].min_level = 0;
    }
    
    /* Clan 0 is always the Independent clan */
    clan_table[0].independent = TRUE;
    
    /* Ensure directory exists */
    {
        char dir_buf[MAX_STRING_LENGTH];
        struct stat st;
        strncpy (dir_buf, CLAN_DIR, sizeof (dir_buf) - 1);
        dir_buf[sizeof (dir_buf) - 1] = '\0';
        
        /* Remove trailing slash if present for stat */
        if (dir_buf[strlen(dir_buf) - 1] == '/')
            dir_buf[strlen(dir_buf) - 1] = '\0';
        
        if (stat (dir_buf, &st) != 0)
        {
            /* Directory doesn't exist, create it */
            if (mkdir (dir_buf, 0755) != 0)
            {
                bugf ("Could not create directory %s: %s", dir_buf, strerror(errno));
            }
            else
            {
                bugf ("Created clan directory: %s", dir_buf);
            }
        }
    }
    
    sprintf (buf, "%sclans.lst", CLAN_DIR);
    
    if (!(fp = fopen (buf, "r")))
    {
        /* File doesn't exist - create it with defaults */
        bugf ("Clan file %s does not exist. Creating initial file.", buf);
        
        /* Set default clan 0 (Independent) */
        clan_table[0].name = str_dup ("");
        clan_table[0].who_name = str_dup ("");
        clan_table[0].deathroom = ROOM_VNUM_MORGUE;
        clan_table[0].recall = ROOM_VNUM_TEMPLE;
        clan_table[0].independent = TRUE;
        clan_table[0].min_level = 0;
        
        /* Initialize remaining clans with empty defaults */
        for (i = 1; i < MAX_CLAN; i++)
        {
            clan_table[i].name = str_dup ("");
            clan_table[i].who_name = str_dup ("");
            clan_table[i].deathroom = ROOM_VNUM_MORGUE;
            clan_table[i].recall = ROOM_VNUM_TEMPLE;
            clan_table[i].independent = TRUE;
            clan_table[i].min_level = 0;
        }
        
        /* Save the initial file */
        save_clans ();
        
        return;
    }
    
    name_buf[0] = '\0';
    who_buf[0] = '\0';
    
    while (fgets (line, sizeof (line), fp) != NULL)
    {
        /* Remove newline */
        line[strcspn (line, "\r\n")] = '\0';
        
        /* Skip empty lines */
        if (line[0] == '\0')
            continue;
        
        /* Check for clan header */
        if (line[0] == '#' && strstr (line, "Clan") != NULL)
        {
            /* Save previous clan if we have one */
            if (clan_num >= 0 && clan_num < MAX_CLAN)
            {
                /* Free old strings if they exist */
                if (clan_table[clan_num].name)
                    free_string (clan_table[clan_num].name);
                if (clan_table[clan_num].who_name)
                    free_string (clan_table[clan_num].who_name);
                
                clan_table[clan_num].name = str_dup (name_buf);
                clan_table[clan_num].who_name = str_dup (who_buf);
                clan_table[clan_num].deathroom = deathroom;
                clan_table[clan_num].recall = recall;
                clan_table[clan_num].independent = independent;
                clan_table[clan_num].min_level = min_level;
            }
            
            /* Parse clan number */
            sscanf (line, "#Clan %d", &clan_num);
            if (clan_num < 0 || clan_num >= MAX_CLAN)
                clan_num = -1;
            
            /* Reset values */
            name_buf[0] = '\0';
            who_buf[0] = '\0';
            deathroom = ROOM_VNUM_MORGUE;
            recall = ROOM_VNUM_TEMPLE;
            independent = FALSE;  /* Default to real clan, not independent */
            min_level = 0;
            continue;
        }
        
        /* Parse fields */
        if (strncasecmp (line, "Name:", (size_t)5) == 0)
        {
            char *val = line + 5;
            while (*val == ' ' || *val == '\t')
                val++;
            strncpy (name_buf, val, sizeof (name_buf) - 1);
            name_buf[sizeof (name_buf) - 1] = '\0';
        }
        else if (strncasecmp (line, "Who:", (size_t)4) == 0)
        {
            char *val = line + 4;
            while (*val == ' ' || *val == '\t')
                val++;
            strncpy (who_buf, val, sizeof (who_buf) - 1);
            who_buf[sizeof (who_buf) - 1] = '\0';
        }
        else if (strncasecmp (line, "Deathroom:", (size_t)10) == 0)
        {
            deathroom = atoi (line + 10);
        }
        else if (strncasecmp (line, "Recall:", (size_t)7) == 0)
        {
            recall = atoi (line + 7);
        }
        else if (strncasecmp (line, "Independent:", (size_t)12) == 0)
        {
            char *val = line + 12;
            while (*val == ' ' || *val == '\t')
                val++;
            independent = (strcasecmp (val, "true") == 0);
        }
        else if (strncasecmp (line, "MinLevel:", (size_t)9) == 0)
        {
            min_level = atoi (line + 9);
        }
    }
    
    /* Save last clan */
    if (clan_num >= 0 && clan_num < MAX_CLAN)
    {
        if (clan_table[clan_num].name)
            free_string (clan_table[clan_num].name);
        if (clan_table[clan_num].who_name)
            free_string (clan_table[clan_num].who_name);
        
        clan_table[clan_num].name = str_dup (name_buf);
        clan_table[clan_num].who_name = str_dup (who_buf);
        clan_table[clan_num].deathroom = deathroom;
        clan_table[clan_num].recall = recall;
        clan_table[clan_num].independent = independent;
        clan_table[clan_num].min_level = min_level;
    }
    
    fclose (fp);
    return;
} 

/* Quick command to manage clan leadership */
void do_clanleader( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan_num;
    
    if (IS_NPC(ch))
        return;
        
    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Only immortals can manage clan leadership.\n\r", ch);
        return;
    }
    
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    
    if (arg1[0] == '\0')
    {
        send_to_char("Syntax: clanleader <player> <clan#|none>\n\r", ch);
        send_to_char("        clanleader <player> show - show current settings\n\r", ch);
        return;
    }
    
    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
        send_to_char("They aren't playing.\n\r", ch);
        return;
    }
    
    if (IS_NPC(victim))
    {
        send_to_char("Not on NPCs.\n\r", ch);
        return;
    }
    
    if (arg2[0] == '\0' || !str_cmp(arg2, "show"))
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%s's clan settings:\n\r", victim->name);
        send_to_char(buf, ch);
        sprintf(buf, "  Clan:   %d (%s)\n\r", victim->clan,
                victim->clan >= 0 && victim->clan < MAX_CLAN ? 
                (clan_table[victim->clan].name ? clan_table[victim->clan].name : "Empty") : "Invalid");
        send_to_char(buf, ch);
        sprintf(buf, "  Leader: %d", victim->pcdata->leader);
        send_to_char(buf, ch);
        if (victim->pcdata->leader >= 0 && victim->pcdata->leader < MAX_CLAN)
        {
            sprintf(buf, " (leader of %s)", 
                    clan_table[victim->pcdata->leader].name ? 
                    clan_table[victim->pcdata->leader].name : "Empty");
            send_to_char(buf, ch);
        }
        send_to_char("\n\r", ch);
        return;
    }
    
    if (!str_cmp(arg2, "none"))
    {
        victim->pcdata->leader = -1;
        send_to_char("Clan leadership removed.\n\r", ch);
        sprintf(buf, "%s has removed your clan leadership.\n\r", ch->name);
        send_to_char(buf, victim);
        return;
    }
    
    clan_num = atoi(arg2);
    if (clan_num < 0 || clan_num >= MAX_CLAN)
    {
        send_to_char("Invalid clan number.\n\r", ch);
        return;
    }
    
    if (!clan_table[clan_num].name || clan_table[clan_num].name[0] == '\0')
    {
        send_to_char("That clan slot is empty.\n\r", ch);
        return;
    }
    
    victim->pcdata->leader = clan_num;
    sprintf(buf, "%s is now set as leader of clan %d (%s).\n\r",
            victim->name, clan_num, clan_table[clan_num].name);
    send_to_char(buf, ch);
    sprintf(buf, "%s has set you as leader of %s.\n\r", 
            ch->name, clan_table[clan_num].name);
    send_to_char(buf, victim);
}

/* Induct command - allows clan leaders to add members */
void do_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;
    
    if (IS_NPC(ch))
        return;
    
    /* Check if character is a clan leader */
    if (ch->pcdata->leader < 0 || ch->pcdata->leader >= MAX_CLAN)
    {
        send_to_char ("You are not a leader of any clan.\n\r", ch);
        return;
    }
    
    clan = ch->pcdata->leader;
    
    /* Verify leader is in the clan they lead */
    if (ch->clan != clan)
    {
        send_to_char ("You are not a member of the clan you lead.\n\r", ch);
        send_to_char ("Contact an immortal to fix your clan membership.\n\r", ch);
        return;
    }
    
    if (clan_table[clan].independent)
    {
        send_to_char ("You cannot induct members into an independent clan.\n\r", ch);
        return;
    }
    
    one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char ("Induct whom into your clan?\n\r", ch);
        return;
    }
    
    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char ("They aren't playing right now.\n\r", ch);
        return;
    }
    
    if (IS_NPC(victim))
    {
        send_to_char ("You cannot induct NPCs into your clan.\n\r", ch);
        return;
    }
    
    /* Check if they've petitioned this clan */
    if (victim->pcdata->petitioning != clan)
    {
        sprintf(buf, "%s has not petitioned to join %s.\n\r",
                victim->name, clan_table[clan].name);
        send_to_char(buf, ch);
        send_to_char("They must use 'petition <clanname>' first.\n\r", ch);
        return;
    }
    
    /* Check minimum level requirement */
    if (victim->level < clan_table[clan].min_level)
    {
        sprintf(buf, "%s must be at least level %d to join %s.\n\r",
                victim->name, clan_table[clan].min_level, clan_table[clan].name);
        send_to_char(buf, ch);
        return;
    }
    
    /* Check if already in this clan */
    if (victim->clan == clan)
    {
        sprintf(buf, "%s is already a member of %s.\n\r",
                victim->name, clan_table[clan].name);
        send_to_char(buf, ch);
        return;
    }
    
    /* Check if in another clan */
    if (victim->clan > 0 && victim->clan != clan && !clan_table[victim->clan].independent)
    {
        sprintf(buf, "%s is already a member of %s.\n\r",
                victim->name, clan_table[victim->clan].name);
        send_to_char(buf, ch);
        send_to_char("They must leave their current clan first.\n\r", ch);
        return;
    }
    
    /* Induct the member */
    victim->clan = clan;
    victim->pcdata->petitioning = -1;  /* Clear petition */
    
    sprintf(buf, "You induct %s into %s.\n\r",
            victim->name, clan_table[clan].name);
    send_to_char(buf, ch);
    
    sprintf(buf, "{Y%s has inducted you into %s!{x\n\r",
            ch->name, clan_table[clan].name);
    send_to_char(buf, victim);
    
    sprintf(buf, "%s has been inducted into %s by %s.",
            victim->name, clan_table[clan].name, ch->name);
    log_string(buf);
    
    save_char_obj(victim);
}

/* Outcast command - allows clan leaders to remove members */
void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;
    
    if (IS_NPC(ch))
        return;
    
    /* Check if character is a clan leader */
    if (ch->pcdata->leader < 0 || ch->pcdata->leader >= MAX_CLAN)
    {
        send_to_char ("You are not a leader of any clan.\n\r", ch);
        return;
    }
    
    clan = ch->pcdata->leader;
    
    /* Verify leader is in the clan they lead */
    if (ch->clan != clan)
    {
        send_to_char ("You are not a member of the clan you lead.\n\r", ch);
        return;
    }
    
    if (clan_table[clan].independent)
    {
        send_to_char ("You cannot outcast from an independent clan.\n\r", ch);
        return;
    }
    
    one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char ("Outcast whom from your clan?\n\r", ch);
        return;
    }
    
    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char ("They aren't playing right now.\n\r", ch);
        return;
    }
    
    if (IS_NPC(victim))
    {
        send_to_char ("NPCs are not clan members.\n\r", ch);
        return;
    }
    
    /* Check if they're in this clan */
    if (victim->clan != clan)
    {
        sprintf(buf, "%s is not a member of %s.\n\r",
                victim->name, clan_table[clan].name);
        send_to_char(buf, ch);
        return;
    }
    
    /* Can't outcast yourself */
    if (victim == ch)
    {
        send_to_char ("You cannot outcast yourself. Use 'clanleader <you> none' to resign.\n\r", ch);
        return;
    }
    
    /* Remove from clan */
    victim->clan = 0;  /* Set to Independent */
    
    /* Remove leadership if they were a leader */
    if (victim->pcdata->leader == clan)
        victim->pcdata->leader = -1;
    
    /* Clear any pending petition */
    victim->pcdata->petitioning = -1;
    
    sprintf(buf, "You have outcast %s from %s.\n\r",
            victim->name, clan_table[clan].name);
    send_to_char(buf, ch);
    
    sprintf(buf, "{R%s has outcast you from %s!{x\n\r",
            ch->name, clan_table[clan].name);
    send_to_char(buf, victim);
    
    sprintf(buf, "%s has been outcast from %s by %s.",
            victim->name, clan_table[clan].name, ch->name);
    log_string(buf);
    
    save_char_obj(victim);
}

/* Petition command - request to join a clan */
void do_petition( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int clan;
    
    if (IS_NPC(ch))
        return;
    
    one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char("Petition to join which clan?\n\r", ch);
        send_to_char("Use 'clanlist' to see available clans.\n\r", ch);
        if (ch->pcdata->petitioning >= 0)
        {
            sprintf(buf, "You are currently petitioning to join %s.\n\r",
                    clan_table[ch->pcdata->petitioning].name);
            send_to_char(buf, ch);
            send_to_char("Use 'petition cancel' to withdraw your petition.\n\r", ch);
        }
        return;
    }
    
    if (!str_cmp(arg, "cancel"))
    {
        if (ch->pcdata->petitioning < 0)
        {
            send_to_char("You are not petitioning any clan.\n\r", ch);
            return;
        }
        sprintf(buf, "You withdraw your petition to join %s.\n\r",
                clan_table[ch->pcdata->petitioning].name);
        send_to_char(buf, ch);
        ch->pcdata->petitioning = -1;
        save_char_obj(ch);
        return;
    }
    
    clan = clan_lookup(arg);
    if (clan < 0 || clan >= MAX_CLAN)
    {
        send_to_char("No such clan exists.\n\r", ch);
        return;
    }
    
    if (!clan_table[clan].name || clan_table[clan].name[0] == '\0')
    {
        send_to_char("That clan does not exist.\n\r", ch);
        return;
    }
    
    if (clan_table[clan].independent)
    {
        send_to_char("You cannot petition to join Independent.\n\r", ch);
        return;
    }
    
    if (ch->clan == clan)
    {
        send_to_char("You are already a member of that clan!\n\r", ch);
        return;
    }
    
    if (ch->clan > 0 && !clan_table[ch->clan].independent)
    {
        sprintf(buf, "You must leave %s before petitioning another clan.\n\r",
                clan_table[ch->clan].name);
        send_to_char(buf, ch);
        return;
    }
    
    if (ch->level < clan_table[clan].min_level)
    {
        sprintf(buf, "You must be at least level %d to petition %s.\n\r",
                clan_table[clan].min_level, clan_table[clan].name);
        send_to_char(buf, ch);
        return;
    }
    
    ch->pcdata->petitioning = clan;
    sprintf(buf, "You petition to join %s.\n\r", clan_table[clan].name);
    send_to_char(buf, ch);
    send_to_char("A clan leader must now induct you to complete your membership.\n\r", ch);
    
    sprintf(buf, "%s has petitioned to join %s.",
            ch->name, clan_table[clan].name);
    log_string(buf);
    
    save_char_obj(ch);
}
