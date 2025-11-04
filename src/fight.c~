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
 *           fight.c - November 3, 2025
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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

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

/*
 * Local functions.
 */
static OBJ_DATA *select_random_armor_piece (CHAR_DATA *victim);
static void apply_armor_wear (CHAR_DATA *victim, int dam);
void check_assist args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args ((CHAR_DATA * ch, CHAR_DATA * victim));
void check_killer args ((CHAR_DATA * ch, CHAR_DATA * victim));
void    crowd_brawl     args( ( CHAR_DATA *ch ) );
bool check_parry args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool    check_phase     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool pole_special args ((CHAR_DATA * ch, CHAR_DATA * victim));
int  axe_special args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam));
bool vorpal_kill args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type));
void dam_message args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
                        int dt, bool immune, bool critical));
void death_cry args ((CHAR_DATA * ch));
void group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute args ((CHAR_DATA * gch, CHAR_DATA * victim, int total_levels));
bool is_safe args ((CHAR_DATA * ch, CHAR_DATA * victim));
void make_corpse args ((CHAR_DATA * ch));
void one_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary ));
void mob_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void raw_kill args ((CHAR_DATA * ch, CHAR_DATA * victim));
void set_fighting args ((CHAR_DATA * ch, CHAR_DATA * victim));
void disarm args ((CHAR_DATA * ch, CHAR_DATA * victim));



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update (void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for (ch = char_list; ch != NULL; ch = ch_next)
    {
        ch_next = ch->next;

        if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
            continue;

    if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
    {
        /* calls from here, because everyone actively fighting
         * (as in, not asleep and in the room, see "if" check above)
         * could pull a person into the fight, either against them
         * or against the oponent.  Also called before a person hits
         * so that no one gets drawn in after a fight ends due to death.
         */
        crowd_brawl(ch);

        multi_hit (ch, victim, TYPE_UNDEFINED);

        /*
         * Swords will very rarely get in an extra round.
         */
        if (get_weapon_sn (ch) == gsn_sword
            && ch->wait <= 0)
        {
            int chance;
            CHAR_DATA * was_fighting;
            CHAR_DATA * vch;

            chance = get_skill (ch, gsn_sword) / 15;

            /*
             * Penalty if the opponent can see.
             */
            if (can_see (victim, ch))
                chance = chance * 3 / 5;

            for (vch = ch->in_room->people; vch != NULL;
                    vch = vch->next_in_room)
            {
                if (vch->fighting == ch
                    || is_same_group (vch->fighting, ch))
                {
                    if (number_percent () < chance)
                    {
                        was_fighting = ch->fighting;
                        ch->fighting = vch;

                        multi_hit (ch, vch, TYPE_UNDEFINED);
                        check_improve (ch, gsn_sword, TRUE, 2);

                        ch->fighting = was_fighting;
                        WAIT_STATE (ch, PULSE_VIOLENCE);
                        break;
                    }
                }
            }
        }
    }
        else
            stop_fighting (ch, FALSE);

        if ((victim = ch->fighting) == NULL)
            continue;

        /*
         * Fun for the whole family!
         */
        check_assist (ch, victim);

        if (IS_NPC (ch))
        {
            if (HAS_TRIGGER (ch, TRIG_FIGHT))
                mp_percent_trigger (ch, victim, NULL, NULL, TRIG_FIGHT);
            if (HAS_TRIGGER (ch, TRIG_HPCNT))
                mp_hprct_trigger (ch, victim);
        }
    }

    return;
}

/* for auto assisting */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
        rch_next = rch->next_in_room;

        if (IS_AWAKE (rch) && rch->fighting == NULL)
        {
            if (RIDDEN(rch) == ch || MOUNTED(rch) == ch)
                multi_hit(rch,victim,TYPE_UNDEFINED);

            /* quick check for ASSIST_PLAYER */
            if (!IS_NPC (ch) && IS_NPC (rch)
                && IS_SET (rch->off_flags, ASSIST_PLAYERS)
                && rch->level + 6 > victim->level)
            {
                do_function (rch, &do_emote, "screams and attacks!");
                multi_hit (rch, victim, TYPE_UNDEFINED);
                continue;
            }

            /* PCs next */
            if (!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM))
            {
                if (((!IS_NPC (rch) && IS_SET (rch->act, PLR_AUTOASSIST))
                     || IS_AFFECTED (rch, AFF_CHARM))
                    && is_same_group (ch, rch) && !is_safe (rch, victim))
                    multi_hit (rch, victim, TYPE_UNDEFINED);

                continue;
            }

            /* now check the NPC cases */

            if (IS_NPC (ch) && !IS_AFFECTED (ch, AFF_CHARM))
            {
                if ((IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALL))
                    || (IS_NPC (rch) && rch->group && rch->group == ch->group)
                    || (IS_NPC (rch) && rch->race == ch->race
                        && IS_SET (rch->off_flags, ASSIST_RACE))
                    || (IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALIGN)
                        && ((IS_GOOD (rch) && IS_GOOD (ch))
                            || (IS_EVIL (rch) && IS_EVIL (ch))
                            || (IS_NEUTRAL (rch) && IS_NEUTRAL (ch))))
                    || (rch->pIndexData == ch->pIndexData
                        && IS_SET (rch->off_flags, ASSIST_VNUM)))
                {
                    CHAR_DATA *vch;
                    CHAR_DATA *target;
                    int number;

                    if (number_bits (1) == 0)
                        continue;

                    target = NULL;
                    number = 0;
                    for (vch = ch->in_room->people; vch; vch = vch->next)
                    {
                        if (can_see (rch, vch)
                            && is_same_group (vch, victim)
                            && number_range (0, number) == 0)
                        {
                            target = vch;
                            number++;
                        }
                    }

                    if (target != NULL)
                    {
                        do_function (rch, &do_emote, "screams and attacks!");
                        multi_hit (rch, target, TYPE_UNDEFINED);
                    }
                }
            }
        }
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance;

    /* decrement the wait */
    if (ch->desc == NULL)
        ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
        ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
        return;

    if (MOUNTED(ch))
    {
        if (!mount_success(ch, MOUNTED(ch), FALSE))
        {
            send_to_char("You fall off your mount as you start your attacks!\n\r", ch);
            return;
        }
    }

    if (IS_NPC (ch))
    {
        mob_hit (ch, victim, dt);
        return;
    }

    one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim)
        return;
    
    /*
     * Extra hit for daggers.
     */
    if (get_weapon_sn (ch) == gsn_dagger)
    {
        chance = get_skill (ch, gsn_dagger) / 2;

        if (IS_AFFECTED (ch, AFF_SLOW))
            chance /= 1.5;

        if (IS_AFFECTED (ch, AFF_HASTE))
            chance *= 2;

        if (number_percent () < chance)
        {
            one_hit (ch, victim, dt, FALSE);
            check_improve (ch, gsn_dagger, TRUE, 4);

            if (ch->fighting != victim)
                return;
        }
    }

    if (get_eq_char (ch, WEAR_SECONDARY))
    {
        one_hit( ch, victim, dt, TRUE );
        if ( ch->fighting != victim )
            return;
    }

    if (IS_AFFECTED (ch, AFF_HASTE))
        one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim || dt == gsn_backstab)
        return;

    chance = get_skill (ch, gsn_second_attack) / 2;

    if (IS_AFFECTED (ch, AFF_SLOW))
        chance /= 2;

    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        check_improve (ch, gsn_second_attack, TRUE, 5);
        if (ch->fighting != victim)
            return;
    }

    chance = get_skill (ch, gsn_third_attack) / 4;

    if (IS_AFFECTED (ch, AFF_SLOW))
        chance = 0;;

    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        check_improve (ch, gsn_third_attack, TRUE, 6);
        if (ch->fighting != victim)
            return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance, number;
    CHAR_DATA *vch, *vch_next;

    one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim)
        return;

    /* Area attack -- BALLS nasty! */

    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
            vch_next = vch->next;
            if ((vch != victim && vch->fighting == ch))
                one_hit (ch, vch, dt, FALSE);
        }
    }

    if (IS_AFFECTED (ch, AFF_HASTE)
        || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
        one_hit (ch, victim, dt, FALSE);

    if (ch->fighting != victim || dt == gsn_backstab)
        return;

    chance = get_skill (ch, gsn_second_attack) / 2;

    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        if (ch->fighting != victim)
            return;
    }

    chance = get_skill (ch, gsn_third_attack) / 4;

    if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance = 0;

    if (number_percent () < chance)
    {
        one_hit (ch, victim, dt, FALSE);
        if (ch->fighting != victim)
            return;
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
        return;

    number = number_range (0, 2);

    if (number == 1 && IS_SET (ch->act, ACT_MAGE))
    {
        /*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET (ch->act, ACT_CLERIC))
    {
        /* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range (0, 8);

    switch (number)
    {
        case (0):
            if (IS_SET (ch->off_flags, OFF_BASH))
                do_function (ch, &do_bash, "");
            break;

        case (1):
            if (IS_SET (ch->off_flags, OFF_BERSERK)
                && !IS_AFFECTED (ch, AFF_BERSERK))
                do_function (ch, &do_berserk, "");
            break;


        case (2):
            if (IS_SET (ch->off_flags, OFF_DISARM)
                || (get_weapon_sn (ch) != gsn_hand_to_hand
                    && (IS_SET (ch->act, ACT_WARRIOR)
                        || IS_SET (ch->act, ACT_THIEF))))
                do_function (ch, &do_disarm, "");
            break;

        case (3):
            if (IS_SET (ch->off_flags, OFF_KICK))
                do_function (ch, &do_kick, "");
            break;

        case (4):
            if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
                do_function (ch, &do_dirt, "");
            break;

        case (5):
            if (IS_SET (ch->off_flags, OFF_TAIL))
            {
                /* do_function(ch, &do_tail, "") */ ;
            }
            break;

        case (6):
            if (IS_SET (ch->off_flags, OFF_TRIP))
                do_function (ch, &do_trip, "");
            break;

        case (7):
            if (IS_SET (ch->off_flags, OFF_CRUSH))
            {
                /* do_function(ch, &do_crush, "") */ ;
            }
            break;
        case (8):
            if (IS_SET (ch->off_flags, OFF_BACKSTAB))
            {
                do_function (ch, &do_backstab, "");
            }
    }
}


