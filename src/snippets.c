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
 *           snippets.c - November 3, 2025
 */            
/***************************************************************************
 * Snippet implementations for Shadows Reborn 
 * This file contains various snippet implementations as well as original
 * code from Shadows Reborn.
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "language_tables.h"

/* Forward declarations */
void trigger_tobacco_spells(CHAR_DATA *ch, OBJ_DATA *pipe);
#include "lookup.h"

/* Finger command by Wench */
void do_finger(CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    char strsave[MAX_INPUT_LENGTH];
    bool email_found = FALSE;
    bool real_found = FALSE;
    bool laston_found = FALSE;
    bool host_found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    char *real = "";
    char *email = "";
    char *host = "";
    long laston;
    bool finished = FALSE;
    int d, h, m, s = 0;

    if(argument[0] == '\0' || argument[0] == '/' || argument[0] == '\\')
    {
        send_to_char("Finger who?\n\r", ch);
        return;
    }

    smash_tilde(argument);
    argument = one_argument(argument, arg);
    strcpy(arg2, argument);
    
    if(!str_cmp(arg, "email"))
    {
        if(arg2[0] == '\0')
        {
            sprintf(buf, "Your email address is currently listed as %s.\n\r",
                ch->pcdata->email ? ch->pcdata->email : "not set");
            send_to_char(buf, ch);
            return;
        }
        free_string(ch->pcdata->email);
        ch->pcdata->email = str_dup(arg2);
        sprintf(buf, "Your email address will now be displayed as %s.\n\r",
            ch->pcdata->email);
        send_to_char(buf, ch);
        return;
    }

    if(!str_cmp(arg, "real"))
    {
        if(arg2[0] == '\0')
        {
            sprintf(buf, "Your real name is currently listed as %s.\n\r",
                ch->pcdata->real_name ? ch->pcdata->real_name : "not set");
            send_to_char(buf, ch);
            return;
        }
        free_string(ch->pcdata->real_name);
        ch->pcdata->real_name = str_dup(arg2);
        sprintf(buf, "Your real name will now appear as %s.\n\r",
            ch->pcdata->real_name);
        send_to_char(buf, ch);
        return;
    }

    if(((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim)))
    {
        sprintf(buf,"Name: %16s   Real Name: %16s\n\r", victim->name,
            victim->pcdata->real_name ? victim->pcdata->real_name : "Not specified");
        send_to_char(buf, ch);
        sprintf(buf, "E-mail address: %35s\n\r", 
            victim->pcdata->email ? victim->pcdata->email : "Not specified");
        send_to_char(buf, ch);
        if(victim->desc != NULL)
            sprintf(buf, "%s is currently playing {rR{Rive{Yrs {Wof {YDr{Rec{rk{x.\n\r", victim->name);
        else
            sprintf(buf, "%s is currently linkdead.\n\r", victim->name);
        send_to_char(buf, ch);
        return;
    }

    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));
    if((fp = fopen(strsave, "r")) == NULL)
    {
        sprintf(buf, "No such player: %s.\n\r", capitalize(arg));
        send_to_char(buf, ch);
        return;
    }

    for(;;)
    {
        char *word;
        fread_to_eol(fp);
        word = feof(fp) ? "End" : fread_word(fp);
        switch(UPPER(word[0]))
        {
            case 'E':
                if(!str_cmp(word, "Email"))
                {
                    email = fread_string(fp);
                    email_found = TRUE;
                }
                if(!str_cmp(word, "End"))
                {
                    finished = TRUE;
                    break;
                }
                break;
            case 'H':
                if(!str_cmp(word, "Host"))
                {
                    host = fread_string(fp);
                    host_found = TRUE;
                }
                break;
            case 'R':
                if(!str_cmp(word, "Real"))
                {
                    real = fread_string(fp);
                    real_found = TRUE;
                }
                break;
            case 'L':
                if(!str_cmp(word, "LogO"))
                {
                    laston = fread_number(fp);
                    laston_found = TRUE;
                }
                break;
            case '#':
                finished = TRUE;
                break;
            default:
                fread_to_eol(fp);
                break;
        }
        if((finished)||(real_found&&laston_found&&email_found))
            break; 
    }

    s = current_time - laston;
    d = s/86400;
    s-=d*86400;
    h = s/3600;
    s -= h*3600;
    m = s/60;
    s -= m*60;

    sprintf(buf, "Name: %16s   Real Name: %16s\n\r", capitalize(arg), 
        real_found ? real: "Not specified.");
    send_to_char(buf, ch);
    sprintf(buf, "E-mail address: %20s\n\r", 
        email_found ? email : "Not specified.");
    send_to_char(buf, ch);
    sprintf(buf, "%s last logged on at %s\r", capitalize(arg),
        laston_found ? ctime(&laston) : "Not found.\n\r");
    send_to_char(buf, ch);
    sprintf(buf, "That was %d days, %d hours, %d minutes and %d seconds ago.\n\r",
        d, h, m, s);
    send_to_char(buf, ch);

    if(IS_IMMORTAL(ch))
    {
        if(host_found)
            sprintf(buf, "%s last logged on from %s\n\r", capitalize(arg), host);
        else
            sprintf(buf, "No logon site found for %s.\n\r", capitalize(arg));
        send_to_char(buf, ch);
    }
    fclose(fp);
    return;
}

/* Forget/Remember commands by Dominic J. Eidson */
void do_forget(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
        return;

    smash_tilde( argument );

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
        if (rch->pcdata->forget[0] == NULL)
        {
            send_to_char("You are not forgetting anyone.\n\r",ch);
            return;
        }
        send_to_char("You are currently forgetting:\n\r",ch);

        for (pos = 0; pos < MAX_FORGET; pos++)
        {
            if (rch->pcdata->forget[pos] == NULL)
                break;

            sprintf(buf,"    %s\n\r",rch->pcdata->forget[pos]);
            send_to_char(buf,ch);
        }
        return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
        if (rch->pcdata->forget[pos] == NULL)
            break;

        if (!str_cmp(arg,rch->pcdata->forget[pos]))
        {
            send_to_char("You have already forgotten that person.\n\r",ch);
            return;
        }
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;

        if (d->connected != CON_PLAYING || !can_see(ch,d->character))
            continue;
        
        wch = ( d->original != NULL ) ? d->original : d->character;

        if (!can_see(ch,wch))
            continue;

        if (!str_cmp(arg,wch->name))
        {
            found = TRUE;
            if (wch == ch)
            {
                send_to_char("You forget yourself for a moment, but it passes.\n\r",ch);
                return;
            }
            if (wch->level >= LEVEL_IMMORTAL)
            {
                send_to_char("That person is very hard to forget.\n\r",ch);
                return;
            }
        }
    }

    if (!found)
    {
        send_to_char("No one by that name is playing.\n\r",ch);
        return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
        if (rch->pcdata->forget[pos] == NULL)
            break;
    }

    if (pos >= MAX_FORGET)
    {
        send_to_char("Sorry, you have reached the forget limit.\n\r",ch);
        return;
    }
  
    /* make a new forget */
    rch->pcdata->forget[pos] = str_dup(arg);
    sprintf(buf,"You are now deaf to %s.\n\r",arg);
    send_to_char(buf,ch);
}

void do_remember(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
        return;
 
    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        if (rch->pcdata->forget[0] == NULL)
        {
            send_to_char("You are not forgetting anyone.\n\r",ch);
            return;
        }
        send_to_char("You are currently forgetting:\n\r",ch);

        for (pos = 0; pos < MAX_FORGET; pos++)
        {
            if (rch->pcdata->forget[pos] == NULL)
                break;

            sprintf(buf,"    %s\n\r",rch->pcdata->forget[pos]);
            send_to_char(buf,ch);
        }
        return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
        if (rch->pcdata->forget[pos] == NULL)
            break;

        if (found)
        {
            rch->pcdata->forget[pos-1] = rch->pcdata->forget[pos];
            rch->pcdata->forget[pos] = NULL;
            continue;
        }

        if(!strcmp(arg,rch->pcdata->forget[pos]))
        {
            send_to_char("Forget removed.\n\r",ch);
            free_string(rch->pcdata->forget[pos]);
            rch->pcdata->forget[pos] = NULL;
            found = TRUE;
        }
    }

    if (!found)
        send_to_char("No one by that name is forgotten.\n\r",ch);
}

