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
 *           magic2.c - November 3, 2025
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


extern char *target_name;

void spell_farsight (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    if (IS_AFFECTED (ch, AFF_BLIND))
    {
        send_to_char ("Maybe it would help if you could see?\n\r", ch);
        return;
    }

    do_function (ch, &do_scan, target_name);
}


void spell_portal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || victim->in_room == NULL
        || !can_see_room (ch, victim->in_room)
        || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
        || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
        || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
        || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)
        || victim->level >= level + 3 || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)    /* NOT trust */
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_NONE))
        || (is_clan (victim) && !is_same_clan (ch, victim)))
    {
        send_to_char ("You failed.\n\r", ch);
        return;
    }

    stone = get_eq_char (ch, WEAR_HOLD);
    if (!IS_IMMORTAL (ch)
        && (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char ("You lack the proper component for this spell.\n\r",
                      ch);
        return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
        act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        extract_obj (stone);
    }

    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 2 + level / 25;
    portal->value[3] = victim->in_room->vnum;

    obj_to_room (portal, ch->in_room);

    act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
    act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);
}

void spell_nexus (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;

    if ((victim = get_char_world (ch, target_name)) == NULL
        || victim == ch
        || (to_room = victim->in_room) == NULL
        || !can_see_room (ch, to_room) || !can_see_room (ch, from_room)
        || IS_SET (to_room->room_flags, ROOM_SAFE)
        || IS_SET (from_room->room_flags, ROOM_SAFE)
        || IS_SET (to_room->room_flags, ROOM_PRIVATE)
        || IS_SET (to_room->room_flags, ROOM_SOLITARY)
        || IS_SET (to_room->room_flags, ROOM_NO_RECALL)
        || IS_SET (from_room->room_flags, ROOM_NO_RECALL)
        || victim->level >= level + 3 || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)    /* NOT trust */
        || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
        || (IS_NPC (victim) && saves_spell (level, victim, DAM_NONE))
        || (is_clan (victim) && !is_same_clan (ch, victim)))
    {
        send_to_char ("You failed.\n\r", ch);
        return;
    }

    stone = get_eq_char (ch, WEAR_HOLD);
    if (!IS_IMMORTAL (ch)
        && (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char ("You lack the proper component for this spell.\n\r",
                      ch);
        return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
        act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        extract_obj (stone);
    }

    /* portal one */
    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;

    obj_to_room (portal, from_room);

    act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
    act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;

    /* portal two */
    portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = 1 + level / 10;
    portal->value[3] = from_room->vnum;

    obj_to_room (portal, to_room);

    if (to_room->people != NULL)
    {
        act ("$p rises up from the ground.", to_room->people, portal, NULL,
             TO_ROOM);
        act ("$p rises up from the ground.", to_room->people, portal, NULL,
             TO_CHAR);
    }
}

/* Marque - mark a room for word of recall */
void spell_marque(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot marque locations.\n\r", ch);
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
    {
        send_to_char("This room cannot be marqued.\n\r", ch);
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY))
    {
        send_to_char("You cannot marque a private location.\n\r", ch);
        return;
    }

    ch->pcdata->marqued_room = ch->in_room->vnum;
    
    act("You marque this location with mystical energy.", ch, NULL, NULL, TO_CHAR);
    act("$n traces glowing runes in the air that fade into nothingness.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You may now use 'word of recall' to return here.\n\r", ch);
}

/* Pylon - create portal to clan's pylon in target area */
void spell_pylon(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    OBJ_DATA *pylon = NULL;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *pylon_room;
    AREA_DATA *area;
    char arg[MAX_INPUT_LENGTH];
    
    target_name = one_argument(target_name, arg);
    
    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use pylons.\n\r", ch);
        return;
    }
    
    if (ch->clan == 0)
    {
        send_to_char("You must be in a clan to use pylons.\n\r", ch);
        return;
    }
    
    if (arg[0] == '\0')
    {
        send_to_char("Which area's pylon do you wish to connect to?\n\r", ch);
        return;
    }
    
    /* Check for warp stone */
    stone = get_eq_char(ch, WEAR_HOLD);
    if (!IS_IMMORTAL(ch) && (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r", ch);
        return;
    }
    
    /* Search for the target area */
    for (area = area_first; area != NULL; area = area->next)
    {
        if (!str_prefix(arg, area->name))
            break;
    }
    
    if (area == NULL)
    {
        send_to_char("No such area found.\n\r", ch);
        return;
    }
    
    /* Search for a pylon in that area belonging to this clan */
    for (pylon = object_list; pylon != NULL; pylon = pylon->next)
    {
        if (pylon->item_type != ITEM_PYLON)
            continue;
            
        if (pylon->in_room == NULL)
            continue;
            
        if (pylon->in_room->area != area)
            continue;
            
        /* value[2] stores the clan number that owns this pylon */
        if (pylon->value[2] == ch->clan)
            break;
    }
    
    if (pylon == NULL)
    {
        send_to_char("Your clan does not have a pylon in that area.\n\r", ch);
        return;
    }
    
    pylon_room = pylon->in_room;
    
    if (pylon_room == NULL)
    {
        send_to_char("The pylon location no longer exists.\n\r", ch);
        return;
    }
    
    /* Consume the warp stone */
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
        act("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        extract_obj(stone);
    }
    
    /* Create portal at caster's location */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
    portal->timer = 20;
    portal->value[0] = -1;  /* Infinite charges */
    portal->value[3] = pylon_room->vnum;  /* Destination vnum */
    
    /* Set descriptive names based on clan and area */
    {
        char short_buf[MAX_STRING_LENGTH];
        char long_buf[MAX_STRING_LENGTH];
        char name_buf[MAX_STRING_LENGTH];
        
        sprintf(name_buf, "%s pylon %s", clan_table[ch->clan].name, area->name);
        sprintf(short_buf, "a pylon to %s", area->name);
        sprintf(long_buf, "A {Mpylon{x to %s hums with energy here.", area->name);
        
        free_string(portal->name);
        portal->name = str_dup(name_buf);
        free_string(portal->short_descr);
        portal->short_descr = str_dup(short_buf);
        free_string(portal->description);
        portal->description = str_dup(long_buf);
    }
    
    obj_to_room(portal, ch->in_room);
    
    act("$p rises from the ground with a hum of energy!", ch, portal, NULL, TO_ROOM);
    act("$p rises from the ground with a hum of energy!", ch, portal, NULL, TO_CHAR);
}