/*
 * Hit one guy once.
 */
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn, skill;
    int dam_type;
    bool result;
    bool critical_strike;

    sn = -1;
    critical_strike = FALSE;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
        return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
        return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if (dt == TYPE_UNDEFINED)
    {
        dt = TYPE_HIT;
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt += wield->value[3];
        else
        {
            /* Check for ohshit skill - holding object without weapon */
            if (!IS_NPC (ch) && get_eq_char (ch, WEAR_HOLD) != NULL 
                && get_skill (ch, gsn_ohshit) > 0)
                dt = TYPE_HIT + gsn_ohshit;  /* Use ohshit skill for damage message */
            else
                dt += ch->dam_type;
        }
    }

    if (dt < TYPE_HIT)
        if (wield != NULL)
            dam_type = attack_table[wield->value[3]].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
        dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn (ch);
    skill = 20 + get_weapon_skill (ch, sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if (IS_NPC (ch))
    {
        thac0_00 = 20;
        thac0_32 = -4;            /* as good as a thief */
        if (IS_SET (ch->act, ACT_WARRIOR))
            thac0_32 = -10;
        else if (IS_SET (ch->act, ACT_THIEF))
            thac0_32 = -4;
        else if (IS_SET (ch->act, ACT_CLERIC))
            thac0_32 = 2;
        else if (IS_SET (ch->act, ACT_MAGE))
            thac0_32 = 6;
    }
    else
    {
        thac0_00 = class_table[ch->class].thac0_00;
        thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0 = interpolate (ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
        thac0 = thac0 / 2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL (ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
        thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

    if (dt == gsn_circle)
        thac0 -= 10 * (100 - get_skill (ch, gsn_circle));

    switch (dam_type)
    {
        case (DAM_PIERCE):
            victim_ac = GET_AC (victim, AC_PIERCE) / 10;
            break;
        case (DAM_BASH):
            victim_ac = GET_AC (victim, AC_BASH) / 10;
            break;
        case (DAM_SLASH):
            victim_ac = GET_AC (victim, AC_SLASH) / 10;
            break;
        default:
            victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
            break;
    };

    if (victim_ac < -15)
        victim_ac = (victim_ac + 15) / 5 - 15;

    if (!can_see (ch, victim))
        victim_ac -= 4;

    if (victim->position < POS_FIGHTING)
        victim_ac += 4;

    if (victim->position < POS_RESTING)
        victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ((diceroll = number_bits (5)) >= 20);

    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {
        /* Miss. */
        damage (ch, victim, 0, dt, dam_type, TRUE, FALSE);
        tail_chain ();
        return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
        if (!ch->pIndexData->new_format)
        {
            dam = number_range (ch->level / 2, ch->level * 3 / 2);
            if (wield != NULL)
                dam += dam / 2;
        }
        else
            dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

    else
    {
        if (sn != -1)
            check_improve (ch, sn, TRUE, 5);
        if (wield != NULL)
        {
            if (wield->pIndexData->new_format)
                dam = dice (wield->value[1], wield->value[2]) * skill / 100;
            else
                dam = number_range (wield->value[1] * skill / 100,
                                    wield->value[2] * skill / 100);

            if (get_eq_char (ch, WEAR_SHIELD) == NULL)    /* no shield = more */
                dam = dam * 11 / 10;

            /* sharpness! */
            if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
            {
                int percent;

                if ((percent = number_percent ()) <= (skill / 8))
                    dam = 2 * dam + (dam * 2 * percent / 100);
            }
        }
        else
        {
            /* Ohshit skill - swing held object */
            if (!IS_NPC (ch) && sn == gsn_ohshit)
            {
                OBJ_DATA *held;
                int weight;
                int dice_num, dice_size;
                
                held = get_eq_char (ch, WEAR_HOLD);
                if (held != NULL)
                {
                    weight = get_obj_weight (held);
                    
                    /* Scale damage based on weight: 10-20 = 1d4, 450 = 3d6 */
                    if (weight <= 20)
                    {
                        dice_num = 1;
                        dice_size = 4;
                    }
                    else if (weight >= 450)
                    {
                        dice_num = 3;
                        dice_size = 6;
                    }
                    else
                    {
                        /* Interpolate between 1d4 and 3d6 */
                        /* dice_num: 1 to 3, dice_size: 4 to 6 */
                        dice_num = 1 + ((weight - 20) * 2) / 430;
                        dice_size = 4 + ((weight - 20) * 2) / 430;
                    }
                    
                    dam = dice (dice_num, dice_size) * skill / 100;
                    
                    /* Minimum damage based on level */
                    if (dam < 1 + ch->level / 10)
                        dam = 1 + ch->level / 10;
                }
                else
                {
                    /* Fallback to hand-to-hand if somehow no held object */
                    dam = number_range (1 + 4 * skill / 100,
                                       2 * ch->level / 3 * skill / 100);
                }
            }
            /* Felar (race 6) get enhanced hand-to-hand damage with their natural claws */
            else if (!IS_NPC (ch) && ch->race == 6)
            {
                /* Felar claws do damage comparable to weapons - scale better with level */
                dam = number_range (3 + 8 * skill / 100 + ch->level / 4,
                                    5 + 12 * skill / 100 + ch->level * 2 / 3);
            }
            else
            {
                dam = number_range (1 + 4 * skill / 100,
                                   2 * ch->level / 3 * skill / 100);
            }
        }
    }

    /*
     * Bonuses.
     */
    if (get_skill (ch, gsn_enhanced_damage) > 0)
    {
        diceroll = number_percent ();
        if (diceroll <= get_skill (ch, gsn_enhanced_damage))
        {
            check_improve (ch, gsn_enhanced_damage, TRUE, 6);
            dam += 2 * (dam * diceroll / 300);
        }
    }

    critical_strike = check_critical(ch,victim);
    if ( critical_strike )
        dam *= 1.40;

    if (!IS_AWAKE (victim))
        dam *= 2;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 3 / 2;

    if (dt == gsn_backstab && wield != NULL)
    {
        if (wield->value[0] != 2)
            dam *= 2 + (ch->level / 10);
        else
            dam *= 2 + (ch->level / 8);
    }

    if (dt == gsn_circle && wield != NULL)
    {
        if (wield->value[0] != 2)
           dam *= 2+ (ch->level / 10);
        else
           dam *=2 + (ch->level / 8);
    }

    dam += GET_DAMROLL (ch) * UMIN (100, skill) / 100;

    /*
     * Critical hit damage modifier for axes.
     */
    dam = axe_special (ch, victim, dam);

    /*
     * Boosted damage for spears, based on dexterity.
     */
    if (get_weapon_sn (ch) == gsn_spear
        && number_percent () < get_skill (ch, gsn_spear))
    {
        dam += get_curr_stat (ch, STAT_DEX)
                    * get_skill (ch, gsn_spear) / 100;
        check_improve (ch, gsn_spear, TRUE, 6);
    }

    if (dam <= 0)
        dam = 1;

    if ( !check_counter( ch, victim, dam, dt ) )
        result = damage( ch, victim, dam, dt, dam_type, TRUE, critical_strike );
    else 
        return;

    /*
     * Knockdown and daze effects for mace and flail.
     */
    if (result && ch->fighting == victim)
    {
        /*
         * Maces knock the opponent down and lag them for a moment.
         */
        if (get_weapon_sn (ch) == gsn_mace
            && number_percent () < get_skill (ch, gsn_mace) / 10)
        {
            WAIT_STATE (victim, PULSE_VIOLENCE*1);
            victim->position = POS_RESTING;

            act ("Your blow knocks $N to the ground.",
                    ch, NULL, victim, TO_CHAR);
            act ("$n knocks you to the ground.",
                    ch, NULL, victim, TO_VICT);
            
            check_improve (ch, gsn_mace, TRUE, 4);
        }

        /*
         * Flails daze the opponent and screw up their casting.
         */
        if (get_weapon_sn (ch) == gsn_flail
            && number_percent () < get_skill (ch, gsn_flail) / 5)
        {
            DAZE_STATE (victim, PULSE_VIOLENCE*4);
            check_improve (ch, gsn_flail, TRUE, 4);
        }
    }

    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    {
        int dam;

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
        {
            int level;
            AFFECT_DATA *poison, af;

            if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;

            if (!saves_spell (level / 2, victim, DAM_POISON))
            {
                send_to_char ("You feel poison coursing through your veins.",
                              victim);
                act ("$n is poisoned by the venom on $p.",
                     victim, wield, NULL, TO_ROOM);

                af.where = TO_AFFECTS;
                af.type = gsn_poison;
                af.level = level * 3 / 4;
                af.duration = level / 2;
                af.location = APPLY_STR;
                af.modifier = -1;
                af.bitvector = AFF_POISON;
                affect_join (victim, &af);
            }

            /* weaken the poison if it's temporary */
            if (poison != NULL)
            {
                poison->level = UMAX (0, poison->level - 2);
                poison->duration = UMAX (0, poison->duration - 1);

                if (poison->level == 0 || poison->duration == 0)
                    act ("The poison on $p has worn off.", ch, wield, NULL,
                         TO_CHAR);
            }
        }


        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC))
        {
            dam = number_range (1, wield->level / 5 + 1);
            act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
            act ("You feel $p drawing your life away.",
                 victim, wield, NULL, TO_CHAR);
            damage (ch, victim, dam, 0, DAM_NEGATIVE, FALSE, FALSE);
            ch->alignment = UMAX (-1000, ch->alignment - 1);
            ch->hit += dam / 2;
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
        {
            dam = number_range (1, wield->level / 4 + 1);
            act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
            act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
            fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_FIRE, FALSE, FALSE);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
        {
            dam = number_range (1, wield->level / 6 + 2);
            act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
            act ("The cold touch of $p surrounds you with ice.",
                 victim, wield, NULL, TO_CHAR);
            cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_COLD, FALSE, FALSE);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_SHOCKING))
        {
            dam = number_range (1, wield->level / 5 + 2);
            act ("$n is struck by lightning from $p.", victim, wield, NULL,
                 TO_ROOM);
            act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
            shock_effect (victim, wield->level / 2, dam, TARGET_CHAR);
            damage (ch, victim, dam, 0, DAM_LIGHTNING, FALSE, FALSE);
        }
    }

    if (result && wield != NULL && wield->item_type == ITEM_WEAPON)
    {
        int wear = 1;

        if (dam > 60)
            wear++;
        if (dam > 120)
            wear++;

        damage_item_condition(wield, wear, ch);
    }

    /* Vorpal weapon code by Kyle Boyd (boyd1@proaxis.com) */
    /* To change probability of decapitation, tweak the number_range arguments */
    if (wield != NULL && IS_WEAPON_STAT (wield, WEAPON_VORPAL)
        && number_range (1, UMAX (50, 400 + MAX_LEVEL - (2 * ch->level) +
                                  victim->level)) == 1
        && IS_SET (victim->parts, PART_HEAD)
        && check_immune (victim, dam_type) != IS_IMMUNE
        && !IS_IMMORTAL (victim)
        && ch != victim
        && !is_safe (ch, victim))
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;
        int parts_buf;

        /* used a modified version of death_cry to make a severed head */

        name = IS_NPC (victim) ? victim->short_descr : victim->name;
        obj = create_object (get_obj_index (OBJ_VNUM_SEVERED_HEAD), 0);
        obj->timer = number_range (20, 30);

        sprintf (buf, obj->short_descr, name);
        free_string (obj->short_descr);
        obj->short_descr = str_dup (buf);

        sprintf (buf, obj->description, name);
        free_string (obj->description);
        obj->description = str_dup (buf);

        /* yummy... severed head for dinner! */

        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET (victim->form, FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET (victim->form, FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room (obj, ch->in_room);

        /* change this if you want a more or less gory death message! */
        act ("$n's head is severed from $s body by $p!", victim, wield, NULL,
             TO_ROOM);
        act ("Your last feeling before you die is $p's weapon severing your head!",
             victim, ch, NULL, TO_CHAR);

        /* parts_buf is used to remove all parts of the body so the death_cry 
           function does not create any other body parts */

        parts_buf = victim->parts;
        victim->parts = 0;
        vorpal_kill (ch, victim, dam, dt, dam_type);
        victim->parts = parts_buf;
        return;
    }

    tail_chain ();
    return;
}


/*
 * Inflict damage from a hit.
 */
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
             int dam_type, bool show, bool critical)
{
    OBJ_DATA *corpse;
    bool immune;

    if (victim->position == POS_DEAD)
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if (dam > 1200 && dt >= TYPE_HIT)
    {
        bug ("Damage: %d: more than 1200 points!", dam);
        dam = 1200;
        if (!IS_IMMORTAL (ch))
        {
            OBJ_DATA *obj;
            obj = get_eq_char (ch, WEAR_WIELD);
            send_to_char ("You really shouldn't cheat.\n\r", ch);
            if (obj != NULL)
                extract_obj (obj);
        }

    }


    /* damage reduction */
    if (dam > 35)
        dam = (dam - 35) / 2 + 35;
    if (dam > 80)
        dam = (dam - 80) / 2 + 80;




    if (victim != ch)
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if (is_safe (ch, victim))
            return FALSE;
        check_killer (ch, victim);

        if (victim->position > POS_STUNNED)
        {
            if (victim->fighting == NULL)
            {
                set_fighting (victim, ch);
                if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_KILL))
                    mp_percent_trigger (victim, ch, NULL, NULL, TRIG_KILL);
            }
            if (victim->timer <= 4)
                victim->position = POS_FIGHTING;
        }

        if (victim->position > POS_STUNNED)
        {
            if (ch->fighting == NULL)
                set_fighting (ch, victim);
        }

        /*
         * More charm stuff.
         */
        if (victim->master == ch)
            stop_follower (victim);
    }

    /*
     * Inviso attacks ... not.
     */
    if (IS_AFFECTED (ch, AFF_INVISIBLE))
    {
        affect_strip (ch, gsn_invis);
        affect_strip (ch, gsn_mass_invis);
        STR_REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
        act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

    /*
     * Damage modifiers.
     */
    if (victim->hate)
    {
        free_string(victim->hate->name);
        victim->hate->name = str_dup(ch->name);
        victim->hate->who = ch;
    }
    else
        start_hating(victim,ch);

    if (dam > 1 && !IS_NPC (victim)
        && victim->pcdata->condition[COND_DRUNK] > 10)
        dam = 9 * dam / 10;

    if (dam > 1 && IS_AFFECTED (victim, AFF_SANCTUARY))
        dam /= 2;

    if (dam > 1 && ((IS_AFFECTED (victim, AFF_PROTECT_EVIL) && IS_EVIL (ch))
                    || (IS_AFFECTED (victim, AFF_PROTECT_GOOD)
                        && IS_GOOD (ch))))
        dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if (dt >= TYPE_HIT && ch != victim)
    {
        if (check_parry (ch, victim))
            return FALSE;
        if (check_dodge (ch, victim))
            return FALSE;
        if (check_shield_block (ch, victim))
            return FALSE;
        if ( check_phase( ch, victim) )
            return FALSE;
        if (pole_special (ch, victim))
            return FALSE;

    }

    switch (check_immune (victim, dam_type))
    {
        case (IS_IMMUNE):
            immune = TRUE;
            dam = 0;
            break;
        case (IS_RESISTANT):
            dam -= dam / 3;
            break;
        case (IS_VULNERABLE):
            dam += dam / 2;
            break;
    }

    if (show)
    {
        /* Special message for ohshit skill with held object */
        if (dt == TYPE_HIT + gsn_ohshit && !IS_NPC(ch))
        {
            OBJ_DATA *held = get_eq_char(ch, WEAR_HOLD);
            if (held != NULL)
            {
                act ("Your wild swing with $p connects!", ch, held, victim, TO_CHAR);
                act ("$n's wild swing with $p connects!", ch, held, victim, TO_NOTVICT);
                act ("$n's wild swing with $p connects!", ch, held, victim, TO_VICT);
            }
        }
        
        dam_message (ch, victim, dam, dt, immune, critical);
    }

    if (dam == 0)
        return FALSE;

    if (dam > 0 && dt >= TYPE_HIT && !IS_NPC(victim))
        apply_armor_wear(victim, dam);

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if (!IS_NPC (victim)
        && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
        victim->hit = 1;
    update_pos (victim);

    switch (victim->position)
    {
        case POS_MORTAL:
            act ("$n is mortally wounded, and will die soon, if not aided.",
                 victim, NULL, NULL, TO_ROOM);
            send_to_char
                ("You are mortally wounded, and will die soon, if not aided.\n\r",
                 victim);
            break;

        case POS_INCAP:
            act ("$n is incapacitated and will slowly die, if not aided.",
                 victim, NULL, NULL, TO_ROOM);
            send_to_char
                ("You are incapacitated and will slowly die, if not aided.\n\r",
                 victim);
            break;

        case POS_STUNNED:
            act ("$n is stunned, but will probably recover.",
                 victim, NULL, NULL, TO_ROOM);
            send_to_char ("You are stunned, but will probably recover.\n\r",
                          victim);
            break;

        case POS_DEAD:
            act ("{R$n is DEAD!!{x", victim, 0, 0, TO_ROOM);
            send_to_char ("{RYou have been KILLED!!{x\n\r\n\r", victim);
            break;

        default:
            if (dam > victim->max_hit / 4)
                send_to_char ("{RThat really did HURT!{x\n\r", victim);
            if (victim->hit < victim->max_hit / 4)
                send_to_char ("{RYou sure are BLEEDING!{x\n\r", victim);
                gain_condition( victim, COND_BLEEDING, 1 );
            break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE (victim))
        stop_fighting (victim, FALSE);

    /*
     * Payoff for killing things.
     */
    if (victim->position == POS_DEAD)
    {
        group_gain (ch, victim);

        if (!IS_NPC (victim))
        {
            sprintf (log_buf, "%s killed by %s at %d",
                     victim->name,
                     (IS_NPC (ch) ? ch->short_descr : ch->name),
                     ch->in_room->vnum);
            log_string (log_buf);

            /*
             * Dying penalty:
             * 2/3 way back to previous level.
             */
            if (victim->exp > exp_per_level (victim, victim->pcdata->points)
                * victim->level)
                gain_exp (victim,
                          (2 *
                           (exp_per_level (victim, victim->pcdata->points) *
                            victim->level - victim->exp) / 3) + 50);
        }

        sprintf (log_buf, "%s got toasted by %s at %s [room %d]",
                 (IS_NPC (victim) ? victim->short_descr : victim->name),
                 (IS_NPC (ch) ? ch->short_descr : ch->name),
                 ch->in_room->name, ch->in_room->vnum);

        if (IS_NPC (victim))
            wiznet (log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
        else
            wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

        /*
         * Death trigger
         */
        if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_DEATH))
        {
            victim->position = POS_STANDING;
            mp_percent_trigger (victim, ch, NULL, NULL, TRIG_DEATH);
        }

        /* Make the head for a bountied PC */
        if(!IS_NPC(victim) && !IS_NPC(ch)
        && is_bountied(victim))
        {
            char buf[MAX_STRING_LENGTH];
            char *name;
            OBJ_DATA *obj;

            name = str_dup(victim->name);
            obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD),0);

            obj->extra_flags |= ITEM_NODROP|ITEM_NOUNCURSE;

            sprintf(buf, "%s %s", "head", name);
            free_string( obj->name );
            obj->name = str_dup( buf);

            sprintf( buf, obj->short_descr, name );
            free_string( obj->short_descr );
            obj->short_descr = str_dup( buf );

            sprintf( buf, obj->description, name );
            free_string( obj->description );
            obj->description = str_dup( buf );

            obj_to_char(obj,ch);
            free_string(name);
        }

        raw_kill (ch, victim);
        /* dump the flags */
        if (ch != victim && !IS_NPC (ch) && !is_same_clan (ch, victim))
        {
            if (IS_SET (victim->act, PLR_KILLER))
                REMOVE_BIT (victim->act, PLR_KILLER);
            else
                REMOVE_BIT (victim->act, PLR_THIEF);
        }

        /* RT new auto commands */

        if (!IS_NPC (ch)
            && (corpse =
                get_obj_list (ch, "corpse", ch->in_room->contents)) != NULL
            && corpse->item_type == ITEM_CORPSE_NPC
            && can_see_obj (ch, corpse))
        {
            OBJ_DATA *coins;

            corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

            if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)
            {                    /* exists and not empty */
                do_function (ch, &do_get, "all corpse");
            }

            if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&    /* exists and not empty */
                !IS_SET (ch->act, PLR_AUTOLOOT))
            {
                if ((coins = get_obj_list (ch, "gcash", corpse->contains))
                    != NULL)
                {
                    do_function (ch, &do_get, "all.gcash corpse");
                }
            }

            if (IS_SET (ch->act, PLR_AUTOSAC))
            {
                if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
                    && corpse->contains)
                {
                    return TRUE;    /* leave if corpse has treasure */
                }
                else
                {
                    do_function (ch, &do_sacrifice, "corpse");
                }
            }
        }

        return TRUE;
    }

    if (victim == ch)
        return TRUE;

    /*
     * Take care of link dead people.
     */
    if (!IS_NPC (victim) && victim->desc == NULL)
    {
        if (number_range (0, victim->wait) == 0)
        {
            do_function (victim, &do_recall, "");
            return TRUE;
        }
    }

    /*
     * Wimp out?
     */
    if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
        if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
             && victim->hit < victim->max_hit / 5)
            || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
                && victim->master->in_room != victim->in_room))
        {
            do_function (victim, &do_flee, "");
        }
    }

    if (!IS_NPC (victim)
        && victim->hit > 0
        && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
    {
        do_function (victim, &do_flee, "");
    }

    tail_chain ();
    return TRUE;
}

bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim->fighting == ch || victim == ch)
        return FALSE;

    if (IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL)
        return FALSE;

    /* killing mobiles */
    if (IS_NPC (victim))
    {

        /* safe room? */
        if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
        {
            send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
        }

        if (victim->pIndexData->pShop != NULL)
        {
            send_to_char ("The shopkeeper wouldn't like that.\n\r", ch);
            return TRUE;
        }

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
            || IS_SET (victim->act, ACT_PRACTICE)
            || IS_SET (victim->act, ACT_IS_HEALER)
            || IS_SET (victim->act, ACT_IS_CHANGER))
        {
            send_to_char ("I don't think Om would approve.\n\r", ch);
            return TRUE;
        }

        if (!IS_NPC (ch))
        {
            /* no pets */
            if (IS_SET (victim->act, ACT_PET))
            {
                act ("But $N looks so cute and cuddly...",
                     ch, NULL, victim, TO_CHAR);
                return TRUE;
            }

            /* no charmed creatures unless owner */
            if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master)
            {
                send_to_char ("You don't own that monster.\n\r", ch);
                return TRUE;
            }
        }
    }
    /* killing players */
    else
    {
        /* Arena rooms allow all PvP - no restrictions */
        if (victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_ARENA))
        {
            return FALSE;  /* Allow PvP in arena */
        }
        
        /* NPC doing the killing */
        if (IS_NPC (ch))
        {
            /* safe room check */
            if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
            {
                send_to_char ("Not in this room.\n\r", ch);
                return TRUE;
            }

            /* charmed mobs and pets cannot attack players while owned */
            if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
                && ch->master->fighting != victim)
            {
                send_to_char ("Players are your friends!\n\r", ch);
                return TRUE;
            }
        }
        /* player doing the killing */
        else
        {
            if (!is_clan (ch))
            {
                send_to_char ("Join a clan if you want to kill players.\n\r",
                              ch);
                return TRUE;
            }

            if (IS_SET (victim->act, PLR_KILLER)
                || IS_SET (victim->act, PLR_THIEF))
                return FALSE;

            if (!is_clan (victim))
            {
                send_to_char ("They aren't in a clan, leave them alone.\n\r",
                              ch);
                return TRUE;
            }

            if (ch->level > victim->level + 8)
            {
                send_to_char ("Pick on someone your own size.\n\r", ch);
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
        return TRUE;

    if (victim->fighting == ch || victim == ch)
        return FALSE;

    if (IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL && !area)
        return FALSE;

    /* killing mobiles */
    if (IS_NPC (victim))
    {
        /* safe room? */
        if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
            return TRUE;

        if (victim->pIndexData->pShop != NULL)
            return TRUE;

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
            || IS_SET (victim->act, ACT_PRACTICE)
            || IS_SET (victim->act, ACT_IS_HEALER)
            || IS_SET (victim->act, ACT_IS_CHANGER))
            return TRUE;

        if (!IS_NPC (ch))
        {
            /* no pets */
            if (IS_SET (victim->act, ACT_PET))
                return TRUE;

            /* no charmed creatures unless owner */
            if (IS_AFFECTED (victim, AFF_CHARM)
                && (area || ch != victim->master))
                return TRUE;

            /* legal kill? -- cannot hit mob fighting non-group member */
            if (victim->fighting != NULL
                && !is_same_group (ch, victim->fighting)) return TRUE;
        }
        else
        {
            /* area effect spells do not hit other mobs */
            if (area && !is_same_group (victim, ch->fighting))
                return TRUE;
        }
    }
    /* killing players */
    else
    {
        /* Arena rooms allow all PvP - no restrictions */
        if (victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_ARENA))
        {
            return FALSE;  /* Allow PvP spells in arena */
        }
        
        if (area && IS_IMMORTAL (victim) && victim->level > LEVEL_IMMORTAL)
            return TRUE;

        /* NPC doing the killing */
        if (IS_NPC (ch))
        {
            /* charmed mobs and pets cannot attack players while owned */
            if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
                && ch->master->fighting != victim)
                return TRUE;

            /* safe room? */
            if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
                return TRUE;

            /* legal kill? -- mobs only hit players grouped with opponent */
            if (ch->fighting != NULL && !is_same_group (ch->fighting, victim))
                return TRUE;
        }

        /* player doing the killing */
        else
        {
            if (!is_clan (ch))
                return TRUE;

            if (IS_SET (victim->act, PLR_KILLER)
                || IS_SET (victim->act, PLR_THIEF))
                return FALSE;

            if (!is_clan (victim))
                return TRUE;

            if (ch->level > victim->level + 8)
                return TRUE;
        }

    }
    return FALSE;
}

static OBJ_DATA *select_random_armor_piece (CHAR_DATA *victim)
{
    OBJ_DATA *choices[MAX_WEAR];
    int count = 0;
    int i;

    for (i = 0; i < MAX_WEAR; i++)
    {
        OBJ_DATA *obj;

        if (i == WEAR_WIELD || i == WEAR_SECONDARY || i == WEAR_HOLD
            || i == WEAR_LIGHT || i == WEAR_COMM)
            continue;

        obj = get_eq_char(victim, i);

        if (obj == NULL)
            continue;

        if (obj->condition <= 0)
            continue;

        if (obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_CLOTHING)
            continue;

        choices[count++] = obj;
    }

    if (count == 0)
        return NULL;

    return choices[number_range(0, count - 1)];
}


static void apply_armor_wear (CHAR_DATA *victim, int dam)
{
    OBJ_DATA *armor;
    int wear;

    armor = select_random_armor_piece(victim);

    if (armor == NULL)
        return;

    wear = 1;

    if (dam > 60)
        wear++;
    if (dam > 120)
        wear++;

    damage_item_condition(armor, wear, victim);
}


/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer (CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MAX_STRING_LENGTH];
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL)
        victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if (IS_NPC (victim)
        || IS_SET (victim->act, PLR_KILLER)
        || IS_SET (victim->act, PLR_THIEF))
        return;

    /*
     * Charm-o-rama.
     */
    if (STR_IS_SET (ch->affected_by, AFF_CHARM))
    {
        if (ch->master == NULL)
        {
            char buf[MAX_STRING_LENGTH];

            sprintf (buf, "Check_killer: %s bad AFF_CHARM",
                     IS_NPC (ch) ? ch->short_descr : ch->name);
            bug (buf, 0);
            affect_strip (ch, gsn_charm_person);
            STR_REMOVE_BIT (ch->affected_by, AFF_CHARM);
            return;
        }
/*
    send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
      SET_BIT(ch->master->act, PLR_KILLER);
*/

        stop_follower (ch);
        return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if (IS_NPC (ch)
        || ch == victim || ch->level >= LEVEL_IMMORTAL || !is_clan (ch)
        || IS_SET (ch->act, PLR_KILLER) || ch->fighting == victim)
        return;

    send_to_char ("*** You are now a KILLER!! ***\n\r", ch);
    SET_BIT (ch->act, PLR_KILLER);
    sprintf (buf, "$N is attempting to murder %s", victim->name);
    wiznet (buf, ch, NULL, WIZ_FLAGS, 0, 0);
    save_char_obj (ch);
    return;
}



