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
 *           magic_orc.c - November 13, 2025
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
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include "merc.h"
 #include "interp.h"
 #include "magic.h"
 #include "recycle.h"

 extern char *target_name;
 bool saves_dispel args ((int dis_level, int spell_level, int duration));
 
 void spell_groth_skihea (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT (obj, ITEM_GROTH_SKIHEA))
        {
            act ("$p is already empowered by the spirits.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT (obj, ITEM_GOOD))
        {
            AFFECT_DATA *paf;

            paf = affect_find (obj->affected, gsn_curse);
            if (!saves_dispel
                (level, paf != NULL ? paf->level : obj->level, 0))
            {
                if (paf != NULL)
                    affect_remove_obj (obj, paf);
                act ("$p glows a pale red.", ch, obj, NULL, TO_ALL);
                REMOVE_BIT (obj->extra_flags, ITEM_GOOD);
                return;
            }
            else
            {
                act ("The goodness of $p is too powerful for you to overcome.",
                     ch, obj, NULL, TO_CHAR);
                return;
            }
        }

        af.where = TO_OBJECT;
        af.type = sn;
        af.level = level;
        af.duration = 6 + level;
        af.location = APPLY_SAVES;
        af.modifier = -1;
        af.bitvector = ITEM_GROTH_SKIHEA;
        affect_to_obj (obj, &af);

        act ("$p glows with an unholy aura.", ch, obj, NULL, TO_ALL);

        if (obj->wear_loc != WEAR_NONE)
            ch->saving_throw -= 1;
        return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if (victim->position == POS_FIGHTING || is_affected (victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already empowered by the spirits.\n\r", ch);
        else
            act ("$N already has the favor of the spirits.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_DAMROLL;
    af.modifier = level / 16;
    af.bitvector = 0;
    affect_to_char (victim, &af);
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_HIT;
    af.modifier = level / 2;
    af.bitvector = 0;
    affect_to_char (victim, &af);


    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 12;
    affect_to_char (victim, &af);
    send_to_char ("You feel empowered by the spirits.\n\r", victim);
    if (ch != victim)
        act ("You grant $N the favor of the spirits.", ch, NULL, victim,
             TO_CHAR);
    return;
}

void spell_ugah (int sn, int level, CHAR_DATA * ch, void *vo,
    int target)
{
OBJ_DATA *grubs;

grubs = create_object (get_obj_index (OBJ_VNUM_GRUBS), 0);
grubs->value[0] = level / 2;
grubs->value[1] = level;
obj_to_room (grubs, ch->in_room);
act ("$p wriggle out of the ground.", ch, grubs, NULL, TO_ROOM);
act ("$p wriggle out of the ground.", ch, grubs, NULL, TO_CHAR);
return;
}

/* Bahg Grock - Call forth Morgoth's demons */
void spell_bahg_grock (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (ch->alignment > -800)
    {
        send_to_char ("You are not evil enough to call forth Morgoth's demons!\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("You cannot cast this on yourself.\n\r", ch);
        return;
    }

    dam = dice(level, 10);
    
    act ("{RDemonic spirits from the darkest pits swarm around $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{R$n calls forth Morgoth's demons to torment $N!{x", ch, NULL, victim, TO_NOTVICT);
    act ("{RDemonic spirits tear at your flesh!{x", ch, NULL, victim, TO_VICT);
    
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
    return;
}

/* Ghaash Bogh - Calls all that is unholy */
void spell_ghaash_bogh (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int align_bonus;

    if (victim == ch)
    {
        send_to_char ("You cannot cast this on yourself.\n\r", ch);
        return;
    }

    /* Base damage */
    dam = dice(level, 8);
    
    /* Extra damage based on victim's alignment (good victims take more damage) */
    align_bonus = victim->alignment / 100;
    if (align_bonus > 0)
        dam += align_bonus;
    
    if (victim->alignment > 500)
    {
        act ("{DThe forces of darkness strike $N with terrible fury!{x", ch, NULL, victim, TO_CHAR);
        act ("{D$n calls down the forces of darkness upon $N!{x", ch, NULL, victim, TO_NOTVICT);
        act ("{DUnholy power tears at your very soul!{x", ch, NULL, victim, TO_VICT);
    }
    else
    {
        act ("{DAn unholy presence engulfs $N!{x", ch, NULL, victim, TO_CHAR);
        act ("{D$n unleashes dark powers upon $N!{x", ch, NULL, victim, TO_NOTVICT);
        act ("{DDarkness claws at your essence!{x", ch, NULL, victim, TO_VICT);
    }
    
    damage (ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
    return;
}

/* Demonspawn - Summon a demon */
void spell_demonspawn (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *demon;
    AFFECT_DATA af;
    int i;

    if (is_affected(ch, sn))
    {
        send_to_char ("You cannot summon another demon yet.\n\r", ch);
        return;
    }

    if (number_percent() > (50 + level))
    {
        send_to_char ("You fail to control the demon and it escapes!\n\r", ch);
        return;
    }

    demon = create_mobile (get_mob_index (MOB_VNUM_DEMON));
    
    /* Set demon stats based on caster level */
    for (i = 0; i < MAX_STATS; i++)
        demon->perm_stat[i] = UMIN(25, 11 + level / 5);
    
    demon->max_hit = ch->level * 20 + number_range(ch->level * ch->level, ch->level * ch->level * 2);
    demon->hit = demon->max_hit;
    demon->max_mana = 100;
    demon->mana = demon->max_mana;
    demon->alignment = -1000;
    demon->level = ch->level;
    demon->armor[AC_PIERCE] = interpolate(demon->level, 100, -100);
    demon->armor[AC_BASH] = interpolate(demon->level, 100, -100);
    demon->armor[AC_SLASH] = interpolate(demon->level, 100, -100);
    demon->armor[AC_EXOTIC] = interpolate(demon->level, 100, -100);
    demon->gold = 0;
    demon->timer = 0;
    demon->damage[DICE_NUMBER] = number_range(level / 15, level / 10);
    demon->damage[DICE_TYPE] = number_range(level / 3, level / 2);
    demon->damage[DICE_BONUS] = number_range(level / 8, level / 6);

    char_to_room (demon, ch->in_room);
    send_to_char ("A demon materializes from the darkest planes!\n\r", ch);
    act ("$n summons a demon from the abyss!", ch, NULL, NULL, TO_ROOM);

    /* Make demon charmed and a pet */
    SET_BIT (demon->act, ACT_PET);
    af.where = TO_AFFECTS;
    af.type = gsn_charm_person;
    af.level = level;
    af.duration = -1;  /* Permanent until killed */
    af.bitvector = AFF_CHARM;
    af.modifier = 0;
    af.location = 0;
    affect_to_char (demon, &af);
    
    add_follower (demon, ch);
    demon->leader = ch;

    /* Put a cooldown on the caster */
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.bitvector = 0;
    af.modifier = 0;
    af.location = 0;
    affect_to_char (ch, &af);

    return;
}

/* Typhoon - Summons tropical storms */
void spell_typhoon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level, 12);
    
    if (saves_spell(level, victim, DAM_DROWNING))
        dam /= 2;

    act ("{CA massive typhoon engulfs $N!{x", ch, NULL, victim, TO_CHAR);
    act ("{C$n summons a torrential typhoon upon $N!{x", ch, NULL, victim, TO_NOTVICT);
    act ("{CYou are battered by torrential rain and wind!{x", ch, NULL, victim, TO_VICT);
    
    damage (ch, victim, dam, sn, DAM_DROWNING, TRUE, TRUE);
    return;
}

/* Dead Soul - Summon dead soul light */
void spell_dead_soul (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *soul;

    soul = create_object (get_obj_index (OBJ_VNUM_DEAD_SOUL), 0);
    soul->timer = 10 + level;
    obj_to_char (soul, ch);
    
    act ("{DA tormented soul appears and follows you!{x", ch, soul, NULL, TO_CHAR);
    act ("{D$n summons the soul of a dead human!{x", ch, soul, NULL, TO_ROOM);
    return;
}

/* Fire Shield - Aura of flame */
void spell_fire_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already surrounded by a fire shield.\n\r", ch);
        else
            act ("$N is already protected by a fire shield.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    act ("{R$n is surrounded by a shield of flames!{x", victim, NULL, NULL, TO_ROOM);
    act ("{RYou are surrounded by a shield of flames!{x", victim, NULL, NULL, TO_CHAR);
    return;
}

/* Ice Shield - Aura of frost */
void spell_ice_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already surrounded by an ice shield.\n\r", ch);
        else
            act ("$N is already protected by an ice shield.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    act ("{C$n is surrounded by a shield of ice!{x", victim, NULL, NULL, TO_ROOM);
    act ("{CYou are surrounded by a shield of ice!{x", victim, NULL, NULL, TO_CHAR);
    return;
}

/* Lightning Shield - Aura of electricity */
void spell_lightning_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        if (victim == ch)
            send_to_char ("You are already surrounded by a lightning shield.\n\r", ch);
        else
            act ("$N is already protected by a lightning shield.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    act ("{Y$n is surrounded by a shield of lightning!{x", victim, NULL, NULL, TO_ROOM);
    act ("{YYou are surrounded by a shield of lightning!{x", victim, NULL, NULL, TO_CHAR);
    return;
}

/* Blood Bath - Summons fountain of blood */
void spell_blood_bath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *fountain;

    fountain = create_object (get_obj_index (OBJ_VNUM_BLOODBATH), 0);
    fountain->timer = level;
    obj_to_room (fountain, ch->in_room);
    
    act ("{R$p bubbles up from the ground!{x", ch, fountain, NULL, TO_ROOM);
    act ("{RYou summon $p from the depths!{x", ch, fountain, NULL, TO_CHAR);
    return;
}

/* Invoke Water - Same as create water */
void spell_invoke_water (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if (obj->item_type != ITEM_DRINK_CON)
    {
        send_to_char ("It is unable to hold water.\n\r", ch);
        return;
    }

    if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
    {
        send_to_char ("It contains some other liquid.\n\r", ch);
        return;
    }

    water = UMIN (level * (weather_info->sky >= SKY_RAINING ? 4 : 2),
                  obj->value[0] - obj->value[1]);

    if (water > 0)
    {
        obj->value[2] = LIQ_WATER;
        obj->value[1] += water;
        if (!is_name ("water", obj->name))
        {
            char buf[MAX_STRING_LENGTH];
            sprintf (buf, "%s water", obj->name);
            free_string (obj->name);
            obj->name = str_dup (buf);
        }
        act ("{C$p is filled with water.{x", ch, obj, NULL, TO_CHAR);
    }

    return;
}

/* Decompose - Similar to heat metal */
void spell_decompose (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

    if (!saves_spell (level + 2, victim, DAM_ACID) 
        && !IS_SET (victim->imm_flags, IMM_ACID))
    {
        for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
        {
            obj_next = obj_lose->next_content;
            if (number_range (1, 2 * level) > obj_lose->level
                && !saves_spell (level, victim, DAM_ACID)
                && !IS_OBJ_STAT (obj_lose, ITEM_NONMETAL)
                && !IS_OBJ_STAT (obj_lose, ITEM_BURN_PROOF))
            {
                switch (obj_lose->item_type)
                {
                case ITEM_ARMOR:
                case ITEM_WEAPON:
                case ITEM_CONTAINER:
                    fail = FALSE;
                    act ("{g$p decomposes and crumbles!{x", victim, obj_lose, NULL, TO_CHAR);
                    act ("{g$p decomposes and crumbles!{x", victim, obj_lose, NULL, TO_ROOM);
                    extract_obj (obj_lose);
                    dam += dice (2, level / 2);
                }
            }
        }
    }

    if (fail)
    {
        send_to_char ("Your spell had no effect.\n\r", ch);
        send_to_char ("You feel momentarily weakened.\n\r", victim);
        return;
    }

    dam = (dam > 100) ? 100 : dam;
    damage (ch, victim, dam, sn, DAM_ACID, TRUE, TRUE);
    return;
}

/* Battlefavor - Damroll and hitroll buff */
void spell_battlefavor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected (victim, sn))
    {
        if (victim == ch)
            send_to_char ("You already have the battlefavor.\n\r", ch);
        else
            act ("$N already has battlefavor.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_DAMROLL;
    af.modifier = level / 5;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    af.location = APPLY_HITROLL;
    af.modifier = level / 8;
    affect_to_char (victim, &af);

    send_to_char ("{WYou feel empowered for battle!{x\n\r", victim);
    if (ch != victim)
        act ("{WYou grant $N favor in battle!{x", ch, NULL, victim, TO_CHAR);
    return;
}

/* Forcefield - room barrier spell (apply to caster) */
void spell_forcefield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    AFFECT_DATA af;
    
    if (is_affected(ch, sn))
    {
        send_to_char("You already have a forcefield active.\n\r", ch);
        return;
    }
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = 0;
    af.modifier = ch->in_room->vnum;  /* Store room vnum */
    af.bitvector = 0;
    affect_to_char(ch, &af);
    
    send_to_char("{YAn impenetrable forcefield surrounds the room!{x\n\r", ch);
    act("{Y$n creates a shimmering forcefield around the room!{x", ch, NULL, NULL, TO_ROOM);
    return;
}

/* Sphere of Summoning - enhanced summon */
void spell_sphere_summoning (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    bool bypass_nosummon = (number_range(1, 100) <= 2);  /* 2% chance */

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || victim->level >= level + 5
        || (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL)
        || victim->fighting != NULL
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
        || (!bypass_nosummon && !IS_NPC (victim) && IS_SET (victim->act, PLR_NOSUMMON))
        || (IS_NPC (victim) && saves_spell (level - 5, victim, DAM_OTHER)))
    {
        send_to_char ("The sphere of summoning fails.\n\r", ch);
        return;
    }

    if (bypass_nosummon && !IS_NPC(victim) && IS_SET(victim->act, PLR_NOSUMMON))
    {
        act ("{RA powerful sphere of energy forces you through the planes!{x", ch, NULL, victim, TO_VICT);
    }

    act ("{Y$n creates a glowing sphere of energy!{x", ch, NULL, NULL, TO_ROOM);
    act ("$n is pulled into a sphere of energy and vanishes!", victim, NULL, NULL, TO_ROOM);
    char_from_room (victim);
    char_to_room (victim, ch->in_room);
    act ("$n appears within a sphere of energy!", victim, NULL, NULL, TO_ROOM);
    act ("$n has summoned you through a sphere of energy!", ch, NULL, victim, TO_VICT);
    do_function (victim, &do_look, "auto");
    return;
}

/* Mage Barrier - mage-only AC boost */
void spell_mage_barrier (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim != ch)
    {
        send_to_char("You can only cast this on yourself.\n\r", ch);
        return;
    }

    if (ch->class != 0 && ch->class != 4)  /* mage(0) or sorcerer(4) */
    {
        send_to_char("Only mages can cast this spell.\n\r", ch);
        return;
    }

    if (is_affected(ch, sn))
    {
        send_to_char("You already have a mage barrier.\n\r", ch);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -80;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    send_to_char("{MA shimmering magical barrier surrounds you!{x\n\r", ch);
    act("{M$n is surrounded by a shimmering magical barrier!{x", ch, NULL, NULL, TO_ROOM);
    return;
}

/* Soul Twist - flip alignment */
void spell_soul_twist (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (saves_spell(level, victim, DAM_MENTAL))
    {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    victim->alignment = -victim->alignment;
    
    act("{D$N's soul twists and writhes!{x", ch, NULL, victim, TO_CHAR);
    act("{DYour soul twists as your alignment is reversed!{x", ch, NULL, victim, TO_VICT);
    act("{D$N's soul twists and writhes!{x", ch, NULL, victim, TO_NOTVICT);
    return;
}

/* Turn - flee or switch sides */
void spell_turn (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int count = 0;

    if (saves_spell(level, victim, DAM_MENTAL))
    {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    /* Count how many are fighting the victim */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->fighting == victim)
            count++;
    }

    if (count <= 1)  /* One on one */
    {
        act("$N is frightened and flees!", ch, NULL, victim, TO_CHAR);
        act("You are overcome with fear!", ch, NULL, victim, TO_VICT);
        do_function(victim, &do_flee, "");
    }
    else  /* Multiple mobs */
    {
        /* Switch victim to fight for caster */
        if (victim->fighting)
            stop_fighting(victim, FALSE);
        
        /* Find someone attacking the caster to fight */
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (vch->fighting == ch && vch != victim)
            {
                set_fighting(victim, vch);
                act("$N is confused and attacks $n!", vch, NULL, victim, TO_NOTVICT);
                act("You confuse $N into fighting for you!", ch, NULL, victim, TO_CHAR);
                act("You are confused about who you're fighting!", ch, NULL, victim, TO_VICT);
                break;
            }
        }
    }
    return;
}

/* Thin - prevent eating/drinking */
void spell_thin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        send_to_char("They are already unable to eat or drink.\n\r", ch);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 5;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    act("$N looks gaunt and unable to consume food or drink!", ch, NULL, victim, TO_CHAR);
    act("You feel unable to eat or drink!", ch, NULL, victim, TO_VICT);
    act("$N looks gaunt and weakened!", ch, NULL, victim, TO_NOTVICT);
    return;
}

/* Weapon of Faith - enchant weapon */
void spell_weapon_faith (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON)
    {
        send_to_char("That isn't a weapon.\n\r", ch);
        return;
    }

    if (IS_OBJ_STAT(obj, ITEM_BLESS) || IS_WEAPON_STAT(obj, WEAPON_FLAMING) || 
        IS_WEAPON_STAT(obj, WEAPON_FROST) || IS_WEAPON_STAT(obj, WEAPON_SHOCKING))
    {
        send_to_char("This weapon already has a special power.\n\r", ch);
        return;
    }

    af.where = TO_WEAPON;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = 0;
    af.modifier = 0;
    
    /* Power based on alignment */
    if (ch->alignment >= 350)
        af.bitvector = WEAPON_SHOCKING;  /* Good - holy lightning */
    else if (ch->alignment <= -350)
        af.bitvector = WEAPON_VAMPIRIC;  /* Evil - vampiric */
    else
        af.bitvector = WEAPON_FLAMING;   /* Neutral - flame */
    
    affect_to_obj(obj, &af);

    act("$p glows with divine power!", ch, obj, NULL, TO_CHAR);
    act("$p glows with divine power!", ch, obj, NULL, TO_ROOM);
    return;
}

/* Infest - summon plague rats */
void spell_infest (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *rat;
    AFFECT_DATA af;
    int num_rats;
    int i;

    send_to_char("{gPlague-infested rats swarm into the room!{x\n\r", ch);
    act("{g$n summons a swarm of plague-infested rats!{x", ch, NULL, NULL, TO_ROOM);

    /* Summon 2-4 plague rats */
    num_rats = number_range(2, 4);
    
    for (i = 0; i < num_rats; i++)
    {
        rat = create_mobile(get_mob_index(MOB_VNUM_DEMON));
        
        /* Configure as a plague rat */
        free_string(rat->name);
        rat->name = str_dup("plague rat diseased");
        free_string(rat->short_descr);
        rat->short_descr = str_dup("a plague-infested rat");
        free_string(rat->long_descr);
        rat->long_descr = str_dup("A diseased rat scurries about, spreading plague.\n\r");
        
        rat->level = 1;
        rat->max_hit = 20;
        rat->hit = rat->max_hit;
        rat->damage[DICE_NUMBER] = 1;
        rat->damage[DICE_TYPE] = 3;
        rat->damage[DICE_BONUS] = 0;
        rat->armor[AC_PIERCE] = 50;
        rat->armor[AC_BASH] = 50;
        rat->armor[AC_SLASH] = 50;
        rat->armor[AC_EXOTIC] = 50;
        
        /* Give rat plague affect */
        af.where = TO_AFFECTS;
        af.type = gsn_plague;
        af.level = level;
        af.duration = -1;
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AFF_PLAGUE;
        affect_to_char(rat, &af);
        
        /* Make aggressive */
        SET_BIT(rat->act, ACT_AGGRESSIVE);
        
        /* Summon duration based on caster level */
        rat->timer = level / 5;
        
        char_to_room(rat, ch->in_room);
    }

    return;
}

/* Touch of Death - life/mana/move drain */
void spell_touch_death (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *gch;
    int dam, drain;

    if (ch->alignment > -350)
    {
        send_to_char("You are not evil enough to use touch of death!\n\r", ch);
        return;
    }

    dam = dice(level, 10);
    drain = dam / 2;

    if (saves_spell(level, victim, DAM_NEGATIVE))
        dam /= 2;

    act("{D$n's touch drains the life from $N!{x", ch, NULL, victim, TO_NOTVICT);
    act("{DYour touch drains the life from $N!{x", ch, NULL, victim, TO_CHAR);
    act("{D$n's touch drains your life force!{x", ch, NULL, victim, TO_VICT);

    damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);

    /* Drain to caster and group */
    ch->hit = UMIN(ch->max_hit, ch->hit + drain);
    ch->mana = UMIN(ch->max_mana, ch->mana + drain / 2);
    ch->move = UMIN(ch->max_move, ch->move + drain / 2);

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (is_same_group(gch, ch) && gch != ch)
        {
            gch->hit = UMIN(gch->max_hit, gch->hit + drain / 4);
            gch->mana = UMIN(gch->max_mana, gch->mana + drain / 4);
            send_to_char("You feel energy flowing into you!\n\r", gch);
        }
    }

    return;
}

/* Ghoul Touch - terror + slow + damage */
void spell_ghoul_touch (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice(level, 8);

    if (saves_spell(level, victim, DAM_NEGATIVE))
        dam /= 2;

    act("{g$n touches $N with ghoulish hands!{x", ch, NULL, victim, TO_NOTVICT);
    act("{gYou touch $N with ghoulish energy!{x", ch, NULL, victim, TO_CHAR);
    act("{g$n's ghoulish touch freezes you!{x", ch, NULL, victim, TO_VICT);

    damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);

    if (!saves_spell(level, victim, DAM_NEGATIVE))
    {
        int slow_sn = skill_lookup("slow");
        if (slow_sn > 0 && !is_affected(victim, slow_sn))
        {
            af.where = TO_AFFECTS;
            af.type = slow_sn;
            af.level = level;
            af.duration = level / 10;
            af.location = APPLY_DEX;
            af.modifier = -2;
            af.bitvector = 0;
            affect_to_char(victim, &af);

            send_to_char("You feel yourself slowing down.\n\r", victim);
        }
    }

    return;
}

/* Crippling Touch - damage + move drain */
void spell_crippling_touch (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, move_drain;

    dam = dice(level, 9);
    move_drain = level * 2;

    if (saves_spell(level, victim, DAM_NEGATIVE))
    {
        dam /= 2;
        move_drain /= 2;
    }

    act("{D$n's touch cripples $N!{x", ch, NULL, victim, TO_NOTVICT);
    act("{DYour touch cripples $N!{x", ch, NULL, victim, TO_CHAR);
    act("{D$n's crippling touch drains your energy!{x", ch, NULL, victim, TO_VICT);

    damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
    
    victim->move = UMAX(0, victim->move - move_drain);
    send_to_char("You feel your movement drained!\n\r", victim);

    return;
}

/* Blackmantle - raise AC + slow regen */
void spell_blackmantle (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        send_to_char("They are already surrounded by a blackmantle.\n\r", ch);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 4;
    af.location = APPLY_AC;
    af.modifier = level;  /* Raises AC = more vulnerable */
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_HITROLL;
    af.modifier = -level / 10;
    affect_to_char(victim, &af);

    act("{D$N is surrounded by a black energy field!{x", ch, NULL, victim, TO_CHAR);
    act("{DA black energy field surrounds you, weakening your defenses!{x", ch, NULL, victim, TO_VICT);
    act("{D$N is surrounded by a black energy field!{x", ch, NULL, victim, TO_NOTVICT);

    return;
}

/* Dark Rune - frost enchantment */
void spell_dark_rune (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON)
    {
        send_to_char("That isn't a weapon.\n\r", ch);
        return;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_FROST))
    {
        send_to_char("This weapon already has a frost rune.\n\r", ch);
        return;
    }

    af.where = TO_WEAPON;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj, &af);

    act("{CA dark rune of frost appears on $p!{x", ch, obj, NULL, TO_CHAR);
    act("{CA dark rune of frost appears on $p!{x", ch, obj, NULL, TO_ROOM);
    return;
}

