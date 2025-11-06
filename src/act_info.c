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
 *           act_info.c - November 3, 2025
 */            
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

/*   QuickMUD - The Lazy Man's ROM - $Id: act_info.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

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
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

#ifndef IS_STAFF
#define IS_STAFF(ch)        ((ch)->level >= LEVEL_IMMORTAL)
#endif

#ifndef IS_REAL_HERO
#define IS_REAL_HERO(ch)    ((ch)->level >= LEVEL_HERO && (ch)->level < LEVEL_IMMORTAL)
#endif

void look_window( CHAR_DATA *ch, OBJ_DATA *obj );

char *const where_name[] = {
    "{C<{Wused as light{C>{x     ",
    "{C<{Wworn on finger{C>{x    ",
    "{C<{Wworn on finger{C>{x    ",
    "{C<{Wworn around neck{C>{x  ",
    "{C<{Wworn around neck{C>{x  ",
    "{C<{Wworn on torso{C>{x     ",
    "{C<{Wworn on head{C>{x      ",
    "{C<{Wworn on legs{C>{x      ",
    "{C<{Wworn on feet{C>{x      ",
    "{C<{Wworn on hands{C>{x     ",
    "{C<{Wworn on arms{C>{x      ",
    "{C<{Wworn as shield{C>{x    ",
    "{C<{Wworn about body{C>{x   ",
    "{C<{Wworn about waist{C>{x  ",
    "{C<{Wworn around wrist{C>{x ",
    "{C<{Wworn around wrist{C>{x ",
    "{C<{Wwielded{C>{x           ",
    "{C<{Wheld{C>{x              ",
    "{C<{Wfloating nearby{C>{x   ",
    "{C<{Wsecondary weapon{C>{x  ",
    "{C<{Wcommstone link{C>{x    ",
};


/* for  keeping track of the player count */
int max_on = 0;
int boot_high = 0;
/*
 * Local functions.
 */
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
void show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch,
                              bool fShort, bool fShowNothing));
void show_char_to_char_0 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args ((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args ((CHAR_DATA * ch));



char *format_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
        || (obj->description == NULL || obj->description[0] == '\0'))
        return buf;

        if (!IS_SET(ch->comm, COMM_LONG) )
    {
	strcat( buf, "{x[{w.{R.{Y.{M.{y.{W.{G.{x]");
	if ( IS_OBJ_STAT(obj, ITEM_INVIS)	)   buf[5] = 'V';
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	&& IS_OBJ_STAT(obj, ITEM_EVIL)		)   buf[8] = 'E';
	if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
	&&  IS_OBJ_STAT(obj,ITEM_BLESS)		)   buf[11] = 'B';
	if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	&& IS_OBJ_STAT(obj, ITEM_MAGIC)		)   buf[14] = 'M';
	if ( IS_OBJ_STAT(obj, ITEM_GLOW)	)   buf[17] = 'G';
	if ( IS_OBJ_STAT(obj, ITEM_HUM)		)   buf[20] = 'H';
	if ( IS_OBJ_STAT(obj, ITEM_QUEST)	)   buf[23] = 'Q';
	if (!strcmp(buf, "{x[{w.{R.{Y.{M.{y.{W.{G.{x]") )
	    buf[0] = '\0';
    }
    else 
    {
    if (IS_OBJ_STAT (obj, ITEM_INVIS))
        strcat (buf, "{W({CInvis{W){x ");
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
        strcat (buf, "{W({RRed Aura{W){x ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
        strcat (buf, "{W({YGolden Aura{W){x ");
    if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
        strcat (buf, "{W({MMagical{W){x ");
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        strcat (buf, "{W({yGlowing{W){x ");
    if (IS_OBJ_STAT (obj, ITEM_HUM))
        strcat (buf, "{W({GHumming{W){x ");
    if (IS_OBJ_STAT (obj, ITEM_QUESTITEM))
	strcat (buf, "{W({CA{MQS{CT{W){x ");
    }

    if (buf[0] != '\0')
    {
	strcat(buf, " ");
    }
    if (fShort)
    {
        if (obj->short_descr != NULL)
            strcat (buf, obj->short_descr);
    }
    else
    {
        if (obj->description != NULL)
            strcat (buf, obj->description);
    }
    if (strlen(buf)<=0)
	strcat(buf,"This object has no description. Please inform the IMP.");


    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
                        bool fShowNothing)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if (ch->desc == NULL)
        return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf ();

    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_content)
        count++;
    prgpstrShow = alloc_mem (count * sizeof (char *));
    prgnShow = alloc_mem (count * sizeof (int));
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for (obj = list; obj != NULL; obj = obj->next_content)
    {
        if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
        {
            pstrShow = format_obj_to_char (obj, ch, fShort);

            fCombine = FALSE;

            if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for (iShow = nShow - 1; iShow >= 0; iShow--)
                {
                    if (!strcmp (prgpstrShow[iShow], pstrShow))
                    {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if (!fCombine)
            {
                prgpstrShow[nShow] = str_dup (pstrShow);
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    for (iShow = 0; iShow < nShow; iShow++)
    {
        if (prgpstrShow[iShow][0] == '\0')
        {
            free_string (prgpstrShow[iShow]);
            continue;
        }

        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
        {
            if (prgnShow[iShow] != 1)
            {
                sprintf (buf, "(%2d) ", prgnShow[iShow]);
                add_buf (output, buf);
            }
            else
            {
                add_buf (output, "     ");
            }
        }
        add_buf (output, prgpstrShow[iShow]);
        add_buf (output, "\n\r");
        free_string (prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0)
    {
        if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
            send_to_char ("     ", ch);
        send_to_char ("Nothing.\n\r", ch);
    }
    page_to_char (buf_string (output), ch);

    /*
     * Clean up.
     */
    free_buf (output);
    free_mem (prgpstrShow, count * sizeof (char *));
    free_mem (prgnShow, count * sizeof (int));

    return;
}



void show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (!IS_SET(ch->comm, COMM_LONG) )
    {
	strcat( buf, "{x[{y.{D.{M.{b.{m.{R.{Y.{W.{G.{x]");
	if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) buf[5] = 'V';
	if ( IS_AFFECTED(victim, AFF_HIDE)        ) buf[8] = 'H';
	if ( IS_AFFECTED(victim, AFF_CHARM)       ) buf[11] = 'C';
	if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) buf[14] = 'T';
	if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) buf[17] = 'P';
	if ( IS_EVIL(victim)
	&& IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) buf[20] = 'E';
	if ( IS_GOOD(victim)
	&&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) buf[23] = 'G';
	if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) buf[26] = 'S';
	/*if ( victim->on_gquest )
	{
	    if (!IS_NPC(victim) || IS_IMMORTAL(ch))
		buf[38] = 'Q';
	}*/
	if (!strcmp(buf, "{x[{y.{D.{M.{b.{m.{R.{Y.{W.{G.{x]") )
	    buf[0] = '\0';
	if ( IS_SET(victim->comm,COMM_AFK  )      ) strcat( buf, "[{yAFK{x]");
	if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "({WWizi{x)");
    }
    else
    {
        if (IS_SET (victim->comm, COMM_AFK))
            strcat (buf, "{W[{GA{YF{GK{W]{x ");
        if (IS_AFFECTED (victim, AFF_INVISIBLE))
            strcat (buf, "{W({CInvis{W){x ");
        if (victim->invis_level >= LEVEL_HERO)
            strcat (buf, "{W({CWizi{W){x ");
        if (IS_AFFECTED (victim, AFF_HIDE))
            strcat (buf, "{W({DHide{W){x ");
        if (IS_AFFECTED (victim, AFF_CHARM))
            strcat (buf, "{W({mCharmed{W){x ");
        if (IS_AFFECTED (victim, AFF_PASS_DOOR))
            strcat (buf, "{W{cTranslucent{W){x ");
        if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
            strcat (buf, "{W({MPink Aura{W){x ");
        if (IS_EVIL (victim) && IS_AFFECTED (ch, AFF_DETECT_EVIL))
            strcat (buf, "{W({RRed Aura{W){x ");
        if (IS_GOOD (victim) && IS_AFFECTED (ch, AFF_DETECT_GOOD))
            strcat (buf, "{W({yGolden Aura{W){x ");
        if (IS_AFFECTED (victim, AFF_SANCTUARY))
            strcat (buf, "{W({wWhite Aura{W){x ");
        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_KILLER))
            strcat (buf, "{W({rKILLER{W){x ");  
        if (!IS_NPC (victim) && IS_SET (victim->act, PLR_THIEF))
            strcat (buf, "{W({rTHIEF{W){x ");
        if (!IS_NPC(victim) && !victim->desc)
            strcat( buf, "{r({RLinkdead{r){x " );
    }
    if ( RIDDEN(victim) )
        if ( ch != RIDDEN(victim) )
            strcat( buf, "(Ridden) " );
        else
            strcat( buf, "(Your mount) " );

    /* Show Quest Mob targets in the room -Koqlb*/
    if (IS_NPC(victim) && !IS_NPC(ch) && ch->pcdata && ch->pcdata->questmob > 0 
       && victim->pIndexData->vnum == ch->pcdata->questmob)
        strcat( buf, "{W[{RTA{rRG{RET{W]{x ");
    if (victim->position == victim->start_pos
        && victim->long_descr[0] != '\0')
    {
        strcat (buf, victim->long_descr);
        send_to_char (buf, ch);
        return;
    }

    strcat (buf, PERS (victim, ch));
    if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
        && victim->position == POS_STANDING && ch->on == NULL)
        strcat (buf, victim->pcdata->title);

    switch (victim->position)
    {
        case POS_DEAD:
            strcat (buf, " is DEAD!!");
            break;
        case POS_MORTAL:
            strcat (buf, " is mortally wounded.");
            break;
        case POS_INCAP:
            strcat (buf, " is incapacitated.");
            break;
        case POS_STUNNED:
            strcat (buf, " is lying here stunned.");
            break;
        case POS_SLEEPING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], SLEEP_AT))
                {
                    sprintf (message, " is sleeping at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], SLEEP_ON))
                {
                    sprintf (message, " is sleeping on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is sleeping in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is sleeping here.");
            break;
        case POS_RESTING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], REST_AT))
                {
                    sprintf (message, " is resting at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], REST_ON))
                {
                    sprintf (message, " is resting on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is resting in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is resting here.");
            break;
        case POS_SITTING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], SIT_AT))
                {
                    sprintf (message, " is sitting at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], SIT_ON))
                {
                    sprintf (message, " is sitting on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is sitting in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else
                strcat (buf, " is sitting here.");
            break;
        case POS_STANDING:
            if (victim->on != NULL)
            {
                if (IS_SET (victim->on->value[2], STAND_AT))
                {
                    sprintf (message, " is standing at %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else if (IS_SET (victim->on->value[2], STAND_ON))
                {
                    sprintf (message, " is standing on %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
                else
                {
                    sprintf (message, " is standing in %s.",
                             victim->on->short_descr);
                    strcat (buf, message);
                }
            }
            else if ( MOUNTED(victim) )
            {
                strcat( buf, " is here, riding " );
                strcat( buf, MOUNTED(victim)->short_descr );
                strcat( buf, ".");
            }
            else
                strcat (buf, " is here.");
            break;
        case POS_FIGHTING:
            strcat (buf, " is here, fighting ");
            if (victim->fighting == NULL)
                strcat (buf, "thin air??");
            else if (victim->fighting == ch)
                strcat (buf, "YOU!");
            else if (victim->in_room == victim->fighting->in_room)
            {
                strcat (buf, PERS (victim->fighting, ch));
                strcat (buf, ".");
            }
            else
                strcat (buf, "someone who left??");
            break;
    }

    strcat (buf, "\n\r");
    buf[0] = UPPER (buf[0]);
    send_to_char (buf, ch);
    return;
}



void show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if (can_see (victim, ch))
    {
        if (ch == victim)
            act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
        else
        {
            act ("$n looks at you.", ch, NULL, victim, TO_VICT);
            act ("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
        }
    }

    if (victim->description[0] != '\0')
    {
        send_to_char (victim->description, ch);
    }
    else
    {
        act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
    }

    if ( MOUNTED(victim) )
    {
        sprintf( buf, "%s is riding %s.\n\r", victim->name, MOUNTED(victim)->short_descr);
        send_to_char( buf, ch);
    }
    if ( RIDDEN(victim) )
    {
        sprintf( buf, "%s is being ridden by %s.\n\r", victim->short_descr, RIDDEN(victim)->name );
        send_to_char( buf, ch);
    }

    if (victim->max_hit > 0)
        percent = (100 * victim->hit) / victim->max_hit;
    else
        percent = -1;

    strcpy (buf, PERS (victim, ch));

    if (percent >= 100)
        strcat (buf, " {Wis in excellent condition.{x\n\r");
    else if (percent >= 90)
        strcat (buf, " {yhas a few scratches.{x\n\r");
    else if (percent >= 75)
        strcat (buf, " {yhas some small wounds and bruises.{x\n\r");
    else if (percent >= 50)
        strcat (buf, " {Yhas quite a few wounds.{x\n\r");
    else if (percent >= 30)
        strcat (buf, " {Bhas some big nasty wounds and scratches.{x\n\r");
    else if (percent >= 15)
        strcat (buf, " {rlooks pretty hurt.{x\n\r");
    else if (percent >= 0)
        strcat (buf, " {Ris in awful condition.{x\n\r");
    else
        strcat (buf, " {Ris bleeding to death.{x\n\r");

    buf[0] = UPPER (buf[0]);
    send_to_char (buf, ch);

    found = FALSE;
    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
        if ((obj = get_eq_char (victim, iWear)) != NULL
            && can_see_obj (ch, obj))
        {
            if (!found)
            {
                send_to_char ("\n\r", ch);
                act ("$N is using:", ch, NULL, victim, TO_CHAR);
                found = TRUE;
            }
            send_to_char (where_name[iWear], ch);
            send_to_char (format_obj_to_char (obj, ch, TRUE), ch);
            send_to_char ("\n\r", ch);
        }
    }

    /* Kobolds with peek skill automatically peek, others must succeed on skill check */
    if (victim != ch && !IS_NPC (ch) && get_skill(ch, gsn_peek) > 0)
    {
        int peek_skill = get_skill(ch, gsn_peek);
        bool is_kobold = (ch->race == race_lookup("kobold"));
        
        /* Kobolds get a bonus and always succeed if they have the skill */
        if (is_kobold || number_percent() < peek_skill)
        {
            if (is_kobold)
                send_to_char ("\n\r{DYour kobold cunning reveals their belongings:{x\n\r", ch);
            else
                send_to_char ("\n\rYou peek at the inventory:\n\r", ch);
            
            show_list_to_char (victim->carrying, ch, TRUE, TRUE);
            
            if (!is_kobold)
                check_improve (ch, gsn_peek, TRUE, 4);
        }
        else if (!is_kobold)
        {
            /* Failed peek attempt for non-kobolds */
            check_improve (ch, gsn_peek, FALSE, 4);
        }
    }

    return;
}



void show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
    CHAR_DATA *rch;

    for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
        if (rch == ch)
            continue;

        if (get_trust (ch) < rch->invis_level)
            continue;

        if (can_see (ch, rch))
        {
            show_char_to_char_0 (rch, ch);
            if( MOUNTED(rch) && (rch->in_room == MOUNTED(rch)->in_room) )
                show_char_to_char_0 ( MOUNTED(rch), ch );
        }
        else if (room_is_dark (ch->in_room)
                 && IS_AFFECTED (rch, AFF_INFRARED))
        {
            send_to_char ("You see glowing red eyes watching YOU!\n\r", ch);
        }
    }

    return;
}



bool check_blind (CHAR_DATA * ch)
{

    if (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
        return TRUE;

    if (IS_AFFECTED (ch, AFF_BLIND))
    {
        send_to_char ("You can't see a thing!\n\r", ch);
        return FALSE;
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        if (ch->lines == 0)
            send_to_char ("You do not page long messages.\n\r", ch);
        else
        {
            sprintf (buf, "You currently display %d lines per page.\n\r",
                     ch->lines + 2);
            send_to_char (buf, ch);
        }
        return;
    }

    if (!is_number (arg))
    {
        send_to_char ("You must provide a number.\n\r", ch);
        return;
    }

    lines = atoi (arg);

    if (lines == 0)
    {
        send_to_char ("Paging disabled.\n\r", ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
        send_to_char ("You must provide a reasonable number.\n\r", ch);
        return;
    }

    sprintf (buf, "Scroll set to %d lines.\n\r", lines);
    send_to_char (buf, ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;

    col = 0;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
        snprintf (buf, sizeof(buf), "%-12.12s", social_table[iSocial].name);
        send_to_char (buf, ch);
        if (++col % 6 == 0)
            send_to_char ("\n\r", ch);
    }

    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
    return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "motd");
}

void do_imotd (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "imotd");
}

void do_rules (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "rules");
}

void do_story (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "story");
}

/* Moved to wizlist.c
void do_wizlist (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "wizlist");
}
*/

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist (CHAR_DATA * ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC (ch))
        return;

    send_to_char ("   action     status\n\r", ch);
    send_to_char ("---------------------\n\r", ch);

    send_to_char ("autoassist     ", ch);
    if (IS_SET (ch->act, PLR_AUTOASSIST))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autodamage     ", ch);
    if (IS_SET (ch->act, PLR_AUTODAMAGE))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autoexit       ", ch);
    if (IS_SET (ch->act, PLR_AUTOEXIT))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autogold       ", ch);
    if (IS_SET (ch->act, PLR_AUTOGOLD))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autoloot       ", ch);
    if (IS_SET (ch->act, PLR_AUTOLOOT))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autosac        ", ch);
    if (IS_SET (ch->act, PLR_AUTOSAC))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("autosplit      ", ch);
    if (IS_SET (ch->act, PLR_AUTOSPLIT))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);
        
    send_to_char ("autotick       ", ch);
    if (IS_SET (ch->act, PLR_AUTOTICK))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("telnetga       ", ch);
    if (IS_SET (ch->comm, COMM_TELNET_GA))
	    send_to_char ("{GON{x\n\r", ch);
    else
	    send_to_char ("{ROFF{x\n\r",ch);

    send_to_char ("compact mode   ", ch);
    if (IS_SET (ch->comm, COMM_COMPACT))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("prompt         ", ch);
    if (IS_SET (ch->comm, COMM_PROMPT))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("combine items  ", ch);
    if (IS_SET (ch->comm, COMM_COMBINE))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

        send_to_char("long flags     ",ch);
        if (IS_SET(ch->comm,COMM_LONG))
        send_to_char("{GON{x\n\r",ch);
        else
        send_to_char("{ROFF{x\n\r",ch);
    
    if (!IS_SET (ch->act, PLR_CANLOOT))
        send_to_char ("Your corpse is safe from thieves.\n\r", ch);
    else
        send_to_char ("Your corpse may be looted.\n\r", ch);

    if (IS_SET (ch->act, PLR_NOSUMMON))
        send_to_char ("You cannot be summoned.\n\r", ch);
    else
        send_to_char ("You can be summoned.\n\r", ch);

    if (IS_SET (ch->act, PLR_NOFOLLOW))
        send_to_char ("You do not welcome followers.\n\r", ch);
    else
        send_to_char ("You accept followers.\n\r", ch);
}