/*
 * Check for parry.
 */
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;
    int attacker_weapon_sn;
    int defender_weapon_sn;
    int attacker_weapon_skill;
    int defender_weapon_skill;
    int weapon_knowledge_bonus;
    int own_weapon_bonus;
    OBJ_DATA *wield;

    if (!IS_AWAKE (victim))
        return FALSE;

    chance = get_skill (victim, gsn_parry) / 2;

    wield = get_eq_char (victim, WEAR_WIELD);
    if (wield == NULL)
    {
        /* Felar (race 6) can parry with their long claws */
        if (!IS_NPC (victim) && victim->race == 6)
        {
            /* Felar can parry without a weapon */
            defender_weapon_sn = gsn_hand_to_hand;
        }
        else if (IS_NPC (victim))
        {
            chance /= 2;
            defender_weapon_sn = gsn_hand_to_hand;
        }
        else
            return FALSE;
    }
    else
    {
        /* Get the defender's weapon skill */
        defender_weapon_sn = get_weapon_sn (victim);
    }

    /* Get the attacker's weapon type */
    attacker_weapon_sn = get_weapon_sn (ch);

    /* Calculate defender's weapon skill with their own weapon */
    defender_weapon_skill = get_weapon_skill (victim, defender_weapon_sn);

    /*
     * Bonus for knowing the attacker's weapon type.
     * If you're an expert with swords, you're better at parrying sword attacks
     * because you know exactly what attack patterns to expect.
     * Example: If attacker uses a sword and you have high sword skill,
     * you can better predict and parry their sword attacks.
     */
    if (attacker_weapon_sn != -1 && attacker_weapon_sn != gsn_hand_to_hand)
    {
        /* Get defender's skill with the attacker's weapon type */
        attacker_weapon_skill = get_weapon_skill (victim, attacker_weapon_sn);
        /* Add up to 20% bonus based on knowledge of attacker's weapon type */
        weapon_knowledge_bonus = attacker_weapon_skill / 5;
        chance += weapon_knowledge_bonus;
    }

    /*
     * Bonus for your own weapon skill.
     * If you're wielding a mace and are an expert mace fighter,
     * you're better at parrying because you know how to use your weapon effectively.
     * For felar, this represents their skill at using their claws defensively.
     */
    if (defender_weapon_sn != -1)
    {
        /* Add up to 15% bonus based on your weapon skill */
        own_weapon_bonus = defender_weapon_skill / 7;
        chance += own_weapon_bonus;
    }

    if (!can_see (ch, victim))
        chance /= 2;

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You parry $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N parries your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_parry, TRUE, 6);
    
    /* 
     * Whip innate ability.
     */
    if (get_weapon_sn (victim) == gsn_whip)
    {
        chance = get_skill (victim, gsn_whip) / 7;
        
        if (number_percent () < chance)
        {
            disarm (victim, ch);
            check_improve (victim, gsn_whip, TRUE, 3);
        }
    }
    else if (get_weapon_sn (ch) == gsn_whip)
    {
        chance = get_skill (ch, gsn_whip) / 8;

        if (number_percent () < chance)
        {
            disarm (ch, victim);
            check_improve (ch, gsn_whip, TRUE, 3);
        }
    }
    
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;


    chance = get_skill (victim, gsn_shield_block) / 5 + 3;


    if (get_eq_char (victim, WEAR_SHIELD) == NULL)
        return FALSE;

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You block $n's attack with your shield.", ch, NULL, victim,
         TO_VICT);
    act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_shield_block, TRUE, 6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;

    if ( MOUNTED(victim) )
        return FALSE;

    chance = get_skill (victim, gsn_dodge) / 2;

    if (!can_see (victim, ch))
        chance /= 2;

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
    act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
    check_improve (victim, gsn_dodge, TRUE, 6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos (CHAR_DATA * victim)
{
    if (victim->hit > 0)
    {
        if (victim->position <= POS_STUNNED)
            victim->position = POS_STANDING;
        return;
    }

    if (IS_NPC (victim) && victim->hit < 1)
    {
        victim->position = POS_DEAD;
        return;
    }

    if (victim->hit <= -11)
    {
        victim->position = POS_DEAD;
        return;
    }

    if (victim->hit <= -6)
        victim->position = POS_MORTAL;
    else if (victim->hit <= -3)
        victim->position = POS_INCAP;
    else
        victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (ch->fighting != NULL)
    {
        bug ("Set_fighting: already fighting", 0);
        return;
    }

    if (IS_AFFECTED (ch, AFF_SLEEP))
        affect_strip (ch, gsn_sleep);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    /* Track PK attempts - when a player attacks another player (not in arena) */
    if (!IS_NPC(ch) && !IS_NPC(victim) && ch != victim)
    {
        if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_ARENA))
        {
            ch->pkattempt++;
        }
    }

    return;
}



/*
 * Stop fights.
 */
void stop_fighting (CHAR_DATA * ch, bool fBoth)
{
    CHAR_DATA *fch;

    for (fch = char_list; fch != NULL; fch = fch->next)
    {
        if (fch == ch || (fBoth && fch->fighting == ch))
        {
            fch->fighting = NULL;
            fch->position = IS_NPC (fch) ? fch->default_pos : POS_STANDING;
            update_pos (fch);
        }
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse (CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
  ROOM_INDEX_DATA *location;
    location = get_room_index ( ROOM_VNUM_MORGUE );


    if (IS_NPC (ch))
    {
        name = ch->short_descr;
        corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
        corpse->timer = number_range (3, 6);
        if (ch->gold > 0)
        {
            obj_to_obj (create_money (ch->gold, ch->silver), corpse);
            ch->gold = 0;
            ch->silver = 0;
        }
        corpse->cost = 0;
    }
    else
    {
        name = ch->name;
        corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer = number_range (25, 40);
        REMOVE_BIT (ch->act, PLR_CANLOOT);
        if (!is_clan (ch))
            corpse->owner = str_dup (ch->name);
        else
        {
            corpse->owner = NULL;
            if (ch->gold > 1 || ch->silver > 1)
            {
                obj_to_obj (create_money (ch->gold / 2, ch->silver / 2),
                            corpse);
                ch->gold -= ch->gold / 2;
                ch->silver -= ch->silver / 2;
            }
        }

        corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf (buf, corpse->short_descr, name);
    free_string (corpse->short_descr);
    corpse->short_descr = str_dup (buf);

    sprintf (buf, corpse->description, name);
    free_string (corpse->description);
    corpse->description = str_dup (buf);

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
        bool floating = FALSE;

        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_FLOAT)
            floating = TRUE;
        obj_from_char (obj);
        if (obj->item_type == ITEM_POTION)
            obj->timer = number_range (500, 1000);
        if (obj->item_type == ITEM_SCROLL)
            obj->timer = number_range (1000, 2500);
        if (obj->item_type == ITEM_MANUAL)
            obj->timer = number_range (1000, 2500);
        if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH) && !floating)
        {
            obj->timer = number_range (5, 10);
            REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
        }
        REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

        if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
            extract_obj (obj);
        else if (floating)
        {
            if (IS_OBJ_STAT (obj, ITEM_ROT_DEATH))
            {                    /* get rid of it! */
                if (obj->contains != NULL)
                {
                    OBJ_DATA *in, *in_next;

                    act ("$p evaporates,scattering its contents.",
                         ch, obj, NULL, TO_ROOM);
                    for (in = obj->contains; in != NULL; in = in_next)
                    {
                        in_next = in->next_content;
                        obj_from_obj (in);
                        obj_to_room (in, ch->in_room);
                    }
                }
                else
                    act ("$p evaporates.", ch, obj, NULL, TO_ROOM);
                extract_obj (obj);
            }
            else
            {
                act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
                obj_to_room (obj, ch->in_room);
            }
        }
        else
            obj_to_obj (obj, corpse);
    }

//    obj_to_room (corpse, ch->in_room);
      if (IS_NPC(ch))
    obj_to_room( corpse, ch->in_room );
    else
    obj_to_room (corpse,location);
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry (CHAR_DATA * ch)
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch (number_bits (4))
    {
        case 0:
            msg = "$n hits the ground ... DEAD.";
            break;
        case 1:
            if (ch->material == 0)
            {
                msg = "$n splatters blood on your armor.";
                break;
            }
        case 2:
            if (IS_SET (ch->parts, PART_GUTS))
            {
                msg = "$n spills $s guts all over the floor.";
                vnum = OBJ_VNUM_GUTS;
            }
            break;
        case 3:
            if (IS_SET (ch->parts, PART_HEAD))
            {
                msg = "$n's severed head plops on the ground.";
                vnum = OBJ_VNUM_SEVERED_HEAD;
            }
            break;
        case 4:
            if (IS_SET (ch->parts, PART_HEART))
            {
                msg = "$n's heart is torn from $s chest.";
                vnum = OBJ_VNUM_TORN_HEART;
            }
            break;
        case 5:
            if (IS_SET (ch->parts, PART_ARMS))
            {
                msg = "$n's arm is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_ARM;
            }
            break;
        case 6:
            if (IS_SET (ch->parts, PART_LEGS))
            {
                msg = "$n's leg is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_LEG;
            }
            break;
        case 7:
            if (IS_SET (ch->parts, PART_BRAINS))
            {
                msg =
                    "$n's head is shattered, and $s brains splash all over you.";
                vnum = OBJ_VNUM_BRAINS;
            }
    }

    act (msg, ch, NULL, NULL, TO_ROOM);

    if (vnum != 0)
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;

        name = IS_NPC (ch) ? ch->short_descr : ch->name;
        obj = create_object (get_obj_index (vnum), 0);
        obj->timer = number_range (4, 7);

        sprintf (buf, obj->short_descr, name);
        free_string (obj->short_descr);
        obj->short_descr = str_dup (buf);

        sprintf (buf, obj->description, name);
        free_string (obj->description);
        obj->description = str_dup (buf);

        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET (ch->form, FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET (ch->form, FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room (obj, ch->in_room);
    }

    if (IS_NPC (ch))
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++)
    {
        EXIT_DATA *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL
            && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
        {
            ch->in_room = pexit->u1.to_room;
            act (msg, ch, NULL, NULL, TO_ROOM);
        }
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int i;
    bool arena_death = FALSE;
    ROOM_INDEX_DATA *arena_start;

    stop_fighting (victim, TRUE);
    
    /* Check if this is an arena death */
    if (!IS_NPC(victim) && victim->in_room && 
        IS_SET(victim->in_room->room_flags, ROOM_ARENA))
    {
        arena_death = TRUE;
    }
    
    /* Track kills and deaths */
    if (ch && victim)
    {
        if (arena_death && !IS_NPC(victim) && !IS_NPC(ch) && ch != victim)
        {
            /* Track arena kills/deaths separately */
            ch->arenakill++;
            victim->arenadeath++;
        }
        else if (!arena_death)
        {
            /* Normal kill/death tracking */
            if (IS_NPC(victim) && !IS_NPC(ch)) 
                ch->mkill++;
            if (!IS_NPC(victim) && IS_NPC(ch)) 
                victim->mdeath++;
            if (!IS_NPC(victim) && !IS_NPC(ch) && ch != victim)
            {
                ch->pkill++;
                victim->pdeath++;
            }
        }
    }
    
    /* Handle arena deaths */
    if (arena_death)
    {
        /* Clear all affects */
        while (victim->affected)
            affect_remove (victim, victim->affected);
        STR_COPY_STR( victim->affected_by, race_table[victim->race].aff, AFF_FLAGS );
        
        /* Restore to full health */
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;
        victim->position = POS_STANDING;
        
        /* Send messages */
        act ("{RYou have been defeated in the arena!{x", victim, NULL, NULL, TO_CHAR);
        act ("{R$n has been defeated in the arena!{x", victim, NULL, NULL, TO_ROOM);
        
        /* Teleport to arena start room */
        arena_start = get_room_index(ROOM_VNUM_ARENA_START);
        if (arena_start != NULL)
        {
            char_from_room(victim);
            char_to_room(victim, arena_start);
            act ("{Y$n appears in a flash of light, looking defeated.{x", victim, NULL, NULL, TO_ROOM);
            do_function(victim, &do_look, "auto");
        }
        else
        {
            bug("raw_kill: ROOM_VNUM_ARENA_START (%d) does not exist!", ROOM_VNUM_ARENA_START);
        }
        
        return;
    }
    
    /* Normal death processing */
    death_cry (victim);
    make_corpse (victim);

    if (IS_NPC (victim))
    {
        victim->pIndexData->killed++;
        kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
        extract_char (victim, TRUE);
        return;
    }

    extract_char (victim, FALSE);
    while (victim->affected)
        affect_remove (victim, victim->affected);
    STR_COPY_STR( victim->affected_by, race_table[victim->race].aff, AFF_FLAGS );
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    victim->position = POS_RESTING;
    victim->hit = UMAX (1, victim->hit);
    victim->mana = UMAX (1, victim->mana);
    victim->move = UMAX (1, victim->move);
/*  save_char_obj( victim ); we're stable enough to not need this :) */
    return;
}



