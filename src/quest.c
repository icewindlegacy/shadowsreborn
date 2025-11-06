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
 *           quest.c - November 3, 2025
 */            
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
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
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 8322
#define QUEST_ITEM2 8323
#define QUEST_ITEM3 8324
#define QUEST_ITEM4 17543
#define QUEST_ITEM5 8325

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 8326
#define QUEST_OBJQUEST2 8327
#define QUEST_OBJQUEST3 8328
#define QUEST_OBJQUEST4 8329
#define QUEST_OBJQUEST5 8330

/* Local functions */

void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update       args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance             args(( int num ));
ROOM_INDEX_DATA         *find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY CLEAR TRANSFER.\n\r", ch);
        send_to_char("For more information, type 'HELP QUEST'.\n\r", ch);
        return;
    }
    if (!strcmp(arg1, "info"))
    {
        if (IS_SET(ch->act,PLR_QUESTING))
        {
            if (ch->pcdata->questmob == -1 && ch->pcdata->questgiver->short_descr != NULL)
            {
                sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r", ch->pcdata->questgiver->short_descr);
                send_to_char (buf, ch);
            }
            else if (ch->pcdata->questobj > 0)
            {
                questinfoobj = get_obj_index (ch->pcdata->questobj);
                if (questinfoobj != NULL)
                {
                    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r", questinfoobj->name);
                    send_to_char (buf, ch);
                }
                else send_to_char ("You aren't currently on a quest.\n\r",ch);
                return;
            }
            else if (ch->pcdata->questmob > 0)
            {
                questinfo = get_mob_index(ch->pcdata->questmob);
                if (questinfo != NULL)
                {
                    sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r", questinfo->short_descr);
                    send_to_char(buf, ch);
                }
                else send_to_char ("You aren't currently on a quest.\n\r",ch);
                return;
            }
        }
        else
            send_to_char ("You aren't currently on a quest.\n\r",ch);
        return;
    }
    if (!strcmp(arg1, "points"))
    {
        sprintf(buf, "You have %d glory points.\n\r",ch->pcdata->quest_curr);
        send_to_char(buf, ch);
        return;
    }
    else if (!strcmp(arg1, "transfer"))
    {
        CHAR_DATA *victim;
        int amount;
        
        if (IS_NPC(ch))
        {
            send_to_char("NPCs cannot transfer quest points.\n\r", ch);
            return;
        }
        
        if (arg2[0] == '\0' || argument[0] == '\0')
        {
            send_to_char("Syntax: quest transfer <amount> <player>\n\r", ch);
            return;
        }
        
        if (!is_number(arg2))
        {
            send_to_char("The amount must be a number.\n\r", ch);
            return;
        }
        
        amount = atoi(arg2);
        
        if (amount <= 0)
        {
            send_to_char("You must transfer a positive amount of quest points.\n\r", ch);
            return;
        }
        
        if (ch->pcdata->quest_curr < amount)
        {
            sprintf(buf, "You only have %d quest points.\n\r", ch->pcdata->quest_curr);
            send_to_char(buf, ch);
            return;
        }
        
        if ((victim = get_char_world(ch, argument)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        
        if (IS_NPC(victim))
        {
            send_to_char("You cannot transfer quest points to NPCs.\n\r", ch);
            return;
        }
        
        if (victim == ch)
        {
            send_to_char("You cannot transfer quest points to yourself.\n\r", ch);
            return;
        }
        
        /* Perform the transfer */
        ch->pcdata->quest_curr -= amount;
        victim->pcdata->quest_curr += amount;
        
        sprintf(buf, "You transfer %d quest points to %s.\n\r", amount, victim->name);
        send_to_char(buf, ch);
        
        sprintf(buf, "%s has transferred %d quest points to you!\n\r", ch->name, amount);
        send_to_char(buf, victim);
        
        sprintf(buf, "%s transferred %d quest points to %s.", ch->name, amount, victim->name);
        log_string(buf);
        
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
        if (!IS_SET(ch->act,PLR_QUESTING))
        {
            send_to_char ("You aren't currently on a quest.\n\r",ch);
            if (ch->pcdata->nextquest > 1)
            {
                sprintf (buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->pcdata->nextquest);
                send_to_char(buf, ch);
            }
            else if (ch->pcdata->nextquest == 1)
            {
                sprintf (buf, "There is less than a minute remaining until you can go on another quest.\n\r");
                send_to_char(buf, ch);
            }
        }
        else if (ch->pcdata->countdown > 0)
        {
            sprintf (buf, "Time left for current quest: %d\n\r", ch->pcdata->countdown);
            send_to_char (buf, ch);
        }
        return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup ("spec_questmaster")) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup ("spec_questmaster"))
    {
        send_to_char ("You can't do that here.\n\r", ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
        send_to_char ("Wait until the fighting stops.\n\r", ch);
        return;
    }

    ch->pcdata->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up glory points :> Make the item worth their while. */

    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a list of quest items.", ch, NULL, questman, TO_CHAR);
        
        /* Show quest items - create instances dynamically */
        bool has_quest_items = FALSE;
        
        /* Check if questmaster has any quest items in inventory first */
        if (questman->carrying != NULL) {
            OBJ_DATA *obj;
            
            for (obj = questman->carrying; obj != NULL; obj = obj->next_content) {
                if (IS_OBJ_STAT(obj, ITEM_QUEST)) {
                    if (!has_quest_items) {
                        send_to_char("  [{WCost{w]     [{BName{w]\n\r",ch);
                        has_quest_items = TRUE;
                    }
                    sprintf(buf, "   {W%-4d{w       {b%s{w\n\r", 
                            QUEST_COST(obj), obj->short_descr);
                    send_to_char(buf, ch);
                }
            }
        }
        
        /* If no items found, show a message */
        if (!has_quest_items)
            send_to_char("  Nothing.\n\r",ch);
        
        send_to_char("\n\rTo buy an item, type 'Qwest buy <item>'.\n\r",ch);
        return;
    }

    else if (!strcmp(arg1, "buy"))
    {
        bool found=FALSE;
        if (arg2[0] == '\0')
        {
            send_to_char("To buy an item, type 'Qwest buy <item>'.\n\r",ch);
            return;
        }
        
        /* Check quest items in questmaster's inventory */
        if (questman->carrying != NULL) {
            OBJ_DATA *obj;
            for (obj = questman->carrying; obj != NULL; obj = obj->next_content) {
                if (IS_OBJ_STAT(obj, ITEM_QUEST) && 
                    (is_name(arg2, obj->name) || is_name(arg2, obj->short_descr))) {
                    found = TRUE;
                    int qcost = QUEST_COST(obj);
                    if (ch->pcdata->quest_curr >= qcost) {
                        ch->pcdata->quest_curr -= qcost;
                        /* Create a new instance instead of moving the original */
                        OBJ_DATA *new_obj = create_object(obj->pIndexData, ch->level);
                        obj_to_char(new_obj, ch);
                        sprintf(buf, "In exchange for %d glory, %s gives you %s.\n\r",
                                qcost, questman->short_descr, new_obj->short_descr);
                        send_to_char(buf, ch);
                        break;
                    } else {
                        sprintf(buf, "Sorry, %s, but you don't have enough glory for that.", ch->name);
                        do_say(questman, buf);
                        return;
                    }
                }
            }
        }
        
        if(!found)
        {
            sprintf(buf, "I don't have that item, %s.",ch->name);
            do_say(questman, buf);
        }
        return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);
        if (IS_SET(ch->act,PLR_QUESTING))
        {
            sprintf(buf, "But you're already on a quest!");
            do_say(questman, buf);
            return;
        }
        if (ch->pcdata->nextquest > 0)
        {
            sprintf(buf, "You're very brave, %s, but let someone else have a chance.", ch->name);
            do_say(questman, buf);
            sprintf(buf, "Come back later.");
            do_say(questman, buf);
            return;
        }

        sprintf(buf, "Thank you, brave %s!",ch->name);
        do_say(questman, buf);
        ch->pcdata->questmob = 0;
        ch->pcdata->questobj = 0;

        generate_quest(ch, questman);

        if (ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0)
        {
            ch->pcdata->countdown = number_range(10,30);
            SET_BIT(ch->act,PLR_QUESTING);
            sprintf(buf, "You have %d minutes to complete this quest.",ch->pcdata->countdown);
            do_say(questman, buf);
            sprintf(buf, "May the gods go with you!");
            do_say(questman, buf);
        }
        return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
        act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
        if (ch->pcdata->questgiver != questman)
        {
            sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
            do_say(questman,buf);
            return;
        }

        if (IS_SET(ch->act,PLR_QUESTING))
        {
            if (ch->pcdata->questmob == -1)
            {
                int reward, pointreward, pracreward;

                reward = number_range(25,45);
                pointreward = number_range(25,75);

                sprintf(buf, "Congratulations on completing your quest!");
                do_say(questman,buf);
                sprintf(buf,"As a reward, I am giving you %d glory points, and %d silver.", pointreward, reward);
                do_say(questman,buf);
                if (chance(15))
                {
                    pracreward = number_range(1,6);
                    sprintf(buf, "You gain %d practices!\n\r",pracreward);
                    send_to_char(buf, ch);
                    ch->practice += pracreward;
                }

                REMOVE_BIT(ch->act,PLR_QUESTING);
                ch->pcdata->questgiver = NULL;
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
                ch->pcdata->questobj = 0;
                ch->pcdata->nextquest = 10;
                ch->silver += reward;
                ch->pcdata->quest_curr += pointreward;

                return;
            }
            else if (ch->pcdata->questobj > 0)
            {
                bool obj_found = FALSE;

                for (obj = ch->carrying; obj != NULL; obj= obj_next)
                {
                    obj_next = obj->next_content;

                    if (obj != NULL && obj->pIndexData->vnum == ch->pcdata->questobj)
                    {
                        obj_found = TRUE;
                        break;
                    }
                }
                if (obj_found == TRUE)
                {
                    int reward, pointreward, pracreward;

                    reward = number_range(25,45);
                    pointreward = number_range(25,75);

                    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
                    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

                    sprintf(buf, "Congratulations on completing your quest!");
                    do_say(questman,buf);
                    sprintf(buf,"As a reward, I am giving you %d glory points, and %d silver.", pointreward, reward);
                    do_say(questman,buf);
                    if (chance(15))
                    {
                        pracreward = number_range(1,6);
                        sprintf(buf, "You gain %d practices!\n\r",pracreward);
                        send_to_char(buf, ch);
                        ch->practice += pracreward;
                    }

                    REMOVE_BIT(ch->act,PLR_QUESTING);
                    ch->pcdata->questgiver = NULL;
                    ch->pcdata->countdown = 0;
                    ch->pcdata->questmob = 0;
                    ch->pcdata->questobj = 0;
                    ch->pcdata->nextquest = 10;
                    ch->silver += reward;
                    ch->pcdata->quest_curr += pointreward;
                    extract_obj(obj);
                    return;
                }
                else
                {
                    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                    do_say(questman, buf);
                    return;
                }
                return;
            }
            else if ((ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0))
            {
                sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                do_say(questman, buf);
                return;
            }
        }
        if (ch->pcdata->nextquest > 0)
            sprintf(buf,"But you didn't complete your quest in time!");
        else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
        do_say(questman, buf);
        return;
    }
    else if (!str_cmp( arg1, "quit" ))
    {
        act( "$n informs $N $e does not want the quest.", ch, NULL, questman,TO_ROOM);
        act ("You inform $N you do not want the quest.",ch, NULL, questman, TO_CHAR);
        if (ch->pcdata->questgiver != questman)
        {
            sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
            do_say(questman,buf);
            return;
        }

        else
            {     
           sprintf(buf, "I'm sorry to hear that you can't complete the quest.");
                do_say(questman,buf);

                REMOVE_BIT(ch->act, PLR_QUESTING);
                ch->pcdata->questgiver = NULL;
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
                ch->pcdata->questobj = 0;
                ch->pcdata->nextquest = 10;
                return;
            }
        }
    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY QUIT.\n\r",ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    char buf [MAX_STRING_LENGTH];

    /*  Randomly selects a mob from the world mob list. If you don't
        want a mob to be selected, make sure it is immune to summon.
        Or, you could add a new mob flag called ACT_NOQUEST. The mob
        is selected for both mob and obj quests, even tho in the obj
        quest the mob is not used. This is done to assure the level
        of difficulty for the area isn't too great for the player. */

    for (victim = char_list; victim != NULL; victim = victim->next)
    {
        if (!IS_NPC(victim)) continue;

        if (quest_level_diff(ch->level, victim->level) == TRUE
            && !IS_SET(victim->imm_flags, IMM_SUMMON)
            && !IS_SET(victim->act, ACT_TRAIN)
    		&& !IS_SET(victim->act, ACT_PRACTICE)
    		&& !IS_SET(victim->act, ACT_GAIN)
    		&& !IS_SET(victim->act, ACT_IS_HEALER)
    		&& !IS_SET(victim->act, ACT_IS_CHANGER)
		&& !IS_SET(victim->act, ACT_PET)
		&& !IS_AFFECTED(victim, AFF_CHARM)
		&& !IS_AFFECTED(victim, AFF_INVISIBLE)
		&& !IS_AFFECTED(victim, AFF_HIDE)
		&& !IS_AFFECTED(victim, AFF_SNEAK)
                && !IS_SET(victim->imm_flags, IMM_WEAPON | IMM_MAGIC)
            && victim->pIndexData != NULL
            && victim->pIndexData->pShop == NULL
            && !IS_SET(victim->act, ACT_PET)
            && chance(15)) break;
    }

    if ( victim == NULL  )
    {
        do_say(questman, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, "Try again later.");
        ch->pcdata->nextquest = 2;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->pcdata->nextquest = 2;
        return;
    }

    /* Quest to kill a mob */

    switch(number_range(0,1))
    {
        case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.", victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
        break;

        case 1:
        sprintf(buf, "The kingdom's most heinous criminal, %s, has escaped from the dungeon!", victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "Since the escape, %s has murdered %d civillians!", victim->short_descr, number_range(2,20));
        do_say(questman, buf);
        do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
        break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!", victim->short_descr, room->name);
        do_say(questman, buf);

        /* I changed my area names so that they have just the name of the area
           and none of the level stuff. You may want to comment these next two
           lines. - Vassago */

        sprintf(buf, "That location is in the general area of %s.", room->area->name);
        do_say(questman, buf);
    }
    ch->pcdata->questmob = victim->pIndexData->vnum;
    return;
}