void do_autoassist (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOASSIST))
    {
        send_to_char ("Autoassist removed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOASSIST);
    }
    else
    {
        send_to_char ("You will now assist when needed.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOASSIST);
    }
}

void do_autodamage (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTODAMAGE))
    {
        send_to_char ("Autodamage is currently OFF.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTODAMAGE);
    }
    else
    {
        send_to_char ("Autodamage is currently ON.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTODAMAGE);
    }
}

void do_autoexit (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOEXIT))
    {
        send_to_char ("Exits will no longer be displayed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOEXIT);
    }
    else
    {
        send_to_char ("Exits will now be displayed.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOEXIT);
    }
}

void do_autogold (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOGOLD))
    {
        send_to_char ("Autogold removed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOGOLD);
    }
    else
    {
        send_to_char ("Automatic gold looting set.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOGOLD);
    }
}

void do_autoloot (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOLOOT))
    {
        send_to_char ("Autolooting removed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOLOOT);
    }
    else
    {
        send_to_char ("Automatic corpse looting set.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOLOOT);
    }
}

void do_autosac (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOSAC))
    {
        send_to_char ("Autosacrificing removed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOSAC);
    }
    else
    {
        send_to_char ("Automatic corpse sacrificing set.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOSAC);
    }
}

void do_autosplit (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOSPLIT))
    {
        send_to_char ("Autosplitting removed.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
    }
    else
    {
        send_to_char ("Automatic gold splitting set.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOSPLIT);
    }
}

void do_autotick (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_AUTOTICK))
    {
        send_to_char ("You will no longer see tick messages.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_AUTOTICK);
    }
    else
    {
        send_to_char ("Tick messages set.\n\r", ch);
        SET_BIT (ch->act, PLR_AUTOTICK);
    }
}


void do_autoall (CHAR_DATA *ch, char * argument)
{
    if (IS_NPC(ch))
        return;

    if (!strcmp (argument, "on"))
    {
        SET_BIT(ch->act,PLR_AUTOASSIST);
        SET_BIT(ch->act,PLR_AUTOEXIT);
        SET_BIT(ch->act,PLR_AUTOGOLD);
        SET_BIT(ch->act,PLR_AUTOLOOT);
        SET_BIT(ch->act,PLR_AUTOSAC);
        SET_BIT(ch->act,PLR_AUTOSPLIT);
        SET_BIT(ch->act,PLR_AUTOTICK);

        send_to_char("All autos turned on.\n\r",ch);
    }
    else if (!strcmp (argument, "off"))
    {
        REMOVE_BIT (ch->act, PLR_AUTOASSIST);
        REMOVE_BIT (ch->act, PLR_AUTOEXIT);
        REMOVE_BIT (ch->act, PLR_AUTOGOLD);
        REMOVE_BIT (ch->act, PLR_AUTOLOOT);
        REMOVE_BIT (ch->act, PLR_AUTOSAC);
        REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
        REMOVE_BIT (ch->act, PLR_AUTOTICK);

        send_to_char("All autos turned off.\n\r", ch);
    }
    else
        send_to_char("Usage: autoall [on|off]\n\r", ch);
}

void do_brief (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_BRIEF))
    {
        send_to_char ("Full descriptions activated.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_BRIEF);
    }
    else
    {
        send_to_char ("Short descriptions activated.\n\r", ch);
        SET_BIT (ch->comm, COMM_BRIEF);
    }
}

void do_compact (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_COMPACT))
    {
        send_to_char ("Compact mode removed.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_COMPACT);
    }
    else
    {
        send_to_char ("Compact mode set.\n\r", ch);
        SET_BIT (ch->comm, COMM_COMPACT);
    }
}

void do_long(CHAR_DATA *ch, char *argument)
{
    if (!IS_SET(ch->comm,COMM_LONG))
    {
      send_to_char("Long flags activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_LONG);
    }
    else
    {
      send_to_char("Short flags activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_LONG);
    }
}


void do_show (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
    {
        send_to_char ("Affects will no longer be shown in score.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_SHOW_AFFECTS);
    }
    else
    {
        send_to_char ("Affects will now be shown in score.\n\r", ch);
        SET_BIT (ch->comm, COMM_SHOW_AFFECTS);
    }
}

void do_display(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char buf[MSL];
    int i, x;

    const char *def_prompts[][2] = {
        { "Stock Rom",  "{c<%hhp %mm %vmv>{x " },
        { "Colorized Stock Eldoria", "{R%h{whp {M%m{wpm {G%v{wmv{c->{x "},
        { "Standard", "{R%h{whp {M%m{wpm {G%v{wmv {Ytnl: %X{c->{x "},
        { "Full Featured", "{R%h{w({R%H{w){Whitp {M%m{w({M%M{w){Wmana {G%v{w({G%V{w){Wmove {Ytnl: %X{c->%c %b %B{x "},
        { "Non Magic Featured", "{R%h{w({R%H{w){Whp {G%v{w({G%V{w){Wmv {Ytnl: %X{c->%c %b %B{x "},
        { "Full Featured Quest", "{R%h{w({R%H{w){Whitp {M%m{w({M%M{w){Wmana {G%v{w({G%V{w){Wmove {Ytnl: %X{c->%c %b %B%c %q %I %T{x "},
        { "Non Magic Quester's Delight", "{R%h{w({R%H{w){Whp {G%v{w({G%V{w){Wmv {Ytnl: %X{c->%c %b %B%c %q %I %T{x "},
        { "Immortal Basic", "[%R - %r] [%e]%c [%z] %o "},
        { "\n", "\n"}
    };

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_char("The following pre-set prompts are availible...\n\r", ch);

        for (i = 0; *def_prompts[i][0] != '\n'; i++) {
            /* skip ?Immortal Basic? if player isn?t an immortal */
            if (!IS_IMMORTAL(ch) && !str_cmp(def_prompts[i][0], "Immortal Basic"))
                continue;

            sprintf(buf,"  %d. %-25s  %s\r\n", i,def_prompts[i][0],def_prompts[i][1]);
            send_to_char(buf,ch);
        }

        send_to_char(
            "Usage: display <number>\r\n"
            "To create your own prompt, use \"prompt <str>\".\r\n", ch);
    }
    else if (!isdigit(*arg))
        send_to_char(
            "Usage: display <number>\r\n"
            "Type \"display\" without arguments for a list of preset prompts.\r\n",ch);
    else {
        i = atoi(arg);
        if (i < 0) {
            send_to_char("The number cannot be negative.\r\n", ch);
        } else {
            for (x = 0; *def_prompts[x][0] != '\n'; x++);

            if (i >= x) {
                sprintf(buf,"The range for the prompt number is 0-%d.\r\n", x);
                send_to_char(buf,ch);
            } else {
                /* also prevent non-immortals from choosing it by number */
                if (!IS_IMMORTAL(ch) && !str_cmp(def_prompts[i][0], "Immortal Basic")) {
                    send_to_char("That prompt is for Immortals only.\r\n", ch);
                    return;
                }

                if (ch->prompt)
                    free_string(ch->prompt);
                ch->prompt = str_dup(def_prompts[i][1]);
                sprintf(buf, "Set your prompt to the %s preset prompt.\r\n",def_prompts[i][0]);
                send_to_char(buf,ch);
            }
        }
    }
}

void do_prompt (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_PROMPT))
        {
            send_to_char ("You will no longer see prompts.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_PROMPT);
        }
        else
        {
            send_to_char ("You will now see prompts.\n\r", ch);
            SET_BIT (ch->comm, COMM_PROMPT);
        }
        return;
    }

    if (!strcmp (argument, "all"))
        strcpy (buf, "<%hhp %mm %vmv> ");
    else
    {
        if (strlen (argument) > 50)
            argument[50] = '\0';
        strcpy (buf, argument);
        smash_tilde (buf);
        if (str_suffix ("%c", buf))
            strcat (buf, " ");

    }

    free_string (ch->prompt);
    ch->prompt = str_dup (buf);
    sprintf (buf, "Prompt set to %s\n\r", ch->prompt);
    send_to_char (buf, ch);
    return;
}

void do_combine (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_COMBINE))
    {
        send_to_char ("Long inventory selected.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_COMBINE);
    }
    else
    {
        send_to_char ("Combined inventory selected.\n\r", ch);
        SET_BIT (ch->comm, COMM_COMBINE);
    }
}

void do_noloot (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_CANLOOT))
    {
        send_to_char ("Your corpse is now safe from thieves.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_CANLOOT);
    }
    else
    {
        send_to_char ("Your corpse may now be looted.\n\r", ch);
        SET_BIT (ch->act, PLR_CANLOOT);
    }
}

void do_nofollow (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    if (IS_SET (ch->act, PLR_NOFOLLOW))
    {
        send_to_char ("You now accept followers.\n\r", ch);
        REMOVE_BIT (ch->act, PLR_NOFOLLOW);
    }
    else
    {
        send_to_char ("You no longer accept followers.\n\r", ch);
        SET_BIT (ch->act, PLR_NOFOLLOW);
        die_follower (ch);
    }
}

void do_nosummon (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
    {
        if (IS_SET (ch->imm_flags, IMM_SUMMON))
        {
            send_to_char ("You are no longer immune to summon.\n\r", ch);
            REMOVE_BIT (ch->imm_flags, IMM_SUMMON);
        }
        else
        {
            send_to_char ("You are now immune to summoning.\n\r", ch);
            SET_BIT (ch->imm_flags, IMM_SUMMON);
        }
    }
    else
    {
        if (IS_SET (ch->act, PLR_NOSUMMON))
        {
            send_to_char ("You are no longer immune to summon.\n\r", ch);
            REMOVE_BIT (ch->act, PLR_NOSUMMON);
        }
        else
        {
            send_to_char ("You are now immune to summoning.\n\r", ch);
            SET_BIT (ch->act, PLR_NOSUMMON);
        }
    }
}