void group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim))
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if (victim == ch)
        return;

    members = 0;
    group_levels = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (is_same_group (gch, ch))
        {
            members++;
            group_levels += IS_NPC (gch) ? gch->level / 2 : gch->level;
        }
    }

    if (members == 0)
    {
        bug ("Group_gain: members.", members);
        members = 1;
        group_levels = ch->level;
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;

        if (!is_same_group (gch, ch) || IS_NPC (gch))
            continue;

/*    Taken out, add it back if you want it
    if ( gch->level - lch->level >= 5 )
    {
        send_to_char( "You are too high for this group.\n\r", gch );
        continue;
    }

    if ( gch->level - lch->level <= -5 )
    {
        send_to_char( "You are too low for this group.\n\r", gch );
        continue;
    }
*/

        xp = xp_compute (gch, victim, group_levels);
        
        if(IS_SET(gch->act, PLR_NOEXP))
        {
            send_to_char("You have NOEXP turned on! You gained no exp!\n\r", gch);
            xp = 0;
        }

        /* Quest completion check */
        if (!IS_NPC(gch) && IS_SET(gch->act,PLR_QUESTING )
            && IS_NPC(victim))
        {
            if (gch->pcdata->questmob == victim->pIndexData->vnum)
            {
               send_to_char("You have almost completed your quest!\n\r",gch);
        send_to_char(
        "Return to the questmaster before your time runs out.\n\r",gch);
                gch->pcdata->questmob = -1;
            }
        }
        
        if(xp > 0)
        {
            sprintf (buf, "You receive %d experience points.\n\r", xp);
            send_to_char (buf, gch);
            gain_exp (gch, xp);
        }

        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE)
                continue;

            if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch))
                || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch))
                || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
            {
                act ("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
                act ("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
            }
        }

        /* Give experience to relic items */
        for (obj = gch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE)
                continue;
            
            if (IS_OBJ_STAT(obj, ITEM_RELIC) && obj->xp_tolevel > 0)
            {
                gain_object_exp(gch, obj, xp);
            }
        }
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range;
    int change;
    int time_per_level;

    level_range = victim->level - gch->level;
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    } 
    
    if (level_range > 4)
	base_exp = 160 + 20 * (level_range - 4);

    /* jerome : base xp addition ---------------------- */

      if(IS_NPC(victim) )/* at max a mob with all worth 5 level above his own*/
       {
         if( is_affected( victim, skill_lookup("sanctuary") ) )
            base_exp = (base_exp * 130) / 100;
         if( is_affected( victim, skill_lookup("haste") ) )
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_AREA_ATTACK) ) 
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_BACKSTAB) ) 
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_FAST) )
            base_exp = (base_exp * 120) / 100;
         if( IS_SET(victim->off_flags,OFF_DODGE) )
            base_exp = (base_exp * 110) / 100;
         if( IS_SET(victim->off_flags,OFF_PARRY) )
            base_exp = (base_exp * 110) / 100;

         if( victim->spec_fun != 0 )
          {
           if(   !str_cmp(spec_name(victim->spec_fun),"spec_breath_any")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_acid")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_fire")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_frost")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_gas")
              || !str_cmp(spec_name(victim->spec_fun),"spec_breath_lightning")
             )
             base_exp = (base_exp * 125) / 100;
           
           else if(   !str_cmp(spec_name(victim->spec_fun),"spec_cast_cleric")
                   || !str_cmp(spec_name(victim->spec_fun),"spec_cast_mage")
                   || !str_cmp(spec_name(victim->spec_fun),"spec_cast_undead")
                  ) 
                base_exp = (base_exp * 120) / 100;
         
           else if( !str_cmp(spec_name(victim->spec_fun),"spec_poison") )
                base_exp = (base_exp * 110) / 100;
          }
       }
    /*    back to normal code    -------------------- */

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
	gch->alignment -= change;
    }
    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

        else if (victim->alignment > 250)
	    xp = (base_exp * 3)/4; 

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 6)
    	xp = 10 * xp / (gch->level + 4);

    /* less at high */
    if (gch->level > 35 )
	xp =  15 * xp / (gch->level - 25 );

    /* reduce for playing time */
    
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)
                          /* make it a curve */
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 12;
    }
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    xp = xp * gch->level/( UMAX(1,total_levels -1) );

    return xp;
}


