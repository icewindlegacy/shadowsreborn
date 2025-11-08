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
 *           update.c - November 3, 2025
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
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "music.h"
#include "olc.h"

/*
 * Local functions.
 */
int hit_gain args ((CHAR_DATA * ch));
int mana_gain args ((CHAR_DATA * ch));
int move_gain args ((CHAR_DATA * ch));
void mobile_update args ((void));
void weather_update args ((void));
void char_update args ((void));
void obj_update args ((void));
void auction_update args ((void));
void quest_update args ((void));
void aggr_update args ((void));
void darkness_update args ((void));
void make_corpse args ((CHAR_DATA * ch));
void lightning args ((void));

/* used for saving */

int save_number = 0;

/* OLC autosave function */
void olcautosave args ((void));

/* External OLC functions */
extern void save_area_list args ((void));
extern void save_area args ((AREA_DATA * pArea));



/*
 * Advancement stuff.
 */
void advance_level (CHAR_DATA * ch, bool hide)
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    /*
     *  when a character levels their share_level increases.
     *  when a character levels shares_bought per level reverts to zero.
     *  Gothar Bank Services -1997
     */
    ch->pcdata->share_level = ch->level;
    ch->pcdata->shares_bought = 0;

    ch->pcdata->last_level =
        (ch->played + (int) (current_time - ch->logon)) / 3600;



    add_hp =
        con_app[get_curr_stat (ch, STAT_CON)].hitp +
        number_range (class_table[ch->class].hp_min,
                      class_table[ch->class].hp_max);
    add_mana = number_range (2, (2 * get_curr_stat (ch, STAT_INT)
                                 + get_curr_stat (ch, STAT_WIS)) / 5);
    if (!class_table[ch->class].fMana)
        add_mana /= 2;
    add_move = number_range (1, (get_curr_stat (ch, STAT_CON)
                                 + get_curr_stat (ch, STAT_DEX)) / 6);
    add_prac = wis_app[get_curr_stat (ch, STAT_WIS)].practice;

    add_hp = add_hp * 9 / 10;
    add_mana = add_mana * 9 / 10;
    add_move = add_move * 9 / 10;

    add_hp = UMAX (2, add_hp);
    add_mana = UMAX (2, add_mana);
    add_move = UMAX (6, add_move);

    ch->max_hit += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;
    ch->train += 1;

    ch->pcdata->perm_hit += add_hp;
    ch->pcdata->perm_mana += add_mana;
    ch->pcdata->perm_move += add_move;

    if (!hide)
    {
        sprintf (buf,
                 "You gain %d hit point%s, %d mana, %d move, and %d practice%s.\n\r",
                 add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
                 add_prac, add_prac == 1 ? "" : "s");
        send_to_char (buf, ch);
        affect_strip(ch,gsn_plague);
        affect_strip(ch,gsn_poison);
        affect_strip(ch,gsn_blindness);
        affect_strip(ch,gsn_sleep);
        affect_strip(ch,gsn_curse);

        ch->hit        = ch->max_hit;
        ch->mana       = ch->max_mana;
        ch->move       = ch->max_move;
        update_pos( ch);
send_to_char( "The Gods have given you the power to continue.\n\r You have been fully restored.\n\r ", ch );
    }
    return;
}



void gain_exp (CHAR_DATA * ch, int gain)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch) || ch->level >= LEVEL_HERO)
        return;

    ch->exp = UMAX (exp_per_level (ch, ch->pcdata->points), ch->exp + gain);
    while (ch->level < LEVEL_HERO && ch->exp >=
           exp_per_level (ch, ch->pcdata->points) * (ch->level + 1))
    {
        send_to_char ("{GYou raise a level!!  {x", ch);
        ch->level += 1;
        sprintf (buf, "%s gained level %d", ch->name, ch->level);
        log_string (buf);
        sprintf (buf, "$N has attained level %d!", ch->level);
        wiznet (buf, ch, NULL, WIZ_LEVELS, 0, 0);
        advance_level (ch, FALSE);
        save_char_obj (ch);
    }

    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain (CHAR_DATA * ch)
{
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch))
    {
        gain = 5 + ch->level;
        if (IS_AFFECTED (ch, AFF_REGENERATION))
            gain *= 2;

        switch (ch->position)
        {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain = 3 * gain / 2;
                break;
            case POS_RESTING:
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }


    }
    else
    {
        gain = UMAX (3, get_curr_stat (ch, STAT_CON) - 3 + ch->level / 2);
        gain += class_table[ch->class].hp_max - 10;
        number = number_percent ();
        if (number < get_skill (ch, gsn_fast_healing))
        {
            gain += number * gain / 100;
            /* Wolfen get double fast healing benefit */
            if (!IS_NPC(ch) && ch->race == race_lookup("wolfen"))
                gain += number * gain / 100;
            if (ch->hit < ch->max_hit)
                check_improve (ch, gsn_fast_healing, TRUE, 8);
        }

        switch (ch->position)
        {
            default:
                gain /= 4;
                break;
            case POS_SLEEPING:
                break;
            case POS_RESTING:
                gain /= 2;
                break;
            case POS_FIGHTING:
                gain /= 6;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;

    }

    gain = gain * ch->in_room->heal_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;

    return UMIN (gain, ch->max_hit - ch->hit);
}



