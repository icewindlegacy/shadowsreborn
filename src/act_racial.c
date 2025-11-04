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
 *           act_racial.c - November 3, 2025
 */            
/***************************************************************************
 *  Racial abilities for Shadows Reborn                                     *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

/*
 * Pixie Dust - Pixie racial ability
 * Grants 3 random beneficial spells for 10 ticks
 * Causes weariness for 15 ticks (prevents re-use)
 */
void do_dust (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    int spell_list[9];
    int selected_spells[3];
    int i, j, random_index;
    int num_spells = 9;
    bool already_selected;
    
    /* Check if character is a pixie */
    if (ch->race != race_lookup("pixie"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already weary */
    if (IS_AFFECTED(ch, AFF_WEARINESS))
    {
        send_to_char("You are too weary to use pixie dust again right now.\n\r", ch);
        return;
    }
    
    /* Build list of beneficial spells */
    spell_list[0] = skill_lookup("haste");
    spell_list[1] = skill_lookup("giant strength");
    spell_list[2] = skill_lookup("sanctuary");
    spell_list[3] = skill_lookup("shield");
    spell_list[4] = skill_lookup("armor");
    spell_list[5] = skill_lookup("frenzy");
    spell_list[6] = skill_lookup("stone skin");
    spell_list[7] = skill_lookup("bark skin");
    spell_list[8] = skill_lookup("bless");
    
    /* Select 3 random unique spells */
    for (i = 0; i < 3; i++)
    {
        do
        {
            random_index = number_range(0, num_spells - 1);
            already_selected = FALSE;
            
            /* Check if this spell was already selected */
            for (j = 0; j < i; j++)
            {
                if (selected_spells[j] == spell_list[random_index])
                {
                    already_selected = TRUE;
                    break;
                }
            }
        } while (already_selected);
        
        selected_spells[i] = spell_list[random_index];
    }
    
    /* Apply the selected spells */
    act("{mYou scatter shimmering pixie dust around yourself!{x", ch, NULL, NULL, TO_CHAR);
    act("{m$n scatters shimmering pixie dust, which swirls around $m!{x", ch, NULL, NULL, TO_ROOM);
    
    for (i = 0; i < 3; i++)
    {
        if (selected_spells[i] > 0 && selected_spells[i] < MAX_SKILL)
        {
            af.where     = TO_AFFECTS;
            af.type      = selected_spells[i];
            af.level     = ch->level;
            af.duration  = 10;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = 0;
            
            /* Set bitvector based on spell */
            if (selected_spells[i] == skill_lookup("haste"))
                af.bitvector = AFF_HASTE;
            else if (selected_spells[i] == skill_lookup("sanctuary"))
                af.bitvector = AFF_SANCTUARY;
            else if (selected_spells[i] == skill_lookup("giant strength"))
            {
                af.location = APPLY_STR;
                af.modifier = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
            }
            else if (selected_spells[i] == skill_lookup("shield"))
            {
                af.location = APPLY_AC;
                af.modifier = -20;
            }
            else if (selected_spells[i] == skill_lookup("armor"))
            {
                af.location = APPLY_AC;
                af.modifier = -20;
            }
            else if (selected_spells[i] == skill_lookup("frenzy"))
            {
                af.location = APPLY_HITROLL;
                af.modifier = ch->level / 6;
                af.bitvector = AFF_BERSERK;
            }
            else if (selected_spells[i] == skill_lookup("stone skin"))
            {
                af.location = APPLY_AC;
                af.modifier = -40;
            }
            else if (selected_spells[i] == skill_lookup("bark skin"))
            {
                af.location = APPLY_AC;
                af.modifier = -40;
            }
            else if (selected_spells[i] == skill_lookup("bless"))
            {
                af.location = APPLY_HITROLL;
                af.modifier = ch->level / 8;
            }
            
            affect_to_char(ch, &af);
        }
    }
    
    send_to_char("{YYou glow with fae magic!{x\n\r", ch);
    
    /* Apply weariness for 15 ticks */
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weariness");
    af.level     = ch->level;
    af.duration  = 15;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_WEARINESS;
    affect_to_char(ch, &af);
    
    send_to_char("{DYou feel weary from the exertion.{x\n\r", ch);
}

/*
 * Slam - Verbeeg racial ability
 * Picks up and slams the enemy to the ground for heavy damage
 */
void do_slam (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;

    one_argument (argument, arg);

    /* Check if character is a verbeeg */
    if (!IS_NPC(ch) && ch->race != race_lookup("verbeeg"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("Slam whom?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You can't slam yourself.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    /* Heavy damage scaling with level */
    dam = number_range(ch->level * 2, ch->level * 3);
    dam += get_curr_stat(ch, STAT_STR) * 2;  /* Strength bonus */

    act("{5You grab $N, lift $M overhead, and SLAM $M into the ground!{x", ch, NULL, victim, TO_CHAR);
    act("{5$n grabs you, lifts you overhead, and SLAMS you into the ground!{x", ch, NULL, victim, TO_VICT);
    act("{5$n grabs $N, lifts $M overhead, and SLAMS $M into the ground!{x", ch, NULL, victim, TO_NOTVICT);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    damage(ch, victim, dam, gsn_slam, DAM_BASH, FALSE, FALSE);
    
    /* Victim is knocked down */
    if (victim->position > POS_RESTING && !IS_AFFECTED(victim, AFF_FLYING))
    {
        victim->position = POS_RESTING;
        DAZE_STATE(victim, PULSE_VIOLENCE);
    }
    
    check_killer(ch, victim);
}

/*
 * Repair Protocols - Cyborg racial ability
 * Self-repair to full health with 40 tick cooldown
 */
void do_protocol (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    
    /* Check if character is a cyborg */
    if (!IS_NPC(ch) && ch->race != race_lookup("cyborg"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already weary from repair */
    if (IS_AFFECTED(ch, AFF_WEARINESS))
    {
        send_to_char("Your repair protocols are still recharging.\n\r", ch);
        return;
    }
    
    /* Full heal */
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    
    act("{C$n's mechanical parts whir and click as $e repairs $mself!{x", ch, NULL, NULL, TO_ROOM);
    send_to_char("{CYour repair protocols activate, restoring you to full functionality!{x\n\r", ch);
    
    /* Apply weariness for 40 ticks */
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weariness");
    af.level     = ch->level;
    af.duration  = 40;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_WEARINESS;
    affect_to_char(ch, &af);
    
    send_to_char("{DYour repair protocols enter cooldown mode.{x\n\r", ch);
}

/*
 * Throw Rock - Ogre racial ability
 * Throws environmental object based on sector type
 */
void do_throw_rock (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *projectile;
    char throw_msg[MAX_STRING_LENGTH];
    char hit_msg[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int dam;
    int sector;

    one_argument (argument, arg);

    /* Check if character is an ogre */
    if (!IS_NPC(ch) && ch->race != race_lookup("ogre"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("Throw something at whom?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("That would be silly.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    /* Determine what to throw based on sector */
    sector = ch->in_room->sector_type;
    
    switch(sector)
    {
        case SECT_INSIDE:
            projectile = "brick";
            sprintf(throw_msg, "$n rips a brick from a nearby wall and hurls it at $N!");
            sprintf(hit_msg, "You rip a brick from a nearby wall and hurl it at $N!");
            break;
        case SECT_CITY:
            projectile = "cobblestone";
            sprintf(throw_msg, "$n rips up a cobblestone and hurls it at $N!");
            sprintf(hit_msg, "You rip up a cobblestone and hurl it at $N!");
            break;
        case SECT_FIELD:
        case SECT_HILLS:
            projectile = "stone";
            sprintf(throw_msg, "$n picks up a large stone and hurls it at $N!");
            sprintf(hit_msg, "You pick up a large stone and hurl it at $N!");
            break;
        case SECT_MOUNTAIN:
            projectile = "boulder";
            sprintf(throw_msg, "$n hefts a massive boulder and hurls it at $N!");
            sprintf(hit_msg, "You heft a massive boulder and hurl it at $N!");
            break;
        case SECT_FOREST:
            projectile = "uprooted tree";
            sprintf(throw_msg, "$n rips a tree from the ground and hurls it at $N!");
            sprintf(hit_msg, "You rip a tree from the ground and hurl it at $N!");
            break;
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
            send_to_char("There's nothing here to throw!\n\r", ch);
            return;
        case SECT_DESERT:
            projectile = "rock";
            sprintf(throw_msg, "$n scoops up a rock and hurls it at $N!");
            sprintf(hit_msg, "You scoop up a rock and hurl it at $N!");
            break;
        default:
            projectile = "rock";
            sprintf(throw_msg, "$n picks up a rock and hurls it at $N!");
            sprintf(hit_msg, "You pick up a rock and hurl it at $N!");
            break;
    }

    /* Decent damage */
    dam = number_range(ch->level, ch->level * 2);
    dam += get_curr_stat(ch, STAT_STR);

    act(hit_msg, ch, NULL, victim, TO_CHAR);
    act(throw_msg, ch, NULL, victim, TO_NOTVICT);
    act("$n hurls a $t at you!", ch, projectile, victim, TO_VICT);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    damage(ch, victim, dam, gsn_throw_rock, DAM_BASH, FALSE, FALSE);
    check_killer(ch, victim);
}

/*
 * Charge - Minotaur racial ability
 * Combat opener that gores the victim with horns
 */
void do_charge (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;

    one_argument (argument, arg);

    /* Check if character is a minotaur */
    if (!IS_NPC(ch) && ch->race != race_lookup("minotaur"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Charge whom?\n\r", ch);
        return;
    }

    if (ch->fighting != NULL)
    {
        send_to_char("You need room to build up speed for a charge.\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("That would hurt!\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    if (IS_NPC(victim) && victim->fighting != NULL && !is_same_group(ch, victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r", ch);
        return;
    }

    WAIT_STATE(ch, skill_table[gsn_backstab].beats);
    
    /* Heavy damage, similar to backstab */
    dam = number_range(ch->level * 2, ch->level * 4);
    dam += get_curr_stat(ch, STAT_STR) * 3;

    act("{R$n lowers $s head and CHARGES at you, horns first!{x", ch, NULL, victim, TO_VICT);
    act("{RYou lower your head and CHARGE at $N, horns first!{x", ch, NULL, victim, TO_CHAR);
    act("{R$n lowers $s head and CHARGES at $N, horns first!{x", ch, NULL, victim, TO_NOTVICT);

    damage(ch, victim, dam, gsn_charge, DAM_PIERCE, FALSE, FALSE);
    check_killer(ch, victim);
}

/*
 * Stoneform - Gargoyle racial ability
 * Transforms skin to stone with escalating bonuses by level
 */
void do_stoneform (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    int ac_bonus, str_bonus, con_bonus, hit_bonus, dam_bonus, save_bonus, dex_penalty;
    
    /* Check if character is a gargoyle */
    if (!IS_NPC(ch) && ch->race != race_lookup("gargoyle"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already in stoneform */
    if (is_affected(ch, gsn_stoneform))
    {
        send_to_char("You are already in stoneform.\n\r", ch);
        return;
    }
    
    /* Calculate bonuses based on level */
    ac_bonus = -10;
    str_bonus = 0;
    con_bonus = 0;
    hit_bonus = 0;
    dam_bonus = 0;
    save_bonus = 0;
    dex_penalty = -2;  /* Always -2 DEX */
    
    if (ch->level >= 5)
        con_bonus += 1;
    
    if (ch->level >= 10)
    {
        con_bonus += 1;  /* Total +2 */
        str_bonus += 1;
    }
    
    if (ch->level >= 15)
    {
        str_bonus += 1;  /* Total +2 */
        dam_bonus += 5;
    }
    
    if (ch->level >= 20)
    {
        dam_bonus += 10;  /* Total +15 */
        hit_bonus += 5;
    }
    
    if (ch->level >= 30)
    {
        dam_bonus += 10;  /* Total +25 */
        hit_bonus += 5;   /* Total +10 */
    }
    
    if (ch->level >= 40)
    {
        save_bonus -= 10;
    }
    
    if (ch->level >= 50)
    {
        ac_bonus -= 10;    /* Total -20 */
        con_bonus += 3;    /* Total +5 */
        str_bonus += 3;    /* Total +5 */
        hit_bonus += 15;   /* Total +25 */
        save_bonus -= 10;  /* Total -20 */
    }
    
    /* Apply AC bonus */
    af.where     = TO_AFFECTS;
    af.type      = gsn_stoneform;
    af.level     = ch->level;
    af.duration  = 100;
    af.location  = APPLY_AC;
    af.modifier  = ac_bonus;
    af.bitvector = 0;
    affect_to_char(ch, &af);
    
    /* Apply DEX penalty */
    af.location  = APPLY_DEX;
    af.modifier  = dex_penalty;
    affect_to_char(ch, &af);
    
    /* Apply STR bonus if any */
    if (str_bonus > 0)
    {
        af.location  = APPLY_STR;
        af.modifier  = str_bonus;
        affect_to_char(ch, &af);
    }
    
    /* Apply CON bonus if any */
    if (con_bonus > 0)
    {
        af.location  = APPLY_CON;
        af.modifier  = con_bonus;
        affect_to_char(ch, &af);
    }
    
    /* Apply hitroll bonus if any */
    if (hit_bonus > 0)
    {
        af.location  = APPLY_HITROLL;
        af.modifier  = hit_bonus;
        affect_to_char(ch, &af);
    }
    
    /* Apply damroll bonus if any */
    if (dam_bonus > 0)
    {
        af.location  = APPLY_DAMROLL;
        af.modifier  = dam_bonus;
        affect_to_char(ch, &af);
    }
    
    /* Apply saves bonus if any */
    if (save_bonus < 0)
    {
        af.location  = APPLY_SAVING_SPELL;
        af.modifier  = save_bonus;
        affect_to_char(ch, &af);
    }
    
    act("{8Your skin transforms into hard stone!{x", ch, NULL, NULL, TO_CHAR);
    act("{8$n's skin transforms into hard stone!{x", ch, NULL, NULL, TO_ROOM);
}

/*
 * Flesh - Gargoyle racial ability
 * Turns off stoneform so it can be reactivated for updated bonuses
 */
void do_flesh (CHAR_DATA *ch, char *argument)
{
    /* Check if character is a gargoyle */
    if (!IS_NPC(ch) && ch->race != race_lookup("gargoyle"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if in stoneform */
    if (!is_affected(ch, gsn_stoneform))
    {
        send_to_char("You are not in stoneform.\n\r", ch);
        return;
    }
    
    /* Remove all stoneform affects */
    affect_strip(ch, gsn_stoneform);
    
    send_to_char("{8Your skin softens and returns to flesh.{x\n\r", ch);
    act("{8$n's skin softens and returns to flesh.{x", ch, NULL, NULL, TO_ROOM);
}

/*
 * Regeneration - Troll racial ability
 * Restores 50% HP out of combat, 25% HP in combat
 */
void do_regeneration (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    int heal_amount;
    
    /* Check if character is a troll */
    if (!IS_NPC(ch) && ch->race != race_lookup("troll"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already weary from regeneration */
    if (IS_AFFECTED(ch, AFF_WEARINESS))
    {
        send_to_char("Your regenerative abilities are still recovering.\n\r", ch);
        return;
    }
    
    /* Determine heal amount based on combat status */
    if (ch->fighting != NULL)
    {
        heal_amount = ch->max_hit / 4;  /* 25% in combat */
        send_to_char("{GYour wounds rapidly close as you regenerate!{x\n\r", ch);
        act("{G$n's wounds rapidly close as $e regenerates!{x", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
        heal_amount = ch->max_hit / 2;  /* 50% out of combat */
        send_to_char("{GYour body surges with regenerative power!{x\n\r", ch);
        act("{G$n's body surges with regenerative power!{x", ch, NULL, NULL, TO_ROOM);
    }
    
    ch->hit = UMIN(ch->hit + heal_amount, ch->max_hit);
    
    /* Apply weariness for 5 ticks */
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weariness");
    af.level     = ch->level;
    af.duration  = 5;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_WEARINESS;
    affect_to_char(ch, &af);
}

/*
 * Poisonous Bite - Naga racial ability
 * Combat attack with chance to poison
 */
void do_bite (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int dam;
    int poison_chance;

    one_argument (argument, arg);

    /* Check if character is a naga */
    if (!IS_NPC(ch) && ch->race != race_lookup("naga"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("Bite whom?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You can't bite yourself.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    /* Moderate damage */
    dam = number_range(ch->level / 2, ch->level);
    dam += get_curr_stat(ch, STAT_STR);

    act("{gYou strike $N with your venomous fangs!{x", ch, NULL, victim, TO_CHAR);
    act("{g$n strikes you with $s venomous fangs!{x", ch, NULL, victim, TO_VICT);
    act("{g$n strikes $N with $s venomous fangs!{x", ch, NULL, victim, TO_NOTVICT);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    damage(ch, victim, dam, gsn_backstab, DAM_PIERCE, FALSE, FALSE);
    
    /* Chance to poison - 50% base + level difference */
    poison_chance = 50 + (ch->level - victim->level) * 2;
    
    if (number_percent() < poison_chance && !IS_AFFECTED(victim, AFF_POISON) 
        && !saves_spell(ch->level, victim, DAM_POISON))
    {
        af.where     = TO_AFFECTS;
        af.type      = gsn_poison;
        af.level     = ch->level;
        af.duration  = ch->level / 5;
        af.location  = APPLY_STR;
        af.modifier  = -2;
        af.bitvector = AFF_POISON;
        affect_join(victim, &af);
        send_to_char("{gYou feel poison coursing through your veins!{x\n\r", victim);
        act("{g$n looks very ill.{x", victim, NULL, NULL, TO_ROOM);
    }
    
    check_killer(ch, victim);
}

/*
 * Tail Whip - Lizard-man racial ability
 * Combat attack with chance to trip
 */
void do_tailwhip (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;
    int trip_chance;

    one_argument (argument, arg);

    /* Check if character is a lizard-man */
    if (!IS_NPC(ch) && ch->race != race_lookup("lizard-man"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("Whip whom with your tail?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You can't whip yourself with your tail.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    /* Moderate damage */
    dam = number_range(ch->level / 2, ch->level + ch->level / 4);
    dam += get_curr_stat(ch, STAT_STR);

    act("{2You swing your powerful tail at $N!{x", ch, NULL, victim, TO_CHAR);
    act("{2$n swings $s powerful tail at you!{x", ch, NULL, victim, TO_VICT);
    act("{2$n swings $s powerful tail at $N!{x", ch, NULL, victim, TO_NOTVICT);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    damage(ch, victim, dam, gsn_trip, DAM_BASH, FALSE, FALSE);
    
    /* Chance to trip - based on size difference and level */
    trip_chance = 40 + (ch->size - victim->size) * 10 + (ch->level - victim->level);
    
    if (victim->position > POS_DEAD && number_percent() < trip_chance 
        && !IS_AFFECTED(victim, AFF_FLYING))
    {
        victim->position = POS_RESTING;
        DAZE_STATE(victim, PULSE_VIOLENCE);
        act("{2You knock $N to the ground!{x", ch, NULL, victim, TO_CHAR);
        act("{2$n knocks you to the ground!{x", ch, NULL, victim, TO_VICT);
        act("{2$n knocks $N to the ground!{x", ch, NULL, victim, TO_NOTVICT);
    }
    
    check_killer(ch, victim);
}

/*
 * Bloodlust - Uruk-hai racial ability
 * Buff: +10 hitroll, +15 damroll, +3 STR
 */
void do_bloodlust (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    
    /* Check if character is an uruk-hai */
    if (!IS_NPC(ch) && ch->race != race_lookup("uruk-hai"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already in bloodlust */
    if (is_affected(ch, skill_lookup("berserk")))
    {
        send_to_char("You are already in a battle rage.\n\r", ch);
        return;
    }
    
    send_to_char("{RYou whip yourself into a savage BLOODLUST!{x\n\r", ch);
    act("{R$n whips $mself into a savage BLOODLUST!{x", ch, NULL, NULL, TO_ROOM);
    
    /* +10 hitroll */
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("berserk");
    af.level     = ch->level;
    af.duration  = ch->level / 3;
    af.location  = APPLY_HITROLL;
    af.modifier  = 10;
    af.bitvector = 0;
    affect_to_char(ch, &af);
    
    /* +15 damroll */
    af.location  = APPLY_DAMROLL;
    af.modifier  = 15;
    affect_to_char(ch, &af);
    
    /* +3 STR */
    af.location  = APPLY_STR;
    af.modifier  = 3;
    affect_to_char(ch, &af);
}

/*
 * Fade - Stalker racial ability
 * Toggle invisibility
 */
void do_fade (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    
    /* Check if character is a stalker */
    if (!IS_NPC(ch) && ch->race != race_lookup("stalker"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already faded */
    if (IS_AFFECTED(ch, AFF_INVISIBLE))
    {
        send_to_char("You are already faded into darkness.\n\r", ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("invisibility");
    af.level     = ch->level;
    af.duration  = -1;  /* Permanent until dispelled or combat */
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char(ch, &af);
    
    send_to_char("{8You fade into the darkness.{x\n\r", ch);
    act("{8$n fades into the darkness.{x", ch, NULL, NULL, TO_ROOM);
}

/*
 * Flanking - Kenku racial ability
 * Backstab variant - full damage if opener, 2/3 damage in combat
 */
void do_flank (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int dam_mult;

    one_argument (argument, arg);

    /* Check if character is a kenku */
    if (!IS_NPC(ch) && ch->race != race_lookup("kenku"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (arg[0] == '\0')
    {
        /* If already in combat, can flank current opponent */
        if (ch->fighting != NULL)
        {
            victim = ch->fighting;
        }
        else
        {
            send_to_char("Flank whom?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You can't flank yourself.\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
        return;

    if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
        send_to_char("You need a weapon to flank.\n\r", ch);
        return;
    }

    /* Determine damage multiplier */
    if (ch->fighting == NULL)
    {
        /* Combat opener - full backstab damage */
        dam_mult = 1;
        act("{6You slip behind $N and strike!{x", ch, NULL, victim, TO_CHAR);
        act("{6$n slips behind you and strikes!{x", ch, NULL, victim, TO_VICT);
        act("{6$n slips behind $N and strikes!{x", ch, NULL, victim, TO_NOTVICT);
    }
    else
    {
        /* In combat - 2/3 backstab damage */
        dam_mult = 2;  /* Will divide by 3 later */
        act("{6You slip to $N's flank and strike!{x", ch, NULL, victim, TO_CHAR);
        act("{6$n slips to your flank and strikes!{x", ch, NULL, victim, TO_VICT);
        act("{6$n slips to $N's flank and strikes!{x", ch, NULL, victim, TO_NOTVICT);
    }

    WAIT_STATE(ch, skill_table[gsn_backstab].beats);
    
    /* Use backstab gsn but adjust damage */
    if (dam_mult == 1)
    {
        multi_hit(ch, victim, gsn_backstab);
    }
    else
    {
        /* 2/3 damage version */
        int dam = (ch->level * 2 / 3) + number_range(ch->level, ch->level * 2);
        dam = (dam * 2) / 3;
        damage(ch, victim, dam, gsn_backstab, DAM_NONE, TRUE, FALSE);
    }
    
    check_killer(ch, victim);
}

/*
 * Darkness - Drow racial ability
 * Creates impenetrable darkness for 2 ticks
 */
void do_darkness (CHAR_DATA *ch, char *argument)
{
    /* Check if character is a drow */
    if (!IS_NPC(ch) && ch->race != race_lookup("drow"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if room already has magical darkness */
    if (IS_SET(ch->in_room->room_flags, ROOM_MALEDICTION))
    {
        send_to_char("This room is already shrouded in magical darkness.\n\r", ch);
        return;
    }
    
    SET_BIT(ch->in_room->room_flags, ROOM_MALEDICTION);
    ch->in_room->darkness_timer = 2;  /* 2 ticks */
    
    send_to_char("{8You summon a globe of absolute darkness!{x\n\r", ch);
    act("{8$n summons a globe of absolute darkness!{x", ch, NULL, NULL, TO_ROOM);
    act("{8Everything goes pitch black!{x", ch, NULL, NULL, TO_ROOM);
}

/*
 * Goblin Mob - Hobgoblin racial ability
 * Summons a trio of goblins as a charmed pet
 */
void do_goblinmob (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gob;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;
    
    /* Check if character is a hobgoblin */
    if (!IS_NPC(ch) && ch->race != race_lookup("hobgoblin"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    /* Check if already has a pet */
    if (ch->pet != NULL)
    {
        send_to_char("You already have a pet following you.\n\r", ch);
        return;
    }
    
    /* Check for weariness cooldown */
    if (IS_AFFECTED(ch, AFF_WEARINESS))
    {
        send_to_char("You cannot summon more goblins yet.\n\r", ch);
        return;
    }
    
    /* Try to load the goblin trio mob - if it doesn't exist, create a basic one */
    pMobIndex = get_mob_index(MOB_VNUM_GOBLIN_TRIO);
    
    if (pMobIndex == NULL)
    {
        send_to_char("The goblin mob is not available (MOB_VNUM 3091 not found).\n\r", ch);
        send_to_char("An immortal needs to create a mob with vnum 3091 called 'trio of goblins'.\n\r", ch);
        return;
    }
    
    gob = create_mobile(pMobIndex);
    char_to_room(gob, ch->in_room);
    
    /* Make it a charmed pet */
    SET_BIT(gob->act, ACT_PET);
    STR_SET_BIT(gob->affected_by, AFF_CHARM);
    gob->leader = ch;
    gob->master = ch;
    ch->pet = gob;
    add_follower(gob, ch);
    
    send_to_char("{yYou whistle sharply and three goblins scramble to your aid!{x\n\r", ch);
    act("{y$n whistles sharply and three goblins scramble to $s aid!{x", ch, NULL, NULL, TO_ROOM);
    
    /* Apply weariness for 20 ticks */
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weariness");
    af.level     = ch->level;
    af.duration  = 20;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_WEARINESS;
    affect_to_char(ch, &af);
}

/*
 * War Chant - Orc racial ability
 * Buffs all orc-faction allies in the room with HP/MOVE/STR bonuses
 */
void do_chant (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;
    int affected = 0;

    /* Check if character is an orc */
    if (!IS_NPC(ch) && ch->race != race_lookup("orc"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    act("{RYou pound your chest and launch into a thunderous war chant!{x", ch, NULL, NULL, TO_CHAR);
    act("{R$n pounds $s chest and launches into a thunderous war chant!{x", ch, NULL, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (vch->faction != FACTION_ORCISH)
            continue;

        /* Refresh existing chant effects */
        if (is_affected(vch, gsn_chant))
            affect_strip(vch, gsn_chant);

        af.where     = TO_AFFECTS;
        af.type      = gsn_chant;
        af.level     = ch->level;
        af.duration  = 20;
        af.bitvector = 0;

        /* +20 hit points */
        af.location  = APPLY_HIT;
        af.modifier  = 20;
        affect_to_char(vch, &af);

        /* +40 move */
        af.location  = APPLY_MOVE;
        af.modifier  = 40;
        affect_to_char(vch, &af);

        /* +3 strength */
        af.location  = APPLY_STR;
        af.modifier  = 3;
        affect_to_char(vch, &af);

        /* Top off current pools to match the new maximums */
        vch->hit = UMIN(vch->hit + 20, vch->max_hit);
        vch->move = UMIN(vch->move + 40, vch->max_move);

        if (vch == ch)
        {
            send_to_char("{RThe war chant fills you with savage vigor!{x\n\r", ch);
        }
        else
        {
            act("{R$n's war chant fills you with savage vigor!{x", ch, NULL, vch, TO_VICT);
        }

        affected++;
    }

    if (affected == 0)
    {
        send_to_char("No orcish allies are here to answer your chant.\n\r", ch);
    }
}

/*
 * War Cry - Orc racial ability
 * Intimidates all enemies in the room with a battle cry
 */
void do_warcry (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;
    int affected_count = 0;
    
    /* Check if character is an orc */
    if (!IS_NPC(ch) && ch->race != race_lookup("orc"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    if (ch->fighting == NULL)
    {
        send_to_char("You must be in combat to use your war cry.\n\r", ch);
        return;
    }
    
    send_to_char("{RYOU LET OUT A TERRIFYING WAR CRY!{x\n\r", ch);
    act("{R$n lets out a TERRIFYING WAR CRY!{x", ch, NULL, NULL, TO_ROOM);
    
    WAIT_STATE(ch, PULSE_VIOLENCE);
    
    /* Affect all fighting enemies in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
        vch_next = vch->next_in_room;
        
        /* Skip non-enemies */
        if (vch == ch || is_same_group(ch, vch))
            continue;
            
        /* Skip those already affected */
        if (is_affected(vch, gsn_warcry))
            continue;
            
        /* Only affect those fighting or hostile */
        if (vch->fighting != ch && ch->fighting != vch)
            continue;
        
        /* Intimidation effect - chance to flee for NPCs */
        if (IS_NPC(vch) && number_percent() < 25 + (ch->level - vch->level))
        {
            /* Mob flees in fear */
            act("{yYou flee in terror from the war cry!{x", vch, NULL, NULL, TO_CHAR);
            act("{y$n flees in terror!{x", vch, NULL, NULL, TO_ROOM);
            /* Don't actually make them flee, just skip to next victim */
            continue;
        }
        
        /* Apply combat penalties */
        af.where     = TO_AFFECTS;
        af.type      = gsn_warcry;
        af.level     = ch->level;
        af.duration  = 3 + ch->level / 10;
        af.location  = APPLY_HITROLL;
        af.modifier  = -5;
        af.bitvector = 0;
        affect_to_char(vch, &af);
        
        af.location  = APPLY_DAMROLL;
        af.modifier  = -5;
        affect_to_char(vch, &af);
        
        send_to_char("{yYou are intimidated by the war cry!{x\n\r", vch);
        affected_count++;
    }
    
    if (affected_count > 0)
    {
        send_to_char("{yYour enemies cower before your might!{x\n\r", ch);
    }
}