void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  bool immune, bool critical)
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack = "attack";
    char punct;
    OBJ_DATA *crit_wield = NULL;

    if (ch == NULL || victim == NULL)
        return;

    int pain_percent;
    pain_percent = 1000 * dam/victim->max_hit;
         
         if ( dam  ==   0 ) { vs = "miss";	vp = "misses";		}
        else if ( pain_percent <=   5 ) { vs = "{gscratch";	vp = "{gscratches";	}
        else if ( pain_percent <=  10 ) { vs = "{Ggraze";	vp = "{Ggrazes";		}
        else if ( pain_percent <=  25 ) { vs = "{ghit";	vp = "{ghits";		}
        else if ( pain_percent <=  30 ) { vs = "{cinjure";	vp = "{cinjures";		}
        else if ( pain_percent <=  40 ) { vs = "{cwound";	vp = "{cwounds";		}
        else if ( pain_percent <=  55 ) { vs = "{Cmaul";       vp = "{Cmauls";		}
        else if ( pain_percent <=  65 ) { vs = "{ydestroy";	vp = "{ydestroys";	}
        else if ( pain_percent <=  70 ) { vs = "{ydestroy";	vp = "{ydestroys";	}
        else if ( pain_percent <=  80 ) { vs = "{rmaim";	vp = "{rmaims";		}
        else if ( pain_percent <=  90 ) { vs = "{rMUTILATE";	vp = "{rMUTILATES";	}
        else if ( pain_percent <=  100 ) { vs = "{RDISEMBOWEL";	vp = "{RDISEMBOWELS";	}
        else if ( pain_percent <=  120 ) { vs = "{RDISMEMBER";	vp = "{RDISMEMBERS";	}
        else if ( pain_percent <=  140 ) { vs = "{RMASSACRE";	vp = "{RMASSACRES";	}
        else if ( pain_percent <=  160 ) { vs = "{RMANGLE";	vp = "{RMANGLES";		}
        else if ( pain_percent <=  180 ) { vs = "{W*** {RDEMOLISH ***{W==={R";
                     vp = "{W*** {RDEMOLISHES {W==={R";			}
        else if ( pain_percent <=  190 ) { vs = "{W*** {RDEVASTATE ***{W==={R";
                     vp = "{W*** {RDEVASTATES {W==={R";			}
        else if ( pain_percent <= 210)  { vs = "{W=== {ROBLITERATE {W==={R";
                     vp = "{W=== {ROBLITERATES {W==={R";		}
        else if ( pain_percent <= 230)  { vs = "{r>>> ANNIHILATE <<<";
                     vp = "{r>>> ANNIHILATES <<<";		}
        else if ( pain_percent <= 250)  { vs = "{R<<< ERADICATE >>>";
                     vp = "{R<<< ERADICATES >>>";			}
        else                   { vs = "{Mdo {r**{RUNS{YPE{WA{YKA{RBLE{r***{M things to";
                     vp = "{Mdoes {r**{RUNS{YPE{WA{YKA{RBLE{r***{} things to";		}
    
        punct   = (pain_percent <= 90) ? '.' : '!';
    

    if (dt == TYPE_HIT)
    {
        if (ch == victim)
        {
            sprintf (buf1, "{3$n %s {3$melf%c{x", vp, punct);
            sprintf (buf2, "{2You %s {2yourself%c{x", vs, punct);
        }
        else
        {
            sprintf (buf1, "{3$n %s {3$N%c{x", vp, punct);
            sprintf (buf2, "{2You %s {2$N%c{x", vs, punct);
            sprintf (buf3, "{4$n %s {4you%c{x", vp, punct);
        }
    }
    else
    {
        if (dt >= 0 && dt < MAX_SKILL)
            attack = skill_table[dt].noun_damage;
        else if (dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
            attack = attack_table[dt - TYPE_HIT].noun;
        else
        {
            bug ("Dam_message: bad dt %d.", dt);
            dt = TYPE_HIT;
            attack = attack_table[0].name;
        }

        if (immune)
        {
            if (ch == victim)
            {
                sprintf (buf1, "{3$n is unaffected by $s own %s.{x", attack);
                sprintf (buf2, "{2Luckily, you are immune to that.{x");
            }
            else
            {
                sprintf (buf1, "{3$N is unaffected by $n's %s!{x", attack);
                sprintf (buf2, "{2$N is unaffected by your %s!{x", attack);
                sprintf (buf3, "{4$n's %s is powerless against you.{x",
                         attack);
            }
        }
        else
        {
            if (ch == victim)
            {
                if (IS_SET(ch->act, PLR_AUTODAMAGE))
                {
                    sprintf (buf1, "{3$n's %s %s {3$m%c (%d){x", attack, vp, punct, dam);
                    sprintf (buf2, "{2Your %s %s {2you%c (%d){x", attack, vp, punct, dam);
                }
                else
                {
                    sprintf (buf1, "{3$n's %s %s {3$m%c{x", attack, vp, punct);
                    sprintf (buf2, "{2Your %s %s {2you%c{x", attack, vp, punct);
                }
            }
            else
            {
                /* Check for critical strike and modify message */
                if (critical && ch != victim)
                {
                    crit_wield = get_eq_char(ch, WEAR_WIELD);
                    if (crit_wield != NULL)
                    {
                        if (IS_SET(ch->act, PLR_AUTODAMAGE))
                        {
                            sprintf (buf1, "{3$n {YCRITICALLY STRIKES {3$N with $p%c (%d){x", punct, dam);
                            sprintf (buf2, "{2You {YCRITICALLY STRIKE {2$N with $p (%d){x", dam);
                            sprintf (buf3, "{4$n {YCRITICALLY STRIKES {4you with $p%c (%d){x", punct, dam);
                        }
                        else
                        {
                            sprintf (buf1, "{3$n {YCRITICALLY STRIKES {3$N with $p%c{x", punct);
                            sprintf (buf2, "{2You {YCRITICALLY STRIKE {2$N with $p%c{x", punct);
                            sprintf (buf3, "{4$n {YCRITICALLY STRIKES {4you with $p%c{x", punct);
                        }
                    }
                    else
                    {
                        /* Fallback if no weapon somehow */
                        if (IS_SET(ch->act, PLR_AUTODAMAGE))
                        {
                            sprintf (buf1, "{3$n's %s {YCRITICALLY STRIKES {3$N%c (%d){x", attack, punct, dam);
                            sprintf (buf2, "{2Your %s {YCRITICALLY STRIKES {2$N (%d){x", attack, dam);
                            sprintf (buf3, "{4$n's %s {YCRITICALLY STRIKES {4you%c (%d){x", attack, punct, dam);
                        }
                        else
                        {
                            sprintf (buf1, "{3$n's %s {YCRITICALLY STRIKES {3$N%c{x", attack, punct);
                            sprintf (buf2, "{2Your %s {YCRITICALLY STRIKES {2$N%c{x", attack, punct);
                            sprintf (buf3, "{4$n's %s {YCRITICALLY STRIKES {4you%c{x", attack, punct);
                        }
                    }
                }
                else if (IS_SET(ch->act, PLR_AUTODAMAGE))
                {
                    sprintf (buf1, "{3$n's %s %s {3$N%c (%d){x", attack, vp, punct, dam);
                    sprintf (buf2, "{2Your %s %s {2$N%c (%d){x", attack, vp, punct, dam);
                    sprintf (buf3, "{4$n's %s %s {4you%c (%d){x", attack, vp, punct, dam);
                }
                else
                {
                    sprintf (buf1, "{3$n's %s %s {3$N%c{x", attack, vp, punct);
                    sprintf (buf2, "{2Your %s %s {2$N%c{x", attack, vp, punct);
                    sprintf (buf3, "{4$n's %s %s {4you%c{x", attack, vp, punct);
                }
            }
        }
    }

    if (ch == victim)
    {
        act (buf1, ch, NULL, NULL, TO_ROOM);
        act (buf2, ch, NULL, NULL, TO_CHAR);
    }
    else
    {
        /* For critical strikes with weapons, use the weapon object in act() */
        if (critical && crit_wield != NULL)
        {
            act (buf1, ch, crit_wield, victim, TO_NOTVICT);
            act (buf2, ch, crit_wield, victim, TO_CHAR);
            act (buf3, ch, crit_wield, victim, TO_VICT);
        }
        else
        {
            act (buf1, ch, NULL, victim, TO_NOTVICT);
            act (buf2, ch, NULL, victim, TO_CHAR);
            
            /* Show damage numbers to victim if autodamage is enabled */
            if (IS_SET(victim->act, PLR_AUTODAMAGE))
            {
                char buf3_damage[256];
                sprintf (buf3_damage, "{4$n's %s %s {4you%c (%d){x", attack, vp, punct, dam);
                act (buf3_damage, ch, NULL, victim, TO_VICT);
            }
            else
            {
                act (buf3, ch, NULL, victim, TO_VICT);
            }
        }
    }
    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
        return;

    if (IS_OBJ_STAT (obj, ITEM_NOREMOVE))
    {
        act ("{5$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but your weapon won't budge!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_NOTVICT);
        return;
    }

    act ("{5$n DISARMS you and sends your weapon flying!{x",
         ch, NULL, victim, TO_VICT);
    act ("{5You disarm $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{5$n disarms $N!{x", ch, NULL, victim, TO_NOTVICT);

    obj_from_char (obj);
    if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
        obj_to_char (obj, victim);
    else
    {
        obj_to_room (obj, victim->in_room);
        if (IS_NPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
            get_obj (victim, obj, NULL);
    }

    return;
}

void do_berserk (CHAR_DATA * ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill (ch, gsn_berserk)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
        send_to_char ("You turn red in the face, but nothing happens.\n\r",
                      ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_BERSERK) || is_affected (ch, gsn_berserk)
        || is_affected (ch, skill_lookup ("frenzy")))
    {
        send_to_char ("You get a little madder.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CALM))
    {
        send_to_char ("You're feeling to mellow to berserk.\n\r", ch);
        return;
    }

    if (ch->mana < 50)
    {
        send_to_char ("You can't get up enough energy.\n\r", ch);
        return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;

    if (number_percent () < chance)
    {
        AFFECT_DATA af;

        WAIT_STATE (ch, PULSE_VIOLENCE);
        ch->mana -= 50;
        ch->move /= 2;

        /* heal a little damage */
        ch->hit += ch->level * 2;
        ch->hit = UMIN (ch->hit, ch->max_hit);

        send_to_char ("Your pulse races as you are consumed by rage!\n\r",
                      ch);
        act ("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
        check_improve (ch, gsn_berserk, TRUE, 2);

        af.where = TO_AFFECTS;
        af.type = gsn_berserk;
        af.level = ch->level;
        af.duration = number_fuzzy (ch->level / 8);
        af.modifier = UMAX (1, ch->level / 5);
        af.bitvector = AFF_BERSERK;

        af.location = APPLY_HITROLL;
        affect_to_char (ch, &af);

        af.location = APPLY_DAMROLL;
        affect_to_char (ch, &af);

        af.modifier = UMAX (10, 10 * (ch->level / 5));
        af.location = APPLY_AC;
        affect_to_char (ch, &af);
    }

    else
    {
        WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
        ch->mana -= 25;
        ch->move /= 2;

        send_to_char ("Your pulse speeds up, but nothing happens.\n\r", ch);
        check_improve (ch, gsn_berserk, FALSE, 2);
    }
}

void do_bash (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't bash while riding!\n\r", ch);
        return;
    }

    if ((chance = get_skill (ch, gsn_bash)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BASH))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {
        send_to_char ("Bashing? What's that?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char ("But you aren't fighting anyone!\n\r", ch);
            return;
        }
    }

    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("You'll have to let $M get back up first.", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You try to bash your brains out, but fail.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 15;
    else
        chance += (ch->size - victim->size) * 10;


    /* stats */
    chance += get_curr_stat (ch, STAT_STR);
    chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC (victim, AC_BASH) / 25;
    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
    {                            /*
                                   act("{5$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
                                   act("{5$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
                                   WAIT_STATE(ch,skill_table[gsn_bash].beats);
                                   return; */
        chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent () < chance)
    {

        act ("{5$n sends you sprawling with a powerful bash!{x",
             ch, NULL, victim, TO_VICT);
        act ("{5You slam into $N, and send $M flying!{x", ch, NULL, victim,
             TO_CHAR);
        act ("{5$n sends $N sprawling with a powerful bash.{x", ch, NULL,
             victim, TO_NOTVICT);
        check_improve (ch, gsn_bash, TRUE, 1);

        DAZE_STATE (victim, 3 * PULSE_VIOLENCE);
        WAIT_STATE (ch, skill_table[gsn_bash].beats);
        victim->position = POS_RESTING;
        damage (ch, victim, number_range (2, 2 + 2 * ch->size + chance / 20),
                gsn_bash, DAM_BASH, FALSE, FALSE);

        if (RIDDEN(victim))
        {
            mount_success(RIDDEN(victim), victim, FALSE);
        }

    }
    else
    {
        damage (ch, victim, 0, gsn_bash, DAM_BASH, FALSE, FALSE);
        act ("{5You fall flat on your face!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n falls flat on $s face.{x", ch, NULL, victim, TO_NOTVICT);
        act ("{5You evade $n's bash, causing $m to fall flat on $s face.{x",
             ch, NULL, victim, TO_VICT);
        check_improve (ch, gsn_bash, FALSE, 1);
        ch->position = POS_RESTING;
        WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
    }
    check_killer (ch, victim);
}

void do_headbutt (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, daze_chance;

    one_argument (argument, arg);

    /* Check if character is a dwarf */
    if (!IS_NPC(ch) && ch->race != race_lookup("dwarf"))
    {
        send_to_char ("Headbutting? What's that?\n\r", ch);
        return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You can't headbutt while riding!\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char ("But you aren't fighting anyone!\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("You'll have to let $M get back up first.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You headbutt yourself. OUCH!\n\r", ch);
        damage (ch, ch, number_range(5, 15), gsn_headbutt, DAM_BASH, FALSE, FALSE);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* Base chance - similar to bash but slightly higher */
    chance = 50 + get_curr_stat(ch, STAT_STR);

    /* Size matters */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 15;
    else
        chance += (ch->size - victim->size) * 10;

    /* Dex helps avoid it */
    chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC (victim, AC_BASH) / 25;

    /* Speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 30;

    /* Level difference */
    chance += (ch->level - victim->level);

    /* Dodge check */
    if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
    {
        chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
    }

    /* Attempt the headbutt */
    if (number_percent () < chance)
    {
        act ("{5You smash your forehead into $N's face!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n smashes $s forehead into your face!{x", ch, NULL, victim, TO_VICT);
        act ("{5$n smashes $s forehead into $N's face.{x", ch, NULL, victim, TO_NOTVICT);

        WAIT_STATE (ch, skill_table[gsn_bash].beats);

        /* Damage from the headbutt */
        damage (ch, victim, number_range (5, 5 + 2 * ch->size + chance / 15),
                gsn_headbutt, DAM_BASH, FALSE, FALSE);

        /* Chance to daze the victim */
        daze_chance = 30 + (ch->level - victim->level) * 2;
        if (number_percent() < daze_chance)
        {
            DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
            act ("{Y$N looks dazed from the blow!{x", ch, NULL, victim, TO_CHAR);
            act ("{YYou are dazed from the blow!{x", ch, NULL, victim, TO_VICT);
            act ("{Y$N looks dazed from the blow!{x", ch, NULL, victim, TO_NOTVICT);
        }
    }
    else
    {
        damage (ch, victim, 0, gsn_headbutt, DAM_BASH, FALSE, FALSE);
        act ("{5You miss your headbutt and stagger forward!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n misses a headbutt and staggers forward.{x", ch, NULL, victim, TO_NOTVICT);
        act ("{5You evade $n's headbutt!{x", ch, NULL, victim, TO_VICT);
        WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
    }
    check_killer (ch, victim);
}

void do_dirt (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't dirt while riding!\n\r", ch);
        return;
    }

    if ((chance = get_skill (ch, gsn_dirt)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK_DIRT))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
        send_to_char ("You get your feet dirty.\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char ("But you aren't in combat!\n\r", ch);
            return;
        }
    }

    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (victim, AFF_BLIND))
    {
        act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("Very funny.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= 2 * get_curr_stat (victim, STAT_DEX);

    /* speed  */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
        chance += 1;

    /* terrain */

    switch (ch->in_room->sector_type)
    {
        case (SECT_INSIDE):
            chance -= 20;
            break;
        case (SECT_CITY):
            chance -= 10;
            break;
        case (SECT_FIELD):
            chance += 5;
            break;
        case (SECT_FOREST):
            break;
        case (SECT_HILLS):
            break;
        case (SECT_MOUNTAIN):
            chance -= 10;
            break;
        case (SECT_WATER_SWIM):
            chance = 0;
            break;
        case (SECT_WATER_NOSWIM):
            chance = 0;
            break;
        case (SECT_AIR):
            chance = 0;
            break;
        case (SECT_DESERT):
            chance += 10;
            break;
    }

    if (chance == 0)
    {
        send_to_char ("There isn't any dirt to kick.\n\r", ch);
        return;
    }

    /* now the attack */
    if (number_percent () < chance)
    {
        AFFECT_DATA af;
        act ("{5$n is blinded by the dirt in $s eyes!{x", victim, NULL, NULL,
             TO_ROOM);
        act ("{5$n kicks dirt in your eyes!{x", ch, NULL, victim, TO_VICT);
        damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, FALSE, FALSE);
        send_to_char ("{5You can't see a thing!{x\n\r", victim);
        check_improve (ch, gsn_dirt, TRUE, 2);
        WAIT_STATE (ch, skill_table[gsn_dirt].beats);

        af.where = TO_AFFECTS;
        af.type = gsn_dirt;
        af.level = ch->level;
        af.duration = 0;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.bitvector = AFF_BLIND;

        affect_to_char (victim, &af);
    }
    else
    {
        damage (ch, victim, 0, gsn_dirt, DAM_NONE, TRUE, FALSE);
        check_improve (ch, gsn_dirt, FALSE, 2);
        WAIT_STATE (ch, skill_table[gsn_dirt].beats);
    }
    check_killer (ch, victim);
}

void do_trip (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument (argument, arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't trip while riding!\n\r", ch);
        return;
    }

    if ((chance = get_skill (ch, gsn_trip)) == 0
        || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_TRIP))
        || (!IS_NPC (ch)
            && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
        send_to_char ("Tripping?  What's that?\n\r", ch);
        return;
    }


    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char ("But you aren't fighting anyone!\n\r", ch);
            return;
        }
    }

    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (victim, AFF_FLYING))
    {
        act ("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim->position < POS_FIGHTING)
    {
        act ("$N is already down.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("{5You fall flat on your face!{x\n\r", ch);
        WAIT_STATE (ch, 2 * skill_table[gsn_trip].beats);
        act ("{5$n trips over $s own feet!{x", ch, NULL, NULL, TO_ROOM);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;    /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
        chance += 10;
    if (IS_SET (victim->off_flags, OFF_FAST)
        || IS_AFFECTED (victim, AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent () < chance)
    {
        act ("{5$n trips you and you go down!{x", ch, NULL, victim, TO_VICT);
        act ("{5You trip $N and $N goes down!{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n trips $N, sending $M to the ground.{x", ch, NULL, victim,
             TO_NOTVICT);
        check_improve (ch, gsn_trip, TRUE, 1);

        DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
        WAIT_STATE (ch, skill_table[gsn_trip].beats);
        victim->position = POS_RESTING;
        damage (ch, victim, number_range (2, 2 + 2 * victim->size), gsn_trip,
                DAM_BASH, TRUE, FALSE);
    }
    else
    {
        damage (ch, victim, 0, gsn_trip, DAM_BASH, TRUE, FALSE);
        WAIT_STATE (ch, skill_table[gsn_trip].beats * 2 / 3);
        check_improve (ch, gsn_trip, FALSE, 1);
    }
    check_killer (ch, victim);
}



void do_kill (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Kill whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }
/*  Allow player killing
    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
        &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }
*/
    if (victim == ch)
    {
        send_to_char ("You hit yourself.  Ouch!\n\r", ch);
        multi_hit (ch, ch, TYPE_UNDEFINED);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
        send_to_char ("You do the best you can!\n\r", ch);
        return;
    }

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    check_killer (ch, victim);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return;
}



void do_murde (CHAR_DATA * ch, char *argument)
{
    send_to_char ("If you want to MURDER, spell it out.\n\r", ch);
    return;
}



void do_murder (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Murder whom?\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM)
        || (IS_NPC (ch) && IS_SET (ch->act, ACT_PET)))
        return;

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("Suicide is a mortal sin.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
        act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
        send_to_char ("You do the best you can!\n\r", ch);
        return;
    }

    WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
    if (IS_NPC (ch))
        sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
        sprintf (buf, "Help!  I am being attacked by %s!", ch->name);
    do_function (victim, &do_yell, buf);
    check_killer (ch, victim);
    multi_hit (ch, victim, TYPE_UNDEFINED);
    return;
}



void do_backstab (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't backstab while riding!\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char ("Backstab whom?\n\r", ch);
        return;
    }

    if (ch->fighting != NULL)
    {
        send_to_char ("You're facing the wrong end.\n\r", ch);
        return;
    }

    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("How can you sneak up on yourself?\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) &&
        victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
        send_to_char ("You need to wield a weapon to backstab.\n\r", ch);
        return;
    }

    if (victim->hit < victim->max_hit / 3)
    {
        act ("$N is hurt and suspicious ... you can't sneak up.",
             ch, NULL, victim, TO_CHAR);
        return;
    }

    check_killer (ch, victim);
    WAIT_STATE (ch, skill_table[gsn_backstab].beats);
    if (number_percent () < get_skill (ch, gsn_backstab)
        || (get_skill (ch, gsn_backstab) >= 2 && !IS_AWAKE (victim)))
    {
        check_improve (ch, gsn_backstab, TRUE, 1);
        multi_hit (ch, victim, gsn_backstab);
    }
    else
    {
        check_improve (ch, gsn_backstab, FALSE, 1);
        damage (ch, victim, 0, gsn_backstab, DAM_NONE, TRUE, FALSE);
    }

    return;
}

/*
 * Felar racial ability - Rake with natural claws
 */
void do_rake (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;
    int move_cost;

    one_argument (argument, arg);

    /* Felar only */
    if (IS_NPC (ch) || ch->race != 6) /* felar is race 6 */
    {
        send_to_char ("Only felar can use their natural claws to rake.\n\r", ch);
        return;
    }

    if (MOUNTED (ch))
    {
        send_to_char ("You can't rake while riding!\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        if (ch->fighting == NULL)
        {
            send_to_char ("Rake whom?\n\r", ch);
            return;
        }
        victim = ch->fighting;
    }
    else
    {
        if ((victim = get_char_room (ch, arg)) == NULL)
        {
            send_to_char ("They aren't here.\n\r", ch);
            return;
        }
    }

    if (victim == ch)
    {
        send_to_char ("You cannot rake yourself.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (ch->fighting == NULL && victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    /* Must be in combat or starting combat */
    if (ch->fighting != victim && ch->fighting != NULL)
    {
        send_to_char ("You're already fighting someone else.\n\r", ch);
        return;
    }

    /* Check if already used this round - similar delay to backstab */
    if (ch->wait > 0)
    {
        send_to_char ("You need to wait a moment before raking again.\n\r", ch);
        return;
    }

    /* Calculate movement cost (10% of max) */
    move_cost = ch->max_move / 10;
    if (ch->move < move_cost)
    {
        send_to_char ("You're too exhausted.\n\r", ch);
        return;
    }

    check_killer (ch, victim);
    set_fighting (ch, victim);

    /* Cost 10% movement */
    ch->move -= move_cost;

    /* Delay similar to backstab - prevents spam */
    WAIT_STATE (ch, skill_table[gsn_backstab].beats);

    /* Calculate damage scaling with level */
    dam = number_range (ch->level * 2 / 3, ch->level * 4 / 3);

    /* Apply weapon skill bonus if they have hand-to-hand */
    {
        int skill = get_skill (ch, gsn_hand_to_hand);
        if (skill > 0)
            dam = dam * (100 + skill) / 100;
    }

    /* Enhanced damage for felar claws */
    dam = dam * 11 / 10;

    /* Apply damage */
    act ("$n rakes $N with $s sharp claws!", ch, NULL, victim, TO_NOTVICT);
    act ("You rake $N with your sharp claws!", ch, NULL, victim, TO_CHAR);
    act ("$n rakes you with $s sharp claws!", ch, NULL, victim, TO_VICT);

    damage (ch, victim, dam, TYPE_HIT + DAM_SLASH, DAM_SLASH, TRUE, FALSE);

    return;
}



void do_flee (CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ((victim = ch->fighting) == NULL)
    {
        if (ch->position == POS_FIGHTING)
            ch->position = POS_STANDING;
        send_to_char ("You aren't fighting anyone.\n\r", ch);
        return;
    }

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++)
    {
        EXIT_DATA *pexit;
        int door;

        door = number_door ();
        if ((pexit = was_in->exit[door]) == 0
            || pexit->u1.to_room == NULL
            || IS_SET (pexit->exit_info, EX_CLOSED)
            || number_range (0, ch->daze) != 0 || (IS_NPC (ch)
                                                   && IS_SET (pexit->u1.
                                                              to_room->
                                                              room_flags,
                                                              ROOM_NO_MOB)))
            continue;

        move_char (ch, door, FALSE);
        if ((now_in = ch->in_room) == was_in)
            continue;

        ch->in_room = was_in;
        act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
        ch->in_room = now_in;

        if (!IS_NPC (ch))
        {
            send_to_char ("You flee from combat!\n\r", ch);
            if ((ch->class == 2) && (number_percent () < 3 * (ch->level / 2)))
                send_to_char ("You snuck away safely.\n\r", ch);
            else
            {
                send_to_char ("You lost 10 exp.\n\r", ch);
                gain_exp (ch, -10);
            }
        }

        stop_fighting (ch, TRUE);
        return;
    }

    send_to_char ("PANIC! You couldn't escape!\n\r", ch);
    return;
}



void do_rescue (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        send_to_char ("Rescue whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("What about fleeing instead?\n\r", ch);
        return;
    }

    if (!IS_NPC (ch) && IS_NPC (victim))
    {
        send_to_char ("Doesn't need your help!\n\r", ch);
        return;
    }

    if (ch->fighting == victim)
    {
        send_to_char ("Too late.\n\r", ch);
        return;
    }

    if ((fch = victim->fighting) == NULL)
    {
        send_to_char ("That person is not fighting right now.\n\r", ch);
        return;
    }

    if (IS_NPC (fch) && !is_same_group (ch, victim))
    {
        send_to_char ("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    WAIT_STATE (ch, skill_table[gsn_rescue].beats);
    if (number_percent () > get_skill (ch, gsn_rescue))
    {
        send_to_char ("You fail the rescue.\n\r", ch);
        check_improve (ch, gsn_rescue, FALSE, 1);
        return;
    }

    act ("{5You rescue $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{5$n rescues you!{x", ch, NULL, victim, TO_VICT);
    act ("{5$n rescues $N!{x", ch, NULL, victim, TO_NOTVICT);
    check_improve (ch, gsn_rescue, TRUE, 1);

    stop_fighting (fch, FALSE);
    stop_fighting (victim, FALSE);

    check_killer (ch, fch);
    set_fighting (ch, fch);
    set_fighting (fch, ch);
    return;
}



void do_kick (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;

    if ( MOUNTED(ch) )
    {
        send_to_char("You can't kick while riding!\n\r", ch);
        return;
    }

    if (!IS_NPC (ch)
        && ch->level < skill_table[gsn_kick].skill_level[ch->class])
    {
        send_to_char ("You better leave the martial arts to fighters.\n\r",
                      ch);
        return;
    }

    if (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK))
        return;

    if ((victim = ch->fighting) == NULL)
    {
        send_to_char ("You aren't fighting anyone.\n\r", ch);
        return;
    }

    WAIT_STATE (ch, skill_table[gsn_kick].beats);
    
    /* Centaurs kick twice with bonus damage */
    if (!IS_NPC(ch) && ch->race == race_lookup("centaur"))
    {
        if (get_skill (ch, gsn_kick) > number_percent ())
        {
            act ("{5You rear up and kick $N with both hind legs!{x", ch, NULL, victim, TO_CHAR);
            act ("{5$n rears up and kicks you with both hind legs!{x", ch, NULL, victim, TO_VICT);
            act ("{5$n rears up and kicks $N with both hind legs!{x", ch, NULL, victim, TO_NOTVICT);
            
            /* First kick - bonus damage */
            damage (ch, victim, number_range (1, ch->level) + ch->level / 4, gsn_kick, DAM_BASH,
                    TRUE, FALSE);
            /* Second kick - bonus damage */
            if (victim->position > POS_DEAD)
                damage (ch, victim, number_range (1, ch->level) + ch->level / 4, gsn_kick, DAM_BASH,
                        TRUE, FALSE);
            check_improve (ch, gsn_kick, TRUE, 1);
        }
        else
        {
            damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE, FALSE);
            check_improve (ch, gsn_kick, FALSE, 1);
        }
    }
    else
    {
        /* Normal kick for non-centaurs */
        if (get_skill (ch, gsn_kick) > number_percent ())
        {
            damage (ch, victim, number_range (1, ch->level), gsn_kick, DAM_BASH,
                    TRUE, FALSE);
            check_improve (ch, gsn_kick, TRUE, 1);
        }
        else
        {
            damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE, FALSE);
            check_improve (ch, gsn_kick, FALSE, 1);
        }
    }
    check_killer (ch, victim);
    return;
}




void do_disarm (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill (ch, gsn_disarm)) == 0)
    {
        send_to_char ("You don't know how to disarm opponents.\n\r", ch);
        return;
    }

    if (get_eq_char (ch, WEAR_WIELD) == NULL
        && ((hth = get_skill (ch, gsn_hand_to_hand)) == 0
            || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM))))
    {
        send_to_char ("You must wield a weapon to disarm.\n\r", ch);
        return;
    }

    if ((victim = ch->fighting) == NULL)
    {
        send_to_char ("You aren't fighting anyone.\n\r", ch);
        return;
    }

    if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    {
        send_to_char ("Your opponent is not wielding a weapon.\n\r", ch);
        return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch));
    vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim));
    ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim));

    /* modifiers */

    /* skill */
    if (get_eq_char (ch, WEAR_WIELD) == NULL)
        chance = chance * hth / 150;
    else
        chance = chance * ch_weapon / 100;

    chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

    /* dex vs. strength */
    chance += get_curr_stat (ch, STAT_DEX);
    chance -= 2 * get_curr_stat (victim, STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* and now the attack */
    if (number_percent () < chance)
    {
        WAIT_STATE (ch, skill_table[gsn_disarm].beats);
        disarm (ch, victim);
        check_improve (ch, gsn_disarm, TRUE, 1);
    }
    else
    {
        WAIT_STATE (ch, skill_table[gsn_disarm].beats);
        act ("{5You fail to disarm $N.{x", ch, NULL, victim, TO_CHAR);
        act ("{5$n tries to disarm you, but fails.{x", ch, NULL, victim,
             TO_VICT);
        act ("{5$n tries to disarm $N, but fails.{x", ch, NULL, victim,
             TO_NOTVICT);
        check_improve (ch, gsn_disarm, FALSE, 1);
    }
    check_killer (ch, victim);
    return;
}