int mana_gain (CHAR_DATA * ch)
{
    int gain;
    int number;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch))
    {
        gain = 5 + ch->level;
        switch (ch->position)
        {
            default:
                gain /= 2;
                break;
            case POS_SLEEPING:
                gain = 3 * gain / 2;
                break;
            case POS_RESTING:
                break;
            case POS_FIGHTING:
                gain /= 3;
                break;
        }
    }
    else
    {
        gain = (get_curr_stat (ch, STAT_WIS)
                + get_curr_stat (ch, STAT_INT) + ch->level) / 2;
        number = number_percent ();
        if (number < get_skill (ch, gsn_meditation))
        {
            gain += number * gain / 100;
            if (ch->mana < ch->max_mana)
                check_improve (ch, gsn_meditation, TRUE, 8);
        }
        if (!class_table[ch->class].fMana)
            gain /= 2;

        switch (ch->position)
        {
            default:
                gain /= 4;
                break;
            case POS_SLEEPING:
                break;
            case POS_RESTING:
                gain /= 2;
                break;
            case POS_FIGHTING:
                gain /= 6;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;

    }

    gain = gain * ch->in_room->mana_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[4] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;

    return UMIN (gain, ch->max_mana - ch->mana);
}



int move_gain (CHAR_DATA * ch)
{
    int gain;

    if (ch->in_room == NULL)
        return 0;

    if (IS_NPC (ch))
    {
        gain = ch->level;
    }
    else
    {
        gain = UMAX (15, ch->level);

        switch (ch->position)
        {
            case POS_SLEEPING:
                gain += get_curr_stat (ch, STAT_DEX);
                break;
            case POS_RESTING:
                gain += get_curr_stat (ch, STAT_DEX) / 2;
                break;
        }

        if (ch->pcdata->condition[COND_HUNGER] == 0)
            gain /= 2;

        if (ch->pcdata->condition[COND_THIRST] == 0)
            gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
        gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED (ch, AFF_POISON))
        gain /= 4;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED (ch, AFF_HASTE) || IS_AFFECTED (ch, AFF_SLOW))
        gain /= 2;

    return UMIN (gain, ch->max_move - ch->move);
}