/* Mirror Image - create duplicates */
void spell_mirror_image (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *image;
    AFFECT_DATA af;
    int num_images;
    int i;
    char buf[MAX_STRING_LENGTH];

    if (is_affected(ch, sn))
    {
        send_to_char("You already have mirror images.\n\r", ch);
        return;
    }

    num_images = UMIN(5, level / 10);
    if (num_images < 1)
        num_images = 1;

    send_to_char("{WYou create mirror images of yourself!{x\n\r", ch);
    act("{WMultiple images of $n appear!{x", ch, NULL, NULL, TO_ROOM);

    for (i = 0; i < num_images; i++)
    {
        image = create_mobile(get_mob_index(MOB_VNUM_DEMON));
        
        /* Copy name and descriptions exactly from caster to make them indistinguishable */
        free_string(image->name);
        sprintf(buf, "%s image duplicate", ch->name);
        image->name = str_dup(buf);
        
        free_string(image->short_descr);
        if (ch->short_descr && ch->short_descr[0] != '\0')
            image->short_descr = str_dup(ch->short_descr);
        else
            image->short_descr = str_dup(ch->name);
        
        free_string(image->long_descr);
        if (ch->long_descr && ch->long_descr[0] != '\0')
            image->long_descr = str_dup(ch->long_descr);
        else
        {
            sprintf(buf, "%s%s is here.\n\r", ch->name, ch->pcdata ? ch->pcdata->title : "");
            image->long_descr = str_dup(buf);
        }
        
        /* Make it look like the caster but be harmless */
        image->level = 1;
        image->sex = ch->sex;  /* Copy sex so "look" shows him/her not "it" */
        image->max_hit = 5;
        image->hit = image->max_hit;
        image->damage[DICE_NUMBER] = 1;
        image->damage[DICE_TYPE] = 1;
        image->damage[DICE_BONUS] = 0;
        image->armor[AC_PIERCE] = 100;
        image->armor[AC_BASH] = 100;
        image->armor[AC_SLASH] = 100;
        image->armor[AC_EXOTIC] = 100;
        
        /* Just an illusion that wanders independently */
        image->timer = level / 4;
        
        /* Don't make them follow - they wander independently */
        char_to_room(image, ch->in_room);
    }

    /* Add cooldown to caster */
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 4;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    return;
}

