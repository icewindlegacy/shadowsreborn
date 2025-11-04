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
 *           magic_orc.c - November 3, 2025
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
    af.duration = 6 + level;
    af.location = APPLY_HITROLL;
    af.modifier = level / 8;
    af.bitvector = 0;
    affect_to_char (victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 8;
    affect_to_char (victim, &af);
    send_to_char ("You feel empowered by the spirits.\n\r", victim);
    if (ch != victim)
        act ("You grant $N the favor of the spirits.", ch, NULL, victim,
             TO_CHAR);
    return;
}

void spell_ugha (int sn, int level, CHAR_DATA * ch, void *vo,
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