void gain_condition (CHAR_DATA * ch, int iCond, int value)
{
    int condition;
    int new_condition;

    if (value == 0 || IS_NPC (ch) || ch->level >= LEVEL_IMMORTAL)
        return;

    condition = ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond] = URANGE (0, condition + value, 48);
    new_condition = ch->pcdata->condition[iCond];

    /* Send graduated warning messages for hunger and thirst */
    if (value < 0)  /* Only when decreasing */
    {
        if (iCond == COND_HUNGER)
        {
            if (new_condition == 15 && condition > 15)
                send_to_char ("{YYour stomach rumbles.{x\n\r", ch);
            else if (new_condition == 10 && condition > 10)
                send_to_char ("{YYou are getting very hungry.{x\n\r", ch);
            else if (new_condition == 5 && condition > 5)
                send_to_char ("{RYour stomach aches with hunger!{x\n\r", ch);
            else if (new_condition == 3 && condition > 3)
                send_to_char ("{RYou are starving!{x\n\r", ch);
        }
        else if (iCond == COND_THIRST)
        {
            if (new_condition == 15 && condition > 15)
                send_to_char ("You feel parched.\n\r", ch);
            else if (new_condition == 10 && condition > 10)
                send_to_char ("{YYou are getting very thirsty.{x\n\r", ch);
            else if (new_condition == 5 && condition > 5)
                send_to_char ("{RYour throat is parched and dry!{x\n\r", ch);
            else if (new_condition == 3 && condition > 3)
                send_to_char ("{RYou are dying of thirst!{x\n\r", ch);
        }
    }

    if (ch->pcdata->condition[iCond] == 0)
    {
        switch (iCond)
        {
            case COND_HUNGER:
                /* DEATH FROM HUNGER */
                ch->hit -= 5;
                if (ch->hit > 0 )
                  {
                   send_to_char ("You are hungry.\n\r", ch);
                  }
                if (ch->hit <= 0 )
                   {

                     act("{RYou have died from starvation!{x",ch,NULL,NULL,TO_CHAR);
                     act("{R$n withers away and dies from starvation!{x",ch,NULL,NULL,TO_ROOM);
                     if (ch->exp > exp_per_level (ch, ch->pcdata->points) * ch->level)
                             gain_exp (ch, (2 * (exp_per_level (ch, ch->pcdata->points)
                                      * ch->level - ch->exp) / 3) + 50);

                     sprintf (log_buf, "%s died from hunger in %s [room %d]",
                             ch->name, ch->in_room->name, ch->in_room->vnum);

                     wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

                      make_corpse (ch);
                      int i;

                      extract_char (ch, FALSE);
                      while (ch->affected)
                      affect_remove (ch, ch->affected);
                      for (i = 0; i < 4; i++)
                          ch->armor[i] = 100;
                      ch->position = POS_RESTING;
                      ch->hit = UMAX (1, ch->hit);
                      ch->mana = UMAX (1, ch->mana);
                      ch->move = UMAX (1, ch->move);
                      ch->pcdata->condition[COND_THIRST] = 48;
                      ch->pcdata->condition[COND_HUNGER] = 48;
                      ch->pcdata->condition[COND_FULL] = 48;

                      return;
                    }

                break;

            case COND_THIRST:

                 /* DEATH FROM THIRST */
                    ch->hit -= 5;
                    if (ch->hit > 0 )
                       {
                         send_to_char ("You are thirsty.\n\r", ch);
                       }
                    if (ch->hit <= 0 )
                       {
                          act("{RYou have died from dehydration!{x",ch,NULL,NULL,TO_CHAR);
                          act("{R$n withers away and dies from dehydration!{x",ch,NULL,NULL,TO_ROOM);
                          if (ch->exp > exp_per_level (ch, ch->pcdata->points) * ch->level)
                                  gain_exp (ch, (2 * (exp_per_level (ch, ch->pcdata->points)
                                           * ch->level - ch->exp) / 3) + 50);

                          sprintf (log_buf, "%s died from dehydration in %s [room %d]",
                                            ch->name, ch->in_room->name, ch->in_room->vnum);

                          wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

                          make_corpse (ch);
                          int i;

                          extract_char (ch, FALSE);
                          while (ch->affected)
                          affect_remove (ch, ch->affected);
                          for (i = 0; i < 4; i++)
                              ch->armor[i] = 100;
                          ch->position = POS_RESTING;
                          ch->hit = UMAX (1, ch->hit);
                          ch->mana = UMAX (1, ch->mana);
                          ch->move = UMAX (1, ch->move);
                          ch->pcdata->condition[COND_THIRST] = 48;
                          ch->pcdata->condition[COND_HUNGER] = 48;
                          ch->pcdata->condition[COND_FULL] = 48;

                          return;
                       }

                break;

            case COND_DRUNK:
                if (condition != 0)
                    send_to_char ("You are sober.\n\r", ch);
                break;

        }
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;

        if (!IS_NPC (ch) || ch->in_room == NULL
            || IS_AFFECTED (ch, AFF_CHARM)) continue;

        if (ch->in_room->area->empty && !IS_SET (ch->act, ACT_UPDATE_ALWAYS))
            continue;

        /* Examine call for special procedure */
        if (ch->spec_fun != 0)
        {
            if ((*ch->spec_fun) (ch))
                continue;
        }

        if (ch->pIndexData->pShop != NULL)    /* give him some gold */
            if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
            {
                ch->gold +=
                    ch->pIndexData->wealth * number_range (1, 20) / 5000000;
                ch->silver +=
                    ch->pIndexData->wealth * number_range (1, 20) / 50000;
            }

        /*
         * Check triggers only if mobile still in default position
         */
        if (ch->position == ch->pIndexData->default_pos)
        {
            /* Delay */
            if (HAS_TRIGGER (ch, TRIG_DELAY) && ch->mprog_delay > 0)
            {
                if (--ch->mprog_delay <= 0)
                {
                    mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_DELAY);
                    continue;
                }
            }
            if (HAS_TRIGGER (ch, TRIG_RANDOM))
            {
                if (mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_RANDOM))
                    continue;
            }
        }

        /* That's all for sleeping / busy monster, and empty zones */
        if (ch->position != POS_STANDING)
            continue;

        /* Scavenge */
        if (IS_SET (ch->act, ACT_SCAVENGER)
            && ch->in_room->contents != NULL && number_bits (6) == 0)
        {
            OBJ_DATA *obj;
            OBJ_DATA *obj_best;
            int max;

            max = 1;
            obj_best = 0;
            for (obj = ch->in_room->contents; obj; obj = obj->next_content)
            {
                if (CAN_WEAR (obj, ITEM_TAKE) && can_loot (ch, obj)
                    && obj->cost > max && obj->cost > 0)
                {
                    obj_best = obj;
                    max = obj->cost;
                }
            }

            if (obj_best)
            {
                obj_from_room (obj_best);
                obj_to_char (obj_best, ch);
                act ("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
            }
        }

        /*Hate*/
        {
        CHAR_DATA *rch;

        for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
        {

        if ( is_hating(ch,rch) )
        {
                do_say( ch, "I remember the likes of you!");
                multi_hit( ch, rch, TYPE_UNDEFINED );
                break;
        }
        }
        }

        /* Wander */
        if (!IS_SET (ch->act, ACT_SENTINEL)
            && number_bits (3) == 0
            && (door = number_bits (5)) <= 5
            && (pexit = ch->in_room->exit[door]) != NULL
            && pexit->u1.to_room != NULL
            && !IS_SET (pexit->exit_info, EX_CLOSED)
            && !IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)
            && (!IS_SET (ch->act, ACT_STAY_AREA)
                || pexit->u1.to_room->area == ch->in_room->area)
            && (!IS_SET (ch->act, ACT_OUTDOORS)
                || !IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS))
            && (!IS_SET (ch->act, ACT_INDOORS)
                || IS_SET (pexit->u1.to_room->room_flags, ROOM_INDOORS)))
        {
            /* If this mobile has a rider, ensure rider moves with mount */
            if (RIDDEN(ch))
            {
                CHAR_DATA *rider = RIDDEN(ch);
                /* Rider is mounted, so they move together - move_char handles this */
                move_char (ch, door, FALSE);
            }
            else
            {
                move_char (ch, door, FALSE);
            }
        }
    }

    return;
}



/*
 * Update the weather.
 */