void do_look (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number, count;

    if (ch->desc == NULL)
        return;

    if (ch->position < POS_SLEEPING)
    {
        send_to_char ("You can't see anything but stars!\n\r", ch);
        return;
    }

    if (ch->position == POS_SLEEPING)
    {
        send_to_char ("You can't see anything, you're sleeping!\n\r", ch);
        return;
    }

    if (!check_blind (ch))
        return;

    /* Check for magical darkness first - only drow can see through it */
    if (!IS_NPC (ch) && !IS_SET (ch->act, PLR_HOLYLIGHT) 
        && IS_SET(ch->in_room->room_flags, ROOM_MALEDICTION))
    {
        if (ch->race != race_lookup("drow"))
        {
            send_to_char ("It is pitch black ... \n\r", ch);
            show_char_to_char (ch->in_room->people, ch);
            return;
        }
    }

    if (!IS_NPC (ch)
        && !IS_SET (ch->act, PLR_HOLYLIGHT) && room_is_dark (ch->in_room))
    {
        send_to_char ("It is pitch black ... \n\r", ch);
        show_char_to_char (ch->in_room->people, ch);
        return;
    }

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    number = number_argument (arg1, arg3);
    count = 0;

    if (arg1[0] == '\0' || !str_cmp (arg1, "auto"))
    {
        /* 'look' or 'look auto' */
        send_to_char ("{s", ch);
        send_to_char (ch->in_room->name, ch);
        send_to_char ("{x", ch);

        if ((IS_IMMORTAL (ch)
             && (IS_NPC (ch) || IS_SET (ch->act, PLR_HOLYLIGHT)))
            || IS_BUILDER (ch, ch->in_room->area))
        {
            sprintf (buf, "{r [{RRoom %d{r]{x", ch->in_room->vnum);
            send_to_char (buf, ch);
        }

        send_to_char ("\n\r", ch);

        if (arg1[0] == '\0'
            || (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
        {
            send_to_char ("  ", ch);
            send_to_char ("{S", ch);
            send_to_char (ch->in_room->description, ch);
            send_to_char ("{x", ch);
        }

        if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
        {
            send_to_char ("\n\r", ch);
            do_function (ch, &do_exits, "auto");
        }

        show_list_to_char (ch->in_room->contents, ch, FALSE, FALSE);
        show_char_to_char (ch->in_room->people, ch);
        return;
    }

    if (!str_cmp (arg1, "sky"))
    {
        if (!IS_OUTSIDE(ch))
        {
            send_to_char("You can't see the sky indoors.\n\r", ch);
            return;
        }
        else
        {
            look_sky(ch);
            return;
        }
    }

    if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in")
        || !str_cmp (arg1, "on"))
    {
        /* 'look in' */
        if (arg2[0] == '\0')
        {
            send_to_char ("Look in what?\n\r", ch);
            return;
        }

        if ((obj = get_obj_here (ch, arg2)) == NULL)
        {
            send_to_char ("You do not see that here.\n\r", ch);
            return;
        }

        switch (obj->item_type)
        {
            default:
                send_to_char ("That is not a container.\n\r", ch);
                break;

            case ITEM_DRINK_CON:
                if (obj->value[1] <= 0)
                {
                    send_to_char ("It is empty.\n\r", ch);
                    break;
                }

                sprintf (buf, "It's %sfilled with  a %s liquid.\n\r",
                         obj->value[1] < obj->value[0] / 4
                         ? "less than half-" :
                         obj->value[1] < 3 * obj->value[0] / 4
                         ? "about half-" : "more than half-",
                         liq_table[obj->value[2]].liq_color);

                send_to_char (buf, ch);
                break;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                if (IS_SET (obj->value[1], CONT_CLOSED))
                {
                    send_to_char ("It is closed.\n\r", ch);
                    break;
                }

                act ("$p holds:", ch, obj, NULL, TO_CHAR);
                show_list_to_char (obj->contains, ch, TRUE, TRUE);
                break;
        }
        return;
    }

    if ((victim = get_char_room (ch, arg1)) != NULL)
    {
        show_char_to_char_1 (victim, ch);
        return;
    }

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
        if (can_see_obj (ch, obj))
        {                        /* player can see object */
            pdesc = get_extra_descr (arg3, obj->extra_descr);
            if (pdesc != NULL)
            {
                if (++count == number)
                {
                    send_to_char (pdesc, ch);
                    return;
                }
                else
                    continue;
            }

            pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
            if (pdesc != NULL)
            {
                if (++count == number)
                {
                    send_to_char (pdesc, ch);
                    return;
                }
                else
                    continue;
            }

            if (is_name (arg3, obj->name))
                if (++count == number)
                {
                    send_to_char (obj->description, ch);
                    send_to_char ("\n\r", ch);
                    return;
                }
        }
    }

    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
    {
        if (can_see_obj (ch, obj))
        {
            pdesc = get_extra_descr (arg3, obj->extra_descr);
            if (pdesc != NULL)
                if (++count == number)
                {
                    send_to_char (pdesc, ch);
                    return;
                }

            pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
            if (pdesc != NULL)
                if (++count == number)
                {
                    send_to_char (pdesc, ch);
                    return;
                }

            if (is_name (arg3, obj->name))
                if (++count == number)
                {
                    if (obj->item_type == ITEM_WINDOW)
                    {
                        look_window(ch, obj);
                        return;
                    }
                    send_to_char (obj->description, ch);
                    send_to_char ("\n\r", ch);
                    return;
                }
        }
    }

    pdesc = get_extra_descr (arg3, ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
        if (++count == number)
        {
            send_to_char (pdesc, ch);
            return;
        }
    }

    if (count > 0 && count != number)
    {
        if (count == 1)
            sprintf (buf, "You only see one %s here.\n\r", arg3);
        else
            sprintf (buf, "You only see %d of those here.\n\r", count);

        send_to_char (buf, ch);
        return;
    }

    if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
        door = 0;
    else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
        door = 1;
    else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
        door = 2;
    else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
        door = 3;
    else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
        door = 4;
    else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
        door = 5;
    else
    {
        send_to_char ("You do not see that here.\n\r", ch);
        return;
    }

    /* 'look direction' */
    if ((pexit = ch->in_room->exit[door]) == NULL)
    {
        send_to_char ("Nothing special there.\n\r", ch);
        return;
    }

    if (pexit->description != NULL && pexit->description[0] != '\0')
        send_to_char (pexit->description, ch);
    else
        send_to_char ("Nothing special there.\n\r", ch);

    if (pexit->keyword != NULL
        && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ')
    {
        if (IS_SET (pexit->exit_info, EX_CLOSED))
        {
            act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        }
        else if (IS_SET (pexit->exit_info, EX_ISDOOR))
        {
            act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
        }
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_look, argument);
}

void do_examine (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Examine what?\n\r", ch);
        return;
    }

    do_function (ch, &do_look, arg);

    if ((obj = get_obj_here (ch, arg)) != NULL)
    {
        switch (obj->item_type)
        {
            default:
                break;

            case ITEM_JUKEBOX:
                do_function (ch, &do_play, "list");
                break;

            case ITEM_MONEY:
                if (obj->value[0] == 0)
                {
                    if (obj->value[1] == 0)
                        sprintf (buf,
                                 "Odd...there's no coins in the pile.\n\r");
                    else if (obj->value[1] == 1)
                        sprintf (buf, "Wow. One gold coin.\n\r");
                    else
                        sprintf (buf,
                                 "There are %d gold coins in the pile.\n\r",
                                 obj->value[1]);
                }
                else if (obj->value[1] == 0)
                {
                    if (obj->value[0] == 1)
                        sprintf (buf, "Wow. One silver coin.\n\r");
                    else
                        sprintf (buf,
                                 "There are %d silver coins in the pile.\n\r",
                                 obj->value[0]);
                }
                else
                    sprintf (buf,
                             "There are %d gold and %d silver coins in the pile.\n\r",
                             obj->value[1], obj->value[0]);
                send_to_char (buf, ch);
                break;

            case ITEM_DRINK_CON:
            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                sprintf (buf, "in %s", argument);
                do_function (ch, &do_look, buf);
        }
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
 void do_exits (CHAR_DATA * ch, char *argument)
 {
     extern char *const dir_name[];
     char buf[MAX_STRING_LENGTH];
     EXIT_DATA *pexit;
     bool found;
     bool fAuto;
     int door;
 
     fAuto = !str_cmp (argument, "auto");
 
     if (!check_blind (ch))
         return;
 
     if (fAuto)
         sprintf (buf, "{o[Exits:");
     else if (IS_IMMORTAL (ch))
         sprintf (buf, "Obvious exits from room %d:\n\r", ch->in_room->vnum);
     else
         sprintf (buf, "Obvious exits:\n\r");
 
     found = FALSE;
     for (door = 0; door <= 5; door++)
     {
         if ((pexit = ch->in_room->exit[door]) != NULL
             && pexit->u1.to_room != NULL
             && can_see_room (ch, pexit->u1.to_room)
             && !IS_SET (pexit->exit_info, EX_CLOSED))
         {
             found = TRUE;
             if (fAuto)
             {
                 strcat (buf, " ");
                 strcat (buf, dir_name[door]);
             }
             else
             {
                 sprintf (buf + strlen (buf), "%-5s - %s",
                          capitalize (dir_name[door]),
                          room_is_dark (pexit->u1.to_room)
                          ? "Too dark to tell" : pexit->u1.to_room->name);
                 if (IS_IMMORTAL (ch))
                     sprintf (buf + strlen (buf),
                              " (room %d)\n\r", pexit->u1.to_room->vnum);
                 else
                     sprintf (buf + strlen (buf), "\n\r");
             }
         }
     }
 
     if (!found)
         strcat (buf, fAuto ? " none" : "None.\n\r");
 
     if (fAuto)
         strcat (buf, "]{x\n\r");
 
     send_to_char (buf, ch);
     return;
 }
void do_worth (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        sprintf (buf, "You have %ld gold and %ld silver.\n\r",
                 ch->gold, ch->silver);
        send_to_char (buf, ch);
        return;
    }

    sprintf (buf,
             "You have %ld gold, %ld silver, and %d experience (%d exp to level).\n\r",
             ch->gold, ch->silver, ch->exp,
             (ch->level + 1) * exp_per_level (ch,
                                              ch->pcdata->points) - ch->exp);

    send_to_char (buf, ch);

    return;
}


void do_oldscore (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf (buf,
             "You are %s%s, level %d, %d years old (%d hours).\n\r",
             ch->name,
             IS_NPC (ch) ? "" : ch->pcdata->title,
             ch->level, get_age (ch),
             (ch->played + (int) (current_time - ch->logon)) / 3600);
    send_to_char (buf, ch);

    if (get_trust (ch) != ch->level)
    {
        sprintf (buf, "You are trusted at level %d.\n\r", get_trust (ch));
        send_to_char (buf, ch);
    }

    sprintf (buf, "Race: %s  Sex: %s  Class: %s\n\r",
             race_table[ch->race].name,
             ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
             IS_NPC (ch) ? "mobile" : class_table[ch->class].name);
    send_to_char (buf, ch);


    sprintf (buf,
             "You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move);
    send_to_char (buf, ch);

    sprintf (buf,
             "You have %d practices and %d training sessions.\n\r",
             ch->practice, ch->train);
    send_to_char (buf, ch);

    sprintf (buf,
             "You are carrying %d/%d items with weight %ld/%d pounds.\n\r",
             ch->carry_number, can_carry_n (ch),
             get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
    send_to_char (buf, ch);

    sprintf (buf,
             "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
             ch->perm_stat[STAT_STR],
             get_curr_stat (ch, STAT_STR),
             ch->perm_stat[STAT_INT],
             get_curr_stat (ch, STAT_INT),
             ch->perm_stat[STAT_WIS],
             get_curr_stat (ch, STAT_WIS),
             ch->perm_stat[STAT_DEX],
             get_curr_stat (ch, STAT_DEX),
             ch->perm_stat[STAT_CON], get_curr_stat (ch, STAT_CON));
    send_to_char (buf, ch);

    sprintf (buf,
             "You have scored %d exp, and have %ld gold and %ld silver coins.\n\r",
             ch->exp, ch->gold, ch->silver);
    send_to_char (buf, ch);

    /* RT shows exp to level */
    if (!IS_NPC (ch) && ch->level < LEVEL_HERO)
    {
        sprintf (buf,
                 "You need %d exp to level.\n\r",
                 ((ch->level + 1) * exp_per_level (ch, ch->pcdata->points) -
                  ch->exp));
        send_to_char (buf, ch);
    }

    sprintf (buf, "Wimpy set to %d hit points.\n\r", ch->wimpy);
    send_to_char (buf, ch);

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        send_to_char ("You are drunk.\n\r", ch);
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
        send_to_char ("You are thirsty.\n\r", ch);
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
        send_to_char ("You are hungry.\n\r", ch);

    switch (ch->position)
    {
        case POS_DEAD:
            send_to_char ("You are DEAD!!\n\r", ch);
            break;
        case POS_MORTAL:
            send_to_char ("You are mortally wounded.\n\r", ch);
            break;
        case POS_INCAP:
            send_to_char ("You are incapacitated.\n\r", ch);
            break;
        case POS_STUNNED:
            send_to_char ("You are stunned.\n\r", ch);
            break;
        case POS_SLEEPING:
            send_to_char ("You are sleeping.\n\r", ch);
            break;
        case POS_RESTING:
            send_to_char ("You are resting.\n\r", ch);
            break;
        case POS_SITTING:
            send_to_char ("You are sitting.\n\r", ch);
            break;
        case POS_STANDING:
            if(MOUNTED(ch))
            {
                sprintf( buf, "You are riding on %s.\n\r", MOUNTED(ch)->short_descr );
                send_to_char( buf, ch);
            }
            else
            {
                send_to_char ("You are standing.\n\r", ch);
            }
            break;
        case POS_FIGHTING:
            send_to_char ("You are fighting.\n\r", ch);
            break;
    }

    if(RIDDEN(ch))
    {
        sprintf( buf, "You are ridden by %s.\n\r",
            IS_NPC(RIDDEN(ch)) ? RIDDEN(ch)->short_descr : RIDDEN(ch)->name);
        send_to_char( buf, ch);
    }

    /* print AC values */
    if (ch->level >= 25)
    {
        sprintf (buf, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
                 GET_AC (ch, AC_PIERCE),
                 GET_AC (ch, AC_BASH),
                 GET_AC (ch, AC_SLASH), GET_AC (ch, AC_EXOTIC));
        send_to_char (buf, ch);
    }

    for (i = 0; i < 4; i++)
    {
        char *temp;

        switch (i)
        {
            case (AC_PIERCE):
                temp = "piercing";
                break;
            case (AC_BASH):
                temp = "bashing";
                break;
            case (AC_SLASH):
                temp = "slashing";
                break;
            case (AC_EXOTIC):
                temp = "magic";
                break;
            default:
                temp = "error";
                break;
        }

        send_to_char ("You are ", ch);

        if (GET_AC (ch, i) >= 101)
            sprintf (buf, "hopelessly vulnerable to %s.\n\r", temp);
        else if (GET_AC (ch, i) >= 80)
            sprintf (buf, "defenseless against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= 60)
            sprintf (buf, "barely protected from %s.\n\r", temp);
        else if (GET_AC (ch, i) >= 40)
            sprintf (buf, "slightly armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= 20)
            sprintf (buf, "somewhat armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= 0)
            sprintf (buf, "armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= -20)
            sprintf (buf, "well-armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= -40)
            sprintf (buf, "very well-armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= -60)
            sprintf (buf, "heavily armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= -80)
            sprintf (buf, "superbly armored against %s.\n\r", temp);
        else if (GET_AC (ch, i) >= -100)
            sprintf (buf, "almost invulnerable to %s.\n\r", temp);
        else
            sprintf (buf, "divinely armored against %s.\n\r", temp);

        send_to_char (buf, ch);
    }


    /* RT wizinvis and holy light */
    if (IS_IMMORTAL (ch))
    {
        send_to_char ("Holy Light: ", ch);
        if (IS_SET (ch->act, PLR_HOLYLIGHT))
            send_to_char ("on", ch);
        else
            send_to_char ("off", ch);

        if (ch->invis_level)
        {
            sprintf (buf, "  Invisible: level %d", ch->invis_level);
            send_to_char (buf, ch);
        }

        if (ch->incog_level)
        {
            sprintf (buf, "  Incognito: level %d", ch->incog_level);
            send_to_char (buf, ch);
        }
        send_to_char ("\n\r", ch);
    }

    if (ch->level >= 15)
    {
        sprintf (buf, "Hitroll: %d  Damroll: %d.\n\r",
                 GET_HITROLL (ch), GET_DAMROLL (ch));
        send_to_char (buf, ch);
    }

    if (ch->level >= 10)
    {
        sprintf (buf, "Alignment: %d.  ", ch->alignment);
        send_to_char (buf, ch);
    }

    send_to_char ("You are ", ch);
    if (ch->alignment > 900)
        send_to_char ("angelic.\n\r", ch);
    else if (ch->alignment > 700)
        send_to_char ("saintly.\n\r", ch);
    else if (ch->alignment > 350)
        send_to_char ("good.\n\r", ch);
    else if (ch->alignment > 100)
        send_to_char ("kind.\n\r", ch);
    else if (ch->alignment > -100)
        send_to_char ("neutral.\n\r", ch);
    else if (ch->alignment > -350)
        send_to_char ("mean.\n\r", ch);
    else if (ch->alignment > -700)
        send_to_char ("evil.\n\r", ch);
    else if (ch->alignment > -900)
        send_to_char ("demonic.\n\r", ch);
    else
        send_to_char ("satanic.\n\r", ch);

    if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
        do_function (ch, &do_affects, "");
}

void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char pos_str[32];
    int i;
    int total_played;
    int exp_per_lvl;
    int exp_to_level;
    char *armor_desc[4];

    total_played = (ch->played + (int) (current_time - ch->logon)) / 3600;

    if (IS_NPC(ch))
        return;

    /* Position string */
    switch (ch->position)
    {
        case POS_DEAD:      strcpy(pos_str, "Dead"); break;
        case POS_MORTAL:   strcpy(pos_str, "Mortally Wounded"); break;
        case POS_INCAP:    strcpy(pos_str, "Incapacitated"); break;
        case POS_STUNNED:  strcpy(pos_str, "Stunned"); break;
        case POS_SLEEPING: strcpy(pos_str, "Sleeping"); break;
        case POS_RESTING:  strcpy(pos_str, "Resting"); break;
        case POS_SITTING:  strcpy(pos_str, "Sitting"); break;
        case POS_FIGHTING: strcpy(pos_str, "Fighting"); break;
        case POS_STANDING: strcpy(pos_str, "Standing"); break;
        default:            strcpy(pos_str, "Unknown"); break;
    }

    /* Armor descriptions */
    for (i = 0; i < 4; i++)
    {
        if (GET_AC(ch, i) >= 101)
            armor_desc[i] = "hopelessly vulnerable";
        else if (GET_AC(ch, i) >= 80)
            armor_desc[i] = "defenseless";
        else if (GET_AC(ch, i) >= 60)
            armor_desc[i] = "barely protected";
        else if (GET_AC(ch, i) >= 40)
            armor_desc[i] = "slightly armored";
        else if (GET_AC(ch, i) >= 20)
            armor_desc[i] = "somewhat armored";
        else if (GET_AC(ch, i) >= 0)
            armor_desc[i] = "armored";
        else if (GET_AC(ch, i) >= -20)
            armor_desc[i] = "well-armored";
        else if (GET_AC(ch, i) >= -40)
            armor_desc[i] = "very well-armored";
        else if (GET_AC(ch, i) >= -60)
            armor_desc[i] = "heavily armored";
        else if (GET_AC(ch, i) >= -80)
            armor_desc[i] = "superbly armored";
        else if (GET_AC(ch, i) >= -100)
            armor_desc[i] = "almost invulnerable";
        else
            armor_desc[i] = "divinely armored";
    }

    exp_per_lvl = exp_per_level(ch, ch->pcdata->points);
    if (ch->level < LEVEL_HERO)
        exp_to_level = ((ch->level + 1) * exp_per_lvl) - ch->exp;
    else
        exp_to_level = 0;

    /* Header */
    send_to_char("**********************************************************************\n\r", ch);

    /* Line 1: Name, Level, Age, Time(hrs), Last(hrs) */
    sprintf(buf, "  Name: %8s Level: {C%3d{x Age: %3d Time(hrs): %5d Last(hrs): {C%d{x\n\r",
            ch->name, ch->level, get_age(ch), total_played, 0);
    send_to_char(buf, ch);

    /* Line 2: Race, Sex, Class */
    sprintf(buf, "  Race: %8s Sex: %6s Class: %s\n\r",
            race_table[ch->race].name,
            ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
            IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    send_to_char(buf, ch);

    /* Line 3: Clan */
    sprintf(buf, "  Clan: %s \n\r", clan_table[ch->clan].who_name);
    send_to_char(buf, ch);

    /* Line 4: CPS, Exp/level, Total Exp, Exp/next level */
    sprintf(buf, "  CPS: %3d Exp/level: %5d Total Exp: %7d Exp/next level: {C%6d{x\n\r",
            ch->pcdata->quest_curr, exp_per_lvl, ch->exp, exp_to_level);
    send_to_char(buf, ch);

    send_to_char("**********************************************************************\n\r", ch);
    if (!IS_NPC(ch))
    {
	sprintf(buf, "                Normal: Kills- %4d  Deaths-    %4d\n\r",
	ch->mkill, ch->mdeath);
	send_to_char(buf, ch);
    sprintf(buf, "                Arena: Kills- %4d  Deaths- %4d\n\r",
	ch->arenakill, ch->arenadeath);
	send_to_char(buf, ch);
	sprintf(buf, "                Pkill: Attempts- %4d  Kills- %4d  Deaths-    %4d\n\r",
	ch->pkattempt, ch->pkill, ch->pdeath);
	send_to_char(buf, ch);
    }
    /* Separator */
    send_to_char("**********************************************************************\n\r", ch);

    /* Stats block */
    sprintf(buf, "    HP: {R%5d{x/%5d Str: %2d({C%2d{x)   Pracs: %10d   Wimpy: %5d\n\r",
            ch->hit, ch->max_hit, ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
            ch->practice, ch->wimpy);
    send_to_char(buf, ch);

    sprintf(buf, "  Mana: {M%5d{x/%5d Int: %2d({C%2d{x)   Train: %10d     Hit: %5d\n\r",
            ch->mana, ch->max_mana, ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
            ch->train, GET_HITROLL(ch));
    send_to_char(buf, ch);

    sprintf(buf, "  Move: {G%5d{x/%5d Wis: %2d({C%2d{x)   Quest: %10d     Dam: %5d\n\r",
            ch->move, ch->max_move, ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
            ch->pcdata->quest_curr, GET_DAMROLL(ch));
    send_to_char(buf, ch);

    sprintf(buf, "  Gold: %10ld  Dex: %2d({C%2d{x)   Carry: {C%4d{x/%5d   Align: %6d\n\r",
            ch->gold, ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
            ch->carry_number, can_carry_n(ch), ch->alignment);
    send_to_char(buf, ch);

    sprintf(buf, "  Silv: %10ld  Con: %2d({C%2d{x)  Weight: {C%4ld{x/%5d     Pos: %s\n\r",
            ch->silver, ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
            get_carry_weight(ch) / 10, can_carry_w(ch) / 10, pos_str);
    send_to_char(buf, ch);

    /* Separator */
    send_to_char("**********************************************************************\n\r", ch);

    /* Armor line */
    sprintf(buf, "Armor: pierce: %4d bash: %4d slash: %4d magic: %4d Save: %4d\n\r",
            GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
            GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC), ch->saving_throw);
    send_to_char(buf, ch);

    /* Armor descriptions */
    sprintf(buf, "You are %s against piercing.\n\r", armor_desc[AC_PIERCE]);
    send_to_char(buf, ch);
    sprintf(buf, "You are %s against bashing.\n\r", armor_desc[AC_BASH]);
    send_to_char(buf, ch);
    sprintf(buf, "You are %s against slashing.\n\r", armor_desc[AC_SLASH]);
    send_to_char(buf, ch);
    sprintf(buf, "You are %s against magic.\n\r", armor_desc[AC_EXOTIC]);
    send_to_char(buf, ch);

    /* Separator */
    send_to_char("**********************************************************************\n\r", ch);
     /* Condition info */
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        send_to_char ("You are drunk.\n\r", ch);
    
    /* Thirst messages - graduated based on severity */
    if (!IS_NPC (ch))
    {
        int thirst = ch->pcdata->condition[COND_THIRST];
        if (thirst == 0)
            send_to_char ("{RYou are dying of thirst!{x\n\r", ch);
        else if (thirst <= 5)
            send_to_char ("{RYour throat is parched and dry.{x\n\r", ch);
        else if (thirst <= 10)
            send_to_char ("{YYou are very thirsty.{x\n\r", ch);
        else if (thirst <= 15)
            send_to_char ("{YYou are thirsty.{x\n\r", ch);
        else if (thirst <= 20)
            send_to_char ("You could use a drink.\n\r", ch);
    }
    
    /* Hunger messages - graduated based on severity */
    if (!IS_NPC (ch))
    {
        int hunger = ch->pcdata->condition[COND_HUNGER];
        if (hunger == 0)
            send_to_char ("{RYou are starving to death!{x\n\r", ch);
        else if (hunger <= 5)
            send_to_char ("{RYour stomach aches with hunger.{x\n\r", ch);
        else if (hunger <= 10)
            send_to_char ("{YYou are very hungry.{x\n\r", ch);
        else if (hunger <= 15)
            send_to_char ("{YYour stomach rumbles.{x\n\r", ch);
        else if (hunger <= 20)
            send_to_char ("You feel a bit peckish.\n\r", ch);
    }
    
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_BLEEDING] > 0)
        send_to_char ("You are {Rbleeding{x\n\r", ch);

    /* Immortal info */
    if (IS_IMMORTAL(ch))
    {
        sprintf(buf, "Holylight: %s   ", IS_SET(ch->act, PLR_HOLYLIGHT) ? "On" : "Off");
        send_to_char(buf, ch);
        
        if (ch->invis_level)
        {
            sprintf(buf, "Wizinvis: On (Lv. %2d)     ", ch->invis_level);
            send_to_char(buf, ch);
        }
        else
        {
            send_to_char("Wizinvis: Off                ", ch);
        }

        if (ch->incog_level)
        {
            sprintf(buf, "Incog: On (Lv.%2d)\n\r", ch->incog_level);
            send_to_char(buf, ch);
        }
        else
        {
            send_to_char("Incog: Off\n\r", ch);
        }

        /* Poofin/Poofout - on same line */
        if ((ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0') ||
            (ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0'))
        {
            if (ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0')
            {
                sprintf(buf, "Poofin: %s", ch->pcdata->bamfin);
                send_to_char(buf, ch);
            }
            if (ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' &&
                ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0')
            {
                send_to_char("   ", ch);
            }
            if (ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0')
            {
                sprintf(buf, "Poofout: %s", ch->pcdata->bamfout);
                send_to_char(buf, ch);
            }
            send_to_char("\n\r", ch);
        }
    }
}

void do_score2( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;

    output = new_buf();
    sprintf( buf,
	"{R                                          ________________{0\n\r");
	add_buf(output,buf);
    sprintf( buf,
	"{R ________________________________________/{x%s {G%s{x\n\r",
        ch->pcdata->pretit,
	ch->name);
    add_buf(output,buf);
    sprintf( buf, "{R| {GYou are%s{x\n\r",
	IS_NPC(ch) ? ", the mobile." : ch->pcdata->title);
    add_buf(output,buf);

    sprintf( buf,
	"{R| {xYou are level {B%d{x, and are {B%d{x years old.\n\r",
	ch->level, get_age(ch) );
    add_buf(output,buf);

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "{R| {xYou are trusted at level {B%d{x.\n\r",
	    get_trust( ch ) );
	add_buf(output,buf);
    }

    sprintf(buf, "{R| {xYou are a %s {W%s{x {G%s{x\n\r",
	ch->sex == 0 ? "{cSexless" : ch->sex == 1 ? "{bMale" : "{mFemale",
	race_table[ch->race].name,
 	IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    add_buf(output,buf);
    sprintf( buf,
	"{R| {xYou have {B%d{x practices and {B%d{x training sessions.\n\r",
	ch->practice, ch->train);
    add_buf(output,buf);
    sprintf( buf,
	"{R| {xCarrying {G%d{x/{B%d{x items at {G%ld{x/{B%d{x pounds.\n\r",
	ch->carry_number, can_carry_n(ch),
	get_carry_weight(ch) / 10, can_carry_w(ch) /10 );
    add_buf(output,buf);
    sprintf( buf,
	"{R| {r%d/{R%dHPs{x, {b%d/{B%dMana{x, {g%d/{G%dMoves{0.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move);
    add_buf(output,buf);
    if ( ch->level >= 1 )
    {
    sprintf( buf, "{R| {YHitroll: {W%d   {YDamroll: {W%d   {YSaves:  {W%d{x\n\r",GET_HITROLL(ch),GET_DAMROLL(ch),ch->saving_throw);
	add_buf(output,buf);
    }
    sprintf( buf, "{R ----------------------------------------------------------{x\n\r");
    add_buf(output,buf);
    sprintf( buf, "{R|    {wStr: {R%d{M({r%d{M){x    {R|         {D-{w={WARMOR{w={D-{0\n\r",ch->perm_stat[STAT_STR],get_curr_stat(ch,STAT_STR));
    add_buf(output,buf);
    sprintf( buf, "{R|    {wInt: {R%d{M({r%d{M){x    {R|      {cPierce:{C  %d{x\n\r",ch->perm_stat[STAT_INT],get_curr_stat(ch,STAT_INT),GET_AC(ch,AC_PIERCE));
    add_buf(output,buf);
    sprintf( buf, "{R|    {wWis: {R%d{M({r%d{M){x    {R|      {cBash:  {C  %d{x\n\r",ch->perm_stat[STAT_WIS],get_curr_stat(ch,STAT_WIS),GET_AC(ch,AC_BASH));
    add_buf(output,buf);
    sprintf( buf, "{R|    {wDex: {R%d{M({r%d{M){x    {R|      {cSlash: {C  %d{x\n\r",ch->perm_stat[STAT_DEX],get_curr_stat(ch,STAT_DEX),GET_AC(ch,AC_SLASH));
    add_buf(output,buf);
    sprintf( buf, "{R|    {wCon: {R%d{M({r%d{M){x    {R|      {cExotic:{C  %d{x\n\r",ch->perm_stat[STAT_CON],get_curr_stat(ch,STAT_CON),GET_AC(ch,AC_EXOTIC));
    add_buf(output,buf);
    sprintf( buf, "{R ----------------------------------------------------------{x\n\r");
    add_buf(output,buf);
    sprintf( buf,"{R| {xYou have {Y%ld{x gold and {w%ld{x silver coins. You have {B%d{x glory.\n\r",
        ch->gold, ch->silver, ch->pcdata->quest_curr);
    add_buf(output,buf);
    
    if (!IS_NPC(ch) && ch->level == LEVEL_HERO)
    {
	sprintf( buf,"{R| {xYou have scored {C%d exp{x.\n\r",ch->exp);
	add_buf(output,buf);
    } else if (!IS_NPC(ch) && ch->level < LEVEL_HERO) {
	sprintf( buf,"{R| {xYou have scored {C%d exp{x. You need {C%d exp{x to level.\n\r",
	    ch->exp, ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
	add_buf(output,buf);
    }

    if (ch->wimpy)
    {
	sprintf( buf, "{R| {xWimpy set to %d hit points.\n\r", ch->wimpy );
	add_buf(output,buf);
    }
 

    sprintf(buf, "{R| {xYou are ");
    add_buf(output,buf);
         if ( ch->alignment >  900 ) sprintf(buf, "{Wangelic{x.");
    else if ( ch->alignment >  700 ) sprintf(buf, "{Wsaintly{x.");
    else if ( ch->alignment >  350 ) sprintf(buf, "{wgood{x.");
    else if ( ch->alignment >  100 ) sprintf(buf, "kind.");
    else if ( ch->alignment > -100 ) sprintf(buf, "neutral.");
    else if ( ch->alignment > -350 ) sprintf(buf, "mean.");
    else if ( ch->alignment > -700 ) sprintf(buf, "{revil{x.");
    else if ( ch->alignment > -900 ) sprintf(buf, "{Rdemonic{x.");
    else                             sprintf(buf, "{Rsatanic{x.");

    add_buf(output,buf);

    if ( ch->level >= 1 )
    {
	sprintf( buf, "   {xAlignment: {B%d{x\n\r", ch->alignment );
	add_buf(output,buf);
    }



    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(ch))
    {
      sprintf(buf,"{R| {WHoly Light: {x");
      add_buf(output,buf);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
        sprintf(buf,"{YOn{x");
      else
        sprintf(buf,"{YOff{x");

      add_buf(output,buf);
 
      if (ch->invis_level)
      {
        sprintf( buf, "{W  Wizi {Y%d{x",ch->invis_level);
	add_buf(output,buf);
      }

      if (ch->incog_level)
      {
	sprintf(buf,"{W  Incog {Y%d{x",ch->incog_level);
	add_buf(output,buf);
      }
      sprintf(buf,"\n\r");
      add_buf(output,buf);
    }


    sprintf( buf, "{R ----------------------------------------------------------{x\n\r");
    add_buf(output,buf);
    page_to_char( buf_string(output), ch ); 
    free_buf(output); 

    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
	do_affects(ch,"");
}

void do_affects (CHAR_DATA * ch, char *argument)
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->affected != NULL)
    {
        send_to_char ("You are affected by the following spells:\n\r", ch);
        for (paf = ch->affected; paf != NULL; paf = paf->next)
        {
            if (paf_last != NULL && paf->type == paf_last->type)
                if (ch->level >= 20)
                    sprintf (buf, "                      ");
                else
                    continue;
            else
                sprintf (buf, "Spell: %-15s", skill_table[paf->type].name);

            send_to_char (buf, ch);

            if (ch->level >= 20)
            {
                sprintf (buf,
                         ": modifies %s by %d ",
                         affect_loc_name (paf->location), paf->modifier);
                send_to_char (buf, ch);
                if (paf->duration == -1)
                    sprintf (buf, "permanently");
                else
                    sprintf (buf, "for %d hours", paf->duration);
                send_to_char (buf, ch);
            }

            send_to_char ("\n\r", ch);
            paf_last = paf;
        }
    }
    else
        send_to_char ("You are not affected by any spells.\n\r", ch);

    return;
}



char *const day_name[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *const month_name[] = {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time (CHAR_DATA * ch, char *argument)
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day = time_info.day + 1;

    if (day > 4 && day < 20)
        suf = "th";
    else if (day % 10 == 1)
        suf = "st";
    else if (day % 10 == 2)
        suf = "nd";
    else if (day % 10 == 3)
        suf = "rd";
    else
        suf = "th";

    sprintf (buf,
             "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
             (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
             time_info.hour >= 12 ? "pm" : "am",
             day_name[day % 7], day, suf, month_name[time_info.month]);
    send_to_char (buf, ch);
    sprintf (buf, "ROM started up at %s\n\rThe system time is %s.\n\r",
             str_boot_time, (char *) ctime (&current_time));

    send_to_char (buf, ch);
    return;
}



void do_weather (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    static char *const sky_look[4] = {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };

    if (!IS_OUTSIDE (ch))
    {
        send_to_char ("You can't see the weather indoors.\n\r", ch);
        return;
    }

    sprintf (buf, "The sky is %s and %s.\n\r",
             sky_look[weather_info[ch->in_room->sector_type].sky],
             weather_info[ch->in_room->sector_type].change >= 0
             ? "a warm southerly breeze blows"
             : "a cold northern gust blows");
    send_to_char (buf, ch);
    return;
}
void do_help (CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    int level;

    output = new_buf ();

    if (argument[0] == '\0')
        argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

        if (level > get_trust (ch))
            continue;

        if (is_name (argall, pHelp->keyword))
        {
            /* add seperator if found */
            if (found)
                add_buf (output,
                         "\n\r============================================================\n\r\n\r");
            if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
            {
                add_buf (output, pHelp->keyword);
                add_buf (output, "\n\r");
            }

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if (pHelp->text[0] == '.')
                add_buf (output, pHelp->text + 1);
            else
                add_buf (output, pHelp->text);
            found = TRUE;
            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING
                && ch->desc->connected != CON_GEN_GROUPS)
                break;
        }
    }

    if (!found)
	{
        send_to_char ("No help on that word.\n\r", ch);
		/*
		 * Let's log unmet help requests so studious IMP's can improve their help files ;-)
		 * But to avoid idiots, we will check the length of the help request, and trim to
		 * a reasonable length (set it by redefining MAX_CMD_LEN in merc.h).  -- JR
		 */
		if (strlen(argall) > MAX_CMD_LEN)
		{
			argall[MAX_CMD_LEN - 1] = '\0';
			mud_logf ("Excessive command length: %s requested %s.", ch, argall);
			send_to_char ("That was rude!\n\r", ch);
		}
		/* OHELPS_FILE is the "orphaned helps" files. Defined in merc.h -- JR */
		else
		{
			append_file (ch, OHELPS_FILE, argall);
		}
	}
    else
        page_to_char (buf_string (output), ch);
    free_buf (output);
}

void do_help2 (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    int level;
    int count = 0;

    output = new_buf ();

    if (argument[0] == '\0')
        argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

        if (level > get_trust (ch))
            continue;

        if (is_name (argall, pHelp->keyword))
        {
            /* add seperator if found */
            if (found)
            {
                send_to_char("\n\r", ch);
                send_to_char("More then one help file found for what you where looking for\n\r", ch);
                send_to_char("Here is the list of them all\n\r", ch);
                send_to_char("{r=========={R=========={b=========={B=========={g=========={G=========={x\n\r\n\r", ch);
                for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
                {
                    level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;
                    
                    if (level > get_trust (ch))
                        continue;

                    if (is_name (argall, pHelp->keyword))
                    {
                        if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
                        {
                            count++;
                            sprintf (buf,
                                "{B<{G%-2d{R:{x ", count);
                            send_to_char (buf, ch);

                            send_to_char(pHelp->keyword, ch);
                            send_to_char(" {B>{x\n\r", ch);
                        }	
                    }
                }
                sprintf (buf,
                    "\n\rThere was {R%d{x keywords found for what you where looking for", count);
                send_to_char (buf, ch);
                return;
            }
            if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
            {
                add_buf (output, pHelp->keyword);
            }

            /* 
             * Strip leading '.' to allow initial blanks. 
             */ 
            if (strlen (argall) == 1) 
                ; // DJR HACK - display only keywords 
            else if (pHelp->text[0] == '.') 
                add_buf (output, pHelp->text + 1); 
            else 
                add_buf (output, pHelp->text); 
            found = TRUE; 
            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
            &&  		    ch->desc->connected != CON_GEN_GROUPS)
                break;
        }
    }

    if (!found)
    {
        send_to_char ("We are always working on new help files\n\r", ch);
        send_to_char ("So always check back for the help file you are looking for", ch);
        sprintf (log_buf, "NO HELP: %s could not find a help file for %s", ch->name, argone );
        log_string (log_buf);
    }
    else
        page_to_char(buf_string(output), ch);
    free_buf (output);
}


/* whois command */
void do_whois (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("You must provide a name.\n\r", ch);
        return;
    }

    output = new_buf ();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;
        char const *class;

        if (d->connected != CON_PLAYING || !can_see (ch, d->character))
            continue;

        wch = (d->original != NULL) ? d->original : d->character;

        if (!can_see (ch, wch))
            continue;

        if (!str_prefix (arg, wch->name))
        {
            found = TRUE;

            /* work out the printing */
            class = class_table[wch->class].who_name;
            switch (wch->level)
            {
                case MAX_LEVEL - 0:
                    class = "IMP";
                    break;
                case MAX_LEVEL - 1:
                    class = "CRE";
                    break;
                case MAX_LEVEL - 2:
                    class = "SUP";
                    break;
                case MAX_LEVEL - 3:
                    class = "DEI";
                    break;
                case MAX_LEVEL - 4:
                    class = "GOD";
                    break;
                case MAX_LEVEL - 5:
                    class = "IMM";
                    break;
                case MAX_LEVEL - 6:
                    class = "DEM";
                    break;
                case MAX_LEVEL - 7:
                    class = "ANG";
                    break;
                case MAX_LEVEL - 8:
                    class = "AVA";
                    break;
            }

            /* a little formatting */
            sprintf (buf, "[%2d %6s %s] %s%s%s%s%s%s%s%s%s\n\r",
                     wch->level,
                     wch->race <
                     MAX_PC_RACE ? pc_race_table[wch->
                                                 race].who_name : "     ",
                     class, wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
                     wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                     clan_table[wch->clan].who_name, IS_SET (wch->comm,
                                                             COMM_AFK) ?
                     "[AFK] " : "", IS_SET (wch->act,
                                            PLR_KILLER) ? "(KILLER) " : "",
                     IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "",
                     wch->pcdata->pretit, wch->name, IS_NPC (wch) ? "" : wch->pcdata->title);
            add_buf (output, buf);
        }
    }

    if (!found)
    {
        send_to_char ("No one of that name is playing.\n\r", ch);
        return;
    }

    page_to_char (buf_string (output), ch);
    free_buf (output);
}

/*
 * New 'who' command originally by Alander of Rivers of Mud,
 * Modified by Mendanbar/Kerrigan of StarCraft MUD.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char staff_buf[MSL];
	char player_buf[MSL];
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int staff = 0, players = 0;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
 
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
 
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
 
        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {
 
            /*
             * Look for classes to turn on.
             */
            if (!str_prefix(arg,"immortals"))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup(arg);
                if (iClass == -1)
                {
                    iRace = race_lookup(arg);
 
                    if (iRace == 0 || iRace >= MAX_PC_RACE)
		    {
			if (!str_prefix(arg,"clan"))
			    fClan = TRUE;
			else
		        {
			    iClan = clan_lookup(arg);
			    if (iClan)
			    {
				fClanRestrict = TRUE;
			   	rgfClan[iClan] = TRUE;
			    }
			    else
			    {
                        	send_to_char(
                            	"That's not a valid race, class, or clan.\n\r",
				   ch);
                            	return;
			    }
                        }
		    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }
 
    /*
     * Now show matching chars.
     */
 
        strcpy(staff_buf,       "Staff currently online\r\n"
                                "----------------------\r\n");
        strcpy(player_buf,      "Players currently online\r\n"
                                "------------------------\r\n");
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;
 
	if (!can_see(ch,wch))
	    continue;
 
        if ( wch->level < iLevelLower
        ||   wch->level > iLevelUpper
        || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
        || ( fClassRestrict && !rgfClass[wch->class] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
            continue;
 
	*buf = '\0';
 
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	default: break;
            {
	case MAX_LEVEL - 0 : class = "    Owner    ";     break;
	case MAX_LEVEL - 1 : class = "    Coder    ";     break;
	case MAX_LEVEL - 2 : class = " HeadBuilder ";     break;
	case MAX_LEVEL - 3 : class = " Sr. Builder ";     break;
	case MAX_LEVEL - 4 : class = "    Admin    ";     break;
	case MAX_LEVEL - 5 : class = "   Lead Qst  ";     break;
	case MAX_LEVEL - 6 : class = "   Builder   ";     break;
	case MAX_LEVEL - 7 : class = "   Questrnr  ";     break;
	case MAX_LEVEL - 8 : class = "   Trainee   ";     break;
            }
	}
 
	/*
	 * Format it up.
	 */
	*buf = '\0';
 
        if (IS_STAFF(wch)) {
	        sprintf(buf, "{y %s {W%s{x {W%s{x%s{x",
        	        class, wch->pcdata->pretit, wch->name, wch->pcdata->title);
	staff++;
    } else if (IS_REAL_HERO(wch)) {
        sprintf(buf, "{x[%3d {YHERO{x] {W%s{x {W%s{x%s{x",
            wch->level, wch->pcdata->pretit, wch->name, wch->pcdata->title);
	players++;
	} else {
		sprintf(buf, "{x[%3d  %3s] {W%s {W%s{x{W%s{x",
		    wch->level,
		    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name 
		    : "NULL",
		    IS_NPC(wch) ? "" : wch->pcdata->pretit,
		    wch->name,
		    IS_NPC(wch) ? "" : wch->pcdata->title);
	players++;
	}
	if (IS_AFFECTED(wch, AFF_INVISIBLE)) strcat(buf, " {x({winvis{x)");
	if (wch->invis_level > 0) sprintf(buf + strlen(buf), " {x({ww%d{x)",wch->invis_level);
	if (wch->incog_level > 0) sprintf(buf + strlen(buf), " {w({yi%d{w){x",wch->incog_level);
	if (IS_SET(wch->act, PLR_KILLER)) strcat(buf, " {R(KILLER){x");
	if (IS_SET(wch->act, PLR_THIEF))  strcat(buf, " {B(THIEF){x");
	if (is_clan(wch))
		sprintf(buf + strlen(buf), " {x<%s{x>",clan_table[wch->clan].who_name);
	if (IS_SET(wch->comm, COMM_AFK)) strcat(buf, " {c[AFK]{x");
		strcat(buf, "{x\r\n");
 
	if (IS_STAFF(wch))      strcat(staff_buf, buf);
	else                    strcat(player_buf, buf);
    }
 
	*buf = '\0';
        if (staff) {
                strcat(buf, staff_buf);
                strcat(buf, "\r\n");
        }
        if (players) {
                strcat(buf, player_buf);
                strcat(buf, "\r\n");
        }
 
	if ((staff + players) == 0) {
        strcat(buf, "No staff or players are currently visible to you.\r\n");
        }
        if (staff)
		sprintf(buf + strlen(buf), "There %s %d visible staff%s", (staff == 1 ? "is" : "are"), staff, players ? " and there" : ".");
        if (players)
	sprintf(buf + strlen(buf), "%s %s %d visible player%s.", staff ? "" : "There", (players == 1 ? "is" : "are"), players, (players == 1 ? "" : "s"));
        strcat(buf, "\r\n");
        if ((staff + players) > boot_high)
                boot_high = staff+players;
 
	sprintf(buf + strlen(buf), "There is a boot time high of %d player%s.\r\n", boot_high, (boot_high == 1 ? "" : "s"));
	page_to_char(buf, ch);
	return;
}
 
void do_who2( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass, iRace, iClan, iLevelLower, iLevelUpper;
    int nNumber, nMatch, immmatch, mortmatch, wlevel;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;

/*
* Initalize Variables.
*/

    immmatch = 0;
	mortmatch = 0;
    nNumber = 0;
	nMatch = 0;
    buf[0] = '\0';
    output = new_buf();


    {
	sprintf (buf, "{Y*{c========{b-----------{c========{Y*{r  {WS{wh{Da{Wd{Do{ww{Ws {CR{cebor{Cn  {Y*{c========{b-----------{c========{Y*{x\n\r");
    send_to_char (buf, ch);
    }     
    
    
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
		rgfClan[iClan] = FALSE;

    /*
     * Parse arguments.
     */
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];

        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;

        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {

            /*
             * Look for classes to turn on.
             */
            if ((!str_prefix(arg,"immortals")) || (!str_prefix(arg, "imm")))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup(arg);
                if (iClass == -1)
                {
                    iRace = race_lookup(arg);

					if (iRace == 0 || iRace >= MAX_PC_RACE)
					{
						if (!str_prefix(arg,"clan"))
							fClan = TRUE;
						else
						{
							iClan = clan_lookup(arg);
							if (iClan)
							{
								fClanRestrict = TRUE;
							   	rgfClan[iClan] = TRUE;
							}
							else
							{
		                       	send_to_char("That's not a valid race, class, or clan.\n\r", ch);
                            	return;
							}
						}
					}
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    } // for

	/*
	 * Count and output the IMMs.
	 */
	sprintf( buf, "{W ___________________{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "{W[{G I m m o r t a l s{W |{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);
	sprintf( buf, "{W[ {GLvl {RRace  {MCls{Y Sex{W ]  Immortal's Name                                    {W|{x\n\r");;
	add_buf(output,buf);
	sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);

	for( wlevel=MAX_LEVEL; wlevel>LEVEL_HERO; wlevel-- )
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
		    CHAR_DATA *wch;
	        char const *class;

	        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
		        continue;

	        wch   = ( d->original != NULL ) ? d->original : d->character;

			if( wch->level != wlevel )
		 	 	continue;

			if (!can_see(ch,wch)  || wch->level < LEVEL_IMMORTAL)
				continue;


	        if ( wch->level < iLevelLower
			    ||   wch->level > iLevelUpper
		        || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
		        || ( fClassRestrict && !rgfClass[wch->class] )
		        || ( fRaceRestrict && !rgfRace[wch->race])
				|| ( fClan && !is_clan(wch))
				|| ( fClanRestrict && !rgfClan[wch->clan]))
		        continue;

	        immmatch++;
	        nMatch++;

	        /*
	         * Figure out what to print for class.
			 */
			class = class_table[wch->class].who_name;
			switch ( wch->level )
			{
			default: break;
	        {
			case MAX_LEVEL - 0 : class = "IMP";     break;
            case MAX_LEVEL - 1 : class = "CRE";     break;
            case MAX_LEVEL - 2 : class = "SUP";     break;
            case MAX_LEVEL - 3 : class = "DEI";     break;
            case MAX_LEVEL - 4 : class = "GOD";     break;
            case MAX_LEVEL - 5 : class = "IMM";     break;
            case MAX_LEVEL - 6 : class = "DEM";     break;
            case MAX_LEVEL - 7 : class = "ANG";     break;
            case MAX_LEVEL - 8 : class = "AVA";     break;
	        }
			}

			/*
			 * Format it up.
			 */
			sprintf( buf, "{W[ {C%3d{G%6s {M%s  %s{W  ] {g %s%s%s%s%s%s%s {W%s{x%s\n\r",
		    	wch->level, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
    			: "     ", class,
    			wch->sex == 0 ? "{GN{x" : wch->sex == 1 ? "{RM{x" : "{MF{x",
				wch->incog_level >= LEVEL_HERO ? "{C({WI{W){x " : "",
				wch->invis_level >= LEVEL_HERO ? "{W({WW{W){x " : "",
				clan_table[wch->clan].who_name,
				IS_SET(wch->comm, COMM_AFK) ? "{W[{RAFK{W]{X " : "",
				IS_SET(wch->act, PLR_KILLER) ? "{W({RK{W){x " : "",
	            IS_SET(wch->act, PLR_THIEF)  ? "{W({RT{W){x "  : "",
				wch->pcdata->pretit, wch->name, IS_NPC(wch) ? "" : wch->pcdata->title );
			add_buf(output,buf);
			    sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);


		}
	}
	/*
	 * Count and output the Morts.
	 */
	sprintf( buf, "{W|{G   M o r t a l s  {W |{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);
	sprintf( buf, "{W| {GLvl{R Race  {MCls{Y Sex{W ]  Mortal's Name{x                                      {W|{x\n\r");;
	add_buf(output,buf);
	sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);

	for( wlevel=LEVEL_HERO; wlevel>0; wlevel-- )
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
	    {
	        CHAR_DATA *wch;
	        char const *class;

	        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	            continue;

	        wch   = ( d->original != NULL ) ? d->original : d->character;

			if( wch->level != wlevel )
			  	continue;

/*			if (!can_see(ch,wch) || wch->level > ch->level || wch->level > (MAX_LEVEL - 5))
			    continue; */
			if (!can_see(ch,wch))
			    continue;

			if ( wch->level < iLevelLower
		        ||   wch->level > iLevelUpper
		        || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
		        || ( fClassRestrict && !rgfClass[wch->class] )
				|| ( fRaceRestrict && !rgfRace[wch->race])
				|| ( fClan && !is_clan(wch))
				|| ( fClanRestrict && !rgfClan[wch->clan]))
			    continue;

			mortmatch++;
			nMatch++;

			/*
			 * Figure out what to print for class.
			 */
			class = class_table[wch->class].who_name;
			switch ( wch->level )
			{
			default: break;
            {
                case MAX_LEVEL - 9 : class = "{YH{GR{YO{x";     break;
            }
			}

			/*
			 * Format it up.
			 */
			sprintf( buf, "{W[ {C%3d{G%6s {M%s  %s{W  ]  %s%s%s%s%s {W%s{x%s\n\r",
		    	wch->level, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
    			: "     ", class,
    			wch->sex == 0 ? "{GN{x" : wch->sex == 1 ? "{RM{x" : "{MF{x",
				clan_table[wch->clan].who_name,
				IS_SET(wch->comm, COMM_AFK) ? "{W[{RAFK{W]{X " : "",
				IS_SET(wch->act, PLR_KILLER) ? "{W[{RK{W]{x " : "",
	            IS_SET(wch->act, PLR_THIEF)  ? "{W[{RT{W]{x "  : "",
				wch->pcdata->pretit, wch->name, IS_NPC(wch) ? "" : wch->pcdata->title );
			add_buf(output,buf);

		}
	}

    max_on = UMAX (nMatch, max_on);
    sprintf( buf, "{W[{c==================={W|{c====================================================={W]{x\n\r");;
	add_buf(output,buf);
	sprintf( buf2,
		"\n\r{GTotal connected: {W%d{G\n\r", nMatch);
	add_buf(output,buf2);
	sprintf( buf2,
		"{GImmortals found: {W%d{G\n\r", immmatch);
	add_buf(output,buf2);
	sprintf( buf2,
		"{GMortals found: {W%d\n\r", mortmatch);
	add_buf(output,buf2);
	sprintf( buf2,
		"{GMost found today: {M%d{x\n\r", max_on );
	add_buf(output,buf2);
	page_to_char( buf_string(output), ch );
	free_buf(output);
	return;

}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_oldwho (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;

    /*
     * Set default arguments.
     */
    iLevelLower = 0;
    iLevelUpper = MAX_LEVEL;
    for (iClass = 0; iClass < MAX_CLASS; iClass++)
        rgfClass[iClass] = FALSE;
    for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
        rgfClan[iClan] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for (;;)
    {
        char arg[MAX_STRING_LENGTH];

        argument = one_argument (argument, arg);
        if (arg[0] == '\0')
            break;

        if (is_number (arg))
        {
            switch (++nNumber)
            {
                case 1:
                    iLevelLower = atoi (arg);
                    break;
                case 2:
                    iLevelUpper = atoi (arg);
                    break;
                default:
                    send_to_char ("Only two level numbers allowed.\n\r", ch);
                    return;
            }
        }
        else
        {

            /*
             * Look for classes to turn on.
             */
            if (!str_prefix (arg, "immortals"))
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup (arg);
                if (iClass == -1)
                {
                    iRace = race_lookup (arg);

                    if (iRace == 0 || iRace >= MAX_PC_RACE)
                    {
                        if (!str_prefix (arg, "clan"))
                            fClan = TRUE;
                        else
                        {
                            iClan = clan_lookup (arg);
                            if (iClan)
                            {
                                fClanRestrict = TRUE;
                                rgfClan[iClan] = TRUE;
                            }
                            else
                            {
                                send_to_char
                                    ("That's not a valid race, class, or clan.\n\r",
                                     ch);
                                return;
                            }
                        }
                    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf ();
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;
        char const *class;

        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if (d->connected != CON_PLAYING || !can_see (ch, d->character))
            continue;

        wch = (d->original != NULL) ? d->original : d->character;

        if (!can_see (ch, wch))
            continue;

        if (wch->level < iLevelLower
            || wch->level > iLevelUpper
            || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
            || (fClassRestrict && !rgfClass[wch->class])
            || (fRaceRestrict && !rgfRace[wch->race])
            || (fClan && !is_clan (wch))
            || (fClanRestrict && !rgfClan[wch->clan]))
            continue;

        nMatch++;

        /*
         * Figure out what to print for class.
         */
        class = class_table[wch->class].who_name;
        switch (wch->level)
        {
            default:
                break;
                {
            case MAX_LEVEL - 0:
                    class = "IMP";
                    break;
            case MAX_LEVEL - 1:
                    class = "CRE";
                    break;
            case MAX_LEVEL - 2:
                    class = "SUP";
                    break;
            case MAX_LEVEL - 3:
                    class = "DEI";
                    break;
            case MAX_LEVEL - 4:
                    class = "GOD";
                    break;
            case MAX_LEVEL - 5:
                    class = "IMM";
                    break;
            case MAX_LEVEL - 6:
                    class = "DEM";
                    break;
            case MAX_LEVEL - 7:
                    class = "ANG";
                    break;
            case MAX_LEVEL - 8:
                    class = "AVA";
                    break;
                }
        }

        /*
         * Format it up.
         */
        sprintf (buf, "[%2d %6s %s] %s%s%s%s%s%s%s%s%s\n\r",
                 wch->level,
                 wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
                 : "     ",
                 class,
                 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
                 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                 clan_table[wch->clan].who_name,
                 IS_SET (wch->comm, COMM_AFK) ? "[AFK] " : "",
                 IS_SET (wch->act, PLR_KILLER) ? "(KILLER) " : "",
                 IS_SET (wch->act, PLR_THIEF) ? "(THIEF) " : "",
                 wch->pcdata->pretit, wch->name, IS_NPC (wch) ? "" : wch->pcdata->title);
        add_buf (output, buf);
    }

    sprintf (buf2, "\n\rPlayers found: %d\n\r", nMatch);
    add_buf (output, buf2);
    page_to_char (buf_string (output), ch);
    free_buf (output);
    return;
}

void do_count (CHAR_DATA * ch, char *argument)
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for (d = descriptor_list; d != NULL; d = d->next)
        if (d->connected == CON_PLAYING && can_see (ch, d->character))
            count++;

    max_on = UMAX (count, max_on);

    if (max_on == count)
        sprintf (buf,
                 "There are %d characters on, the most so far today.\n\r",
                 count);
    else
        sprintf (buf,
                 "There are %d characters on, the most on today was %d.\n\r",
                 count, max_on);

    send_to_char (buf, ch);
}

void do_inventory (CHAR_DATA * ch, char *argument)
{
    send_to_char ("You are carrying:\n\r", ch);
    show_list_to_char (ch->carrying, ch, TRUE, TRUE);
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];

    send_to_char("You are using:\n\r", ch);

    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
        obj = get_eq_char(ch, iWear);

        if (obj == NULL)
        {
            snprintf(buf, sizeof(buf), "%s     ---\r\n", where_name[iWear]);
            send_to_char(buf, ch);
            continue;
        }

        if (can_see_obj(ch, obj))
        {
            int max_cond = get_obj_max_condition(obj);
            int percent;
            char gauge[11];
            char gauge_buf[32];
            const char *color;
            int i;
            char *descr = format_obj_to_char(obj, ch, TRUE);

            if (max_cond <= 0)
                max_cond = 100;

            percent = (obj->condition * 100) / max_cond;
            percent = URANGE(0, percent, 100);

            for (i = 0; i < 10; i++)
                gauge[i] = (i * 10 < percent) ? '#' : '.';
            gauge[10] = '\0';

            if (percent >= 70)
                color = "{G";
            else if (percent >= 35)
                color = "{Y";
            else
                color = "{R";

            snprintf(gauge_buf, sizeof(gauge_buf), "%s[%s]{x", color, gauge);

            snprintf(buf, sizeof(buf), "%s %-40s %s (%3d%%)\r\n",
                     where_name[iWear], descr, gauge_buf, percent);
            send_to_char(buf, ch);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%s something.\r\n", where_name[iWear]);
            send_to_char(buf, ch);
        }

        found = TRUE;
    }
}


void do_equipment2 (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    int iWear;
    bool found = FALSE;

    send_to_char ("{wYou are using:{x\n\r", ch);
    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
        if ((obj = get_eq_char (ch, iWear)) == NULL)
        {
            send_to_char("{w", ch);
            send_to_char(where_name[iWear], ch);
            send_to_char("     ---\r\n", ch);
            continue;
        }
        send_to_char("{Y", ch);
        send_to_char (where_name[iWear], ch);
        send_to_char("{W", ch);
        if (can_see_obj (ch, obj))
        {
            send_to_char("{W", ch);
            send_to_char (format_obj_to_char (obj, ch, TRUE), ch);
            send_to_char ("{x\n\r", ch);
        }
        else
        {
            send_to_char ("{Wsomething.\n\r{x", ch);
        }
        found = TRUE;
    }

    if (!found)
        send_to_char("You are not using anything.\n\r", ch);

    send_to_char("{x", ch);

    return;
}



void do_compare (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    if (arg1[0] == '\0')
    {
        send_to_char ("Compare what to what?\n\r", ch);
        return;
    }

    if ((obj1 = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
        {
            if (obj2->wear_loc != WEAR_NONE && can_see_obj (ch, obj2)
                && obj1->item_type == obj2->item_type
                && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
                break;
        }

        if (obj2 == NULL)
        {
            send_to_char ("You aren't wearing anything comparable.\n\r", ch);
            return;
        }
    }

    else if ((obj2 = get_obj_carry (ch, arg2, ch)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2)
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if (obj1->item_type != obj2->item_type)
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch (obj1->item_type)
        {
            default:
                msg = "You can't compare $p and $P.";
                break;

            case ITEM_ARMOR:
                value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
                value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
                break;

            case ITEM_WEAPON:
                if (obj1->pIndexData->new_format)
                    value1 = (1 + obj1->value[2]) * obj1->value[1];
                else
                    value1 = obj1->value[1] + obj1->value[2];

                if (obj2->pIndexData->new_format)
                    value2 = (1 + obj2->value[2]) * obj2->value[1];
                else
                    value2 = obj2->value[1] + obj2->value[2];
                break;
        }
    }

    if (msg == NULL)
    {
        if (value1 == value2)
            msg = "$p and $P look about the same.";
        else if (value1 > value2)
            msg = "$p looks better than $P.";
        else
            msg = "$p looks worse than $P.";
    }

    act (msg, ch, obj1, obj2, TO_CHAR);
    return;
}



void do_credits (CHAR_DATA * ch, char *argument)
{
    do_function (ch, &do_help, "diku");
    return;
}



void do_where (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Players near you:\n\r", ch);
        found = FALSE;
        for (d = descriptor_list; d; d = d->next)
        {
            if (d->connected == CON_PLAYING
                && (victim = d->character) != NULL && !IS_NPC (victim)
                && victim->in_room != NULL
                && !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
                && (is_room_owner (ch, victim->in_room)
                    || !room_is_private (victim->in_room))
                && victim->in_room->area == ch->in_room->area
                && can_see (ch, victim))
            {
                found = TRUE;
                sprintf (buf, "%-28s %s\n\r",
                         victim->name, victim->in_room->name);
                send_to_char (buf, ch);
            }
        }
        if (!found)
            send_to_char ("None\n\r", ch);
    }
    else
    {
        found = FALSE;
        for (victim = char_list; victim != NULL; victim = victim->next)
        {
            if (victim->in_room != NULL
                && victim->in_room->area == ch->in_room->area
                && !IS_AFFECTED (victim, AFF_HIDE)
                && !IS_AFFECTED (victim, AFF_SNEAK)
                && can_see (ch, victim) && is_name (arg, victim->name))
            {
                found = TRUE;
                sprintf (buf, "%-28s %s\n\r",
                         PERS (victim, ch), victim->in_room->name);
                send_to_char (buf, ch);
                break;
            }
        }
        if (!found)
            act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
    }

    return;
}




void do_consider (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Consider killing whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They're not here.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
    {
        send_to_char ("Don't even think about it.\n\r", ch);
        return;
    }

    diff = victim->level - ch->level;

    if (diff <= -10)
        msg = "You can kill $N naked and weaponless.";
    else if (diff <= -5)
        msg = "$N is no match for you.";
    else if (diff <= -2)
        msg = "$N looks like an easy kill.";
    else if (diff <= 1)
        msg = "The perfect match!";
    else if (diff <= 4)
        msg = "$N says 'Do you feel lucky, punk?'.";
    else if (diff <= 9)
        msg = "$N laughs at you mercilessly.";
    else
        msg = "Death will thank you for your gift.";

    act (msg, ch, NULL, victim, TO_CHAR);
    return;
}



void set_title (CHAR_DATA * ch, char *title)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        bug ("Set_title: NPC.", 0);
        return;
    }

    if (title[0] != '.' && title[0] != ',' && title[0] != '!'
        && title[0] != '?')
    {
        buf[0] = ' ';
        strcpy (buf + 1, title);
    }
    else
    {
        strcpy (buf, title);
    }

    free_string (ch->pcdata->title);
    ch->pcdata->title = str_dup (buf);
    return;
}



void do_title (CHAR_DATA * ch, char *argument)
{
    int i;

    if (IS_NPC (ch))
        return;

	/* Changed this around a bit to do some sanitization first   *
	 * before checking length of the title. Need to come up with *
	 * a centralized user input sanitization scheme. FIXME!      *
	 * JR -- 10/15/00                                            */

    if (strlen (argument) > 45)
        argument[45] = '\0';

    i = strlen(argument);
    if (argument[i-1] == '{' && argument[i-2] != '{')
		argument[i-1] = '\0';

    if (argument[0] == '\0')
    {
        send_to_char ("Change your title to what?\n\r", ch);
        return;
    }

    smash_tilde (argument);
    set_title (ch, argument);
    send_to_char ("Ok.\n\r", ch);
}

void do_pretitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int value;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ch->pcdata->pretit == NULL || ch->pcdata->pretit[0] == '\0' )
        ch->pcdata->pretit = str_dup("{x");

    if ( argument[0] == '\0' )
    {
        sprintf(buf, "Your current pretitle is '%s'.\n\r",ch->pcdata->pretit);
        send_to_char(buf,ch);
        return;
    }

    if ( strlen(argument) > 45 )
    {
        argument[45] = '{';
        argument[46] = 'x';
        argument[47] = '\0';
    }
    else
    {
        value = strlen(argument);
        argument[value] = '{';
        argument[value+1] = 'x';
        argument[value+2] = '\0';
    }

    if (ch->pcdata->pretit != NULL)
        free_string(ch->pcdata->pretit);
    ch->pcdata->pretit = str_dup( argument );
    send_to_char("Done.\n\r",ch);
    return;
}

void do_description (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    /* No argument or "edit" - enter string editor */
    if (argument[0] == '\0')
    {
        string_append (ch, &ch->description);
        return;
    }

    one_argument(argument, arg);

    if (!str_cmp(arg, "edit") || !str_cmp(arg, "write"))
    {
        string_append (ch, &ch->description);
        return;
    }

    if (!str_cmp(arg, "clear"))
    {
        free_string (ch->description);
        ch->description = str_dup ("");
        send_to_char ("Description cleared.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "show"))
    {
        send_to_char ("Your description is:\n\r", ch);
        send_to_char (ch->description ? ch->description : "(None).\n\r", ch);
        return;
    }

    /* Old-style line editing still available for backwards compatibility */
    if (argument[0] != '\0')
    {
        buf[0] = '\0';
        smash_tilde (argument);

        if (argument[0] == '-')
        {
            int len;
            bool found = FALSE;

            if (ch->description == NULL || ch->description[0] == '\0')
            {
                send_to_char ("No lines left to remove.\n\r", ch);
                return;
            }

            strcpy (buf, ch->description);

            for (len = strlen (buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)
                    {            /* back it up */
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else
                    {            /* found the second one */

                        buf[len + 1] = '\0';
                        free_string (ch->description);
                        ch->description = str_dup (buf);
                        send_to_char ("Your description is:\n\r", ch);
                        send_to_char (ch->description ? ch->description :
                                      "(None).\n\r", ch);
                        return;
                    }
                }
            }
            buf[0] = '\0';
            free_string (ch->description);
            ch->description = str_dup (buf);
            send_to_char ("Description cleared.\n\r", ch);
            return;
        }
        if (argument[0] == '+')
        {
            if (ch->description != NULL)
                strcat (buf, ch->description);
            argument++;
            while (isspace (*argument))
                argument++;
        }

        if (strlen (buf) >= 1024)
        {
            send_to_char ("Description too long.\n\r", ch);
            return;
        }

        strcat (buf, argument);
        strcat (buf, "\n\r");
        free_string (ch->description);
        ch->description = str_dup (buf);
    }

    send_to_char ("Your description is:\n\r", ch);
    send_to_char (ch->description ? ch->description : "(None).\n\r", ch);
    return;
}



void do_report (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    sprintf (buf,
             "You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    send_to_char (buf, ch);

    sprintf (buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    act (buf, ch, NULL, NULL, TO_ROOM);

    return;
}



void do_practice (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if (IS_NPC (ch))
        return;

    if (argument[0] == '\0')
    {
        int col;

        col = 0;
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
            if (ch->level < skill_table[sn].skill_level[ch->class]
                || ch->pcdata->learned[sn] < 1 /* skill is not known */ )
                continue;

            sprintf (buf, "%-18s %3d%%  ",
                     skill_table[sn].name, ch->pcdata->learned[sn]);
            send_to_char (buf, ch);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }

        if (col % 3 != 0)
            send_to_char ("\n\r", ch);

        sprintf (buf, "You have %d practice sessions left.\n\r",
                 ch->practice);
        send_to_char (buf, ch);
    }
    else
    {
        CHAR_DATA *mob;
        int adept;

        if (!IS_AWAKE (ch))
        {
            send_to_char ("In your dreams, or what?\n\r", ch);
            return;
        }

        for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
        {
            if (IS_NPC (mob) && IS_SET (mob->act, ACT_PRACTICE))
                break;
        }

        if (mob == NULL)
        {
            send_to_char ("You can't do that here.\n\r", ch);
            return;
        }

        if (ch->practice <= 0)
        {
            send_to_char ("You have no practice sessions left.\n\r", ch);
            return;
        }

        if ((sn = find_spell (ch, argument)) < 0 || (!IS_NPC (ch)
                                                     && (ch->
                                                         pcdata->learned[sn] < 1    /* skill is not known */
                                                         || (ch->level <
                                                             skill_table
                                                             [sn].skill_level
                                                             [ch->class]
                                                             && skill_table
                                                             [sn].skill_level
                                                             [ch->class] < LEVEL_HERO + 1)
                                                         || (skill_table
                                                             [sn].rating[ch->
                                                                         class] ==
                                                             0
                                                             && ch->
                                                             pcdata->learned[sn] < 1))))
        {
            send_to_char ("You can't practice that.\n\r", ch);
            return;
        }

        adept = IS_NPC (ch) ? 100 : class_table[ch->class].skill_adept;

        if (ch->pcdata->learned[sn] >= adept)
        {
            sprintf (buf, "You are already learned at %s.\n\r",
                     skill_table[sn].name);
            send_to_char (buf, ch);
        }
        else
        {
            int rating = skill_table[sn].rating[ch->class];
            
            /* Safety check: if rating is 0 (uninitialized for orc classes), use base class rating */
            if (rating == 0 && ch->class >= 4 && ch->class <= 7)
            {
                /* Map orc classes to base human classes for rating */
                int base_class = ch->class - 4; /* sorcerer->mage, shaman->cleric, knave->thief, berserker->warrior */
                rating = skill_table[sn].rating[base_class];
            }
            
            /* Final safety check to prevent division by zero */
            if (rating <= 0)
            {
                send_to_char ("You can't practice that skill.\n\r", ch);
                return;
            }
            
            ch->practice--;
            ch->pcdata->learned[sn] +=
                int_app[get_curr_stat (ch, STAT_INT)].learn / rating;
            if (ch->pcdata->learned[sn] < adept)
            {
                act ("You practice $T.",
                     ch, NULL, skill_table[sn].name, TO_CHAR);
                act ("$n practices $T.",
                     ch, NULL, skill_table[sn].name, TO_ROOM);
            }
            else
            {
                ch->pcdata->learned[sn] = adept;
                act ("You are now learned at $T.",
                     ch, NULL, skill_table[sn].name, TO_CHAR);
                act ("$n is now learned at $T.",
                     ch, NULL, skill_table[sn].name, TO_ROOM);
            }
        }
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument (argument, arg);

    if (arg[0] == '\0')
        wimpy = ch->max_hit / 5;
    else
        wimpy = atoi (arg);

    if (wimpy < 0)
    {
        send_to_char ("Your courage exceeds your wisdom.\n\r", ch);
        return;
    }

    if (wimpy > ch->max_hit / 2)
    {
        send_to_char ("Such cowardice ill becomes you.\n\r", ch);
        return;
    }

    ch->wimpy = wimpy;
    sprintf (buf, "Wimpy set to %d hit points.\n\r", wimpy);
    send_to_char (buf, ch);
    return;
}



void do_password (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if (IS_NPC (ch))
        return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0')
    {
        if (*argument == cEnd)
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while (isspace (*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0')
    {
        if (*argument == cEnd)
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char ("Syntax: password <old> <new>.\n\r", ch);
        return;
    }

    if (strcmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd))
    {
        WAIT_STATE (ch, 40);
        send_to_char ("Wrong password.  Wait 10 seconds.\n\r", ch);
        return;
    }

    if (strlen (arg2) < 5)
    {
        send_to_char
            ("New password must be at least five characters long.\n\r", ch);
        return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt (arg2, ch->name);
    for (p = pwdnew; *p != '\0'; p++)
    {
        if (*p == '~')
        {
            send_to_char ("New password not acceptable, try again.\n\r", ch);
            return;
        }
    }

    free_string (ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup (pwdnew);
    save_char_obj (ch);
    send_to_char ("Ok.\n\r", ch);
    return;
}

void do_telnetga (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->comm, COMM_TELNET_GA))
	{
		send_to_char ("Telnet GA removed.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_TELNET_GA);
	}
	else
	{
		send_to_char ("Telnet GA enabled.\n\r", ch);
		SET_BIT (ch->comm, COMM_TELNET_GA);
	}
}


void do_showclass(CHAR_DATA *ch, char *argument) 
{ 
 char arg1[MAX_INPUT_LENGTH]; 
 char buf[MAX_STRING_LENGTH]; 
   
 int class,level,skill; 
    
 int i; 
     
 strcpy(buf,""); 
 argument = one_argument(argument, arg1); 
     
 if(arg1[0]=='\0') 
 { 
  send_to_char("Syntax: showclass [class]\n\r",ch); 
  return; 
 } 
 if((class=class_lookup(arg1))==-1) 
 {  
  send_to_char("Class not found.\n\r",ch); 
  return; 
 } 
 
 sprintf(buf,"{xSpells/skills for %s:{x\n\r",class_table[class].name); 
 send_to_char(buf,ch); 
 i=0; 
 for(level=1;level<=LEVEL_HERO;level++) 
 { 
  for(skill=0;skill<MAX_SKILL;skill++) 
  { 
   if(skill_table[skill].skill_level[class]!=level) 
    continue; 
   i++; 
   sprintf(buf,"{CLevel %3d: {x: {c%-20s{x  ", 
    level,skill_table[skill].name); 
   send_to_char(buf,ch); 
   if(i==2) 
   { 
    send_to_char("\n\r",ch); 
    i=0; 
   } 
  }  
 }   
} 

void do_lore( CHAR_DATA *ch, char *argument )
{ 
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_lore].skill_level[ch->class] )
    {
        send_to_char(
            "You would like to what?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What Would you like to know more about?\n\r", ch );
        return;
    }

        if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
        {
            send_to_char( "You cannot lore that because you do not have that.\n\r", ch );
            return;
        }


      if ( number_percent( ) < get_skill(ch,gsn_lore))
       {
        send_to_char( "You learn more about this object:\n\r", ch );
        check_improve(ch,gsn_lore,TRUE,4);

    sprintf( buf, "Name(s): %s\n\r",
        obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
        obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
        wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Weight: %d (10th pounds)\n\r",
        obj->weight / 10);
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d\n\r",
        obj->level, obj->cost);
    send_to_char( buf, ch );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf( buf, "Level %d spells of:", obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[1]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[2]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
            {
                send_to_char(" '",ch);
                send_to_char(skill_table[obj->value[4]].name,ch);
                send_to_char("'",ch);
            }
            break;

        case ITEM_MANUAL:
            sprintf( buf, "Level %d skills of:", obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[1]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[2]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
            {
                send_to_char(" '",ch);
                send_to_char(skill_table[obj->value[4]].name,ch);
                send_to_char("'",ch);
            }
            send_to_char( ".\n\r", ch );
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf( buf, "Has %d(%d) charges of level %d",
                obj->value[1], obj->value[2], obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            send_to_char( ".\n\r", ch );
        break;

        case ITEM_DRINK_CON:
            sprintf(buf,"It holds %s-colored %s.\n\r",
                liq_table[obj->value[2]].liq_color,
                liq_table[obj->value[2]].liq_name);
            send_to_char(buf,ch);
            break;


        case ITEM_WEAPON:
            send_to_char("Weapon type is ",ch);
            switch (obj->value[0])
            {
                case(WEAPON_EXOTIC):
                    send_to_char("exotic\n\r",ch);
                    break;
                case(WEAPON_SWORD):
                    send_to_char("sword\n\r",ch);
                    break;
                case(WEAPON_DAGGER):
                    send_to_char("dagger\n\r",ch);
                    break;
                case(WEAPON_SPEAR):
                    send_to_char("spear/staff\n\r",ch);
                    break;
                case(WEAPON_MACE):
                    send_to_char("mace/club\n\r",ch);
                    break;
                case(WEAPON_AXE):
                    send_to_char("axe\n\r",ch);
                    break;
                case(WEAPON_FLAIL):
                    send_to_char("flail\n\r",ch);
                    break;
                case(WEAPON_WHIP):
                    send_to_char("whip\n\r",ch);
                    break;
                case(WEAPON_POLEARM):
                    send_to_char("polearm\n\r",ch);
                    break;
                default:
                    send_to_char("unknown\n\r",ch);
                    break;
            }
            if (obj->pIndexData->new_format)
                sprintf(buf,"Damage is %dd%d (average %d)\n\r",
                    obj->value[1],obj->value[2],
                    (1 + obj->value[2]) * obj->value[1] / 2);
            else
                sprintf( buf, "Damage is %d to %d (average %d)\n\r",
                    obj->value[1], obj->value[2],
                    ( obj->value[1] + obj->value[2] ) / 2 );
            send_to_char( buf, ch );

            sprintf(buf,"Damage noun is %s.\n\r",
                (obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
                    attack_table[obj->value[3]].noun : "undefined");
            send_to_char(buf,ch);

            if (obj->value[4])  /* weapon flags */
            {
                sprintf(buf,"Weapons flags: %s\n\r",
                    weapon_bit_name(obj->value[4]));
                send_to_char(buf,ch);
            }
        break;

        case ITEM_ARMOR:
            sprintf( buf,
            "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
            send_to_char( buf, ch );
        break;

        case ITEM_CONTAINER:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
                    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );

        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d",
            affect_loc_name( paf->location ), paf->modifier,paf->level );
        send_to_char(buf,ch);
        if ( paf->duration > -1)
            sprintf(buf,", %d hours.\n\r",paf->duration);
        else
            sprintf(buf,".\n\r");
        send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d.\n\r",
            affect_loc_name( paf->location ), paf->modifier,paf->level );
        send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }
  }
    return;
}

/*
 * Allows PC to make and store a history.
 * by TAKA
 * (c) 1999 TAKA of the Ghost Dancer MUD Project
 */
void do_history( CHAR_DATA *ch, char *argument )
{
    char buf[MSL], arg[MIL];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;

            if (ch->pcdata->history == NULL || ch->pcdata->history[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }

  			strcpy(buf,ch->pcdata->history);

            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
						free_string(ch->pcdata->history);
						ch->pcdata->history = str_dup(buf);
						send_to_char( "Your history is:\n\r", ch );
						page_to_char( ch->pcdata->history ? ch->pcdata->history :
						    "(None).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
			free_string(ch->pcdata->history);
			ch->pcdata->history = str_dup(buf);
			send_to_char("{RHistory cleared.{x\n\r",ch);
			return;
        }

		if ( argument[0] == '+' )
		{
		    if ( ch->pcdata->history != NULL )
				strcat( buf, ch->pcdata->history );

			argument++;

			while ( isspace(*argument) )
				argument++;
		}

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( !str_cmp( arg, "write") )
	{
		if ( argument[0] == '\0' )
	            string_append( ch, &ch->pcdata->history );
	    return;
	}

    if ( !str_cmp( arg, "clear") )
    {
        free_string(ch->pcdata->history);
        ch->pcdata->history = str_dup("");
        send_to_char("{RHistory cleared.{x\n\r",ch);
        return;
    }

    if ( !str_cmp( arg, "show") )
    {
        send_to_char( "Your history is:\n\r", ch );
        page_to_char( ch->pcdata->history ? ch->pcdata->history : "(None).\n\r", ch );
        return;
    }

    /* If arg has content and it's not a recognized command, show history instead of appending */
    /* This prevents "history <charactername>" from accidentally appending to your history */
    if ( arg[0] != '\0' && argument[0] == '\0' )
    {
        send_to_char( "Your history is:\n\r", ch );
        page_to_char( ch->pcdata->history ? ch->pcdata->history : "(None).\n\r", ch );
        send_to_char( "\n\rValid commands: history [+text], history [-], history write, history clear, history show\n\r", ch );
        send_to_char( "Did you mean 'lookhistory {c<name>{x'?\n\r", ch );
        return;
    }

		if ( strlen(buf) >= 4096)
		{
			send_to_char( "History too long.\n\r", ch );
			return;
		}

		strcat( buf, argument );
		strcat( buf, "\n\r" );
		free_string( ch->pcdata->history );
		ch->pcdata->history = str_dup( buf );
	}

    send_to_char( "Your history is:\n\r", ch );
    page_to_char( ch->pcdata->history ? ch->pcdata->history : "(None).\n\r", ch );
    return;
}

void do_lookhist( CHAR_DATA *ch, char *argument )
{
  char arg1[MIL];
  CHAR_DATA *victim;

  argument = one_argument (argument, arg1);

  if (arg1[0] == '\0')
  {
      send_to_char ("{rWhos history do you want to read?{x\n\r", ch);
      return;
  }

  if ((victim = get_char_world (ch, arg1)) == NULL)
  {
      send_to_char ("{rThey aren't here.{x\n\r", ch);
      return;
  }

    send_to_char( "Their history is:\n\r", ch );
    page_to_char( victim->pcdata->history ? victim->pcdata->history : "(None).\n\r", ch );
    return;

}

/*
 * Window code by Voltec - allows creating "optical gateways" to other parts of the world
 */
void look_window( CHAR_DATA *ch, OBJ_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *window_room;   

    if ( obj->value[0] == 0 )
    {
        sprintf(buf, "%s\n\r", obj->description );
        send_to_char(buf, ch);
        return;
    }

    window_room = get_room_index( obj->value[0] );

    if ( window_room == NULL )
    {
        send_to_char( "!!BUG!! Window looks into a NULL room! Please report!\n\r", ch );
        bug( "Window %d looks into a null room!!!", obj->pIndexData->vnum );
        return;
    }
		 
    if ( !IS_NPC(ch) )
    {
        send_to_char( "Looking through the window you can see ",ch);
        send_to_char( window_room->name, ch );
        send_to_char( "\n\r", ch);
        show_list_to_char( window_room->contents, ch, FALSE, FALSE );
        show_char_to_char( window_room->people,   ch );
        return;
    }
}

void do_bandage( CHAR_DATA *ch, char *argument)
{    
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;    
    int chance;
    int skill;

    one_argument(argument,arg);

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if ( victim->position == POS_DEAD )
    {
        send_to_char( "You can't bandage a corpse.\n\r", ch );
        return;
    }
     
    if (victim->hit == victim->max_hit)
    {
	send_to_char("They don't need to be bandaged.\n\r",ch);
	return;
    }

 if (ch->move < 200)
    {
	send_to_char("{hYou can't get up enough energy.{x\n\r",ch);
	return;
    }
    skill = get_skill( ch, gsn_bandage );
    if ( skill <= 0 )
    {
        send_to_char( "You don't know how to bandage.\n\r", ch );
        return;
    }

    chance = number_range( 1, 25);
    if (chance == 20)
    {
	send_to_char("You failed.\n\r",ch);
	check_improve(ch,gsn_bandage,FALSE,2);
	return;
    }

    if ( number_percent( ) > skill )
    {
        act( "You fail to bandage $N properly.", ch, NULL, victim, TO_CHAR );
        act( "$n tries to bandage you but fails.", ch, NULL, victim, TO_VICT );
        act( "$n tries to bandage $N but fails.", ch, NULL, victim, TO_NOTVICT );
        check_improve( ch, gsn_bandage, FALSE, 3 );
        return;
    }
    if (ch == victim)
    {
    act("$n kneels down and begins to bandage themselves.\n\r",ch,NULL,victim,TO_NOTVICT);
    act("You apply bandages to yourself.\n\r",ch,NULL,victim,TO_CHAR);
    victim->hit = (victim->hit += 200);
    ch->move = (ch->move -=200);
    /* Clear bleeding when bandaging */
    if (!IS_NPC(victim))
        victim->pcdata->condition[COND_BLEEDING] = 0;
    check_improve(ch,gsn_bandage,FALSE,2);
    }else{
    act("$n kneels down and begins to bandage $N.\n\r",ch,NULL,victim,TO_NOTVICT);
    act("$n kneels down and touches bandages to your wounds.\n\r",ch,NULL,victim,TO_VICT);
    act("You apply bandages to $N's wounds.\n\r",ch,NULL,victim,TO_CHAR);
    victim->hit = (victim->hit += 200);
     ch->move = (ch->move -=200);
    /* Clear bleeding when bandaging */
    if (!IS_NPC(victim))
        victim->pcdata->condition[COND_BLEEDING] = 0;
    check_improve(ch,gsn_bandage,FALSE,2);
    return;
}
}

void do_recent( CHAR_DATA *ch, char *argument )
{
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

        for( r = recent_free->next; r != recent_free && r != NULL; r = r->next ) {
            if( r->used ) {
                timeString( r->logOn , lo );
                timeString( r->logOff, lf );
                sprintf( buf, " %-15s | %-15s | %-15s\n\r", r->name, lo, lf );
                send_to_char( buf, ch );
            }
        }
    }
    return;
}

void do_mudinfo( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int current_players = 0;
    float average_players = 0.0;
    float pulse_miss_percent = 0.0;
    float cpu_lag_percent = 0.0;
    float bytes_per_sec_read = 0.0;
    float bytes_per_sec_written = 0.0;
    time_t uptime_seconds;
    int days, hours, minutes, seconds;
    
    one_argument(argument, arg);
    
    /* Reset command - immortals only */
    if (arg[0] != '\0' && !str_cmp(arg, "reset"))
    {
        if (ch->level < LEVEL_IMMORTAL)
        {
            send_to_char("Only immortals can reset mudinfo statistics.\n\r", ch);
            return;
        }
        
        total_logins = 0;
        total_pulses = 0;
        missed_pulses = 0;
        peak_players = 0;
        total_player_seconds = 0;
        bytes_read_total = 0;
        bytes_written_total = 0;
        color_bytes_saved = 0;
        cpu_lag_usec = 0;
        cpu_measurements = 0;
        boot_time = current_time;
        
        send_to_char("MUD statistics have been reset.\n\r", ch);
        sprintf(buf, "%s has reset the mudinfo statistics.", ch->name);
        log_string(buf);
        return;
    }
    
    /* Count current players */
    for (d = descriptor_list; d; d = d->next)
    {
        if (d->connected == CON_PLAYING)
            current_players++;
    }
    
    /* Calculate uptime */
    uptime_seconds = current_time - boot_time;
    if (uptime_seconds < 1) uptime_seconds = 1;  /* Avoid divide by zero */
    
    days = uptime_seconds / 86400;
    hours = (uptime_seconds % 86400) / 3600;
    minutes = (uptime_seconds % 3600) / 60;
    seconds = uptime_seconds % 60;
    
    /* Calculate average players */
    if (total_player_seconds > 0 && uptime_seconds > 0)
    {
        average_players = (float)total_player_seconds / (float)uptime_seconds;
    }
    
    /* Calculate pulse miss percentage */
    if (total_pulses > 0)
    {
        pulse_miss_percent = ((float)missed_pulses / (float)total_pulses) * 100.0;
    }
    
    /* Calculate CPU lag percentage for last minute */
    if (cpu_measurements > 0)
    {
        long expected_usec = cpu_measurements * (1000000 / PULSE_PER_SECOND);
        if (expected_usec > 0)
            cpu_lag_percent = ((float)cpu_lag_usec / (float)expected_usec) * 100.0;
    }
    
    /* Calculate bytes per second */
    bytes_per_sec_read = (float)bytes_read_total / (float)uptime_seconds;
    bytes_per_sec_written = (float)bytes_written_total / (float)uptime_seconds;
    
    /* Display the information */
    send_to_char("\n\r{W╔══════════════════════════════════════════════════════╗{x\n\r", ch);
    
    sprintf(buf, "{W║{x         {YStatistics for %s{x\n\r", MUD_NAME);
    send_to_char(buf, ch);
    
    send_to_char("{W╠══════════════════════════════════════════════════════╣{x\n\r", ch);
    
    sprintf(buf, "{W║{x {CUptime:{x              %3dd %2dh %2dm %2ds\n\r", 
            days, hours, minutes, seconds);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {CPlayers online:{x       %d\n\r", current_players);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {CAverage since boot:{x   %.2f\n\r", average_players);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {CPeak since boot:{x      %d\n\r", peak_players);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {CTotal logins:{x         %ld\n\r", total_logins);
    send_to_char(buf, ch);
    
    send_to_char("{W╠══════════════════════════════════════════════════════╣{x\n\r", ch);
    
    sprintf(buf, "{W║{x {GTotal pulses:{x         %ld\n\r", total_pulses);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {GPulses missed:{x        %ld {G({y%.2f%%{G){x\n\r",
            missed_pulses, pulse_miss_percent);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {GCPU lag (1 min):{x      {y%.2f%%{x\n\r", cpu_lag_percent);
    send_to_char(buf, ch);
    
    send_to_char("{W╠══════════════════════════════════════════════════════╣{x\n\r", ch);
    
    sprintf(buf, "{W║{x {MBytes read:{x           %ld {M({y%.1f/sec{M){x\n\r",
            bytes_read_total, bytes_per_sec_read);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {MBytes written:{x        %ld {M({y%.1f/sec{M){x\n\r",
            bytes_written_total, bytes_per_sec_written);
    send_to_char(buf, ch);
    
    sprintf(buf, "{W║{x {MColor optimized:{x      %ld bytes\n\r", 
            color_bytes_saved);
    send_to_char(buf, ch);
    
    /* Add memory stats if immortal */
    if (ch->level >= LEVEL_IMMORTAL)
    {
        extern int top_area;
        extern int top_mob_index;
        extern int top_obj_index;
        extern int top_room;
        extern int mobile_count;
        extern int nAllocString;
        extern int sAllocString;
        extern int nAllocPerm;
        extern int sAllocPerm;
        
        send_to_char("{W╠══════════════════════════════════════════════════════╣{x\n\r", ch);
        
        send_to_char("{W║{x {RMemory Info (Immortal){x\n\r", ch);
        
        sprintf(buf, "{W║{x {BAreas:{x      %d   {BMobs:{x     %d   {BObjs:{x     %d\n\r",
                top_area, top_mob_index, top_obj_index);
        send_to_char(buf, ch);
        
        sprintf(buf, "{W║{x {BRooms:{x      %d   {BMob Count:{x             %d\n\r",
                top_room, mobile_count);
        send_to_char(buf, ch);
        
        sprintf(buf, "{W║{x {BStrings:{x    %d blocks, %d bytes\n\r",
                nAllocString, sAllocString);
        send_to_char(buf, ch);
        
        sprintf(buf, "{W║{x {BPerms:{x      %d blocks, %d bytes\n\r",
                nAllocPerm, sAllocPerm);
        send_to_char(buf, ch);
    }
    
    send_to_char("{W╚══════════════════════════════════════════════════════╝{x\n\r", ch);
    
    if (ch->level >= LEVEL_IMMORTAL)
    {
        send_to_char("\n\r{DType '{Ymudinfo reset{D' to reset statistics.{x\n\r", ch);
    }
    
    return;
}
 