void do_surrender (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *mob;
    if ((mob = ch->fighting) == NULL)
    {
        send_to_char ("But you're not fighting!\n\r", ch);
        return;
    }
    act ("You surrender to $N!", ch, NULL, mob, TO_CHAR);
    act ("$n surrenders to you!", ch, NULL, mob, TO_VICT);
    act ("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
    stop_fighting (ch, TRUE);

    if (!IS_NPC (ch) && IS_NPC (mob)
        && (!HAS_TRIGGER (mob, TRIG_SURR)
            || !mp_percent_trigger (mob, ch, NULL, NULL, TRIG_SURR)))
    {
        act ("$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR);
        multi_hit (mob, ch, TYPE_UNDEFINED);
    }
}

void do_sla (CHAR_DATA * ch, char *argument)
{
    send_to_char ("If you want to SLAY, spell it out.\n\r", ch);
    return;
}



void do_slay (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        send_to_char ("Slay whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (ch == victim)
    {
        send_to_char ("Suicide is a mortal sin.\n\r", ch);
        return;
    }

    if (!IS_NPC (victim) && victim->level >= get_trust (ch))
    {
        send_to_char ("You failed.\n\r", ch);
        return;
    }

    act ("{1You slay $M in cold blood!{x", ch, NULL, victim, TO_CHAR);
    act ("{1$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT);
    act ("{1$n slays $N in cold blood!{x", ch, NULL, victim, TO_NOTVICT);
    raw_kill (ch, victim);
    return;
}

/* Hate */
bool is_hating(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (!ch->hate || ch->hate->who != victim)
        return FALSE;
    return TRUE;
}

void stop_hating(CHAR_DATA *ch)
{
    if (ch->hate)
    {
        free_string(ch->hate->name);
        free_mem(ch->hate, sizeof(HATE_DATA));
        ch->hate = NULL;
    }
    return;
}

void start_hating(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ch->hate)
        stop_hating(ch);

    ch->hate = alloc_mem(sizeof(HATE_DATA));
    ch->hate->name = str_dup(victim->name);
    ch->hate->who = victim;
    return;
}

bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
        int chance;
        int dam_type;
        OBJ_DATA *wield;

        if (    ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||
                ( !IS_AWAKE(victim) ) ||
                ( !can_see(victim,ch) ) ||
                ( get_skill(victim,gsn_counter) < 1 )
           )
           return FALSE;

        wield = get_eq_char(victim,WEAR_WIELD);

        chance = get_skill(victim,gsn_counter) / 6;
        chance += ( victim->level - ch->level ) / 2;
        chance += 2 * (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
        chance += get_weapon_skill(victim,get_weapon_sn(victim)) -
                        get_weapon_skill(ch,get_weapon_sn(ch));
        chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

    if ( number_percent( ) >= chance )
        return FALSE;

    dt = gsn_counter;

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
    act( "$N reverses your attack!", ch, NULL, victim, TO_CHAR    );

    damage(victim,ch,dam/2, gsn_counter , dam_type ,TRUE, FALSE ); /* DAM MSG NUMBER!! */

    check_improve(victim,gsn_counter,TRUE,6);

    return TRUE;
}

bool check_phase( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_phase) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "Your body phases to avoid $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N's body phases to avoid your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_phase,TRUE,6);
    return TRUE;
}

/*
 * Critical strike function - allows players with 100% skill in the
 * weapon they are using to get a little bonus out of it once in a while.
 */
bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA *obj;

    obj = get_eq_char(ch,WEAR_WIELD);

    if (
         ( get_eq_char(ch,WEAR_WIELD) == NULL ) || 
         ( get_skill(ch,gsn_critical)  <  1 ) ||
         ( get_weapon_skill(ch,get_weapon_sn(ch))  !=  100 ) ||
         ( number_range(0,100) > get_skill(ch,gsn_critical) )
       )
            return FALSE;


    if ( number_range(0,100) > 25 )
            return FALSE;


    /* Now, if it passed all the tests... */
    /* Messages will be shown by dam_message with critical strike text */
    check_improve(ch,gsn_critical,TRUE,6);
    return TRUE;
}

/*
 * Special defensive skill for polearms.  Due to their extended range,
 * polearms are able to keep your opponent from closing on on you.
 */
bool pole_special (CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE (victim))
        return FALSE;

    if (get_weapon_sn (victim) != gsn_polearm)
        return FALSE;

    chance = get_skill (victim, gsn_polearm) / 3;

    if (!can_see (victim, ch))
        chance /= 2;

    /*
     * It's harder to keep another polearm at bay.
     */
    if (get_weapon_sn (ch) != gsn_polearm)
        chance -= chance / 3;

    if (number_percent () >= chance + victim->level - ch->level)
        return FALSE;

    act ("You manage to keep $n out of range.",
            ch, NULL, victim, TO_VICT);
    act ("$N's weapon keeps you out of range.",
            ch, NULL, victim, TO_CHAR);

    check_improve (victim, gsn_polearm, TRUE, 4);
    return TRUE;
}