/*
 * Update all chars, including mobs.
*/
void char_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;

    ch_quit = NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
        save_number = 0;

    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        ch_next = ch->next;

        if (ch->timer > 30)
            ch_quit = ch;

        if (ch->position >= POS_STUNNED)
        {
            /* check to see if we need to go home */
            if (IS_NPC (ch) && ch->zone != NULL
                && ch->zone != ch->in_room->area && ch->desc == NULL
                && ch->fighting == NULL && !IS_AFFECTED (ch, AFF_CHARM)
                && number_percent () < 5)
            {
                act ("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
                extract_char (ch, TRUE);
                continue;
            }

            if (ch->hit < ch->max_hit)
                ch->hit += hit_gain (ch);
            else
                ch->hit = ch->max_hit;

            if (ch->mana < ch->max_mana)
                ch->mana += mana_gain (ch);
            else
                ch->mana = ch->max_mana;

            if (ch->move < ch->max_move)
                ch->move += move_gain (ch);
            else
                ch->move = ch->max_move;

            /* Swimming check for characters in water */
            if(ch->desc && ch->desc->connected == CON_PLAYING)
                if(!IS_NPC(ch))
                {
                    if( ch->in_room && (ch->in_room->sector_type == SECT_WATER_SWIM && !IS_AFFECTED(ch,AFF_FLYING)))
                    {
                        OBJ_DATA *obj;
                        bool has_boat;
                        int chance;
                        
                        /*
                         * Look for a boat.
                         */
                        has_boat = FALSE;
                        
                        if (IS_IMMORTAL(ch))
                            has_boat = TRUE;
                        
                        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
                        {
                            if ( obj->item_type == ITEM_BOAT )
                            {
                                has_boat = TRUE;
                                break;
                            }
                        }
                        if ( !has_boat )
                        {	    
                            if(!IS_NPC(ch) && ch->level < skill_table[gsn_swimming].skill_level[ch->class])
                            {
                                send_to_char("You shouldn't be here, report this to an IMM.\n\r",ch);
                                return;
                            }
                            
                            if((chance = get_skill(ch,gsn_swimming)) == 0)
                            {
                                send_to_char("You shouldn't be here, report this to an IMM.\n\r",ch);
                                return;
                            }
                            
                            if(IS_SET(ch->vuln_flags, VULN_DROWNING))
                                chance /= 2;
                            
                            /*chance -= ((ch->max_move / ch->move) / 2); commented cause you can float easily */
                            /*chance -= (get_carry_weight(ch) / 10); again, floating is easier*/
                            
                            if(ch->race == race_lookup("dwarf")) /* dwarf's can't float. */
                            {
                                chance = 0;
                            }
                            
                            if(number_percent() < chance)
                            {
                                check_improve(ch,gsn_swimming,TRUE, 5);
                            }
                            else
                            {
                                check_improve(ch,gsn_swimming,FALSE, 5);
                                send_to_char("You drown.\n\r",ch);
                                char_from_room(ch);
                                char_to_room(ch, get_room_index(ROOM_VNUM_UNDERWATER));
                                do_look(ch,"auto");
                                if (ch->pet != NULL)
                                {
                                    char_from_room( ch->pet );
                                    char_to_room( ch->pet, ch->in_room );
                                    do_look(ch->pet,"auto");
                                }
                                return;
                            }
                        }
                    }
                }
        }

        if (ch->position == POS_STUNNED)
            update_pos (ch);

        if (!IS_NPC (ch) && ch->level < LEVEL_IMMORTAL)
        {
            OBJ_DATA *obj;

            if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL
                && obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
            {
                if (--obj->value[2] == 0 && ch->in_room != NULL)
                {
                    --ch->in_room->light;
                    act ("$p goes out.", ch, obj, NULL, TO_ROOM);
                    act ("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
                    extract_obj (obj);
                }
                else if (obj->value[2] <= 5 && ch->in_room != NULL)
                    act ("$p flickers.", ch, obj, NULL, TO_CHAR);
            }

            if (IS_IMMORTAL (ch))
                ch->timer = 0;

            if (++ch->timer >= 12)
            {
                if (ch->was_in_room == NULL && ch->in_room != NULL)
                {
                    ch->was_in_room = ch->in_room;
                    if (ch->fighting != NULL)
                        stop_fighting (ch, TRUE);
                    act ("$n disappears into the void.",
                         ch, NULL, NULL, TO_ROOM);
                    send_to_char ("You disappear into the void.\n\r", ch);
                    if (ch->level > 1)
                        save_char_obj (ch);
                    char_from_room (ch);
                    char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
                }
            }

            gain_condition (ch, COND_DRUNK, -1);
            gain_condition (ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2);
            gain_condition (ch, COND_THIRST, -1);
            gain_condition (ch, COND_HUNGER,
                            ch->size > SIZE_MEDIUM ? -2 : -1);
            
            /* Reduce bleeding over time - heals naturally */
            if (ch->pcdata->condition[COND_BLEEDING] > 0)
            {
                /* Reduce faster when resting/sleeping, slower when fighting */
                int bleed_reduction = 1;
                if (ch->position == POS_SLEEPING)
                    bleed_reduction = 3;
                else if (ch->position == POS_RESTING)
                    bleed_reduction = 2;
                else if (ch->position == POS_FIGHTING)
                    bleed_reduction = 0;  /* No healing while fighting */
                    
                gain_condition (ch, COND_BLEEDING, -bleed_reduction);
            }
        }

        for (paf = ch->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            if (paf->duration > 0)
            {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0);
            else
            {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_off)
                    {
                        send_to_char (skill_table[paf->type].msg_off, ch);
                        send_to_char ("\n\r", ch);
                    }
                }

                affect_remove (ch, paf);
            }
        }

        /*
         * Careful with the damages here,
         *   MUST NOT refer to ch after damage taken,
         *   as it may be lethal damage (on NPC).
         */

        if (is_affected (ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

            if (ch->in_room == NULL)
                continue;

            act ("$n writhes in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_ROOM);
            send_to_char ("You writhe in agony from the plague.\n\r", ch);
            for (af = ch->affected; af != NULL; af = af->next)
            {
                if (af->type == gsn_plague)
                    break;
            }

            if (af == NULL)
            {
                STR_REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
                continue;
            }

            if (af->level == 1)
                continue;

            plague.where = TO_AFFECTS;
            plague.type = gsn_plague;
            plague.level = af->level - 1;
            plague.duration = number_range (1, 2 * plague.level);
            plague.location = APPLY_STR;
            plague.modifier = -5;
            plague.bitvector = AFF_PLAGUE;

            for (vch = ch->in_room->people; vch != NULL;
                 vch = vch->next_in_room)
            {
                if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
                    && !IS_IMMORTAL (vch)
                    && !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (4) == 0)
                {
                    send_to_char ("You feel hot and feverish.\n\r", vch);
                    act ("$n shivers and looks very ill.", vch, NULL, NULL,
                         TO_ROOM);
                    affect_join (vch, &plague);
                }
            }

            dam = UMIN (ch->level, af->level / 5 + 1);
            ch->mana -= dam;
            ch->move -= dam;
            damage (ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE, FALSE);
        }
        else if (IS_AFFECTED (ch, AFF_POISON) && ch != NULL
                 && !IS_AFFECTED (ch, AFF_SLOW))
        {
            AFFECT_DATA *poison;

            poison = affect_find (ch->affected, gsn_poison);

            if (poison != NULL)
            {
                act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
                send_to_char ("You shiver and suffer.\n\r", ch);
                damage (ch, ch, poison->level / 10 + 1, gsn_poison,
                        DAM_POISON, FALSE, FALSE);
            }
        }

        else if (ch->position == POS_INCAP && number_range (0, 1) == 0)
        {
            damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE, FALSE);
        }
        else if (ch->position == POS_MORTAL)
        {
            damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE, FALSE);
        }
    }

    /*
     * Process light items and timers on the ground in rooms
     */
    {
        ROOM_INDEX_DATA *room;
        OBJ_DATA *obj, *obj_next;

        for (room = room_index_hash[0]; room != NULL; room = room->next)
        {
            for (obj = room->contents; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                
                /* Process light items */
                if (obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
                {
                    if (--obj->value[2] == 0)
                    {
                        act("$p flickers and goes out.", NULL, obj, NULL, TO_ROOM);
                        extract_obj(obj);
                    }
                    else if (obj->value[2] <= 5)
                    {
                        act("$p flickers.", NULL, obj, NULL, TO_ROOM);
                    }
                }
                
                /* Process timer items */
                if (obj->timer > 0)
                {
                    if (--obj->timer == 0)
                    {
                        act("$p dissolves into nothingness.", NULL, obj, NULL, TO_ROOM);
                        extract_obj(obj);
                    }
                }
            }
        }
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
    	/*
    	 * Edwin's fix for possible pet-induced problem
    	 * JR -- 10/15/00
    	 */
    	if (!IS_VALID(ch))
    	{
        	bug("update_char: Trying to work with an invalidated character.\n",0); 
        	break;
     	}

        ch_next = ch->next;

        if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
        {
            save_char_obj (ch);
        }

        if (ch == ch_quit)
        {
            do_function (ch, &do_quit, "");
        }
    }

    /* 
     * Shopkeepers inform the players when they open or close
     * their shops. Also, at open hour, we give some extra gold 
     * to the Shopkeepers -- I hate bankrupted shopkeepers :)
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        SHOP_DATA *pShop;
        
        ch_next = ch->next;

        if ( !IS_NPC(ch) 
        || ch->in_room == NULL 
        || IS_AFFECTED(ch,AFF_CHARM))
            continue;

        if ( (pShop = ch->pIndexData->pShop) != NULL)
        {
            if (time_info.hour == pShop->open_hour - 1 )
            {
                {
                    if (ch->gold < ch->pIndexData->wealth)
                        ch->gold += ch->pIndexData->wealth;
                }
                act( "$n prepares to open the shop.", ch, NULL, NULL, TO_ROOM);
            }
            else if (time_info.hour == pShop->open_hour)
                act( "$n opens the shop.", ch, NULL, NULL, TO_ROOM);
            else if (time_info.hour == pShop->close_hour - 1)
                act( "$n prepares to close the shop.", ch, NULL, NULL, TO_ROOM);
            else if (time_info.hour == ch->pIndexData->pShop->close_hour)
                act( "$n closes the shop.", ch, NULL, NULL, TO_ROOM);	
        }
    }

    return;
}




/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update (void)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for (obj = object_list; obj != NULL; obj = obj_next)
    {
        CHAR_DATA *rch;
        char *message;

        obj_next = obj->next;

        if ((obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR)
            && obj->condition > 0)
        {
            int chance;

            if (obj->wear_loc != WEAR_NONE)
                chance = 7200;      /* Worn: 3x slower */
            else if (obj->carried_by != NULL)
                chance = 10800;     /* Carried: 3x slower */
            else
                chance = 3600;      /* Ground: 3x slower */

            if (chance < 1)
                chance = 1;

            if (number_range(1, chance) == 1)
                damage_item_condition(obj, 1, obj->carried_by);
        }

        /* go through affects and decrement */
        for (paf = obj->affected; paf != NULL; paf = paf_next)
        {
            paf_next = paf->next;
            if (paf->duration > 0)
            {
                paf->duration--;
                if (number_range (0, 4) == 0 && paf->level > 0)
                    paf->level--;    /* spell strength fades with time */
            }
            else if (paf->duration < 0);
            else
            {
                if (paf_next == NULL
                    || paf_next->type != paf->type || paf_next->duration > 0)
                {
                    if (paf->type > 0 && skill_table[paf->type].msg_obj)
                    {
                        if (obj->carried_by != NULL)
                        {
                            rch = obj->carried_by;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_CHAR);
                        }
                        if (obj->in_room != NULL
                            && obj->in_room->people != NULL)
                        {
                            rch = obj->in_room->people;
                            act (skill_table[paf->type].msg_obj,
                                 rch, obj, NULL, TO_ALL);
                        }
                    }
                }

                affect_remove_obj (obj, paf);
            }
        }

        /* Falling objects in air sectors */
        if (obj->in_room &&
            obj->in_room->sector_type == SECT_AIR &&
            (obj->wear_flags & ITEM_TAKE) &&
            obj->in_room->exit[5] &&
            obj->in_room->exit[5]->u1.to_room)
        {
            ROOM_INDEX_DATA *new_room = obj->in_room->exit[5]->u1.to_room;

            if (( rch = obj->in_room->people ) != NULL )
            {
                act( "$p falls away.", rch, obj, NULL, TO_ROOM );
                act( "$p falls away.", rch, obj, NULL, TO_CHAR );
            }

            obj_from_room(obj);
            obj_to_room(obj, new_room);

            if (( rch = obj->in_room->people ) != NULL )
            {
                act( "$p floats by.", rch, obj, NULL, TO_ROOM );
                act( "$p floats by.", rch, obj, NULL, TO_CHAR );
            }
        }

        if (obj->timer <= 0 || --obj->timer > 0)
            continue;

        switch (obj->item_type)
        {
            default:
                message = "$p crumbles into dust.";
                break;
            case ITEM_FOUNTAIN:
                message = "$p dries up.";
                break;
            case ITEM_CORPSE_NPC:
                message = "$p decays into dust.";
                break;
            case ITEM_CORPSE_PC:
                message = "$p decays into dust.";
                break;
            case ITEM_FOOD:
                message = "$p decomposes.";
                break;
            case ITEM_POTION:
                message = "$p has evaporated from disuse.";
                break;
            case ITEM_PORTAL:
                message = "$p fades out of existence.";
                break;
            case ITEM_PYLON:
                message = "$p crumbles to dust.";
                break;
            case ITEM_CONTAINER:
                if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
                    if (obj->contains)
                        message =
                            "$p flickers and vanishes, spilling its contents on the floor.";
                    else
                        message = "$p flickers and vanishes.";
                else
                    message = "$p crumbles into dust.";
                break;
        }

 
        if (obj->carried_by != NULL)
        {
            if (IS_NPC (obj->carried_by)
                && obj->carried_by->pIndexData->pShop != NULL)
                obj->carried_by->silver += obj->cost / 5;
            else if (!IS_OBJ_STAT(obj, ITEM_HIDDEN))
            {
                act (message, obj->carried_by, obj, NULL, TO_CHAR);
                if (obj->wear_loc == WEAR_FLOAT)
                    act (message, obj->carried_by, obj, NULL, TO_ROOM);
            }
        }
        else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL)
        {
            if (!(obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
                  && !CAN_WEAR (obj->in_obj, ITEM_TAKE))
                && !IS_OBJ_STAT(obj, ITEM_HIDDEN))
            {
                act (message, rch, obj, NULL, TO_ROOM);
                act (message, rch, obj, NULL, TO_CHAR);
            }
        }

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
            && obj->contains)
        {                        /* save the contents */
            OBJ_DATA *t_obj, *next_obj;

            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content;
                obj_from_obj (t_obj);

                if (obj->in_obj)    /* in another object */
                    obj_to_obj (t_obj, obj->in_obj);

                else if (obj->carried_by)    /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj (t_obj);
                        else
                            obj_to_room (t_obj, obj->carried_by->in_room);
                    else
                        obj_to_char (t_obj, obj->carried_by);

                else if (obj->in_room == NULL)    /* destroy it */
                    extract_obj (t_obj);

                else            /* to a room */
                    obj_to_room (t_obj, obj->in_room);
            }
        }

        extract_obj (obj);
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
/*
 * Darkness update - decrement timers and remove expired darkness
 */