/* Award command by Robbert */
void do_award(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_STRING_LENGTH];
    char arg2 [MAX_STRING_LENGTH];
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: award <char> <QP>.\n\r", ch);
        return;
    }
    
    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's!\n\r", ch);
        return;   
    }

  

    value = atoi( arg2 );

    if ( value < -1000 || value > 1000 )
    {
        send_to_char( "Award range is -1000 to 1000.\n\r", ch );
        return;
    }

    if ( value == 0 )
    {
        send_to_char( "Value cannot be 0.\n\r", ch );
        return;
    }

    sprintf( buf, "{wYou have awarded %s {Y%d{w Quest Points!\n\r", victim->name, value );
    send_to_char(buf, ch);
                             
    if ( value >0 )
    {
        victim->pcdata->quest_curr += value;
        sprintf( buf, "{wYou have been awarded {Y%d{w Quest Points by  %s!\n\r", value, ch->name );
        send_to_char( buf, victim );
        //do_function(victim, &do_save, "");
        sprintf( buf, "{R$N awards %d Quest Points to %s{x", value, victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        victim->pcdata->quest_curr += value;
        sprintf( buf, "{wYou have been penalized {Y%d{w Quest Points by %s!\n\r", value, ch->name );
        send_to_char( buf, victim );
        //do_function(victim, &do_save, "");
        sprintf( buf, "$N penalizes %s %d Quest Points.", victim->name, value );
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    return;
}

/* Avatar command by takeda */
void do_avatar( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char( "Syntax: avatar <level>.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
    {
        sprintf(buf, "Level must be 1 to %d.\n\r", MAX_LEVEL );
        send_to_char( buf, ch );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        sprintf(buf, "Your Trust is %d.\n\r",ch->trust);
        send_to_char(buf,ch);
        return;
    }

    /* Your trust stays so you will have all immortal command */
    if(ch->trust == 0) {
        ch->trust = ch->level;
    }

    /* Level gains*/
    if ( level <= ch->level )
    {
        int temp_prac;

        send_to_char( "Lowering a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r",ch );
        temp_prac = ch->practice;
        ch->level    = 1;
        ch->exp      = exp_per_level(ch,ch->pcdata->points);
        ch->max_hit  = 20;
        ch->max_mana = 100;
        ch->max_move = 100;
        ch->practice = 0;
        ch->hit      = ch->max_hit;
        ch->mana     = ch->max_mana;
        ch->move     = ch->max_move;
        advance_level( ch, TRUE );
        ch->practice = temp_prac;
    }
    else
    {
        send_to_char( "Raising a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", ch);
    }

    for ( iLevel = ch->level ; iLevel < level; iLevel++ )
    {
        ch->level += 1;
        advance_level( ch,TRUE);
    }
    sprintf(buf,"You are now level %d.\n\r",ch->level);
    send_to_char(buf,ch);
    ch->exp   = exp_per_level(ch,ch->pcdata->points) * UMAX( 1, ch->level );

    /* Forces the person to remove all the eq....  so level restriction still apply */
    if(ch->level < 103) {
        for (obj = ch->carrying; obj; obj = obj_next)
        {
            obj_next = obj->next_content;

            if (obj->wear_loc != WEAR_NONE && can_see_obj (ch, obj))
                unequip_char (ch, obj);
        }
    }
    save_char_obj(ch);
    return;
}

/* Secset command by Robert Schultz */
void do_secset(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int seclevel;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
        send_to_char("Set who's Security?\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You may not change an NPC's security.\n\r", ch );
        return;
    }

    if ( victim->level > ch->level )
    {
        send_to_char("You may not alter someone who is a higher level than you.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg2);

    if (arg2[0] == '\0')
    {
        send_to_char("Set their security to what level?\n\r", ch);
        return;
    }

    seclevel = atoi(arg2);

    if( (seclevel <= 0) || (seclevel > 9) )
    {
        send_to_char("Security range is from 1 to 9.\n\r", ch);
        return;
    }

    victim->pcdata->security = seclevel;
    send_to_char("Changed players security level.\n\r", ch);
    sprintf(buf, "%s just raised your security to level %d\n\r", ch->name, seclevel);
    send_to_char(buf, victim);

    return;
}

/* World Peace command by Amadeus */
CHAR_DATA *get_char_special( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
        return ch->desc->original;
    else
        return ch;
}

void do_wpeace(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char buf[MAX_STRING_LENGTH];

    rch = get_char_special( ch );

    for ( rch = char_list; rch; rch = rch->next )
    {
        if ( rch->desc == NULL || rch->desc->connected != CON_PLAYING )
            continue;

        if ( rch->fighting )
        {
            sprintf( buf, "%s has declared World Peace.\n\r", ch->name );
            send_to_char( buf, rch );
            stop_fighting( rch, TRUE );
        }
    }

    send_to_char( "You have declared World Peace.\n\r", ch );
    return;
}

/* Wiznetall commands by Ankerous */
void do_wiznetall (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  Wiznetall <on/off>\n\r", ch);
        send_to_char ("  On turns all Wiznet Options on\n\r", ch);
        send_to_char ("  Off turns all Wiznet Options off\n\r", ch);
        return;
    }

    if (!str_cmp (arg, "on"))
    {
        do_function (ch, &do_wiznetallon, argument);
        return;
    }

    if (!str_cmp (arg, "off"))
    {
        do_function (ch, &do_wiznetalloff, argument);
        return;
    }
    /* echo syntax */
    do_function (ch, &do_wiznetall, "");
}

void do_wiznetallon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    SET_BIT(ch->wiznet,WIZ_ON);
    SET_BIT(ch->wiznet,WIZ_PREFIX);
    SET_BIT(ch->wiznet,WIZ_TICKS);
    SET_BIT(ch->wiznet,WIZ_LOGINS);
    SET_BIT(ch->wiznet,WIZ_SITES);
    SET_BIT(ch->wiznet,WIZ_LINKS);
    SET_BIT(ch->wiznet,WIZ_NEWBIE);
    SET_BIT(ch->wiznet,WIZ_SPAM);
    SET_BIT(ch->wiznet,WIZ_DEATHS);
    SET_BIT(ch->wiznet,WIZ_RESETS);
    SET_BIT(ch->wiznet,WIZ_MOBDEATHS);
    SET_BIT(ch->wiznet,WIZ_FLAGS);
    SET_BIT(ch->wiznet,WIZ_PENALTIES);
    SET_BIT(ch->wiznet,WIZ_SACCING);
    SET_BIT(ch->wiznet,WIZ_LEVELS);
    SET_BIT(ch->wiznet,WIZ_LOAD);
    SET_BIT(ch->wiznet,WIZ_RESTORE);
    SET_BIT(ch->wiznet,WIZ_SNOOPS);
    SET_BIT(ch->wiznet,WIZ_SWITCHES);
    SET_BIT(ch->wiznet,WIZ_SECURE);

    send_to_char("{YAll Wiznet Options turned {Gon{x.{x\n\r",ch);
}

void do_wiznetalloff(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    REMOVE_BIT(ch->wiznet,WIZ_ON);
    REMOVE_BIT(ch->wiznet,WIZ_PREFIX);
    REMOVE_BIT(ch->wiznet,WIZ_TICKS);
    REMOVE_BIT(ch->wiznet,WIZ_LOGINS);
    REMOVE_BIT(ch->wiznet,WIZ_SITES);
    REMOVE_BIT(ch->wiznet,WIZ_LINKS);
    REMOVE_BIT(ch->wiznet,WIZ_NEWBIE);
    REMOVE_BIT(ch->wiznet,WIZ_SPAM);
    REMOVE_BIT(ch->wiznet,WIZ_DEATHS);
    REMOVE_BIT(ch->wiznet,WIZ_RESETS);
    REMOVE_BIT(ch->wiznet,WIZ_MOBDEATHS);
    REMOVE_BIT(ch->wiznet,WIZ_FLAGS);
    REMOVE_BIT(ch->wiznet,WIZ_PENALTIES);
    REMOVE_BIT(ch->wiznet,WIZ_SACCING);
    REMOVE_BIT(ch->wiznet,WIZ_LEVELS);
    REMOVE_BIT(ch->wiznet,WIZ_LOAD);
    REMOVE_BIT(ch->wiznet,WIZ_RESTORE);
    REMOVE_BIT(ch->wiznet,WIZ_SNOOPS);
    REMOVE_BIT(ch->wiznet,WIZ_SWITCHES);
    REMOVE_BIT(ch->wiznet,WIZ_SECURE);

    send_to_char("{YAll Wiznet Options turned {Roff{x.{x\n\r",ch);
}

/* Wizify command by Koqlb */
void do_wizify (CHAR_DATA *ch, char * argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;

    argument = one_argument (argument, arg1);
    strcpy (arg2, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char ("Syntax: wizify <char> <security>.\n\r", ch);
        return;
    }

    if ((victim = get_char_world (ch, arg1)) == NULL)
    {
        send_to_char ("They aren't online.\n\r", ch);
        return;
    }

    if (IS_NPC (victim))
    {
        send_to_char ("Mobs can't be Immortals!!!\n\r", ch);
        return;
    }

    value = is_number (arg2) ? atoi (arg2) : 0;

    if (value < 0 || value > 9)
    {
        sprintf (buf, "Builder security range is 0 to 9\n\r.");
        send_to_char (buf, ch);
        return;
    }

    if (victim->level > ch->level )
    {
        send_to_char("Try it on someone your own size.\n\r", ch);
        return;
    }

/*    if (victim->trust >= ch->trust )
    {
        send_to_char("You aren't trusted at a high enough level.\n\r", ch);
        return;
    }
*/
    if (victim->level <= LEVEL_HERO)
    {
        send_to_char("You can only wizify Immortals!\n\r",ch);
        return;
    }

    /*Set the max stats and security*/
    victim->max_hit = 30000;
    victim->max_mana = 30000;
    victim->max_move = 30000;
    victim->perm_stat[STAT_STR] = 25;
    victim->perm_stat[STAT_INT] = 25;
    victim->perm_stat[STAT_WIS] = 25;
    victim->perm_stat[STAT_DEX] = 25;
    victim->perm_stat[STAT_CON] = 25;
    victim->pcdata->security = value;

    /* Set the common act flags for Immortals. */
    SET_BIT (victim->act, PLR_HOLYLIGHT);
    SET_BIT (victim->act, PLR_NOSUMMON);
    REMOVE_BIT(victim->act, PLR_AUTOLOOT);
    REMOVE_BIT(victim->act, PLR_AUTOSAC);
    REMOVE_BIT(victim->act, PLR_CANLOOT);

    /*Restore the character */
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos (victim);

    sprintf(buf, "%s has been Wizified! All stats have been set.\n\r",victim->name);
    send_to_char(buf,ch);
    send_to_char("Alright!!! Suddenly you feel like a TRUE IMMORTAL!\n\r",victim);
    return;
}

/* Wrlist command by Sokol */
void do_wrlist( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *in_room;
    MOB_INDEX_DATA *mob;
    OBJ_INDEX_DATA *obj;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int uvnum;
    int lvnum;
    int MR = 60000;
    int type = -1;
  
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    uvnum = ( is_number( arg2 ) ) ? atoi( arg2 ) : 0;
    lvnum = ( is_number( arg1 ) ) ? atoi( arg1 ) : 0;  

    if ( !str_cmp( arg, "o" ) )
        type = 2;
    if ( !str_cmp( arg, "m" ) )
        type = 1;
    if ( !str_cmp( arg, "r" ) )
        type = 0;

    if ( ( uvnum - lvnum ) > 200 )
    {
        send_to_char( "{WThat range is too large.\n\r{x", ch );
        return;
    }
  
    if ( ( ( uvnum == 0 ) && ( lvnum == 0 ) ) || ( arg[0] == '\0' ) 
        || ( type == -1 ) )
    {
        send_to_char( "{MSyntax: wrlist [type] [lvnum] [uvnum]\n\r{x", ch );
        return;
    }

    if ( uvnum > MR || uvnum < 1 || lvnum > MR || lvnum < 1 || lvnum > uvnum )
    {
        send_to_char( "{WInvalid level(s).\n\r{x", ch );
        return;
    }

    in_room = ch->in_room;  
    if ( type == 0 )
    {
        char_from_room( ch );
    }
    for ( MR = lvnum; MR <= uvnum; MR++ )
    {
        if ( type == 0 )
        {
            if ( ( room = get_room_index( MR ) ) )
            {
                sprintf( log_buf, "{R%-5d  {w%-20s\n\r", room->vnum, room->name );
                send_to_char( log_buf, ch );
                char_to_room( ch, room );
                do_function( ch, &do_resets, "" );
                char_from_room( ch );
            }
        }
        if ( type == 2 )
        {
            if ( ( obj = get_obj_index( MR ) ) )
            {
                sprintf( log_buf, "{R%-5d  {w%-20s\n\r",  obj->vnum, obj->name );
                send_to_char( log_buf, ch );
            }
        }
        if ( type == 1 )
        {
            if ( ( mob = get_mob_index( MR ) ) )
            {
                sprintf( log_buf, "{R%-5d  {w%-20s\n\r", mob->vnum, mob->player_name );
                send_to_char( log_buf, ch );
            }
        }
    }
    if ( type == 0 )
        char_to_room( ch, in_room );
    return;
}

/* Affects long command */
void do_affects_long(CHAR_DATA* ch, char *argument)
{
    if (IS_AFFECTED (ch, AFF_BLIND))
        send_to_char("You are Blinded!\n\r", ch);
    if (IS_AFFECTED (ch, AFF_INVISIBLE))
        send_to_char("You are Invisible.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL))
        send_to_char("You can sense evil.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD))
        send_to_char("You can sense good.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DETECT_INVIS))
        send_to_char("You can sense invisible creatures.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC))
        send_to_char("You can sense magical affects.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DETECT_HIDDEN))
        send_to_char("You can see hidden creatures.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_SANCTUARY))
        send_to_char("You are surrounded by a white aura of sanctuary (Dam 1/2).\n\r", ch);
    if (IS_AFFECTED (ch, AFF_FAERIE_FIRE))
        send_to_char("You are affected by Faerie Fire!.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_INFRARED))
        send_to_char("You can see the heat outlines of creatures in the dark.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_CURSE))
        send_to_char("You have been cursed!\n\r", ch);
    if (IS_AFFECTED (ch, AFF_POISON))
        send_to_char("Poison is in your veins.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_PROTECT_EVIL))
        send_to_char("You are protected from evil.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_PROTECT_GOOD))
        send_to_char("You are protected from good.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_SNEAK))
        send_to_char("You are sneaky.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_HIDE))
        send_to_char("You are hidden.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_SLEEP))
        send_to_char("You are sleeping, very heavily.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_CHARM))
        send_to_char("You are Charmed.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_FLYING))
        send_to_char("You are flying!\n\r", ch);
    if (IS_AFFECTED (ch, AFF_PASS_DOOR))
        send_to_char("You can walk through doors.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_HASTE))
        send_to_char("You are moving very fast.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_CALM))
        send_to_char("You are very peaceful.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_PLAGUE))
        send_to_char("You are suffering from the plague.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_WEAKEN))
        send_to_char("You are weakened.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_DARK_VISION))
        send_to_char("You have dark vision.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_BERSERK))
        send_to_char("You are Berserk!\n\r", ch);
    if (IS_AFFECTED (ch, AFF_SWIM))
        send_to_char("You can swim.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_REGENERATION))
        send_to_char("You are regenerating very fast.\n\r", ch);
    if (IS_AFFECTED (ch, AFF_SLOW))
        send_to_char("You...are...feeling...sluggish...\n\r", ch);
}

/* Cdump command - dump contents of container */
void do_cdump(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *container, *into, *temp_obj, *temp_next;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  /* First, check to ensure they can dump.  */
  argument = one_argument( argument, arg1 );

  if (arg1[0] == '\0')
    {
      send_to_char("What do you want to dump out?\n\r",ch);
      return;
    }

  if ((container = get_obj_carry(ch, arg1, ch)) == NULL)
    {
      act("You don't have a $T.", ch, NULL, arg1, TO_CHAR);
      return;
    }

  if (container->item_type != ITEM_CONTAINER)
    {
      send_to_char("You can't dump that.\n\r",ch);
      return;
    }

  if (IS_SET(container->value[1], CONT_CLOSED))
    {
      act("$d is closed.", ch, NULL, container->short_descr, TO_CHAR);
      return;
    }

  /* Next, check to see if they want to dump into another container.  */
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"into") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if (arg2[0] != '\0')
    {
      /* Prefer obj in inventory other than object in room.  */
      if ((into = get_obj_carry(ch, arg2, ch)) == NULL)
        {
          if ((into = get_obj_here(ch, arg2)) == NULL)
            {
              send_to_char("You don't see that here.\n\r",ch);
              return;
            }
        }

      if (into->item_type != ITEM_CONTAINER)
        {
          send_to_char("You can't dump into that.\n\r",ch);
          return;
        }

      if (IS_SET(into->value[1], CONT_CLOSED))
        {
          act("$d is closed.", ch, NULL, into->short_descr, TO_CHAR);
          return;
        }

      act("You dump out the contents of $p into $P.",ch,container,into,TO_CHAR);
      act("$n dumps out the contents of $p into $P.",ch,container,into,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;

          if ((get_obj_weight(temp_obj) + get_true_weight(into)
                > (into->value[0] * 10))
              ||  (get_obj_weight(temp_obj) > (into->value[3] * 10)))
            act("$P won't fit into $p.",ch,into,temp_obj,TO_CHAR);

          else 
            {
              obj_from_obj(temp_obj);
              obj_to_obj(temp_obj, into);
            }
        }
    }

  /* Dumping to the floor.  */
  else
    {
      act("You dump out the contents of $p.",ch,container,NULL,TO_CHAR);
      act("$n dumps out the contents of $p.",ch,container,NULL,TO_ROOM);

      for (temp_obj=container->contains; temp_obj != NULL; temp_obj=temp_next)
        {
          temp_next = temp_obj->next_content;
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_CHAR);
          act("  ... $p falls out onto the ground.",ch,temp_obj,NULL,TO_ROOM);
  
          obj_from_obj(temp_obj);
          obj_to_room(temp_obj,ch->in_room);
  
          if (IS_OBJ_STAT(temp_obj,ITEM_MELT_DROP))
            {
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_ROOM);
              act("$p dissolves into smoke.",ch,temp_obj,NULL,TO_CHAR);
              extract_obj(temp_obj);
            }
        }
    }
}

/* Noexp command - toggle experience gain */
void do_noexp(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
        send_to_char("No. You are an NPC. Quit talking and die!\n\r", ch);
        return;
    }
    if (!IS_SET(ch->act, PLR_NOEXP))
    {
        SET_BIT(ch->act, PLR_NOEXP);
        send_to_char("You will no longer get exp for kills. Remember to set this back when you want exp for kills!\n\r", ch);
        return;
    }
    else
    {
        REMOVE_BIT(ch->act, PLR_NOEXP);
        send_to_char("You turn your ability to gain exp back on.\n\r", ch);
        return;
    }
}

/* Engage command - change combat targets */
void do_engage(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  /* Check for skill.  */
  if (   (get_skill(ch,gsn_engage) == 0 )
      || (    !IS_NPC(ch)
          &&  (ch->level < skill_table[gsn_engage].skill_level[ch->class])))
    {
      send_to_char("Engage?  You're not even dating!\n\r",ch);  /* Humor. :)  */
      return;
    }

  /* Must be fighting.  */
  if (ch->fighting == NULL)
    {
      send_to_char("You're not fighting anyone.\n\r",ch);
      return;
    }

  one_argument( argument, arg );

  /* Check for argument.  */
  if (arg[0] == '\0')
    {
      send_to_char("Engage who?\n\r",ch);
      return;
    }

  /* Check for victim.  */
  if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("Shadowbox some other time.\n\r",ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Attacking yourself in combat isn't a smart thing.\n\r",ch);
      return;
    }

  if (ch->fighting == victim)
    {
      send_to_char("You're already pummelling that target as hard as you can!\n\r",ch);
      return;
    }

  /* Check for safe.  */
  if (is_safe(ch, victim))
    return;

  /* Check for charm.  */
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  /* This lets higher-level characters engage someone that isn't already fighting them.
     Quite powerful.  Raise level as needed.  */
  if ((victim->fighting != ch) && (ch->level < LEVEL_IMMORTAL))
    {
      send_to_char("But they're not fighting you!\n\r",ch);
      return;
    }

  /* Get chance of success, and allow max 95%.  */
  chance = get_skill(ch,gsn_engage);
  chance = UMIN(chance,95);

  if (number_percent() < chance)
    {
      /* It worked!  */
      stop_fighting(ch,FALSE);

      set_fighting(ch,victim);
      if (victim->fighting == NULL)
        set_fighting(victim,ch);

      check_improve(ch,gsn_engage,TRUE,3);
      act("$n has turned $s attacks toward you!",ch,NULL,victim,TO_VICT);
      act("You turn your attacks toward $N.",ch,NULL,victim,TO_CHAR);
      act("$n has turned $s attacks toward $N!",ch,NULL,victim,TO_NOTVICT);
    }
  else
    {
      /* It failed!  */
      send_to_char("You couldn't get your attack in.\n\r",ch);
      check_improve(ch,gsn_engage,FALSE,3);
    }
}

/* Study skill by Glop - allows studying skills without practicing */
void do_study (CHAR_DATA *ch, char *argument) /* study by Absalom */
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn = 0;
    int i;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Study what?\r\n", ch);
        return;
    }

    if ((obj = get_obj_carry(ch, arg, ch)) == NULL)
    {
        send_to_char("You do not have that item.\r\n", ch);
        return;
    }

    if (obj->item_type != ITEM_STAFF && obj->item_type != ITEM_WAND &&
        obj->item_type != ITEM_SCROLL && obj->item_type != ITEM_MANUAL)
    {
        send_to_char("You can only study scrolls, wands, staves, and manuals.\r\n", ch);
        return;
    }

    act("$n studies $p.", ch, obj, NULL, TO_ROOM);
    act("You study $p.", ch, obj, NULL, TO_CHAR);

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
        sn = obj->value[3];
        if (sn < 0 || sn >= MAX_SKILL)
        {
            return;
        }
        if (number_percent() >= 55)
        {
            send_to_char("You cannot glean any knowledge from it.\r\n", ch);
            act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
            extract_obj(obj);
            return;
        }
        if (ch->pcdata->learned[sn])
        {
            send_to_char("You already know that spell!\r\n", ch);
            return;
        }
        ch->pcdata->learned[sn] = 1;
        act("You have learned the art of $t!", ch, skill_table[sn].name, NULL, TO_CHAR);
        act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
        extract_obj(obj);
        return;
    }

    if (obj->item_type == ITEM_SCROLL)
    {
        sn = obj->value[1];
        if (sn < 0 || sn >= MAX_SKILL)
        {
            return;
        }
        if (number_percent() >= 15)
        {
            send_to_char("You cannot glean any knowledge from it.\r\n", ch);
            act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
            extract_obj(obj);
            return;
        }
        if (ch->pcdata->learned[sn])
        {
            send_to_char("You already know that spell!\r\n", ch);
            return;
        }
        ch->pcdata->learned[sn] = 1;
        act("You have learned the art of $t!", ch, skill_table[sn].name, NULL, TO_CHAR);
        act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
        extract_obj(obj);
        return;
    }

    if (obj->item_type == ITEM_MANUAL)
    {
        /* Find first skill in manual that player doesn't know */
        sn = -1;
        for (i = 1; i <= 4; i++)
        {
            if (obj->value[i] >= 0 && obj->value[i] < MAX_SKILL
                && skill_table[obj->value[i]].name != NULL
                && !ch->pcdata->learned[obj->value[i]])
            {
                sn = obj->value[i];
                break;
            }
        }

        if (sn < 0)
        {
            send_to_char("You cannot glean any knowledge from it.\r\n", ch);
            act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
            extract_obj(obj);
            return;
        }

        if (number_percent() >= 15)
        {
            send_to_char("You cannot glean any knowledge from it.\r\n", ch);
            act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
            extract_obj(obj);
            return;
        }

        ch->pcdata->learned[sn] = 1;
        act("You have learned the art of $t!", ch, skill_table[sn].name, NULL, TO_CHAR);
        act("$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR);
        extract_obj(obj);
        return;
    }
}

/* Addapply command by addapplyV5.txt */
void do_addapply(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA paf;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int affect_modify = 0, bit = 0, enchant_type, pos, i;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char("Syntax for applies: addapply <object> <apply type> <value>\n\r",ch);
        send_to_char("Apply Types: hp str dex int wis con mana\n\r", ch);
        send_to_char("             ac move hitroll damroll saves\n\r\n\r", ch);
        send_to_char("Syntax for affects: addapply <object> affect <affect name>\n\r",ch);
        send_to_char("Affect Names: blind invisible detect_evil detect_invis detect_magic\n\r",ch);
        send_to_char("              detect_hidden detect_good sanctuary faerie_fire infrared\n\r",ch);
        send_to_char("              curse poison protect_evil protect_good sneak hide sleep charm\n\r", ch);
        send_to_char("              flying pass_door haste calm plague weaken dark_vision berserk\n\r", ch);
        send_to_char("              swim regeneration slow\n\r", ch);
        return;
    }
    
    if ((obj = get_obj_here(ch,arg1)) == NULL)
    {
        send_to_char("No such object exists!\n\r",ch);
        return;
    }
    
    if (!str_prefix(arg2,"hp"))
        enchant_type=APPLY_HIT;
    else if (!str_prefix(arg2,"str"))
        enchant_type=APPLY_STR;
    else if (!str_prefix(arg2,"dex"))
        enchant_type=APPLY_DEX;
    else if (!str_prefix(arg2,"int"))
        enchant_type=APPLY_INT;
    else if (!str_prefix(arg2,"wis"))
        enchant_type=APPLY_WIS;
    else if (!str_prefix(arg2,"con"))
        enchant_type=APPLY_CON;
    else if (!str_prefix(arg2,"mana"))
        enchant_type=APPLY_MANA;
    else if (!str_prefix(arg2,"move"))
        enchant_type=APPLY_MOVE;
    else if (!str_prefix(arg2,"ac"))
        enchant_type=APPLY_AC;
    else if (!str_prefix(arg2,"hitroll"))
        enchant_type=APPLY_HITROLL;
    else if (!str_prefix(arg2,"damroll"))
        enchant_type=APPLY_DAMROLL;
    else if (!str_prefix(arg2,"saves"))
        enchant_type=APPLY_SAVING_SPELL;
    else if (!str_prefix(arg2,"affect"))
        enchant_type=APPLY_SPELL_AFFECT;
    else
    {
        send_to_char("That apply is not possible!\n\r",ch);
        return;
    }
    
    if (enchant_type==APPLY_SPELL_AFFECT)
    {
        for (pos = 0; affect_flags[pos].name != NULL; pos++)
            if (!str_cmp(affect_flags[pos].name,arg3))
                bit = affect_flags[pos].bit;
    }
    else
    {
        if ( is_number(arg3) )
            affect_modify=atoi(arg3);  
        else
        {
            send_to_char("Applies require a value.\n\r", ch);
            return;
        }
    }
        
    affect_enchant(obj);

    /* create the affect */
        paf.where	= TO_AFFECTS;
        paf.type	= 0;
    paf.level	= ch->level;
    paf.duration	= -1;
    paf.location	= enchant_type;
    paf.modifier	= affect_modify;
    paf.bitvector	= bit;
    
    if ( enchant_type == APPLY_SPELL_AFFECT )
    {
        /* make table work with skill_lookup */		
        for ( i=0 ; arg3[i] != '\0'; i++ )
        {
            if ( arg3[i] == '_' )
                arg3[i] = ' ';
        }
            
        paf.type      = skill_lookup(arg3);
    }

    affect_to_obj(obj,&paf);

    send_to_char("Ok.\n\r", ch);
}

/* Disable command system by Erwin S. Andreasen */
DISABLED_DATA *disabled_first;

#define END_MARKER	"END" /* for load_disabled() and save_disabled() */

/* Function declarations */
void save_disabled args ((void));

/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/

void do_disable (CHAR_DATA *ch, char *argument)
{
	int i;
	DISABLED_DATA *p,*q;
	char buf[100];
	
	if (IS_NPC(ch))
	{
		send_to_char ("RETURN first.\n\r",ch);
		return;
	}
	
	if (!argument[0]) /* Nothing specified. Show disabled commands. */
	{
		if (!disabled_first) /* Any disabled at all ? */
		{
			send_to_char ("There are no commands disabled.\n\r",ch);
			return;
		}

		send_to_char ("Disabled commands:\n\r"
		              "Command      Level   Disabled by\n\r",ch);
		                
		for (p = disabled_first; p; p = p->next)
		{
			sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
			send_to_char (buf,ch);
		}
		return;
	}
	
	/* command given */

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (!str_cmp(argument, p->command->name))
			break;
			
	if (p) /* this command is disabled */
	{
	/* Optional: The level of the imm to enable the command must equal or exceed level
	   of the one that disabled it */
	
		if (get_trust(ch) < p->level)
		{
			send_to_char ("This command was disabled by a higher power.\n\r",ch);
			return;
		}
		
		/* Remove */
		
		if (disabled_first == p) /* node to be removed == head ? */
			disabled_first = p->next;
		else /* Find the node before this one */
		{
			for (q = disabled_first; q->next != p; q = q->next); /* empty for */
			q->next = p->next;
		}
		
		free_string (p->disabled_by); /* free name of disabler */
		free_mem (p,sizeof(DISABLED_DATA)); /* free node */
		save_disabled(); /* save to disk */
		send_to_char ("Command enabled.\n\r",ch);
	}
	else /* not a disabled command, check if that command exists */
	{
		/* IQ test */
		if (!str_cmp(argument,"disable"))
		{
			send_to_char ("You cannot disable the disable command.\n\r",ch);
			return;
		}

		/* Search for the command */
		for (i = 0; cmd_table[i].name[0] != '\0'; i++)
			if (!str_cmp(cmd_table[i].name, argument))
				break;

		/* Found? */				
		if (cmd_table[i].name[0] == '\0')
		{
			send_to_char ("No such command.\n\r",ch);
			return;
		}

		/* Can the imm use this command at all ? */				
		if (cmd_table[i].level > get_trust(ch))
		{
			send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
			return;
		}
		
		/* Disable the command */
		
		p = alloc_mem (sizeof(DISABLED_DATA));

		p->command = &cmd_table[i];
		p->disabled_by = str_dup (ch->name); /* save name of disabler */
		p->level = get_trust(ch); /* save trust */
		p->next = disabled_first;
		disabled_first = p; /* add before the current first element */
		
		send_to_char ("Command disabled.\n\r",ch);
		save_disabled(); /* save to disk */
	}
}

/* Check if that command is disabled 
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/   
bool check_disabled (const struct cmd_type *command)
{
	DISABLED_DATA *p;
	
	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	char *name;
	int i;
	
	disabled_first = NULL;
	
	fp = fopen (DISABLED_FILE, "r");
	
	if (!fp) /* No disabled file.. no disabled commands : */
		return;
		
	name = fread_word (fp);
	
	while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
	{
		/* Find the command in the table */
		for (i = 0; cmd_table[i].name[0] ; i++)
			if (!str_cmp(cmd_table[i].name, name))
				break;
				
		if (!cmd_table[i].name[0]) /* command does not exist? */
		{
			bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
			fread_number(fp); /* level */
			fread_word(fp); /* disabled_by */
		}
		else /* add new disabled command */
		{
			p = alloc_mem(sizeof(DISABLED_DATA));
			p->command = &cmd_table[i];
			p->level = fread_number(fp);
			p->disabled_by = str_dup(fread_word(fp)); 
			p->next = disabled_first;
			
			disabled_first = p;

		}
		
		name = fread_word(fp);
	}

	fclose (fp);		
}

/* Save disabled commands */
void save_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	
	if (!disabled_first) /* delete file if no commands are disabled */
	{
		unlink (DISABLED_FILE);
		return;
	}
	
	fp = fopen (DISABLED_FILE, "w");
	
	if (!fp)
	{
		bug ("Could not open " DISABLED_FILE " for writing",0);
		return;
	}
	
	for (p = disabled_first; p ; p = p->next)
		fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
		
	fprintf (fp, "%s\n",END_MARKER);
		
	fclose (fp);
}

/* Auction system by Erwin S. Andreasen */

/* Advanced number parsing functions */
int advatoi (const char *s)
{
  char string[MAX_INPUT_LENGTH];
  char *stringptr = string;
  char tempstring[2];
  int number = 0;
  int multiplier = 0;

  strcpy (string,s);

  while ( isdigit (*stringptr))
  {
      strncpy (tempstring,stringptr,1);
      number = (number * 10) + atoi (tempstring);
      stringptr++;
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0;
  }

  while ( isdigit (*stringptr) && (multiplier > 1))
  {
      strncpy (tempstring,stringptr,1);
      multiplier = multiplier / 10;
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr))
    return 0;

  return (number);
}

int parsebet (const int currentbet, const char *argument)
{
  int newbet = 0;
  char string[MAX_INPUT_LENGTH];
  char *stringptr = string;

  strcpy (string,argument);

  if (*stringptr)
  {
    if (isdigit (*stringptr))
      newbet = advatoi (stringptr);
    else if (*stringptr == '+')
    {
        if (strlen (stringptr) == 1)
          newbet = (currentbet * 125) / 100;
        else
          newbet = (currentbet * (100 + atoi (++stringptr))) / 100;
    }
    else if ((*stringptr == '*') || (*stringptr == 'x'))
    {
        if (strlen (stringptr) == 1)
            newbet = currentbet * 2;
        else
            newbet = currentbet * atoi (++stringptr);
    }
  }

  return newbet;
}

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"AUCTION: %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character;
        if (d->connected == CON_PLAYING)
            act (buf, original, NULL, NULL, TO_CHAR);
    }
}


void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0)
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going)
            {
            case 1 :
            case 2 :
            if (auction->bet > 0)
                sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
            else
                sprintf (buf, "%s: going %s (not bet received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));

            talk_auction (buf);
            break;

            case 3 :

            if (auction->bet > 0)
            {
                sprintf (buf, "%s sold to %s for %d.",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
                    auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet;

                auction->item = NULL;
            }
            else
            {
                sprintf (buf, "No bets received for %s - object has been removed.",auction->item->short_descr);
                talk_auction(buf);
                act ("The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = NULL;
            }
            }
        }
}

/* Language functions - based on languages.c snippet */
void do_common (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *och;
    int chance;
    int chance2;
    char *translated;
    char message[MAX_STRING_LENGTH];
    extern char *makedrunk(char *string, CHAR_DATA *ch);

    if ((chance = ch->pcdata->learned[skill_lookup("common")]) == 0)
    {
        send_to_char("You don't know how to speak Common.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Say WHAT in Common?\n\r", ch);
        return;
    }

    /* Apply drunk speech if applicable */
    strncpy(message, argument, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';
    
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        makedrunk(message, ch);

    /* DEBUG: Log what we're about to send */
    {
        extern char log_buf[];
        sprintf(log_buf, "COMMON DEBUG: About to send message='%s'", message);
        log_string(log_buf);
    }

    if (number_percent() < chance)
    {
        act("In Common, you say '$t'", ch, message, NULL, TO_CHAR);

        for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
        {
            if (och == ch)
                continue;

            if (IS_NPC(och) || (chance2 = och->pcdata->learned[skill_lookup("common")]) == 0)
            {
                /* NPCs or those with no skill see translated text */
                translated = translate_text(message, LANG_HUMAN, 0);
                act("In Common, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
            }
            else if (number_percent() < chance2)
            {
                /* Those with skill see original text */
                act("In Common, $n says '$t'", ch, message, och, TO_VICT);
                check_improve(och, skill_lookup("common"), TRUE, 4);
            }
            else
            {
                /* Failed skill check - see translated text */
                translated = translate_text(message, LANG_HUMAN, chance2);
                act("In Common, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
                check_improve(och, skill_lookup("common"), FALSE, 4);
            }
        }
        check_improve(ch, skill_lookup("common"), TRUE, 4);
    }
    else
    {
        check_improve(ch, skill_lookup("common"), FALSE, 4);
        send_to_char("You stumble over the words.\n\r", ch);
    }
}

void do_human (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *och;
    int chance;
    int chance2;
    char *translated;

    if ((chance = ch->pcdata->learned[skill_lookup("human")]) == 0)
    {
        send_to_char("You don't know how to speak Human.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Say WHAT in Human?\n\r", ch);
        return;
    }

    if (number_percent() < chance)
    {
        act("In Human, you say '$t'", ch, argument, NULL, TO_CHAR);

        for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
        {
            if (och == ch)
                continue;

            if (IS_NPC(och) || (chance2 = och->pcdata->learned[skill_lookup("human")]) == 0)
            {
                /* NPCs or those with no skill see translated text */
                translated = translate_text(argument, LANG_HUMAN, 0);
                act("In Human, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
            }
            else if (number_percent() < chance2)
            {
                /* Those with skill see original text */
                act("In Human, $n says '$t'", ch, argument, och, TO_VICT);
                check_improve(och, skill_lookup("human"), TRUE, 4);
            }
            else
            {
                /* Failed skill check - see translated text */
                translated = translate_text(argument, LANG_HUMAN, chance2);
                act("In Human, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
                check_improve(och, skill_lookup("human"), FALSE, 4);
            }
        }
        check_improve(ch, skill_lookup("human"), TRUE, 4);
    }
    else
    {
        check_improve(ch, skill_lookup("human"), FALSE, 4);
        send_to_char("You stumble over the words.\n\r", ch);
    }
}

void do_elvish (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *och;
    int chance;
    int chance2;
    char *translated;

    if ((chance = ch->pcdata->learned[skill_lookup("elvish")]) == 0)
    {
        send_to_char("You don't know how to speak Elvish.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Say WHAT in Elvish?\n\r", ch);
        return;
    }

    if (number_percent() < chance)
    {
        act("In Elvish, you say '$t'", ch, argument, NULL, TO_CHAR);

        for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
        {
            if (och == ch)
                continue;

            if (IS_NPC(och) || (chance2 = och->pcdata->learned[skill_lookup("elvish")]) == 0)
            {
                /* NPCs or those with no skill see translated text */
                translated = translate_text(argument, LANG_ELVISH, 0);
                act("In Elvish, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
            }
            else if (number_percent() < chance2)
            {
                /* Those with skill see original text */
                act("In Elvish, $n says '$t'", ch, argument, och, TO_VICT);
                check_improve(och, skill_lookup("elvish"), TRUE, 4);
            }
            else
            {
                /* Failed skill check - see translated text */
                translated = translate_text(argument, LANG_ELVISH, chance2);
                act("In Elvish, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
                check_improve(och, skill_lookup("elvish"), FALSE, 4);
            }
        }
        check_improve(ch, skill_lookup("elvish"), TRUE, 4);
    }
    else
    {
        check_improve(ch, skill_lookup("elvish"), FALSE, 4);
        send_to_char("You stumble over the words.\n\r", ch);
    }
}

void do_dwarvish (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *och;
    int chance;
    int chance2;
    char *translated;

    if ((chance = ch->pcdata->learned[skill_lookup("dwarvish")]) == 0)
    {
        send_to_char("You don't know how to speak Dwarvish.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Say WHAT in Dwarvish?\n\r", ch);
        return;
    }

    if (number_percent() < chance)
    {
        act("In Dwarvish, you say '$t'", ch, argument, NULL, TO_CHAR);

        for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
        {
            if (och == ch)
                continue;

            if (IS_NPC(och) || (chance2 = och->pcdata->learned[skill_lookup("dwarvish")]) == 0)
            {
                /* NPCs or those with no skill see translated text */
                translated = translate_text(argument, LANG_DWARVISH, 0);
                act("In Dwarvish, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
            }
            else if (number_percent() < chance2)
            {
                /* Those with skill see original text */
                act("In Dwarvish, $n says '$t'", ch, argument, och, TO_VICT);
                check_improve(och, skill_lookup("dwarvish"), TRUE, 4);
            }
            else
            {
                /* Failed skill check - see translated text */
                translated = translate_text(argument, LANG_DWARVISH, chance2);
                act("In Dwarvish, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
                check_improve(och, skill_lookup("dwarvish"), FALSE, 4);
            }
        }
        check_improve(ch, skill_lookup("dwarvish"), TRUE, 4);
    }
    else
    {
        check_improve(ch, skill_lookup("dwarvish"), FALSE, 4);
        send_to_char("You stumble over the words.\n\r", ch);
    }
}

void do_verbeeg (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *och;
    int chance;
    int chance2;
    char *translated;

    if ((chance = ch->pcdata->learned[skill_lookup("verbeeg")]) == 0)
    {
        send_to_char("You don't know how to speak Verbeeg.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Say WHAT in Verbeeg?\n\r", ch);
        return;
    }

    if (number_percent() < chance)
    {
        act("In Verbeeg, you say '$t'", ch, argument, NULL, TO_CHAR);

        for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
        {
            if (och == ch)
                continue;

            if (IS_NPC(och) || (chance2 = och->pcdata->learned[skill_lookup("verbeeg")]) == 0)
            {
                /* NPCs or those with no skill see translated text */
                translated = translate_text(argument, LANG_VERBEEG, 0);
                act("In Verbeeg, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
            }
            else if (number_percent() < chance2)
            {
                /* Those with skill see original text */
                act("In Verbeeg, $n says '$t'", ch, argument, och, TO_VICT);
                check_improve(och, skill_lookup("verbeeg"), TRUE, 4);
            }
            else
            {
                /* Failed skill check - see translated text */
                translated = translate_text(argument, LANG_VERBEEG, chance2);
                act("In Verbeeg, $n says '$t'", ch, translated, och, TO_VICT);
                free_mem(translated, strlen(translated) + 1);
                check_improve(och, skill_lookup("verbeeg"), FALSE, 4);
            }
        }
        check_improve(ch, skill_lookup("verbeeg"), TRUE, 4);
    }
    else
    {
        check_improve(ch, skill_lookup("verbeeg"), FALSE, 4);
        send_to_char("You stumble over the words.\n\r", ch);
    }
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
 
    one_argument( argument, arg );
 
    if (arg[0] == '\0')
    {
        send_to_char("Circle whom?\n\r",ch);
        return;
    }
 
 
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
 
    if ( is_safe( ch, victim ) )
      return;
 
    if (IS_NPC(victim) &&
         victim->fighting != NULL &&
        !is_same_group(ch,victim->fighting))
 
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }
 
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }
 
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
        damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE, FALSE);
    }
 
    return;
}

void do_smoke (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *pObj;
    int amount, range;

    if ( argument[0] == '\0' )
    {
		send_to_char( "{cSmoke what?\n\r", ch );
		return;
    }
	if ( ( pObj = get_obj_carry( ch, argument, ch ) ) == NULL )
	{
		send_to_char( "{cYou need a pipe if you want to smoke tabac.\n\r",ch);
		return;
	}
	if ( pObj->item_type != ITEM_PIPE_TABAC)
	{
		send_to_char( "{cYou need a pipe if you want to smoke tabac.\n\r",ch);
		return;
	}
	/* Check if pipe is packed */
	if ( pObj->value[1] <= 0 )
	{
	    send_to_char( "{cYou have to pack the pipe first.\n\r", ch );
	    return;
	}
	
	/* Check if pipe is lit */
	if ( !IS_SET(pObj->value[4], 1) ) /* Using value[4] as lit flag */
	{
	    send_to_char( "{cYou need to light the pipe first.\n\r", ch );
	    return;
	}
	/* do some randomness */
	range = number_range(1,4);
	switch( range )
	{
		default:
	    	act( "{c$n smokes tabac from $p.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou smoke tabac from $p.", ch, pObj, NULL, TO_CHAR );
	    break;
		case 1:
	    	act( "{c$n inhales deeply from $p.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou inhale deeply from $p.", ch, pObj, NULL, TO_CHAR );
	    break;
		case 2:
	    	act( "{c$n puffs on $p for a moment.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou puff on $p for a moment.", ch, pObj, NULL, TO_CHAR );
	    break;
		case 3:
	    	act( "{c$n draws deeply on $p.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou draw deeply $p.", ch, pObj, NULL, TO_CHAR );
	    break;
	}

	/* if the the char isn't already smoking, add some smoke
	 * if the char is not set to smoke, they cant exhale. can change
	 * this to act or whatever. in AWW, tabac flags are used for other
	 * things, as well.
	 */
	if ( !IS_SET( ch->tabac_flags, TABAC_SMOKING ) )
		SET_BIT(ch->tabac_flags,TABAC_SMOKING);

	/* Mix up the numbers a bit */
	amount = number_range(1,2);
    if (pObj->value[0] > 0)
    {
        pObj->value[1] -= amount;
        amount = number_range(1,6);
        switch ( amount )
        {
			case 1:
			    act( "{c$p crackles and fumes.",  ch, pObj, NULL, TO_ROOM );
			    act( "{c$p crackles and fumes.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 2:
			    act( "{c$p leaks spiralling fingers of smoke.",  ch, pObj, NULL, TO_ROOM );
			    act( "{c$p leaks spiralling fingers of smoke.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 3:
			    act( "{c$p spits tiny red sparks up from it's bowl.",  ch, pObj, NULL, TO_ROOM );
			    act( "{c$p spits tiny red sparks up from it's bowl.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 4:
				act( "{cA thin curl of smoke leaks up from $p's bowl.", ch, pObj, NULL, TO_ROOM );
				act( "{cA thin curl of smoke leaks up from $p's bowl.", ch, pObj, NULL, TO_CHAR );
			break;
		    case 5:
				act( "{cLong threads of smoke rise up from $p", ch, pObj, NULL, TO_ROOM );
				act( "{cLong threads of smoke rise up from $p", ch, pObj, NULL, TO_CHAR );
			break;
			default:
			act( "{c$p glows a deep, dark red color.", ch, pObj, NULL, TO_ROOM );
			act( "{c$p glows a deep, dark red color.", ch, pObj, NULL, TO_CHAR );
			break;
		}
	}
	
	/* Trigger tobacco spell effects if pipe has tobacco with spells */
	if (pObj->value[3] > 0) /* Check if pipe has tobacco with spells */
	{
	    trigger_tobacco_spells(ch, pObj);
	}
	
    return;
}

void do_exhale(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	/* that smoking flag has to be set */
	if (!IS_SET(ch->tabac_flags,TABAC_SMOKING))
	{
		send_to_char("{wBut you aren't smoking from a pipe.\n\r",ch);
		return;
	}

	one_argument( argument, arg );

	/* do the basics */
	if ( arg[0] == '\0')
	{
		stc("You can use many different exhales to enhance your smoking pleasure.\n\r",ch);
		stc("Try these others:\n\r",ch);
		stc("RINGS LINES BALLS WEB PHOENIX HORNS or try EXHALE HELP\n\r",ch);
		act("{c$n exhales some pale gray smoke.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale some pale gray smoke.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	/* send some help */
	if ( !str_prefix( arg, "help" ))
	{
		stc("Before you can exhale, you must first smoke from your pipe.\n\r", ch);
		stc("After you have enjoyed that flavorful inhalation of vapors...\n\r", ch);
		stc("You can use many different exhales to enhance your smoking pleasure.\n\r",ch);
		stc("Try these different types of exhales to impress and astound:\n\r",ch);
		stc("RINGS LINES BALLS WEB PHOENIX HORNS or try EXHALE HELP\n\r",ch);
		return;
	}
	if ( !str_prefix( arg, "rings" ))
	{
		act("{c$n exhales large round smoke rings.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale large round smoke rings.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	if ( !str_prefix( arg, "lines" ))
	{
		act("{c$n exhales a long line of smoke.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale a long line of smoke.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	if ( !str_prefix( arg, "balls" ))
	{
		act("{c$n puffs a big round ball of smoke.", ch, NULL, NULL, TO_ROOM );
		act("{cYou puff a big round ball of smoke.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	if ( !str_prefix( arg, "web" ))
	{
		act("{c$n exhales thin, wispy streams of smoke that merge to form a delicate web.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale thin, wispy streams of smoke that merge to form a delicate web.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	if ( !str_prefix( arg, "phoenix" ))
	{
		act("{c$n exhales lines of smoke that form into a phoenix, only to disperse into ashes.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale lines of smoke that form into a phoenix, only to disperse into ashes.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
	if ( !str_prefix( arg, "horns" ))
	{
		act("{c$n exhales two lines of smoke that settle over $m like a pair of horns.", ch, NULL, NULL, TO_ROOM );
		act("{cYou exhale two lines of smoke that settle over you like a pair of horns.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_SMOKING);
		return;
	}
}

void do_chew (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *pObj;
    int amount;

    if ( argument[0] == '\0' )
    {
		send_to_char( "{cChew what?\n\r", ch );
		return;
    }
	if ( ( pObj = get_obj_carry( ch, argument, ch ) ) == NULL )
	{
		send_to_char( "{cYou need some tabac if you want to chew.\n\r",ch);
		return;
	}
	if ( pObj->item_type != ITEM_CHEW_TABAC)
	{
		send_to_char( "{cYou need some tabac if you want to chew.\n\r",ch);
		return;
	}
	if ( pObj->value[1] <= 0 )
	{
	    send_to_char( "{cYou're all out of tabac.\n\r", ch );
        extract_obj(pObj);
	    return;
	}
    act( "{c$n chews contentedly on some $p.",  ch, pObj, NULL, TO_ROOM );
    act( "{cYou chew contentedly on some $p.", ch, pObj, NULL, TO_CHAR );
	SET_BIT(ch->tabac_flags,TABAC_CHEWING);
	/* Mix up the numbers a bit */
	amount = number_range(1,2);
    if (pObj->value[0] > 0)
    {
        pObj->value[1] -= amount;
        amount = number_range(1,6);
        switch ( amount )
        {
			case 1:
		    act( "{c$n grinds some $p between $s teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 2:
		    act( "{c$n grinds some $p between $s teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 3:
		    act( "{c$n grinds some $p between $s teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
		    break;
		    case 4:
		    act( "{c$n grinds some $p between $s teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
			break;
		    case 5:
		    act( "{c$n grinds some $p between your teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
			break;
			default:
		    act( "{c$n grinds some $p between your teeth.",  ch, pObj, NULL, TO_ROOM );
		    act( "{cYou grind some $p between your teeth.",  ch, pObj, NULL, TO_CHAR );
			break;
		}
	}
    return;
}

void do_spit (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (!IS_SET(ch->tabac_flags,TABAC_CHEWING))
	{
		send_to_char("{wBut you aren't chewing any tabac.\n\r",ch);
		return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0')
	{
		act("{c$n spits out a juicy brown glob of tabac juice.", ch, NULL, NULL, TO_ROOM );
		act("{cYou spit out a juicy brown glob of tabac juice.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_CHEWING);
		return;
	}
	if ( !str_prefix( arg, "stream" ))
	{
		act("{c$n spits a long, thin stream of tabac juice.", ch, NULL, NULL, TO_ROOM );
		act("{cYou spit a long, thin stream of tabac juice.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_CHEWING);
		return;
	}
	if ( !str_prefix( arg, "spray" ))
	{
		act("{c$n spits a long spray of juicy brown tabac juice.", ch, NULL, NULL, TO_ROOM );
		act("{cYou spit a long spray of juicy brown tabac juice.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_CHEWING);
		return;
	}
	if ( !str_prefix( arg, "ball" ))
	{
		act("{c$n hawks up a big ball of brown tabac juice.", ch, NULL, NULL, TO_ROOM );
		act("{cYou hawk up a big ball of brown tabac juice.", ch, NULL, NULL, TO_CHAR );
		REMOVE_BIT(ch->tabac_flags,TABAC_CHEWING);
		return;
	}
}

/* Helper function to trigger tobacco spell effects */
void trigger_tobacco_spells(CHAR_DATA *ch, OBJ_DATA *pipe)
{
    int sn;
    /* Only check pipe->value[3] which contains the tobacco spell */
    if (pipe->value[3] > 0)
    {
        sn = pipe->value[3];
        /* Cast the spell on the character */
        (*skill_table[sn].spell_fun) (sn, ch->level, ch, (void *)ch, TAR_CHAR_SELF);
    }
}

void do_light(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *pipe, *tinderbox;
    
    if (argument[0] == '\0')
    {
        send_to_char("{cLight what?\n\r", ch);
        return;
    }
    
    if ((pipe = get_obj_carry(ch, argument, ch)) == NULL)
    {
        send_to_char("{cYou don't have that.\n\r", ch);
        return;
    }
    
    if (pipe->item_type != ITEM_PIPE_TABAC)
    {
        send_to_char("{cYou can only light pipes.\n\r", ch);
        return;
    }
    
    if (pipe->value[1] <= 0)
    {
        send_to_char("{cYou have to pack the pipe first.\n\r", ch);
        return;
    }
    
    if (IS_SET(pipe->value[4], 1))
    {
        send_to_char("{cThe pipe is already lit.\n\r", ch);
        return;
    }
    
    /* Look for tinderbox */
    tinderbox = NULL;
    for (tinderbox = ch->carrying; tinderbox != NULL; tinderbox = tinderbox->next_content)
    {
        if (tinderbox->item_type == ITEM_TINDERBOX && tinderbox->value[0] > 0)
            break;
    }
    
    if (tinderbox == NULL)
    {
        /* Check if they have a tinderbox but it's empty */
        bool has_empty_tinderbox = FALSE;
        for (tinderbox = ch->carrying; tinderbox != NULL; tinderbox = tinderbox->next_content)
        {
            if (tinderbox->item_type == ITEM_TINDERBOX)
            {
                has_empty_tinderbox = TRUE;
                break;
            }
        }
        
        if (has_empty_tinderbox)
        {
            send_to_char("{cYour tinderbox is empty. Use OLC to set value[0] (uses) on your tinderbox.\n\r", ch);
        }
        else
        {
            send_to_char("{cYou need a tinderbox to light the pipe.\n\r", ch);
        }
        return;
    }
    
    /* Use tinderbox */
    tinderbox->value[0]--;
    if (tinderbox->value[0] <= 0)
    {
        act("{cYour tinderbox is empty.", ch, NULL, NULL, TO_CHAR);
        extract_obj(tinderbox);
    }
    
    /* Light the pipe */
    SET_BIT(pipe->value[4], 1);
    act("{c$n lights $p with $s tinderbox.", ch, pipe, tinderbox, TO_ROOM);
    act("{cYou light $p with $s tinderbox.", ch, pipe, tinderbox, TO_CHAR);
}

void do_pack(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *pipe, *tobacco;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int amount, max_add;
    
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("{cPack what with what?\n\r", ch);
        return;
    }
    
    if ((pipe = get_obj_carry(ch, arg1, ch)) == NULL)
    {
        send_to_char("{cYou don't have that pipe.\n\r", ch);
        return;
    }
    
    if (pipe->item_type != ITEM_PIPE_TABAC)
    {
        send_to_char("{cYou can only pack pipes.\n\r", ch);
        return;
    }
    
    if ((tobacco = get_obj_carry(ch, arg2, ch)) == NULL)
    {
        send_to_char("{cYou don't have that tobacco.\n\r", ch);
        return;
    }
    
    if (tobacco->item_type != ITEM_TOBACCO)
    {
        send_to_char("{cYou can only pack pipes with tobacco.\n\r", ch);
        return;
    }
    
    if (tobacco->value[0] <= 0)
    {
        send_to_char("{cThat tobacco is empty.\n\r", ch);
        return;
    }
    
    /* Calculate how much can be added */
    max_add = pipe->value[2] - pipe->value[1];
    if (max_add <= 0)
    {
        if (pipe->value[2] <= 0)
        {
            send_to_char("{cThe pipe has no capacity set. Use OLC to set value[2] (max capacity).\n\r", ch);
        }
        else
        {
            send_to_char("{cThe pipe is already full.\n\r", ch);
        }
        return;
    }
    
    amount = UMIN(tobacco->value[0], max_add);
    
    /* Add tobacco to pipe */
    pipe->value[1] += amount;
    tobacco->value[0] -= amount;
    
    /* Copy spell effects from tobacco to pipe */
    pipe->value[3] = tobacco->value[1]; /* Store tobacco spell 1 in pipe */
    
    act("{c$n packs $p with $P.", ch, pipe, tobacco, TO_ROOM);
    act("{cYou pack $p with $P.", ch, pipe, tobacco, TO_CHAR);
    
    if (tobacco->value[0] <= 0)
    {
        act("{c$P is now empty.", ch, NULL, tobacco, TO_CHAR);
        extract_obj(tobacco);
    }
}

void do_scrape(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *pipe;
    
    if (argument[0] == '\0')
    {
        send_to_char("{cScrape what?\n\r", ch);
        return;
    }
    
    if ((pipe = get_obj_carry(ch, argument, ch)) == NULL)
    {
        send_to_char("{cYou don't have that.\n\r", ch);
        return;
    }
    
    if (pipe->item_type != ITEM_PIPE_TABAC)
    {
        send_to_char("{cYou can only scrape pipes.\n\r", ch);
        return;
    }
    
    if (pipe->value[1] <= 0)
    {
        send_to_char("{cThe pipe is already empty.\n\r", ch);
        return;
    }
    
    /* Remove all tobacco from pipe */
    pipe->value[1] = 0;
    pipe->value[3] = 0; /* Clear spell effects */
    pipe->value[4] = 0; /* Clear lit status */
    
    act("{c$n scrapes the remaining tobacco from $p.", ch, pipe, NULL, TO_ROOM);
    act("{cYou scrape the remaining tobacco from $p.", ch, pipe, NULL, TO_CHAR);
}

void do_gather(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *firewood;
    int chance;
    
    /* Check if in forest */
    if (ch->in_room->sector_type != SECT_FOREST)
    {
        send_to_char("{cYou can only gather firewood in a forest.\n\r", ch);
        return;
    }
    
    /* Check if already carrying firewood */
    for (firewood = ch->carrying; firewood != NULL; firewood = firewood->next_content)
    {
        if (firewood->pIndexData->vnum == OBJ_VNUM_FIREWOOD)
        {
            send_to_char("{cYou are already carrying firewood.\n\r", ch);
            return;
        }
    }
    
    /* Chance to successfully gather firewood */
    chance = number_percent();
    if (chance > 70) /* 30% success rate */
    {
        send_to_char("{cYou search around but can't find any suitable firewood.\n\r", ch);
        act("{c$n searches around for firewood but finds nothing.", ch, NULL, NULL, TO_ROOM);
        return;
    }
    
    /* Create firewood */
    firewood = create_object(get_obj_index(OBJ_VNUM_FIREWOOD), 0);
    obj_to_char(firewood, ch);
    
    act("{c$n gathers some firewood from the forest.", ch, NULL, NULL, TO_ROOM);
    act("{cYou gather some firewood from the forest.", ch, NULL, NULL, TO_CHAR);
}

void do_campfire(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *firewood, *tinderbox, *campfire;
    bool has_firewood = FALSE, has_tinderbox = FALSE;
    
    /* Check if already a campfire in room */
    for (campfire = ch->in_room->contents; campfire != NULL; campfire = campfire->next_content)
    {
        if (campfire->pIndexData->vnum == OBJ_VNUM_CAMPFIRE)
        {
            send_to_char("{cThere is already a campfire here.\n\r", ch);
            return;
        }
    }
    
    /* Check for firewood */
    for (firewood = ch->carrying; firewood != NULL; firewood = firewood->next_content)
    {
        if (firewood->pIndexData->vnum == OBJ_VNUM_FIREWOOD)
        {
            has_firewood = TRUE;
            break;
        }
    }
    
    /* Check for tinderbox */
    for (tinderbox = ch->carrying; tinderbox != NULL; tinderbox = tinderbox->next_content)
    {
        if (tinderbox->item_type == ITEM_TINDERBOX && tinderbox->value[0] > 0)
        {
            has_tinderbox = TRUE;
            break;
        }
    }
    
    if (!has_firewood)
    {
        send_to_char("{cYou need firewood to make a campfire.\n\r", ch);
        return;
    }
    
    if (!has_tinderbox)
    {
        send_to_char("{cYou need a tinderbox to light the campfire.\n\r", ch);
        return;
    }
    
    /* Use firewood and tinderbox */
    extract_obj(firewood);
    tinderbox->value[0]--;
    if (tinderbox->value[0] <= 0)
    {
        act("{cYour tinderbox is empty.", ch, NULL, NULL, TO_CHAR);
        extract_obj(tinderbox);
    }
    
    /* Create campfire */
    campfire = create_object(get_obj_index(OBJ_VNUM_CAMPFIRE), 0);
    obj_to_room(campfire, ch->in_room);
    
    act("{c$n builds a campfire using firewood and $s tinderbox.", ch, NULL, tinderbox, TO_ROOM);
    act("{cYou build a campfire using firewood and your tinderbox.", ch, NULL, tinderbox, TO_CHAR);
}