/*
 * Critical hit damage modifier for axes.
 */
int axe_special (CHAR_DATA * ch, CHAR_DATA * victim, int dam)
{
    int chance;

    /*
     * Make sure we have an axe.
     */
    if (get_weapon_sn (ch) != gsn_axe)
        return dam;

    /*
     * Base is 1/5th our axe skill.  Bonus if victim can't see us, penalty
     * if we can't see them.
     */
    chance = get_skill (ch, gsn_axe) / 5;

    if (!can_see (victim, ch))
        chance *= 1.5;

    if (!can_see (ch, victim))
        chance /= 3;

    if (number_percent () >= chance)
        return dam;

    /*
     * We got a critical hit!
     */
    dam += dam * (get_skill (ch, gsn_axe) / 2) / 100;
    check_improve (ch, gsn_axe, TRUE, 4);
    return dam;
}

/*
 * Vorpal kill function by Kyle Boyd (boyd1@proaxis.com)
 * Ensures that ch gets exp for kill and alignment is changed.
 */
bool vorpal_kill (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;

    if (victim->position == POS_DEAD)
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if (victim == ch)
    {
        bug ("vorpal_kill: victim == ch", 0);
        return FALSE;
    }

    if (victim != ch)
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if (is_safe (ch, victim))
            return FALSE;
        check_killer (ch, victim);

        if (victim->position > POS_STUNNED)
        {
            if (victim->fighting == NULL)
                set_fighting (victim, ch);
            if (victim->timer <= 4)
                victim->position = POS_FIGHTING;
        }

        if (victim->position > POS_STUNNED)
        {
            if (ch->fighting == NULL)
                set_fighting (ch, victim);

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if (IS_NPC (ch)
                && IS_NPC (victim)
                && IS_AFFECTED (victim, AFF_CHARM)
                && victim->master != NULL
                && victim->master->in_room == ch->in_room
                && number_bits (3) == 0)
            {
                stop_fighting (ch, FALSE);
                multi_hit (ch, victim->master, TYPE_UNDEFINED);
                return FALSE;
            }
        }

        /*
         * More charm stuff.
         */
        if (victim->master == ch)
            stop_follower (victim);
    }

    /*
     * Inviso attacks ... not.
     */
    if (IS_AFFECTED (ch, AFF_INVISIBLE))
    {
        affect_strip (ch, gsn_invis);
        affect_strip (ch, gsn_mass_invis);
        STR_REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
        act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

    /* dam changed to max_hit to get a neat damage message */
    dam = victim->max_hit;
    dam_message (ch, victim, dam, dt, FALSE, FALSE);

    if (dam == 0)
        return FALSE;

    /* Give the ch xp for the hit */
    if (!IS_NPC (ch))
    {
        int xp = 0;
        int members = 0;
        int group_levels = 0;
        CHAR_DATA *gch;

        for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
        {
            if (is_same_group (gch, ch))
            {
                members++;
                group_levels += gch->level;
            }
        }

        xp = xp_compute (ch, victim, group_levels);
        /* Extra exp given for the vorpal kill */
        xp *= 3;
        gain_exp (ch, xp);
    }

    /*
     * KILL the victim.
     * Inform the victim of his new state.
     */
    victim->hit = -20;
    update_pos (victim);

    act ("$n is DEAD!!", victim, 0, 0, TO_ROOM);
    send_to_char ("You have been KILLED!!\n\r\n\r", victim);

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE (victim))
        stop_fighting (victim, FALSE);

    /*
     * Payoff for killing things.
     */
    if (victim->position == POS_DEAD)
    {
        group_gain (ch, victim);

        if (!IS_NPC (victim))
        {
            sprintf (log_buf, "%s decapitated by %s at %d",
                     victim->name,
                     (IS_NPC (ch) ? ch->short_descr : ch->name),
                     victim->in_room->vnum);
            log_string (log_buf);

            /*
             * Dying penalty:
             * 2/3 of exp to next level.
             */
            if (victim->exp > exp_per_level (victim, victim->pcdata->points)
                * victim->level)
                gain_exp (victim, (2 * (exp_per_level (victim,
                                                        victim->pcdata->
                                                        points) * victim->level -
                                        victim->exp) / 3) + 50);
        }

        if (!IS_NPC (victim))
        {
            sprintf (buf, "%s has been brutally decapitated by %s!",
                     victim->name,
                     IS_NPC (ch) ? ch->short_descr : ch->name);
            wiznet (buf, NULL, NULL, WIZ_DEATHS, 0, 0);
        }

        /*
         * Death trigger
         */
        if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_DEATH))
        {
            victim->position = POS_STANDING;
            mp_percent_trigger (victim, ch, NULL, NULL, TRIG_DEATH);
        }

        raw_kill (ch, victim);

        /* RT new auto commands */
        if (!IS_NPC (ch) && IS_NPC (victim))
        {
            corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

            if (IS_SET (ch->act, PLR_AUTOLOOT) &&
                corpse && corpse->contains)    /* exists and not empty */
                do_function (ch, &do_get, "all corpse");

            if (IS_SET (ch->act, PLR_AUTOGOLD) &&
                corpse && corpse->contains &&    /* exists and not empty */
                !IS_SET (ch->act, PLR_AUTOLOOT))
                do_function (ch, &do_get, "gold corpse");

            if (IS_SET (ch->act, PLR_AUTOSAC))
                if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse &&
                    corpse->contains)
                    return TRUE;    /* leave if corpse has treasure */
                else
                    do_function (ch, &do_sacrifice, "corpse");
        }

        return TRUE;
    }

    return TRUE;
}

/* For watchers in room to be drawn into fights.
 * Original idea taken from SneezyMUD.
 * Coded by Thale.
 */
void crowd_brawl(CHAR_DATA *ch)
{
    CHAR_DATA *rch, *rch_next, *vch, *vch_next;
    CHAR_DATA *is_fighting[45];
    SHOP_DATA *pshop;
    int chance;
    int counter;
    int to_fight;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	chance=number_range(1,300);

	if ((rch->fighting == NULL)
	 &&((!IS_IMMORTAL(rch) && !IS_NPC(rch))
	 ||(IS_NPC(rch)
	 &&!(IS_SET(rch->act,ACT_TRAIN)
	 ||  IS_SET(rch->act,ACT_PRACTICE)
	 ||  IS_SET(rch->act,ACT_IS_HEALER)
	 ||  IS_SET(rch->act,ACT_IS_CHANGER)
	 ||  IS_SET(rch->act,ACT_BOUNTY)
	 || ((pshop = rch->pIndexData->pShop) != NULL))))
	 &&IS_AWAKE(rch)
	 &&(chance <= 2))
	{
	  counter = 0;
	  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	  {
	    vch_next = vch->next_in_room;
	    if ((vch->fighting != NULL) && (counter <= 44))
	    {
	      is_fighting[counter] = vch;
	      ++counter;
	      /* if it's an NPC, thrice as likely as a PC to be attacked */
	      /* max of 45 fighting PC/NPCs in array, can't exceed */
	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
		is_fighting[counter] = vch;
		++counter;
	      }
	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
		is_fighting[counter] = vch;
		++counter;
	      }
  	    }
	  }
	  /* random number of mob in array to fight */
	  to_fight = number_range(1,counter);
	  /* (to_fight -1) because arrays start at 0 not at 1 */
	  to_fight -= 1;

	  /* checks needed for PCs only */
	  if (!IS_NPC(rch) && !IS_NPC(is_fighting[to_fight]))
	  {
	    /* if a Harper, won't attack another PC, and vice versa */
	    /* Note: IS_PEACEKEEPER not defined, skipping that check */
	    /* won't attack someone of same clan */
	    if (is_same_clan(rch,is_fighting[to_fight]))
	      continue;

	    /* PK range of 7 level difference in effect between PCs,
	     * and no PC can be drawn in against a victim PC lower
	     * than level 10 (also pursuant to PK rules)
	     */
	    if (((UMAX(rch->level,is_fighting[to_fight]->level) -
		  UMIN(rch->level,is_fighting[to_fight]->level)) > 7)
	       ||(is_fighting[to_fight]->level < 10))
	      continue;
	  }
	  else if (IS_NPC(rch))
	  {
	    /* hack so adept in mud school at diploma beast won't attack
	     * anyone - learned the hard way, oops. :)
	     */
	    if (rch->pIndexData->vnum == 3708)
	      continue;
	  }

	  /* if an immortal is fighting, nothing attacks him/her */
	  if (IS_IMMORTAL(is_fighting[to_fight]))
	    continue;

	  /* resting mobs/players have a smaller chance of being drawn in */
	  if ((rch->position <= POS_RESTING) && (chance > 1))
	    continue;

	  /* gotta see them to attack them */
	  if (!can_see(rch,is_fighting[to_fight]))
	    continue;

	  /* not against group members */
	  if (is_same_group(rch,is_fighting[to_fight]))
	    continue;

	  /* charmed mobs and pets, whether grouped or not */
	  if (IS_AFFECTED(rch,AFF_CHARM)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (IS_SET(rch->act,ACT_PET)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (is_fighting[to_fight] != NULL)
	  {
	    rch->fighting = is_fighting[to_fight];
	    rch->position = POS_FIGHTING;
	    act("You find yourself caught up in the brawl!",rch,NULL,NULL,TO_CHAR);
	    act("$n finds $mself involved in the brawl.",rch,NULL,NULL,TO_ROOM);
	  }
	  else
	    bug("Crowd_brawl - person to fight is NULL.", 0);
	}
    }
    return;
}  /* end of crowd_brawl */