void darkness_update (void)
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *ch;
    int vnum;
    AREA_DATA *area;
    
    for (area = area_first; area != NULL; area = area->next)
    {
        for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++)
        {
            if ((room = get_room_index(vnum)) == NULL)
                continue;
                
            if (!IS_SET(room->room_flags, ROOM_MALEDICTION))
                continue;
                
            if (room->darkness_timer > 0)
            {
                room->darkness_timer--;
                
                if (room->darkness_timer <= 0)
                {
                    REMOVE_BIT(room->room_flags, ROOM_MALEDICTION);
                    
                    /* Notify people in the room */
                    for (ch = room->people; ch != NULL; ch = ch->next_in_room)
                    {
                        send_to_char("{8The magical darkness dissipates.{x\n\r", ch);
                    }
                }
            }
        }
    }
}

void aggr_update (void)
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for (wch = char_list; wch != NULL; wch = wch_next)
    {
        wch_next = wch->next;
        if (IS_NPC (wch)
            || wch->level >= LEVEL_IMMORTAL
            || wch->in_room == NULL || wch->in_room->area->empty) continue;

        for (ch = wch->in_room->people; ch != NULL; ch = ch_next)
        {
            int count;

            ch_next = ch->next_in_room;

            if (!IS_NPC (ch)
                || !IS_SET (ch->act, ACT_AGGRESSIVE)
                || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
                || IS_AFFECTED (ch, AFF_CALM)
                || ch->fighting != NULL || IS_AFFECTED (ch, AFF_CHARM)
                || !IS_AWAKE (ch)
                || (IS_SET (ch->act, ACT_WIMPY) && IS_AWAKE (wch))
                || !can_see (ch, wch) || number_bits (1) == 0)
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;
            victim = NULL;
            for (vch = wch->in_room->people; vch != NULL; vch = vch_next)
            {
                vch_next = vch->next_in_room;

                if (!IS_NPC (vch)
                    && vch->level < LEVEL_IMMORTAL
                    && ch->level >= vch->level - 5
                    && (!IS_SET (ch->act, ACT_WIMPY) || !IS_AWAKE (vch))
                    &&   can_see( ch, vch )
         	        &&   ch->clan != vch->clan )	
                {
                    if (number_range (0, count) == 0)
                        victim = vch;
                    count++;
                }
            }

            if (victim == NULL)
                continue;

            multi_hit (ch, victim, TYPE_UNDEFINED);
        }
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler (void)
{
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    static int pulse_point;
    static int pulse_music;
    static	int	pulse_quest;
    static int  pulse_hint;
    static  int     count;
    	DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

  
    if (--pulse_area <= 0)
    {
        pulse_area = PULSE_AREA;
        /* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
        area_update ();
        quest_update ();
        olcautosave ();
    }
    if ( --pulse_quest	<= 0) 
    { 
	pulse_quest = PULSE_QUEST;
	quest_update	( );
    }
    if (--pulse_music <= 0)
    {
        pulse_music = PULSE_MUSIC;
        song_update ();
    }

    if (--pulse_mobile <= 0)
    {
        pulse_mobile = PULSE_MOBILE;
        mobile_update ();
    }

    if (--pulse_violence <= 0)
    {
        pulse_violence = PULSE_VIOLENCE;
        violence_update ();
    }

    if (--pulse_point <= 0)
    {
        darkness_update();  /* Check for expired darkness */
        
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if (d->character != NULL && d->connected == CON_PLAYING)
            {
                ch = d->character;

                if (IS_SET(ch->act, PLR_AUTOTICK))
                {
                    for (count = 0; tick_table[count]; count++);
                    if (count > 0)
                        send_to_char(tick_table[number_range(0, count - 1)], ch);
                }
            }
        }
        wiznet ("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
        pulse_point = PULSE_TICK;
/* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
        weather_update ();
        char_update ();
        obj_update ();
    }
    if ( --pulse_hint <= 0)
    {
    pulse_hint = PULSE_HINT;
    hint_update ( );
    }
    aggr_update ();
    auction_update ();
    tail_chain ();
    return;
}

/* OLC Autosave - saves all changed areas automatically */
void olcautosave (void)
{
   AREA_DATA *pArea;
   DESCRIPTOR_DATA *d;
   char buf[MAX_INPUT_LENGTH];

   save_area_list();
   for ( d = descriptor_list; d != NULL; d = d->next )
   {
      if ( d->editor)
              send_to_char( "OLC Autosaving:\n\r", d->character );
      else
              log_string( "OLC Autosaving:" );
   }
   sprintf( buf, "None.\n\r" );
            
   for( pArea = area_first; pArea; pArea = pArea->next ) 
   {
       /* Save changed areas. */
       if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
       {
           save_area( pArea );
           sprintf( buf, "%24s - '%s'", pArea->name, pArea->file_name );
           for ( d = descriptor_list; d != NULL; d = d->next )
           {
              if ( d->editor )
              {
                   send_to_char( buf, d->character );
                   send_to_char( "\n\r", d->character );
              }
              else 
                   log_string( buf );
           }   
           REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
       }
   }       
   
   if ( !str_cmp( buf, "None.\n\r" ) )
   {
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
           if ( d->editor )
                   send_to_char( buf, d->character );
           else
                   log_string( "None." );
      }     
   }
    
   return;
}

void gain_object_exp( CHAR_DATA *ch, OBJ_DATA *obj, int gain )
{
	int leftover = 0;

    if ( IS_NPC(ch) || obj->plevel >= 50 )
		return;

    printf_to_char( ch, "%s has gained %d exp.\n\r", capitalize( obj->short_descr ), gain );     
    obj->exp += gain;
    obj->xp_tolevel -= gain;

	if(obj->xp_tolevel <= 0 )
	{	obj->exp += obj->xp_tolevel;
		advance_level_object(ch,obj);
		leftover = ( obj->xp_tolevel * 1 );
		obj->plevel++;
		printf_to_char( ch, "%s has raised to level %d. To see your objects stats lore or identify it.\n\r", capitalize( obj->short_descr ), obj->plevel );
		obj->xp_tolevel = 1500 + ( obj->plevel * 150 );
		obj->xp_tolevel -= leftover;
		return;
	}
	return;
}

void advance_level_object( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int pbonus = number_range( 5, 10 );
    int bonus = number_range( 4, 8 );

    pbonus  = pbonus * 9/10;
    bonus =   bonus * 8/10;

    pbonus  = UMAX(  6, pbonus );
    bonus = UMAX( 1, bonus );

    add_apply(obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel + 1);
    add_apply(obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel + 1); 
    add_apply(obj, APPLY_HIT, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel + 2);
    add_apply(obj, APPLY_MANA, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel + 2);
    add_apply(obj, APPLY_MOVE, pbonus, TO_OBJECT, 0, -1, 0, obj->plevel + 2);

    /* Apply the new affects to the character wearing the item */
    if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
    {
        AFFECT_DATA *paf;
        for (paf = obj->affected; paf != NULL; paf = paf->next)
        {
            if (paf->level == obj->plevel + 2) /* Only apply the new affects */
            {
                affect_modify(obj->carried_by, paf, TRUE);
            }
        }
    }

    if (obj->item_type == ITEM_WEAPON)
    {
        obj->value[1] += bonus/4;
        obj->value[2] += bonus/5;
    } 

    else if (obj->item_type == ITEM_ARMOR)
    {
        obj->value[0] -= UMAX(1, obj->plevel);
        obj->value[1] -= UMAX(1, obj->plevel);
        obj->value[2] -= UMAX(1, obj->plevel);
        obj->value[3] -= (5 * UMAX(1, obj->plevel)) / 10;
    }
   
    return;
}


void hint_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  for(ch=char_list;ch!=NULL;ch=ch_next)
    {
    ch_next = ch->next;

    if(!IS_NPC(ch) && !IS_SET(ch->comm, COMM_NOHINT))
      {
      send_to_char("{W[{GH{gI{rN{gT{W]{x ",ch);
      switch(number_range(0,10))
        {
        default: send_to_char("Type 'display' to see different prompt options.\n\r", ch); break;
        case 0: send_to_char("To disable the hint channel, type 'hint'.\n\r",ch); break;
        case 1: send_to_char("You can request quests at Hassan.\n\r",ch); break;
        case 2: send_to_char("Quest items are relics that gain experience points with you.\n\r",ch); break;
        case 3: send_to_char("Relic items get much more powerful as they level.\n\r",ch); break;
        case 4: send_to_char("You can learn new skills and spells by studying manuals, scrolls, staves, and wands!\n\r",ch); break;
        case 5: send_to_char("Remember to eat and drink - you can die of hunger and thirst!\n\r",ch); break;
        case 6: send_to_char("If you want to see everything a class can do, type 'showclass war', 'showclass mag, 'showclass cle' or 'showclass thi'. \n\r",ch); break;
        case 7: send_to_char("You can train after level 5 by going 3s 3e 1s from recall.\n\r",ch); break;
        case 8: send_to_char("After level 5, you must find your class hall to practice your skills. Type 'help directions' if you don't know where it is.\n\r",ch); break;
        case 9: send_to_char("If you do not want the distraction of global chat channels, you can type 'quiet' to toggle quiet mode on or off.\n\r",ch); break;
        case 10: send_to_char("You are responsible for knowing the rules of the game. Be sure to type 'help rules' to familiarize yourself with them.\n\r",ch); break;
        }
      send_to_char("\n\r",ch);
      }
    }
return;
}

void lightning( void )
{
 DESCRIPTOR_DATA *d;
  for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                && IS_OUTSIDE( d->character )
                && IS_AWAKE  ( d->character )
                && number_chance(15)
                && !IS_IMMORTAL(d->character)
                && d->character->level > 17
                && weather_info[d->character->in_room->sector_type].sky == SKY_LIGHTNING ) {

  send_to_char("{x{RYou see a brilliant flash come down from the sky and then black out!\n\r",d->character);
  act( "$n has been struck by lightning!", d->character, NULL, NULL,TO_ROOM );
if(check_immune(d->character,DAM_LIGHTNING) != IS_IMMUNE) {
 if(d->character->fighting) {stop_fighting(d->character,TRUE); }
if(check_immune(d->character,DAM_LIGHTNING) != IS_RESISTANT) {
 if(d->character->hit > d->character->hit /2) {d->character->hit -= d->character->hit/10; }
  WAIT_STATE(d->character,25); } else {
 if(d->character->hit > d->character->hit /2) {d->character->hit -= d->character->hit/50; }
 WAIT_STATE(d->character,10); }
if(check_immune(d->character,DAM_LIGHTNING) == IS_VULNERABLE) {
d->character->hit -= d->character->hit/2;
WAIT_STATE(d->character,40); } }
} }
}