/* Level differences to search for. Moongate has 350
   levels, so you will want to tweak these greater or
   less than statements for yourself. - Vassago */

bool quest_level_diff(int clevel, int mlevel)
{
    if (clevel < 9 && mlevel < clevel + 2) return TRUE;
    else if (clevel <= 9 && mlevel < clevel + 3 
	  && mlevel > clevel - 5) return TRUE;
    else if (clevel <= 14 && mlevel < clevel + 4 
	  && mlevel > clevel - 5) return TRUE;
    else if (clevel <= 21 && mlevel < clevel + 5 
	  && mlevel > clevel - 4) return TRUE;
    else if (clevel <= 29 && mlevel < clevel + 6 
	  && mlevel > clevel - 3) return TRUE;
    else if (clevel <= 37 && mlevel < clevel + 7 
	  && mlevel > clevel - 2) return TRUE;
    else if (clevel <= 55 && mlevel < clevel + 8 
	  && mlevel > clevel - 1) return TRUE;
    else if(clevel > 55) return TRUE; /* Imms can get anything :) */
    else return FALSE;
}
                
/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->character != NULL && d->connected == CON_PLAYING)
        {

        ch = d->character;

        if (ch->pcdata->nextquest > 0)
        {
            ch->pcdata->nextquest--;
            if (ch->pcdata->nextquest == 0)
            {
                send_to_char("You may now quest again.\n\r",ch);
                return;
            }
        }
        else if (IS_SET(ch->act,PLR_QUESTING))
        {
            if (--ch->pcdata->countdown <= 0)
            {
                char buf [MAX_STRING_LENGTH];

                ch->pcdata->nextquest = 10;
                sprintf(buf, "You have run out of time for your quest!"
                             "\n\rYou may quest again in %d minutes.\n\r", ch->pcdata->nextquest);
                send_to_char(buf, ch);
                REMOVE_BIT(ch->act, PLR_QUESTING);
                ch->pcdata->questgiver = NULL;
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
            }
            if (ch->pcdata->countdown > 0 && ch->pcdata->countdown < 6)
            {
                send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
                return;
            }
        }
        }
    }
    return;
}