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
 *           act_obj.c - November 3, 2025
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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_obj.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
CD *find_keeper args ((CHAR_DATA * ch));
CD *find_armorsmith args ((CHAR_DATA * ch));
int get_cost args ((CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy));
void obj_to_keeper args ((OBJ_DATA * obj, CHAR_DATA * ch));
OD *get_obj_keeper
args ((CHAR_DATA * ch, CHAR_DATA * keeper, char *argument));

static bool is_repairable_item (const OBJ_DATA *obj)
{
    if (obj == NULL)
        return FALSE;

    return (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON);
}


static int repair_cost_per_point (const OBJ_DATA *obj)
{
    /* Simple flat rate: 1 silver per 1% repaired */
    /* This means 100% repair (0% to 100%) costs 1 gold (100 silver) */
    return 1;
}

#undef OD
#undef    CD

/* RT part of the corpse looting code */

bool can_loot (CHAR_DATA * ch, OBJ_DATA * obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL (ch))
        return TRUE;

    if (!obj->owner || obj->owner == NULL)
        return TRUE;

    owner = NULL;
    for (wch = char_list; wch != NULL; wch = wch->next)
        if (!str_cmp (wch->name, obj->owner))
            owner = wch;

    if (owner == NULL)
        return TRUE;

    if (!str_cmp (ch->name, owner->name))
        return TRUE;

    if (!IS_NPC (owner) && IS_SET (owner->act, PLR_CANLOOT))
        return TRUE;

    if (is_same_group (ch, owner))
        return TRUE;

    return FALSE;
}


void get_obj (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if (!CAN_WEAR (obj, ITEM_TAKE))
    {
        send_to_char ("You can't take that.\n\r", ch);
        return;
    }

    if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
        act ("$d: you can't carry that many items.",
             ch, NULL, obj->name, TO_CHAR);
        return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
        && (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
    {
        act ("$d: you can't carry that much weight.",
             ch, NULL, obj->name, TO_CHAR);
        return;
    }

    if (!can_loot (ch, obj))
    {
        act ("Corpse looting is not permitted.", ch, NULL, NULL, TO_CHAR);
        return;
    }

    if (obj->in_room != NULL)
    {
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            if (gch->on == obj)
            {
                act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
                return;
            }
    }

    /* Check for get/put traps */
    if (checkgetput(ch, obj))
        return;

    if (container != NULL)
    {
        if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && get_trust (ch) < obj->level)
        {
            send_to_char ("You are not powerful enough to use it.\n\r", ch);
            return;
        }

        if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR (container, ITEM_TAKE)
            && !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
            obj->timer = 0;
        act ("You get $p from $P.", ch, obj, container, TO_CHAR);
        act ("$n gets $p from $P.", ch, obj, container, TO_ROOM);
        REMOVE_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        obj_from_obj (obj);
        
        /* Save pit if getting from donation pit */
        if (container->pIndexData->vnum == OBJ_VNUM_PIT ||
            container->pIndexData->vnum == OBJ_VNUM_ORC_PIT)
        {
            save_pit(container);
        }
    }
    else
    {
        act ("You get $p.", ch, obj, container, TO_CHAR);
        act ("$n gets $p.", ch, obj, container, TO_ROOM);
        obj_from_room (obj);
        
        /* Clan storeroom check */
        if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
        {
            save_clan_storeroom( ch );
        }
    }

    if (obj->item_type == ITEM_MONEY)
    {
        ch->silver += obj->value[0];
        ch->gold += obj->value[1];
        if (IS_SET (ch->act, PLR_AUTOSPLIT))
        {                        /* AUTOSPLIT code */
            members = 0;
            for (gch = ch->in_room->people; gch != NULL;
                 gch = gch->next_in_room)
            {
                if (!IS_AFFECTED (gch, AFF_CHARM) && is_same_group (gch, ch))
                    members++;
            }

            if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
            {
                sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
                do_function (ch, &do_split, buffer);
            }
        }

        extract_obj (obj);
    }
    else
    {
        obj_to_char (obj, ch);
    }

    return;
}



void do_get (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char rest[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;
    int number, i = 0;

    number = mult_argument(argument, rest);
    strcpy(argument, rest);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (!str_cmp (arg2, "from"))
        argument = one_argument (argument, arg2);

    /* Get type. */
    if (arg1[0] == '\0')
    {
        send_to_char ("Get what?\n\r", ch);
        return;
    }

    if ((number < 1) || (number > 64))
    {
        send_to_char ("Number must be between 1 and 64.\n\r", ch);
        return;
    }

    if (arg2[0] == '\0')
    {
        if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)
            && (number == 1))
        {
            /* 'get obj' */
            obj = get_obj_list (ch, arg1, ch->in_room->contents);
            if (obj == NULL)
            {
                act ("I see no $T here.", ch, NULL, arg1, TO_CHAR);
                return;
            }

            get_obj (ch, obj, NULL);
        }
        else if (str_cmp (arg1, "all")
            && str_prefix ("all.", arg1)
            && (number >= 2))
        {
            found = FALSE;
            for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if (is_name (&arg1[0], obj->name) && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    get_obj (ch, obj, NULL);
                    i++;
                    if (i >= number)
                        return;
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    send_to_char ("I see nothing here.\n\r", ch);
                else
                    act ("I see no $T here.", ch, NULL, &arg1[0], TO_CHAR);
            }
        }
        else
        {
            /* 'get all' or 'get all.obj' */
            found = FALSE;
            for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                    && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    get_obj (ch, obj, NULL);
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    send_to_char ("I see nothing here.\n\r", ch);
                else
                    act ("I see no $T here.", ch, NULL, &arg1[4], TO_CHAR);
            }
        }
    }
    else
    {
        /* 'get ... container' */
        if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
        {
            send_to_char ("You can't do that.\n\r", ch);
            return;
        }

        if ((container = get_obj_here (ch, arg2)) == NULL)
        {
            act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
            return;
        }

        switch (container->item_type)
        {
            default:
                send_to_char ("That's not a container.\n\r", ch);
                return;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
                break;

            case ITEM_CORPSE_PC:
                {

                    if (!can_loot (ch, container))
                    {
                        send_to_char ("You can't do that.\n\r", ch);
                        return;
                    }
                }
        }

        if (IS_SET (container->value[1], CONT_CLOSED))
        {
            act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
            return;
        }

        if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)
            && (number == 1))
        {
            /* 'get obj container' */
            obj = get_obj_list (ch, arg1, container->contains);
            if (obj == NULL)
            {
                act ("I see nothing like that in the $T.",
                     ch, NULL, arg2, TO_CHAR);
                return;
            }
            get_obj (ch, obj, container);
        }
        else if (str_cmp (arg1, "all")
            && str_prefix ("all.", arg1)
            && (number >= 2))
        {
            found = FALSE;
            for (obj = container->contains; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if (is_name (&arg1[0], obj->name) && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    get_obj (ch, obj, container);
                    i++;
                    if (i >= number)
                        return;
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    act ("I see nothing in the $T.",
                        ch, NULL, arg2, TO_CHAR);
                else
                    act ("I see nothing like that in the $T.",
                        ch, NULL, arg2, TO_CHAR);
            }
        }
        else
        {
            /* 'get all container' or 'get all.obj container' */
            found = FALSE;
            for (obj = container->contains; obj != NULL; obj = obj_next)
            {
                obj_next = obj->next_content;
                if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                    && can_see_obj (ch, obj))
                {
                    found = TRUE;
                    if (container->pIndexData->vnum == OBJ_VNUM_PIT
                        && !IS_IMMORTAL (ch))
                    {
                        send_to_char ("Don't be so greedy!\n\r", ch);
                        return;
                    }
                    get_obj (ch, obj, container);
                }
            }

            if (!found)
            {
                if (arg1[3] == '\0')
                    act ("I see nothing in the $T.", ch, NULL, arg2, TO_CHAR);
                else
                    act ("I see nothing like that in the $T.",
                         ch, NULL, arg2, TO_CHAR);
            }
        }
    }

    return;
}



void do_put (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char rest[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    int number, i = 0;

    number = mult_argument(argument, rest);
    strcpy(argument, rest);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (!str_cmp (arg2, "in") || !str_cmp (arg2, "on"))
        argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char ("Put what in what?\n\r", ch);
        return;
    }

    if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
        send_to_char ("You can't do that.\n\r", ch);
        return;
    }

    if ((container = get_obj_here (ch, arg2)) == NULL)
    {
        act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
        return;
    }

    if (container->item_type != ITEM_CONTAINER)
    {
        send_to_char ("That's not a container.\n\r", ch);
        return;
    }

    if (IS_SET (container->value[1], CONT_CLOSED))
    {
        act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
        return;
    }

    /* Check for container traps */
    if (checkgetput(ch, container))
        return;

    if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)
        && (number == 1))
    {
        /* 'put obj container' */
        if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
        {
            send_to_char ("You do not have that item.\n\r", ch);
            return;
        }

        if (obj == container)
        {
            send_to_char ("You can't fold it into itself.\n\r", ch);
            return;
        }

        if (!can_drop_obj (ch, obj))
        {
            send_to_char ("You can't let go of it.\n\r", ch);
            return;
        }

        if (WEIGHT_MULT (obj) != 100)
        {
            send_to_char ("You have a feeling that would be a bad idea.\n\r",
                          ch);
            return;
        }

        if (get_obj_weight (obj) + get_true_weight (container)
            > (container->value[0] * 10)
            || get_obj_weight (obj) > (container->value[3] * 10))
        {
            send_to_char ("It won't fit.\n\r", ch);
            return;
        }

        if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR (container, ITEM_TAKE))
        {
            if (obj->timer)
                SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
            else
                obj->timer = number_range (100, 200);
        }

        obj_from_char (obj);
        obj_to_obj (obj, container);

        if (IS_SET (container->value[1], CONT_PUT_ON))
        {
            act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
            act ("You put $p on $P.", ch, obj, container, TO_CHAR);
        }
        else
        {
            act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
            act ("You put $p in $P.", ch, obj, container, TO_CHAR);
        }
    }
    else if (str_cmp (arg1, "all") && str_prefix ("all.", arg1)
        && (number >= 2))
    {
        /* 'put all container' or 'put all.obj container' */
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((is_name (&arg1[0], obj->name))
                && can_see_obj (ch, obj)
                && WEIGHT_MULT (obj) == 100
                && obj->wear_loc == WEAR_NONE
                && obj != container && can_drop_obj (ch, obj)
                && get_obj_weight (obj) + get_true_weight (container)
                <= (container->value[0] * 10)
                && get_obj_weight (obj) < (container->value[3] * 10))
            {
                if (container->pIndexData->vnum == OBJ_VNUM_PIT
                    && !CAN_WEAR (obj, ITEM_TAKE))
                {
                    if (obj->timer)
                        SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
                    else
                        obj->timer = number_range (100, 200);
                }

                obj_from_char (obj);
                obj_to_obj (obj, container);
                i++;

                if (IS_SET (container->value[1], CONT_PUT_ON))
                {
                    act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p on $P.", ch, obj, container, TO_CHAR);
                }
                else
                {
                    act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p in $P.", ch, obj, container, TO_CHAR);
                }

                if (i >= number)
                    return;
            }
        }
        return;
    }
    else
    {
        /* 'put all container' or 'put all.obj container' */
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
                && can_see_obj (ch, obj)
                && WEIGHT_MULT (obj) == 100
                && obj->wear_loc == WEAR_NONE
                && obj != container && can_drop_obj (ch, obj)
                && get_obj_weight (obj) + get_true_weight (container)
                <= (container->value[0] * 10)
                && get_obj_weight (obj) < (container->value[3] * 10))
            {
                if (container->pIndexData->vnum == OBJ_VNUM_PIT
                    && !CAN_WEAR (obj, ITEM_TAKE))
                {
                    if (obj->timer)
                        SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
                    else
                        obj->timer = number_range (100, 200);
                }

                obj_from_char (obj);
                obj_to_obj (obj, container);

                if (IS_SET (container->value[1], CONT_PUT_ON))
                {
                    act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p on $P.", ch, obj, container, TO_CHAR);
                }
                else
                {
                    act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
                    act ("You put $p in $P.", ch, obj, container, TO_CHAR);
                }
            }
        }
    }

    /* Clan storeroom check */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM)
         && container->carried_by == NULL)
    {
        save_clan_storeroom( ch );
    }

    return;
}



void do_drop (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char rest[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int number, i = 0;

    number = mult_argument(argument, rest);
    strcpy(argument, rest);
    argument = one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Drop what?\n\r", ch);
        return;
    }

    if (is_number (arg))
    {
        /* 'drop NNNN coins' */
        int amount, gold = 0, silver = 0;

        amount = atoi (arg);
        argument = one_argument (argument, arg);
        if (amount <= 0
            || (str_cmp (arg, "coins") && str_cmp (arg, "coin") &&
                str_cmp (arg, "gold") && str_cmp (arg, "silver")))
        {
            send_to_char ("Sorry, you can't do that.\n\r", ch);
            return;
        }

        if (!str_cmp (arg, "coins") || !str_cmp (arg, "coin")
            || !str_cmp (arg, "silver"))
        {
            if (ch->silver < amount)
            {
                send_to_char ("You don't have that much silver.\n\r", ch);
                return;
            }

            ch->silver -= amount;
            silver = amount;
        }

        else
        {
            if (ch->gold < amount)
            {
                send_to_char ("You don't have that much gold.\n\r", ch);
                return;
            }

            ch->gold -= amount;
            gold = amount;
        }

        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            switch (obj->pIndexData->vnum)
            {
                case OBJ_VNUM_SILVER_ONE:
                    silver += 1;
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_GOLD_ONE:
                    gold += 1;
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_SILVER_SOME:
                    silver += obj->value[0];
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_GOLD_SOME:
                    gold += obj->value[1];
                    extract_obj (obj);
                    break;

                case OBJ_VNUM_COINS:
                    silver += obj->value[0];
                    gold += obj->value[1];
                    extract_obj (obj);
                    break;
            }
        }

        obj_to_room (create_money (gold, silver), ch->in_room);
        act ("$n drops some coins.", ch, NULL, NULL, TO_ROOM);
        send_to_char ("OK.\n\r", ch);
        return;
    }

    if (str_cmp (arg, "all") && str_prefix ("all.", arg)
        && (number == 1))
    {
        /* 'drop obj' */
        if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
        {
            send_to_char ("You do not have that item.\n\r", ch);
            return;
        }

        if (!can_drop_obj (ch, obj))
        {
            send_to_char ("You can't let go of it.\n\r", ch);
            return;
        }

        obj_from_char (obj);
        obj_to_room (obj, ch->in_room);
        act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
        act ("You drop $p.", ch, obj, NULL, TO_CHAR);
        if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
        {
            act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
            act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
            extract_obj (obj);
        }
    }
    else if (str_cmp (arg, "all") && str_prefix ("all.", arg)
        && (number >= 2))
    {
        /* 'drop all' or 'drop all.obj' */
        found = FALSE;
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((is_name (&arg[0], obj->name))
                && can_see_obj (ch, obj)
                && obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
            {
                found = TRUE;
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
                act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
                act ("You drop $p.", ch, obj, NULL, TO_CHAR);

                if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
                {
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
                    extract_obj (obj);
                }

                i++;

                if (i >= number)
                    return;
            }
        }

        if (!found)
        {
            if (arg[3] == '\0')
                act ("You are not carrying anything.", ch, NULL, arg, TO_CHAR);
            else
                act ("You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR);
        }
    }
    else
    {
        /* 'drop all' or 'drop all.obj' */
        found = FALSE;
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;

            if ((arg[3] == '\0' || is_name (&arg[4], obj->name))
                && can_see_obj (ch, obj)
                && obj->wear_loc == WEAR_NONE && can_drop_obj (ch, obj))
            {
                found = TRUE;
                obj_from_char (obj);
                obj_to_room (obj, ch->in_room);
                act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
                act ("You drop $p.", ch, obj, NULL, TO_CHAR);
                if (IS_OBJ_STAT (obj, ITEM_MELT_DROP))
                {
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
                    act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
                    extract_obj (obj);
                }
            }
        }

        if (!found)
        {
            if (arg[3] == '\0')
                act ("You are not carrying anything.",
                     ch, NULL, arg, TO_CHAR);
            else
                act ("You are not carrying any $T.",
                     ch, NULL, &arg[4], TO_CHAR);
        }
    }

    /* Clan storeroom saving */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
    {
        save_clan_storeroom( ch );
    }

    return;
}



void do_give (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char rest[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int number, i = 0;

    number = mult_argument(argument, rest);
    strcpy(argument, rest);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char ("Give what to whom?\n\r", ch);
        return;
    }

    if (is_number (arg1))
    {
        /* 'give NNNN coins victim' */
        int amount;
        bool silver;

        amount = atoi (arg1);
        if (amount <= 0
            || (str_cmp (arg2, "coins") && str_cmp (arg2, "coin") &&
                str_cmp (arg2, "gold") && str_cmp (arg2, "silver")))
        {
            send_to_char ("Sorry, you can't do that.\n\r", ch);
            return;
        }

        silver = str_cmp (arg2, "gold");

        argument = one_argument (argument, arg2);
        if (arg2[0] == '\0')
        {
            send_to_char ("Give what to whom?\n\r", ch);
            return;
        }

        if ((victim = get_char_room (ch, arg2)) == NULL)
        {
            send_to_char ("They aren't here.\n\r", ch);
            return;
        }

        if ((!silver && ch->gold < amount) || (silver && ch->silver < amount))
        {
            send_to_char ("You haven't got that much.\n\r", ch);
            return;
        }

        if (silver)
        {
            ch->silver -= amount;
            victim->silver += amount;
        }
        else
        {
            ch->gold -= amount;
            victim->gold += amount;
        }

        sprintf (buf, "$n gives you %d %s.", amount,
                 silver ? "silver" : "gold");
        act (buf, ch, NULL, victim, TO_VICT);
        act ("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
        sprintf (buf, "You give $N %d %s.", amount,
                 silver ? "silver" : "gold");
        act (buf, ch, NULL, victim, TO_CHAR);

        /*
         * Bribe trigger
         */
        if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_BRIBE))
            mp_bribe_trigger (victim, ch, silver ? amount : amount * 100);

        if (IS_NPC (victim) && IS_SET (victim->act, ACT_IS_CHANGER))
        {
            int change;

            change = (silver ? 95 * amount / 100 / 100 : 95 * amount);


            if (!silver && change > victim->silver)
                victim->silver += change;

            if (silver && change > victim->gold)
                victim->gold += change;

            if (change < 1 && can_see (victim, ch))
            {
                act
                    ("$n tells you 'I'm sorry, you did not give me enough to change.'",
                     victim, NULL, ch, TO_VICT);
                ch->reply = victim;
                sprintf (buf, "%d %s %s",
                         amount, silver ? "silver" : "gold", ch->name);
                do_function (victim, &do_give, buf);
            }
            else if (can_see (victim, ch))
            {
                sprintf (buf, "%d %s %s",
                         change, silver ? "gold" : "silver", ch->name);
                do_function (victim, &do_give, buf);
                if (silver)
                {
                    sprintf (buf, "%d silver %s",
                             (95 * amount / 100 - change * 100), ch->name);
                    do_function (victim, &do_give, buf);
                }
                act ("$n tells you 'Thank you, come again.'",
                     victim, NULL, ch, TO_VICT);
                ch->reply = victim;
            }
        }
        return;
    }

    if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    if (obj->wear_loc != WEAR_NONE)
    {
        send_to_char ("You must remove it first.\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg2)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
    {
        act ("$N tells you 'Sorry, you'll have to sell that.'",
             ch, NULL, victim, TO_CHAR);
        ch->reply = victim;
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        send_to_char ("You can't let go of it.\n\r", ch);
        return;
    }

    if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
    {
        act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (get_carry_weight (victim) + get_obj_weight (obj) >
        can_carry_w (victim))
    {
        act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (!can_see_obj (victim, obj))
    {
        act ("$N can't see it.", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (number == 1)
    {
        obj_from_char (obj);
        obj_to_char (obj, victim);
        MOBtrigger = FALSE;
        act ("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
        act ("$n gives you $p.", ch, obj, victim, TO_VICT);
        act ("You give $p to $N.", ch, obj, victim, TO_CHAR);
        MOBtrigger = TRUE;
    }
    else
    {
        for (;;)
        {
            obj_from_char (obj);
            obj_to_char (obj, victim);
            i++;

            if (i >= number)
            {
                MOBtrigger = FALSE;
                act ("$n gives some $p to $N.", ch, obj, victim, TO_NOTVICT);
                act ("$n gives you some $p.", ch, obj, victim, TO_VICT);
                act ("You give some $p to $N.", ch, obj, victim, TO_CHAR);
                MOBtrigger = TRUE;
                return;
            }

            if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
            {
                send_to_char ("You do not have that item.\n\r", ch);
                MOBtrigger = FALSE;
                act ("$n gives some $p to $N.", ch, obj, victim, TO_NOTVICT);
                act ("$n gives you some $p.", ch, obj, victim, TO_VICT);
                act ("You give some $p to $N.", ch, obj, victim, TO_CHAR);
                MOBtrigger = TRUE;
                return;
            }
        }
    }

    /*
     * Give trigger
     */
    if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_GIVE))
        mp_give_trigger (victim, ch, obj);

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent, skill;

    /* find out what */
    if (argument[0] == '\0')
    {
        send_to_char ("Envenom what item?\n\r", ch);
        return;
    }

    obj = get_obj_list (ch, argument, ch->carrying);

    if (obj == NULL)
    {
        send_to_char ("You don't have that item.\n\r", ch);
        return;
    }

    if ((skill = get_skill (ch, gsn_envenom)) < 1)
    {
        send_to_char ("Are you crazy? You'd poison yourself!\n\r", ch);
        return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
        if (IS_OBJ_STAT (obj, ITEM_BLESS)
            || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
        {
            act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (number_percent () < skill)
        {                        /* success! */
            act ("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
            act ("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
            if (!obj->value[3])
            {
                obj->value[3] = 1;
                check_improve (ch, gsn_envenom, TRUE, 4);
            }
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }

        act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
        if (!obj->value[3])
            check_improve (ch, gsn_envenom, FALSE, 4);
        WAIT_STATE (ch, skill_table[gsn_envenom].beats);
        return;
    }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
            || IS_WEAPON_STAT (obj, WEAPON_FROST)
            || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
            || IS_WEAPON_STAT (obj, WEAPON_SHARP)
            || IS_WEAPON_STAT (obj, WEAPON_VORPAL)
            || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
            || IS_OBJ_STAT (obj, ITEM_BLESS)
            || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
        {
            act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
            return;
        }

        if (obj->value[3] < 0
            || attack_table[obj->value[3]].damage == DAM_BASH)
        {
            send_to_char ("You can only envenom edged weapons.\n\r", ch);
            return;
        }

        if (IS_WEAPON_STAT (obj, WEAPON_POISON))
        {
            act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
            return;
        }

        percent = number_percent ();
        if (percent < skill)
        {

            af.where = TO_WEAPON;
            af.type = gsn_poison;
            af.level = ch->level * percent / 100;
            af.duration = ch->level / 2 * percent / 100;
            af.location = 0;
            af.modifier = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj (obj, &af);

            act ("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
            act ("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
            check_improve (ch, gsn_envenom, TRUE, 3);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }
        else
        {
            act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
            check_improve (ch, gsn_envenom, FALSE, 3);
            WAIT_STATE (ch, skill_table[gsn_envenom].beats);
            return;
        }
    }

    act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
    return;
}

void do_fill (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Fill what?\n\r", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    found = FALSE;
    for (fountain = ch->in_room->contents; fountain != NULL;
         fountain = fountain->next_content)
    {
        if (fountain->item_type == ITEM_FOUNTAIN)
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        send_to_char ("There is no fountain here!\n\r", ch);
        return;
    }

    if (obj->item_type != ITEM_DRINK_CON)
    {
        send_to_char ("You can't fill that.\n\r", ch);
        return;
    }

    if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
    {
        send_to_char ("There is already another liquid in it.\n\r", ch);
        return;
    }

    if (obj->value[1] >= obj->value[0])
    {
        send_to_char ("Your container is full.\n\r", ch);
        return;
    }

    sprintf (buf, "You fill $p with %s from $P.",
             liq_table[fountain->value[2]].liq_name);
    act (buf, ch, obj, fountain, TO_CHAR);
    sprintf (buf, "$n fills $p with %s from $P.",
             liq_table[fountain->value[2]].liq_name);
    act (buf, ch, obj, fountain, TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    obj->value[4] = 0; /* jerome : potion filling security fix */
    return;
}

void do_pour (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount,dose=0;
    bool potion = FALSE;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ( out->item_type == ITEM_POTION ) potion = TRUE;

    if ( (out->item_type != ITEM_DRINK_CON) && (!potion) )
    {
	send_to_char("That's not a drink container or a potion.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
        if(  out->value[4] != 0 )potion = TRUE;
        out->value[4] = 0;

        if( !potion )
        {
    	  sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	  act(buf,ch,out,NULL,TO_CHAR);
	
	  sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	  act(buf,ch,out,NULL,TO_ROOM);
        }
        else
        {
         act("You invert $p, spilling the magical liquid all over the ground.",
           ch,out,NULL,TO_CHAR);
         act("$n invert $p, spilling the magical liquid all over the ground.",
           ch,out,NULL,TO_ROOM);
        }
	return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
	vch = get_char_room(ch,argument);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into a drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (   ( (!potion) && (in->value[1] != 0) 
                       && (in->value[2] != out->value[2]) )
         ||(  (potion) && (  ( (in->value[4] != 0 )  
                             &&(in->value[4] != out->pIndexData->vnum) ) 
                           ||( (in->value[1] != 0) 
                             &&(in->value[4] == 0) ) ) ) ) 
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if ( (!potion) && (out->value[1] == 0) )
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }
  
    if( potion )
    {
      dose = liq_table[ in->value[2] ].liq_affect[4];
      amount = UMIN( dose , in->value[0]  - dose );
      in->value[1] += amount;
      if( (in->value[4] != 0) && (in->value[4] != out->pIndexData->vnum ) )
        bug("Error potion mixing in pour command with potion %d",in->value[4]);
      in->value[4] = out->pIndexData->vnum;
      extract_obj( out );
    }
    else
    {
      amount = UMIN(out->value[1],in->value[0] - in->value[1]);
      in->value[1] += amount;
      out->value[1] -= amount;
      in->value[2] = out->value[2];
    } 

    if (vch == NULL)
    {
      if( !potion)
      {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
      }
      else
      {
        act("You pour $p into $P.",ch,out,in,TO_CHAR);
        act("$n pours $p into $P.",ch,out,in,TO_ROOM);
      }

    }
    else
    {
      if(!potion)
       {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
       }
       else
       {
        act("You pour some potion for $N.",ch,NULL,vch,TO_CHAR);
        act("$n pours you some potion.",ch,NULL,vch,TO_VICT);
        act("$n pours some potion for $N.",ch,NULL,vch,TO_NOTVICT);
       }
	
    }
}

void do_drink (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj,*potion;
    int amount;
    int liquid;
    bool split = FALSE;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        for (obj = ch->in_room->contents; obj; obj = obj->next_content)
        {
            if (obj->item_type == ITEM_FOUNTAIN)
                break;
        }

        if (obj == NULL)
        {
            send_to_char ("Drink what?\n\r", ch);
            return;
        }
    }
    else
    {
        if ((obj = get_obj_here (ch, arg)) == NULL)
        {
            send_to_char ("You can't find it.\n\r", ch);
            return;
        }
    }

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
        send_to_char ("You fail to reach your mouth.  *Hic*\n\r", ch);
        return;
    }

    switch (obj->item_type)
    {
        default:
            send_to_char ("You can't drink from that.\n\r", ch);
            return;

        case ITEM_FOUNTAIN:
            if ((liquid = obj->value[2]) < 0)
            {
                bug ("Do_drink: bad liquid number %d.", liquid);
                liquid = obj->value[2] = 0;
            }
            amount = liq_table[liquid].liq_affect[4] * 3;
            break;

        case ITEM_DRINK_CON:
            if (obj->value[1] <= 0)
            {
                send_to_char ("It is already empty.\n\r", ch);
                return;
            }

            if ((liquid = obj->value[2]) < 0)
            {
                bug ("Do_drink: bad liquid number %d.", liquid);
                liquid = obj->value[2] = 0;
            }

            amount = liq_table[liquid].liq_affect[4];
            amount = UMIN (amount, obj->value[1]);
            break;
    }
    if (!IS_NPC (ch) && !IS_IMMORTAL (ch)
        && ch->pcdata->condition[COND_FULL] > 45)
    {
        send_to_char ("You're too full to drink more.\n\r", ch);
        return;
    }

    if( obj->value[4] != 0 )
    { /* we are drinking some potion stored in the container */
         potion = create_object( get_obj_index( obj->value[4]) , 0 );
         if( !auto_quaff(ch,potion) )
          {
            extract_obj(potion);
            split = TRUE;
          }
    }
    else
    {
      act ("$n drinks $T from $p.",
           ch, obj, liq_table[liquid].liq_name, TO_ROOM);
      act ("You drink $T from $p.",
           ch, obj, liq_table[liquid].liq_name, TO_CHAR);

      gain_condition (ch, COND_DRUNK,
                      amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36);
      gain_condition (ch, COND_FULL,
                      amount * liq_table[liquid].liq_affect[COND_FULL] / 4);
      gain_condition (ch, COND_THIRST,
                      amount * liq_table[liquid].liq_affect[COND_THIRST] / 10);
      gain_condition (ch, COND_HUNGER,
                      amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2);

      if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
          send_to_char ("You feel drunk.\n\r", ch);
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
          send_to_char ("You are full.\n\r", ch);
      if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] > 40)
          send_to_char ("Your thirst is quenched.\n\r", ch);
    }

    if (obj->value[3] != 0)
    {
        /* The drink was poisoned ! */
        AFFECT_DATA af;

        act ("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
        send_to_char ("You choke and gag.\n\r", ch);
        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = number_fuzzy (amount);
        af.duration = 3 * amount;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_POISON;
        affect_join (ch, &af);
    }

    if ( (!split) && (obj->value[0] > 0) )
        obj->value[1] -= amount;

    if( obj->value[1] <= 0 )obj->value[4] = 0;

    WAIT_STATE( ch, PULSE_VIOLENCE );

    return;
}



void do_eat (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);
    if (arg[0] == '\0')
    {
        send_to_char ("Eat what?\n\r", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL (ch))
    {
        if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL)
        {
            send_to_char ("That's not edible.\n\r", ch);
            return;
        }

        if (!IS_NPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
        {
            send_to_char ("You are too full to eat more.\n\r", ch);
            return;
        }
    }

    act ("$n eats $p.", ch, obj, NULL, TO_ROOM);
    act ("You eat $p.", ch, obj, NULL, TO_CHAR);

    switch (obj->item_type)
    {

        case ITEM_FOOD:
            if (!IS_NPC (ch))
            {
                int condition;

                condition = ch->pcdata->condition[COND_HUNGER];
                gain_condition (ch, COND_FULL, obj->value[0]);
                gain_condition (ch, COND_HUNGER, obj->value[1]);
                if (condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0)
                    send_to_char ("You are no longer hungry.\n\r", ch);
                else if (ch->pcdata->condition[COND_FULL] > 40)
                    send_to_char ("You are full.\n\r", ch);
            }

            if (obj->value[3] != 0)
            {
                /* The food was poisoned! */
                AFFECT_DATA af;

                act ("$n chokes and gags.", ch, 0, 0, TO_ROOM);
                send_to_char ("You choke and gag.\n\r", ch);

                af.where = TO_AFFECTS;
                af.type = gsn_poison;
                af.level = number_fuzzy (obj->value[0]);
                af.duration = 2 * obj->value[0];
                af.location = APPLY_NONE;
                af.modifier = 0;
                af.bitvector = AFF_POISON;
                affect_join (ch, &af);
            }
            break;

        case ITEM_PILL:
            obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
            obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
            obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
            break;
    }

    extract_obj (obj);
    return;
}



/*
 * Remove an object.
 */
bool remove_obj (CHAR_DATA * ch, int iWear, bool fReplace)
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char (ch, iWear)) == NULL)
        return TRUE;

    if (!fReplace)
        return FALSE;

    if (IS_SET (obj->extra_flags, ITEM_NOREMOVE))
    {
        act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        return FALSE;
    }

    unequip_char (ch, obj);
    act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
    act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
    char buf[MAX_STRING_LENGTH];

    /* Check if item is completely broken (0% condition) */
    if (obj->condition == 0 && is_repairable_item(obj))
    {
        act ("$p is completely ruined and can't be used until repaired!", ch, obj, NULL, TO_CHAR);
        return;
    }

    if (ch->level < obj->level)
    {
        sprintf (buf, "You must be level %d to use this object.\n\r",
                 obj->level);
        send_to_char (buf, ch);
        act ("$n tries to use $p, but is too inexperienced.",
             ch, obj, NULL, TO_ROOM);
        return;
    }

    if (obj->item_type == ITEM_LIGHT)
    {
        if (!remove_obj (ch, WEAR_LIGHT, fReplace))
            return;
        act ("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
        act ("You light $p and hold it.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_LIGHT);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FINGER))
    {
        if (get_eq_char (ch, WEAR_FINGER_L) != NULL
            && get_eq_char (ch, WEAR_FINGER_R) != NULL
            && !remove_obj (ch, WEAR_FINGER_L, fReplace)
            && !remove_obj (ch, WEAR_FINGER_R, fReplace))
            return;

        if (get_eq_char (ch, WEAR_FINGER_L) == NULL)
        {
            act ("$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_FINGER_L);
            return;
        }

        if (get_eq_char (ch, WEAR_FINGER_R) == NULL)
        {
            act ("$n wears $p on $s right finger.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_FINGER_R);
            return;
        }

        bug ("Wear_obj: no free finger.", 0);
        send_to_char ("You already wear two rings.\n\r", ch);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_NECK))
    {
        if (get_eq_char (ch, WEAR_NECK_1) != NULL
            && get_eq_char (ch, WEAR_NECK_2) != NULL
            && !remove_obj (ch, WEAR_NECK_1, fReplace)
            && !remove_obj (ch, WEAR_NECK_2, fReplace))
            return;

        if (get_eq_char (ch, WEAR_NECK_1) == NULL)
        {
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_NECK_1);
            return;
        }

        if (get_eq_char (ch, WEAR_NECK_2) == NULL)
        {
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_NECK_2);
            return;
        }

        bug ("Wear_obj: no free neck.", 0);
        send_to_char ("You already wear two neck items.\n\r", ch);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_BODY))
    {
        if (!remove_obj (ch, WEAR_BODY, fReplace))
            return;
        act ("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your torso.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_BODY);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_HEAD))
    {
        if (!remove_obj (ch, WEAR_HEAD, fReplace))
            return;
        act ("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your head.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HEAD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_LEGS))
    {
        if (!remove_obj (ch, WEAR_LEGS, fReplace))
            return;
        act ("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_LEGS);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FEET))
    {
        if (!remove_obj (ch, WEAR_FEET, fReplace))
            return;
        act ("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your feet.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_FEET);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_HANDS))
    {
        if (!remove_obj (ch, WEAR_HANDS, fReplace))
            return;
        act ("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your hands.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HANDS);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_ARMS))
    {
        if (!remove_obj (ch, WEAR_ARMS, fReplace))
            return;
        act ("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p on your arms.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_ARMS);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_ABOUT))
    {
        if (!remove_obj (ch, WEAR_ABOUT, fReplace))
            return;
        act ("$n wears $p about $s torso.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p about your torso.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_ABOUT);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_WAIST))
    {
        if (!remove_obj (ch, WEAR_WAIST, fReplace))
            return;
        act ("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p about your waist.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_WAIST);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_WRIST))
    {
        if (get_eq_char (ch, WEAR_WRIST_L) != NULL
            && get_eq_char (ch, WEAR_WRIST_R) != NULL
            && !remove_obj (ch, WEAR_WRIST_L, fReplace)
            && !remove_obj (ch, WEAR_WRIST_R, fReplace))
            return;

        if (get_eq_char (ch, WEAR_WRIST_L) == NULL)
        {
            act ("$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your left wrist.",
                 ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_WRIST_L);
            return;
        }

        if (get_eq_char (ch, WEAR_WRIST_R) == NULL)
        {
            act ("$n wears $p around $s right wrist.",
                 ch, obj, NULL, TO_ROOM);
            act ("You wear $p around your right wrist.",
                 ch, obj, NULL, TO_CHAR);
            equip_char (ch, obj, WEAR_WRIST_R);
            return;
        }

        bug ("Wear_obj: no free wrist.", 0);
        send_to_char ("You already wear two wrist items.\n\r", ch);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_SHIELD))
    {
        OBJ_DATA *weapon;

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
            return;
        }

        if (!remove_obj (ch, WEAR_SHIELD, fReplace))
            return;

        weapon = get_eq_char (ch, WEAR_WIELD);
        if (weapon != NULL && ch->size < SIZE_LARGE
            && IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS))
        {
            send_to_char ("Your hands are tied up with your weapon!\n\r", ch);
            return;
        }

        act ("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p as a shield.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_SHIELD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WIELD))
    {
        int sn, skill;

        /* Felar cannot wield weapons - hand-to-hand only */
        if (!IS_NPC (ch) && ch->race == race_lookup ("felar")) /* felar is race 6 */
        {
            send_to_char ("Your natural claws are your only weapons. You cannot wield items.\n\r", ch);
            return;
        }

        if (!remove_obj (ch, WEAR_WIELD, fReplace))
            return;

        if (!IS_NPC (ch)
            && get_obj_weight (obj) >
            (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
        {
            send_to_char ("It is too heavy for you to wield.\n\r", ch);
            return;
        }

        if (!IS_NPC (ch) && ch->size < SIZE_LARGE
            && IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS)
            && get_eq_char (ch, WEAR_SHIELD) != NULL)
        {
            send_to_char ("You need two hands free for that weapon.\n\r", ch);
            return;
        }

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot wield a two-handed weapon while using a secondary weapon.\n\r", ch);
            return;
        }

        act ("$n wields $p.", ch, obj, NULL, TO_ROOM);
        act ("You wield $p.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_WIELD);

        sn = get_weapon_sn (ch);

        if (sn == gsn_hand_to_hand)
            return;

        skill = get_weapon_skill (ch, sn);

        if (skill >= 100)
            act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);
        else if (skill > 85)
            act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);
        else if (skill > 70)
            act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);
        else if (skill > 50)
            act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);
        else if (skill > 25)
            act ("$p feels a little clumsy in your hands.", ch, obj, NULL,
                 TO_CHAR);
        else if (skill > 1)
            act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);
        else
            act ("You don't even know which end is up on $p.",
                 ch, obj, NULL, TO_CHAR);

        return;
    }

    if (CAN_WEAR (obj, ITEM_HOLD))
    {
        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot hold an item while using 2 weapons.\n\r",ch);
            return;
        }

        if (!remove_obj (ch, WEAR_HOLD, fReplace))
            return;
        act ("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
        act ("You hold $p in your hand.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_HOLD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FLOAT))
    {
        if (!remove_obj (ch, WEAR_FLOAT, fReplace))
            return;
        act ("$n releases $p to float next to $m.", ch, obj, NULL, TO_ROOM);
        act ("You release $p and it floats next to you.", ch, obj, NULL,
             TO_CHAR);
        equip_char (ch, obj, WEAR_FLOAT);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_COMM))
    {
        if (!remove_obj (ch, WEAR_COMM, fReplace))
            return;

        if (obj->item_type != ITEM_COMM)
        {
            send_to_char ("That device doesn't seem to attune properly.\n\r", ch);
            return;
        }

        act ("$n wears $p as a commstone.", ch, obj, NULL, TO_ROOM);
        act ("You wear $p as a commstone.", ch, obj, NULL, TO_CHAR);
        equip_char (ch, obj, WEAR_COMM);
        return;
    }

    if (fReplace)
        send_to_char ("You can't wear, wield, or hold that.\n\r", ch);

    return;
}



void do_wear (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Wear, wield, or hold what?\n\r", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        OBJ_DATA *obj_next;

        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
                wear_obj (ch, obj, FALSE);
        }
        return;
    }
    else
    {
        if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
        {
            send_to_char ("You do not have that item.\n\r", ch);
            return;
        }

        wear_obj (ch, obj, TRUE);
    }

    return;
}



void do_remove (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool removed_any = FALSE;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Remove what?\n\r", ch);
        return;
    }

    /* Handle 'all' argument to remove all equipped items */
    if (!str_cmp(arg, "all"))
    {
        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wear_loc != WEAR_NONE)
            {
                remove_obj(ch, obj->wear_loc, TRUE);
                removed_any = TRUE;
            }
        }
        
        if (removed_any)
            send_to_char("You remove all your equipment.\n\r", ch);
        else
            send_to_char("You aren't wearing anything.\n\r", ch);
        return;
    }

    if ((obj = get_obj_wear (ch, arg)) == NULL)
    {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    remove_obj (ch, obj->wear_loc, TRUE);
    return;
}



void do_sacrifice (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int silver;

    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument (argument, arg);

    if (arg[0] == '\0' || !str_cmp (arg, ch->name))
    {
        act ("$n offers $mself to Om, who graciously declines.",
             ch, NULL, NULL, TO_ROOM);
        send_to_char
            ("Om appreciates your offer and may accept it later.\n\r", ch);
        return;
    }

    obj = get_obj_list (ch, arg, ch->in_room->contents);
    if (obj == NULL)
    {
        send_to_char ("You can't find it.\n\r", ch);
        return;
    }

    if (obj->item_type == ITEM_CORPSE_PC)
    {
        if (obj->contains)
        {
            send_to_char ("Om wouldn't like that.\n\r", ch);
            return;
        }
    }


    if (!CAN_WEAR (obj, ITEM_TAKE) || CAN_WEAR (obj, ITEM_NO_SAC))
    {
        act ("$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);
        return;
    }

    if (obj->in_room != NULL)
    {
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            if (gch->on == obj)
            {
                act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
                return;
            }
    }

    silver = UMAX (1, obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
        silver = UMIN (silver, obj->cost);

    if (silver == 1)
        send_to_char
            ("Om gives you one silver coin for your sacrifice.\n\r", ch);
    else
    {
        sprintf (buf,
                 "Om gives you %d silver coins for your sacrifice.\n\r",
                 silver);
        send_to_char (buf, ch);
    }

    ch->silver += silver;

    if (IS_SET (ch->act, PLR_AUTOSPLIT))
    {                            /* AUTOSPLIT code */
        members = 0;
        for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
        {
            if (is_same_group (gch, ch))
                members++;
        }

        if (members > 1 && silver > 1)
        {
            sprintf (buffer, "%d", silver);
            do_function (ch, &do_split, buffer);
        }
    }

    act ("$n sacrifices $p to Om.", ch, obj, NULL, TO_ROOM);
    wiznet ("$N sends up $p as a burnt offering.",
            ch, obj, WIZ_SACCING, 0, 0);
    extract_obj (obj);
    return;
}



void do_quaff (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Quaff what?\n\r", ch);
        return;
    }

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        send_to_char ("You do not have that potion.\n\r", ch);
        return;
    }

    if (obj->item_type != ITEM_POTION)
    {
        send_to_char ("You can quaff only potions.\n\r", ch);
        return;
    }

    if (ch->level < obj->level)
    {
        send_to_char ("This liquid is too powerful for you to drink.\n\r",
                      ch);
        return;
    }

    act ("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
    act ("You quaff $p.", ch, obj, NULL, TO_CHAR);

    obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
    obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
    obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);

    extract_obj (obj);
    return;
}



void do_recite (CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if ((scroll = get_obj_carry (ch, arg1, ch)) == NULL)
    {
        send_to_char ("You do not have that scroll.\n\r", ch);
        return;
    }

    if (scroll->item_type != ITEM_SCROLL)
    {
        send_to_char ("You can recite only scrolls.\n\r", ch);
        return;
    }

    if (ch->level < scroll->level)
    {
        send_to_char ("This scroll is too complex for you to comprehend.\n\r",
                      ch);
        return;
    }

    obj = NULL;
    if (arg2[0] == '\0')
    {
        victim = ch;
    }
    else
    {
        if ((victim = get_char_room (ch, arg2)) == NULL
            && (obj = get_obj_here (ch, arg2)) == NULL)
        {
            send_to_char ("You can't find it.\n\r", ch);
            return;
        }
    }

    act ("$n recites $p.", ch, scroll, NULL, TO_ROOM);
    act ("You recite $p.", ch, scroll, NULL, TO_CHAR);

    if (number_percent () >= 20 + get_skill (ch, gsn_scrolls) * 4 / 5)
    {
        send_to_char ("You mispronounce a syllable.\n\r", ch);
        check_improve (ch, gsn_scrolls, FALSE, 2);
    }

    else
    {
        obj_cast_spell (scroll->value[1], scroll->value[0], ch, victim, obj);
        obj_cast_spell (scroll->value[2], scroll->value[0], ch, victim, obj);
        obj_cast_spell (scroll->value[3], scroll->value[0], ch, victim, obj);
        check_improve (ch, gsn_scrolls, TRUE, 2);
    }

    extract_obj (scroll);
    return;
}



void do_brandish (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ((staff = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
        send_to_char ("You hold nothing in your hand.\n\r", ch);
        return;
    }

    if (staff->item_type != ITEM_STAFF)
    {
        send_to_char ("You can brandish only with a staff.\n\r", ch);
        return;
    }

    if ((sn = staff->value[3]) < 0
        || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
        bug ("Do_brandish: bad sn %d.", sn);
        return;
    }

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

    if (staff->value[2] > 0)
    {
        act ("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
        act ("You brandish $p.", ch, staff, NULL, TO_CHAR);
        if (ch->level < staff->level
            || number_percent () >= 20 + get_skill (ch, gsn_staves) * 4 / 5)
        {
            act ("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
            act ("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
            check_improve (ch, gsn_staves, FALSE, 2);
        }

        else
            for (vch = ch->in_room->people; vch; vch = vch_next)
            {
                vch_next = vch->next_in_room;

                switch (skill_table[sn].target)
                {
                    default:
                        bug ("Do_brandish: bad target for sn %d.", sn);
                        return;

                    case TAR_IGNORE:
                        if (vch != ch)
                            continue;
                        break;

                    case TAR_CHAR_OFFENSIVE:
                        if (IS_NPC (ch) ? IS_NPC (vch) : !IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_DEFENSIVE:
                        if (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch))
                            continue;
                        break;

                    case TAR_CHAR_SELF:
                        if (vch != ch)
                            continue;
                        break;
                }

                obj_cast_spell (staff->value[3], staff->value[0], ch, vch,
                                NULL);
                check_improve (ch, gsn_staves, TRUE, 2);
            }
    }

    if (--staff->value[2] <= 0)
    {
        act ("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
        act ("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
        extract_obj (staff);
    }

    return;
}



void do_zap (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument (argument, arg);
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
        send_to_char ("Zap whom or what?\n\r", ch);
        return;
    }

    if ((wand = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
        send_to_char ("You hold nothing in your hand.\n\r", ch);
        return;
    }

    if (wand->item_type != ITEM_WAND)
    {
        send_to_char ("You can zap only with a wand.\n\r", ch);
        return;
    }

    obj = NULL;
    if (arg[0] == '\0')
    {
        if (ch->fighting != NULL)
        {
            victim = ch->fighting;
        }
        else
        {
            send_to_char ("Zap whom or what?\n\r", ch);
            return;
        }
    }
    else
    {
        if ((victim = get_char_room (ch, arg)) == NULL
            && (obj = get_obj_here (ch, arg)) == NULL)
        {
            send_to_char ("You can't find it.\n\r", ch);
            return;
        }
    }

    WAIT_STATE (ch, 2 * PULSE_VIOLENCE);

    if (wand->value[2] > 0)
    {
        if (victim != NULL)
        {
            act ("$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT);
            act ("You zap $N with $p.", ch, wand, victim, TO_CHAR);
            act ("$n zaps you with $p.", ch, wand, victim, TO_VICT);
        }
        else
        {
            act ("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
            act ("You zap $P with $p.", ch, wand, obj, TO_CHAR);
        }

        if (ch->level < wand->level
            || number_percent () >= 20 + get_skill (ch, gsn_wands) * 4 / 5)
        {
            act ("Your efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_CHAR);
            act ("$n's efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_ROOM);
            check_improve (ch, gsn_wands, FALSE, 2);
        }
        else
        {
            obj_cast_spell (wand->value[3], wand->value[0], ch, victim, obj);
            check_improve (ch, gsn_wands, TRUE, 2);
        }
    }

    if (--wand->value[2] <= 0)
    {
        act ("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
        act ("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
        extract_obj (wand);
    }

    return;
}



void do_steal (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char ("Steal what from whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg2)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char ("That's pointless.\n\r", ch);
        return;
    }

    if (is_safe (ch, victim))
        return;

    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
    {
        send_to_char ("Kill stealing is not permitted.\n\r"
                      "You'd better not -- you might get hit.\n\r", ch);
        return;
    }

    WAIT_STATE (ch, skill_table[gsn_steal].beats);
    percent = number_percent ();

    /* Kobolds get 20% better steal chance */
    if (!IS_NPC(ch) && ch->race == race_lookup("kobold"))
        percent -= 20;

    if (!IS_AWAKE (victim))
        percent -= 10;
    else if (!can_see (victim, ch))
        percent += 25;
    else
        percent += 50;

    if (((ch->level + 7 < victim->level || ch->level - 7 > victim->level)
         && !IS_NPC (victim) && !IS_NPC (ch))
        || (!IS_NPC (ch) && percent > get_skill (ch, gsn_steal))
        || (!IS_NPC (ch) && !is_clan (ch)))
    {
        /*
         * Failure.
         */
        send_to_char ("Oops.\n\r", ch);
        affect_strip (ch, gsn_sneak);
        STR_REMOVE_BIT (ch->affected_by, AFF_SNEAK);

        act ("$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT);
        act ("$n tried to steal from $N.\n\r", ch, NULL, victim, TO_NOTVICT);
        switch (number_range (0, 3))
        {
            case 0:
                sprintf (buf, "%s is a lousy thief!", ch->name);
                break;
            case 1:
                sprintf (buf, "%s couldn't rob %s way out of a paper bag!",
                         ch->name, (ch->sex == 2) ? "her" : "his");
                break;
            case 2:
                sprintf (buf, "%s tried to rob me!", ch->name);
                break;
            case 3:
                sprintf (buf, "Keep your hands out of there, %s!", ch->name);
                break;
        }
        if (!IS_AWAKE (victim))
            do_function (victim, &do_wake, "");
        if (IS_AWAKE (victim))
            do_function (victim, &do_yell, buf);
        if (!IS_NPC (ch))
        {
            if (IS_NPC (victim))
            {
                check_improve (ch, gsn_steal, FALSE, 2);
                multi_hit (victim, ch, TYPE_UNDEFINED);
            }
            else
            {
                sprintf (buf, "$N tried to steal from %s.", victim->name);
                wiznet (buf, ch, NULL, WIZ_FLAGS, 0, 0);
                if (!IS_SET (ch->act, PLR_THIEF))
                {
                    SET_BIT (ch->act, PLR_THIEF);
                    send_to_char ("*** You are now a THIEF!! ***\n\r", ch);
                    save_char_obj (ch);
                }
            }
        }

        return;
    }

    if (!str_cmp (arg1, "coin")
        || !str_cmp (arg1, "coins")
        || !str_cmp (arg1, "gold") || !str_cmp (arg1, "silver"))
    {
        int gold, silver;

        gold = victim->gold * number_range (1, ch->level) / MAX_LEVEL;
        silver = victim->silver * number_range (1, ch->level) / MAX_LEVEL;
        if (gold <= 0 && silver <= 0)
        {
            send_to_char ("You couldn't get any coins.\n\r", ch);
            return;
        }

        ch->gold += gold;
        ch->silver += silver;
        victim->silver -= silver;
        victim->gold -= gold;
        if (silver <= 0)
            sprintf (buf, "Bingo!  You got %d gold coins.\n\r", gold);
        else if (gold <= 0)
            sprintf (buf, "Bingo!  You got %d silver coins.\n\r", silver);
        else
            sprintf (buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
                     silver, gold);

        send_to_char (buf, ch);
        check_improve (ch, gsn_steal, TRUE, 2);
        return;
    }

    if ((obj = get_obj_carry (victim, arg1, ch)) == NULL)
    {
        send_to_char ("You can't find it.\n\r", ch);
        return;
    }

    if (!can_drop_obj (ch, obj)
        || IS_SET (obj->extra_flags, ITEM_INVENTORY)
        || obj->level > ch->level)
    {
        send_to_char ("You can't pry it away.\n\r", ch);
        return;
    }

    if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
        send_to_char ("You have your hands full.\n\r", ch);
        return;
    }

    if (ch->carry_weight + get_obj_weight (obj) > can_carry_w (ch))
    {
        send_to_char ("You can't carry that much weight.\n\r", ch);
        return;
    }

    obj_from_char (obj);
    obj_to_char (obj, ch);
    act ("You pocket $p.", ch, obj, NULL, TO_CHAR);
    check_improve (ch, gsn_steal, TRUE, 2);
    send_to_char ("Got it!\n\r", ch);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper (CHAR_DATA * ch)
{
    /*char buf[MAX_STRING_LENGTH]; */
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room)
    {
        if (IS_NPC (keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
            break;
    }

    if (pShop == NULL)
    {
        send_to_char ("You can't do that here.\n\r", ch);
        return NULL;
    }

    /*
     * Undesirables.
     *
     if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
     {
     do_function(keeper, &do_say, "Killers are not welcome!");
     sprintf(buf, "%s the KILLER is over here!\n\r", ch->name);
     do_function(keeper, &do_yell, buf );
     return NULL;
     }

     if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
     {
     do_function(keeper, &do_say, "Thieves are not welcome!");
     sprintf(buf, "%s the THIEF is over here!\n\r", ch->name);
     do_function(keeper, &do_yell, buf );
     return NULL;
     }
     */
    /*
     * Shop hours with detailed messages.
     */
    if ( time_info.hour == pShop->open_hour - 2
    ||   time_info.hour == pShop->open_hour - 3 )
    {
	act("$N tells you 'We are closed! We will open in a couple hours.'", ch, NULL, keeper, TO_CHAR);
	return NULL;
    }
    else if ( time_info.hour == pShop->open_hour - 1 )
    {
	act("$N tells you 'We are closed! We will open in within the hour.'", ch, NULL, keeper, TO_CHAR);
	return NULL;
    }
    else if (time_info.hour < pShop->open_hour)
    {
        char buf[MAX_STRING_LENGTH];
	sprintf(buf, "$N tells you 'Sorry, we are closed. We will open at %d%s %s.'",
	   pShop->open_hour < 13 ? pShop->open_hour : pShop->open_hour - 12, 
	   pShop->open_hour < 13 ? "am" : "pm",
	   time_info.hour < pShop->open_hour ?  "today" : "tomorrow");

	act(buf, ch, NULL, keeper, TO_CHAR);
	return NULL;
    }

    if (time_info.hour > pShop->close_hour)
    {
        do_function (keeper, &do_say,
                     "Sorry, I am closed. Come back tomorrow.");
        return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if (!can_see (keeper, ch))
    {
        do_function (keeper, &do_say,
                     "I don't trade with folks I can't see.");
        return NULL;
    }

    return keeper;
}

CHAR_DATA *find_armorsmith (CHAR_DATA *ch)
{
    CHAR_DATA *keeper;

    for (keeper = ch->in_room->people; keeper != NULL; keeper = keeper->next_in_room)
    {
        if (!IS_NPC(keeper))
            continue;

        if (!IS_SET(keeper->act, ACT_ARMORSMITH))
            continue;

        if (!IS_AWAKE(keeper))
        {
            act("$n is asleep and can't help you right now.", keeper, NULL, ch, TO_VICT);
            return NULL;
        }

        if (!can_see(keeper, ch))
        {
            act("$n says 'I only work with customers I can see.'", keeper, NULL, ch, TO_VICT);
            ch->reply = keeper;
            return NULL;
        }

        return keeper;
    }

    send_to_char("There is no armorsmith here to help you.\n\r", ch);
    return NULL;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper (OBJ_DATA * obj, CHAR_DATA * ch)
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
        t_obj_next = t_obj->next_content;

        if (obj->pIndexData == t_obj->pIndexData
            && !str_cmp (obj->short_descr, t_obj->short_descr))
        {
            /* if this is an unlimited item, destroy the new one */
            if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY))
            {
                extract_obj (obj);
                return;
            }
            obj->cost = t_obj->cost;    /* keep it standard */
            break;
        }
    }

    if (t_obj == NULL)
    {
        obj->next_content = ch->carrying;
        ch->carrying = obj;
    }
    else
    {
        obj->next_content = t_obj->next_content;
        t_obj->next_content = obj;
    }

    obj->carried_by = ch;
    obj->in_room = NULL;
    obj->in_obj = NULL;
    ch->carry_number += get_obj_number (obj);
    ch->carry_weight += get_obj_weight (obj);
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper (CHAR_DATA * ch, CHAR_DATA * keeper, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = keeper->carrying; obj != NULL; obj = obj->next_content)
    {
        if (obj->wear_loc == WEAR_NONE && can_see_obj (keeper, obj)
            && can_see_obj (ch, obj) && is_name (arg, obj->name))
        {
            if (++count == number)
                return obj;

            /* skip other objects of the same name */
            while (obj->next_content != NULL
                   && obj->pIndexData == obj->next_content->pIndexData
                   && !str_cmp (obj->short_descr,
                                obj->next_content->short_descr)) obj =
                    obj->next_content;
        }
    }

    return NULL;
}

int get_cost (CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
    SHOP_DATA *pShop;
    int cost;

    if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
        return 0;

    if (fBuy)
    {
        cost = obj->cost * pShop->profit_buy / 100;
    }
    else
    {
        OBJ_DATA *obj2;
        int itype;

        cost = 0;
        for (itype = 0; itype < MAX_TRADE; itype++)
        {
            if (obj->item_type == pShop->buy_type[itype])
            {
                cost = obj->cost * pShop->profit_sell / 100;
                break;
            }
        }

        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
            for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content)
            {
                if (obj->pIndexData == obj2->pIndexData
                    && !str_cmp (obj->short_descr, obj2->short_descr))
                {
                    if (IS_OBJ_STAT (obj2, ITEM_INVENTORY))
                        cost /= 2;
                    else
                        cost = cost * 3 / 4;
                }
            }
    }

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
        if (obj->value[1] == 0)
            cost /= 4;
        else
            cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int cost, roll;

    if (argument[0] == '\0')
    {
        send_to_char ("Buy what?\n\r", ch);
        return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP) )
    {
        do_buy_mount( ch, argument );
        return;
    }

    if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
        char arg[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        CHAR_DATA *pet;
        ROOM_INDEX_DATA *pRoomIndexNext;
        ROOM_INDEX_DATA *in_room;

        smash_tilde (argument);

        if (IS_NPC (ch))
            return;

        argument = one_argument (argument, arg);

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index (9706);
        else
            pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
        if (pRoomIndexNext == NULL)
        {
            bug ("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
            send_to_char ("Sorry, you can't buy that here.\n\r", ch);
            return;
        }

        in_room = ch->in_room;
        ch->in_room = pRoomIndexNext;
        pet = get_char_room (ch, arg);
        ch->in_room = in_room;

        if (pet == NULL || !IS_SET (pet->act, ACT_PET))
        {
            send_to_char ("Sorry, you can't buy that here.\n\r", ch);
            return;
        }

        if (ch->pet != NULL)
        {
            send_to_char ("You already own a pet.\n\r", ch);
            return;
        }

        cost = 10 * pet->level * pet->level;

        if ((ch->silver + 100 * ch->gold) < cost)
        {
            send_to_char ("You can't afford it.\n\r", ch);
            return;
        }

        if (ch->level < pet->level)
        {
            send_to_char
                ("You're not powerful enough to master this pet.\n\r", ch);
            return;
        }

        /* haggle */
        roll = number_percent ();
        if (roll < get_skill (ch, gsn_haggle))
        {
            cost -= cost / 2 * roll / 100;
            /* Halflings get 20% better haggle results */
            if (!IS_NPC(ch) && ch->race == race_lookup("halfling"))
                cost -= cost * 20 / 100;
            sprintf (buf, "You haggle the price down to %d coins.\n\r", cost);
            send_to_char (buf, ch);
            check_improve (ch, gsn_haggle, TRUE, 4);

        }

        deduct_cost (ch, cost);
        pet = create_mobile (pet->pIndexData);
        SET_BIT (pet->act, ACT_PET);
        STR_SET_BIT (pet->affected_by, AFF_CHARM);
        pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

        argument = one_argument (argument, arg);
        if (arg[0] != '\0')
        {
            sprintf (buf, "%s %s", pet->name, arg);
            free_string (pet->name);
            pet->name = str_dup (buf);
        }

        sprintf (buf, "%sA neck tag says 'I belong to %s'.\n\r",
                 pet->description, ch->name);
        free_string (pet->description);
        pet->description = str_dup (buf);

        char_to_room (pet, ch->in_room);
        add_follower (pet, ch);
        pet->leader = ch;
        ch->pet = pet;
        send_to_char ("Enjoy your pet.\n\r", ch);
        act ("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
        return;
    }
    else
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj, *t_obj;
        char arg[MAX_INPUT_LENGTH];
        int number, count = 1;

        if ((keeper = find_keeper (ch)) == NULL)
            return;

        /* Check if this is a questmaster - they should use 'quest buy' instead */
        if (keeper->spec_fun == spec_lookup ("spec_questmaster"))
        {
            act ("$n tells you 'Use 'quest buy <item>' to purchase quest items with glory points.'",
                 keeper, NULL, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        number = mult_argument (argument, arg);
        obj = get_obj_keeper (ch, keeper, arg);
        cost = get_cost (keeper, obj, TRUE);

        if (number < 1 || number > 99)
        {
            act ("$n tells you 'Get real!", keeper, NULL, ch, TO_VICT);
            return;
        }

        if (cost <= 0 || !can_see_obj (ch, obj))
        {
            act ("$n tells you 'I don't sell that -- try 'list''.",
                 keeper, NULL, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (!IS_OBJ_STAT (obj, ITEM_INVENTORY))
        {
            for (t_obj = obj->next_content;
                 count < number && t_obj != NULL; t_obj = t_obj->next_content)
            {
                if (t_obj->pIndexData == obj->pIndexData
                    && !str_cmp (t_obj->short_descr, obj->short_descr))
                    count++;
                else
                    break;
            }

            if (count < number)
            {
                act ("$n tells you 'I don't have that many in stock.",
                     keeper, NULL, ch, TO_VICT);
                ch->reply = keeper;
                return;
            }
        }

        if ((ch->silver + ch->gold * 100) < cost * number)
        {
            if (number > 1)
                act ("$n tells you 'You can't afford to buy that many.",
                     keeper, obj, ch, TO_VICT);
            else
                act ("$n tells you 'You can't afford to buy $p'.",
                     keeper, obj, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (obj->level > ch->level)
        {
            act ("$n tells you 'You can't use $p yet'.",
                 keeper, obj, ch, TO_VICT);
            ch->reply = keeper;
            return;
        }

        if (ch->carry_number + number * get_obj_number (obj) >
            can_carry_n (ch))
        {
            send_to_char ("You can't carry that many items.\n\r", ch);
            return;
        }

        if (ch->carry_weight + number * get_obj_weight (obj) >
            can_carry_w (ch))
        {
            send_to_char ("You can't carry that much weight.\n\r", ch);
            return;
        }

        /* haggle */
        roll = number_percent ();
        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
            && roll < get_skill (ch, gsn_haggle))
        {
            cost -= obj->cost / 2 * roll / 100;
            /* Halflings get 20% better haggle results */
            if (!IS_NPC(ch) && ch->race == race_lookup("halfling"))
                cost -= cost * 20 / 100;
            act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
            check_improve (ch, gsn_haggle, TRUE, 4);
        }

        if (number > 1)
        {
            sprintf (buf, "$n buys $p[%d].", number);
            act (buf, ch, obj, NULL, TO_ROOM);
            sprintf (buf, "You buy $p[%d] for %d silver.", number,
                     cost * number);
            act (buf, ch, obj, NULL, TO_CHAR);
        }
        else
        {
            act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
            sprintf (buf, "You buy $p for %d silver.", cost);
            act (buf, ch, obj, NULL, TO_CHAR);
        }
        deduct_cost (ch, cost * number);
        keeper->gold += cost * number / 100;
        keeper->silver += cost * number - (cost * number / 100) * 100;

        for (count = 0; count < number; count++)
        {
            if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
                t_obj = create_object (obj->pIndexData, obj->level);
            else
            {
                t_obj = obj;
                obj = obj->next_content;
                obj_from_char (t_obj);
            }

            if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
                t_obj->timer = 0;
            REMOVE_BIT (t_obj->extra_flags, ITEM_HAD_TIMER);
            obj_to_char (t_obj, ch);
            if (cost < t_obj->cost)
                t_obj->cost = cost;
        }
    }
}



void do_list (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)
      || IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP) )
    {
        ROOM_INDEX_DATA *pRoomIndexNext;
        CHAR_DATA *pet;
        bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index (9706);
        else
            pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);

        if (pRoomIndexNext == NULL)
        {
            bug ("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
            send_to_char ("You can't do that here.\n\r", ch);
            return;
        }

        found = FALSE;
        for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room)
        {
            if (IS_SET (pet->act, ACT_PET) 
                || IS_SET(pet->act, ACT_MOUNT) )
            {
                if (!found)
                {
                    found = TRUE;
                    if (IS_SET(pet->act, ACT_PET))
                        send_to_char ("Pets for sale:\n\r", ch );
                    else if (IS_SET(pet->act, ACT_MOUNT))
                        send_to_char ("Mounts for sale:\n\r", ch );
                }
                sprintf (buf, "[%2d] %8d - %s\n\r",
                         pet->level,
                         10 * pet->level * pet->level, pet->short_descr);
                send_to_char (buf, ch);
            }
        }
        if ( !found )
            if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
                send_to_char ("Sorry, we're out of pets right now.\n\r", ch );
            else
                send_to_char ("Sorry, we're out of mounts right now.\n\r", ch );
        return;
    }
    else
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost, count;
        bool found;
        char arg[MAX_INPUT_LENGTH];

        if ((keeper = find_keeper (ch)) == NULL)
            return;
        one_argument (argument, arg);

        found = FALSE;
        for (obj = keeper->carrying; obj; obj = obj->next_content)
        {
            if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj)
                && (cost = get_cost (keeper, obj, TRUE)) > 0
                && (arg[0] == '\0' || is_name (arg, obj->name)))
            {
                if (!found)
                {
                    found = TRUE;
                    send_to_char ("[Lv Price Qty] Item\n\r", ch);
                }

                if (IS_OBJ_STAT (obj, ITEM_INVENTORY))
                    sprintf (buf, "[%2d %5d -- ] %s\n\r",
                             obj->level, cost, obj->short_descr);
                else
                {
                    count = 1;

                    while (obj->next_content != NULL
                           && obj->pIndexData == obj->next_content->pIndexData
                           && !str_cmp (obj->short_descr,
                                        obj->next_content->short_descr))
                    {
                        obj = obj->next_content;
                        count++;
                    }
                    sprintf (buf, "[%2d %5d %2d ] %s\n\r",
                             obj->level, cost, count, obj->short_descr);
                }
                send_to_char (buf, ch);
            }
        }

        if (!found)
            send_to_char ("You can't buy anything here.\n\r", ch);
        return;
    }
}



void do_repair (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *smith;
    OBJ_DATA *obj;
    OBJ_DATA *target = NULL;
    char arg[MAX_INPUT_LENGTH];
    bool repair_all = FALSE;
    int total_cost = 0;
    int total_points = 0;
    int cost;
    int max_cond;
    int restore;
    char buf[MAX_STRING_LENGTH];

    if ((smith = find_armorsmith(ch)) == NULL)
        return;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        act("$n says 'Syntax: repair <item> or repair all.'", smith, NULL, ch, TO_VICT);
        ch->reply = smith;
        return;
    }

    if (!str_cmp(arg, "all"))
        repair_all = TRUE;

    if (repair_all)
    {
        for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        {
            if (!is_repairable_item(obj))
                continue;

            max_cond = get_obj_max_condition(obj);
            if (max_cond <= 0)
                continue;

            if (obj->condition >= max_cond)
                continue;

            restore = max_cond - obj->condition;
            total_points += restore;
            total_cost += repair_cost_per_point(obj) * restore;
        }

        if (total_points == 0)
        {
            act("$n says 'You don't have anything that needs fixing.'", smith, NULL, ch, TO_VICT);
            ch->reply = smith;
            return;
        }
    }
    else
    {
        int number = number_argument(arg, arg);
        int count = 0;

        for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        {
            if (!is_name(arg, obj->name))
                continue;

            if (++count == number)
            {
                target = obj;
                break;
            }
        }

        if (target == NULL)
        {
            act("$n says 'You don't seem to have that.'", smith, NULL, ch, TO_VICT);
            ch->reply = smith;
            return;
        }

        if (!is_repairable_item(target))
        {
            act("$n says 'I can't do anything with $p.'", smith, target, ch, TO_VICT);
            ch->reply = smith;
            return;
        }

        max_cond = get_obj_max_condition(target);
        if (max_cond <= 0)
            max_cond = 100;

        if (target->condition >= max_cond)
        {
            act("$n says '$p doesn't need any work.'", smith, target, ch, TO_VICT);
            ch->reply = smith;
            return;
        }

        restore = max_cond - target->condition;
        total_points = restore;
        total_cost = repair_cost_per_point(target) * restore;
    }

    if (total_cost <= 0)
        total_cost = 1;

    cost = total_cost;

    if (!IS_NPC(ch))
    {
        int skill = get_skill(ch, gsn_haggle);
        if (skill > 0)
        {
            int roll = number_percent();
            if (roll < skill)
            {
                cost -= cost / 2 * roll / 100;
                if (!IS_NPC(ch) && ch->race == race_lookup("halfling"))
                    cost -= cost * 20 / 100;
                cost = UMAX(1, cost);
                sprintf(buf, "You haggle the price down to %d coins.\n\r", cost);
                send_to_char(buf, ch);
                check_improve(ch, gsn_haggle, TRUE, 4);
            }
        }
    }

    if ((ch->gold * 100 + ch->silver) < cost)
    {
        act("$n says 'You can't afford my rates.'", smith, NULL, ch, TO_VICT);
        ch->reply = smith;
        return;
    }

    int gold = cost / 100;
    int silver = cost % 100;

    if (gold > 0 && silver > 0)
        sprintf(buf, "$n says 'That will be %d gold and %d silver.'", gold, silver);
    else if (gold > 0)
        sprintf(buf, "$n says 'That will be %d gold.'", gold);
    else
        sprintf(buf, "$n says 'That will be %d silver.'", silver);
    act(buf, smith, NULL, ch, TO_VICT);
    ch->reply = smith;

    deduct_cost(ch, cost);
    smith->gold += gold;
    smith->silver += silver;
    while (smith->silver >= 100)
    {
        smith->gold += smith->silver / 100;
        smith->silver %= 100;
    }

    if (repair_all)
    {
        act("$n gets to work repairing your equipment.", smith, NULL, ch, TO_VICT);
        act("$n gets to work repairing $N's equipment.", smith, NULL, ch, TO_NOTVICT);

        for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        {
            if (!is_repairable_item(obj))
                continue;
            max_cond = get_obj_max_condition(obj);
            if (max_cond <= 0 || obj->condition >= max_cond)
                continue;
            obj->condition = max_cond;
        }
    }
    else if (target != NULL)
    {
        target->condition = get_obj_max_condition(target);
        act("$n repairs $p, restoring it to good condition.", smith, target, ch, TO_VICT);
        act("$n repairs $p for $N.", smith, target, ch, TO_NOTVICT);
    }

    act("$n says 'All done.'", smith, NULL, ch, TO_VICT);
    send_to_char("{GYour gear looks as good as new.{x\n\r", ch);
}


void do_sell (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost, roll;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Sell what?\n\r", ch);
        return;
    }

    if ((keeper = find_keeper (ch)) == NULL)
        return;

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        act ("$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT);
        ch->reply = keeper;
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        send_to_char ("You can't let go of it.\n\r", ch);
        return;
    }

    if (!can_see_obj (keeper, obj))
    {
        act ("$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT);
        return;
    }

    if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
        act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
        return;
    }
    if (cost > (keeper->silver + 100 * keeper->gold))
    {
        act ("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
             keeper, obj, ch, TO_VICT);
        return;
    }

    act ("$n sells $p.", ch, obj, NULL, TO_ROOM);
    /* haggle */
    roll = number_percent ();
    if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
        && roll < get_skill (ch, gsn_haggle))
    {
        send_to_char ("You haggle with the shopkeeper.\n\r", ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN (cost, 95 * get_cost (keeper, obj, TRUE) / 100);
        cost = UMIN (cost, (keeper->silver + 100 * keeper->gold));
        check_improve (ch, gsn_haggle, TRUE, 4);
    }
    sprintf (buf, "You sell $p for %d silver and %d gold piece%s.",
             cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
    act (buf, ch, obj, NULL, TO_CHAR);
    ch->gold += cost / 100;
    ch->silver += cost - (cost / 100) * 100;
    deduct_cost (keeper, cost);
    if (keeper->gold < 0)
        keeper->gold = 0;
    if (keeper->silver < 0)
        keeper->silver = 0;

    if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
    {
        extract_obj (obj);
    }
    else
    {
        obj_from_char (obj);
        if (obj->timer)
            SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        else
            obj->timer = number_range (50, 100);
        obj_to_keeper (obj, keeper);
    }

    return;
}



void do_value (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Value what?\n\r", ch);
        return;
    }

    if ((keeper = find_keeper (ch)) == NULL)
        return;

    if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
        act ("$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT);
        ch->reply = keeper;
        return;
    }

    if (!can_see_obj (keeper, obj))
    {
        act ("$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT);
        return;
    }

    if (!can_drop_obj (ch, obj))
    {
        send_to_char ("You can't let go of it.\n\r", ch);
        return;
    }

    if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
        act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
        return;
    }

    sprintf (buf,
             "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
             cost - (cost / 100) * 100, cost / 100);
    act (buf, keeper, obj, ch, TO_VICT);
    ch->reply = keeper;

    return;
}

void do_second (CHAR_DATA *ch, char *argument)
/* wear object as a secondary weapon */
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument, ch); /* find the obj withing ch's inventory */

    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }

    /* check if the char is using a shield or a held weapon */
    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
    {
        send_to_char ("You cannot use a secondary weapon while using a shield or holding an item\n\r",ch);
        return;
    }

    if ( ch->level < obj->level )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r",
            obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM );
        return;
    }

    /* check that the character is using a first weapon at all */
    if (get_eq_char (ch, WEAR_WIELD) == NULL) /* oops - != here was a bit wrong :) */
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }

    /* check if the primary weapon is two-handed */
    OBJ_DATA *primary_weapon = get_eq_char(ch, WEAR_WIELD);
    if (primary_weapon != NULL && IS_OBJ_STAT(primary_weapon, WEAPON_TWO_HANDS))
    {
        send_to_char ("You cannot dual-wield while using a two-handed weapon.\n\r", ch);
        return;
    }

    /* at last - the char uses the weapon */
    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */

    /* char CAN use the item! that didn't take long at aaall */
    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
}

void do_shoot( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *gun;
    OBJ_DATA *obj;
	char *mode;
	int shots, number, recoil;

    argument = one_argument( argument, arg);
    argument = one_argument( argument, arg2 );

	mode = arg;

    /* are there a target mentioned ? */
    if ( arg2[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Shoot at whom or what?\n\r", ch );
	return;
    }

    /* did the user select mode? */
    if ( arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char ("Which mode?\n\r", ch);
	return;
    }

    /* do the character hold something ? */
    if ( ( gun = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    /* is it a gun ? */
    if ( gun->item_type != ITEM_GUN )
    {
	send_to_char( "You need a firearm to shoot.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Shoot whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	/* is it really here ? */
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    /* do we deal with a loaded gun ?*/
    if ( gun->value[2] <= 0 )
    {
	act( "The $p seems to be out of ammo.", ch, gun, NULL, TO_CHAR );
	gun->value[2] = 0;
	return;
    }
	/* Lets see how many shots to fire */
	if (( !str_cmp (mode, "dual")) && (gun->value[4] & GUN_DUAL))
	shots = 2;
	else if (( !str_cmp (mode, "burst")) && (gun->value[4] & GUN_BURST))
	shots = 3;
	else if (( !str_cmp (mode, "auto")) && (gun->value[4] & GUN_AUTO))
	shots = 5;
	else if (( !str_cmp (mode, "single")) && (gun->value[4] & GUN_SINGLE))
	shots = 1;
	else
		{
		send_to_char ("That is not an accepted mode for that weapon.\n\r", ch);
		return;
		}

	gun->value[2] = gun->value[2] - shots;

    /* Do the character know how to handle a gun ? */
    for(number = 0 ; number < shots ; number = number + 1)
	{
 	if ((number_percent() <= get_skill(ch,gsn_clip) && (gun->value[4] & GUN_CLIP)) 
 	|| (number_percent() <= get_skill(ch,gsn_energy) && (gun->value[4] & GUN_ENERGY)) 
 	|| (number_percent() <= get_skill(ch,gsn_shell) && (gun->value[4] & GUN_SHELL)) 
 	|| (number_percent() <= get_skill(ch,gsn_fuel) && (gun->value[4] & GUN_FUEL)) 
 	|| (number_percent() <= get_skill(ch,gsn_rocket) && (gun->value[4] & GUN_ROCKET)) )
	{
	    obj_cast_spell( gun->value[3], gun->value[0], ch, victim, obj );

    /* ...and he shoots... */
    if ( gun->value[2] > 0 )
    {
		if ( victim != NULL )
		{
			act( "$n shoots $N with $p.", ch, gun, victim, TO_NOTVICT );
			act( "You shoot $N with $p.", ch, gun, victim, TO_CHAR );
			act( "$n shoots at you with $p.",ch, gun, victim, TO_VICT );
		}
		else
		{
	    act( "$n shoots $P with $p.", ch, gun, obj, TO_ROOM );
	    act( "You shoot $P with $p.", ch, gun, obj, TO_CHAR );
		}

	}

	else
	{
	    /* a missed shot */
	    act( "Your miss the target.", ch,gun,NULL,TO_CHAR);
	    act( "$n misses with a shot from $p.", ch,gun,NULL,TO_ROOM);
	    check_improve(ch,gsn_clip,FALSE,2);
		return;
	}

	/* now for a little recoil to hit. */
	/*This section could have been coded better, but it works */
	recoil = 0; /* Default: no recoil */
	if (gun->value[4] & GUN_RECOIL_LITTLE && (get_curr_stat(ch,STAT_STR)) < 12)
	recoil = 1;
	if (gun->value[4] & GUN_RECOIL_NORMAL && (get_curr_stat(ch,STAT_STR)) < 16)
	recoil = 1;
	if (gun->value[4] & GUN_RECOIL_NORMAL && (get_curr_stat(ch,STAT_STR)) < 12)
	recoil = 2;
	if (gun->value[4] & GUN_RECOIL_HARD && (get_curr_stat(ch,STAT_STR)) < 25)
	recoil = 1;
	if (gun->value[4] & GUN_RECOIL_HARD && (get_curr_stat(ch,STAT_STR)) < 16)
	recoil = 2;
	if (gun->value[4] & GUN_RECOIL_HARD && (get_curr_stat(ch,STAT_STR)) < 12)
	recoil = 3;
	if (gun->value[4] & GUN_RECOIL_SEVERE && (get_curr_stat(ch,STAT_STR)) < 36)
	recoil = 1;
	if (gun->value[4] & GUN_RECOIL_SEVERE && (get_curr_stat(ch,STAT_STR)) < 25)
	recoil = 2;
	if (gun->value[4] & GUN_RECOIL_SEVERE && (get_curr_stat(ch,STAT_STR)) < 18)
	recoil = 3;
	if (gun->value[4] & GUN_RECOIL_SEVERE && (get_curr_stat(ch,STAT_STR)) < 12)
	recoil = 4;

	if (recoil == 1)
	{
	send_to_char ("The recoil slows you down a bit.\n\r", ch);
	WAIT_STATE(ch,1 * PULSE_VIOLENCE);
	}
	if (recoil == 2)
	{
	send_to_char ("The recoil sends throws you off balance.\n\r", ch);
	WAIT_STATE(ch,2 * PULSE_VIOLENCE);
	}
	if (recoil == 3)
	{
	send_to_char ("The strong recoil almost numbs your arm.\n\r", ch);
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	}
	if (recoil == 4)
	{
	send_to_char ("The recoil almost shatters your hand.\n\r", ch);
	WAIT_STATE(ch,4 * PULSE_VIOLENCE);
	}

	}
	}

}

void do_reload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *ammo;
    OBJ_DATA *gun;
	int charge, maxcharge, oldammo, newammo;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* not the ammo the character thinks he has */
    if ( ( ammo = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that kind of ammo.\n\r", ch );
	return;
    }

    /* nope, that item cannot be loaded into a gun */
    if ( ammo->item_type != ITEM_AMMO )
    {
	send_to_char( "You need ammo to reload.\n\r", ch );
	return;
    }

    /* maybe the character tried to reload something besides a gun */
    if ( arg2[0] == '\0' )
    {
	send_to_char( "You need a gun to reload.\n\r", ch);
	return;
    }
    else
    {
	if   (( gun    = get_obj_here  ( ch, arg2 ) ) == NULL)
	{
	    send_to_char( "You can't find that gun.\n\r", ch );
	    return;
	}
    }
	/* comparing values */
	oldammo = gun->value[3];
	newammo = ammo->value [3];

	/* do the ammo match the gun ? */
	if (((IS_WEAPON_STAT(gun,GUN_CLIP)) && IS_WEAPON_STAT(ammo,GUN_CLIP))
	|| ((IS_WEAPON_STAT(gun,GUN_ENERGY)) && IS_WEAPON_STAT(ammo,GUN_ENERGY))
	|| ((IS_WEAPON_STAT(gun,GUN_SHELL)) && IS_WEAPON_STAT(ammo,GUN_SHELL))
	|| ((IS_WEAPON_STAT(gun,GUN_FUEL)) && IS_WEAPON_STAT(ammo,GUN_FUEL))
	|| ((IS_WEAPON_STAT(gun,GUN_ROCKET)) && IS_WEAPON_STAT(ammo,GUN_ROCKET)))
	{
		if (oldammo != newammo)
		{
			gun->value[2] = 0;
		}
		gun->value[2] = gun->value[2] + ammo->value[1];
		gun->value[3] = ammo->value[3];
	}
	else
	{
	    send_to_char( "Thats the wrong ammo for that weapon.\n\r", ch );
	    return;
	}
	
	/* do we have more ammo in the gun then it can hold */
	charge = gun->value[2];
	maxcharge = gun->value[1];

	if (charge > maxcharge)
	{
		gun->value[2] = gun->value[1];
	}

    act( "$n reloads $p.", ch, gun, NULL, TO_ROOM );
    act( "You reload $p.", ch, gun, NULL, TO_CHAR );
 

    extract_obj( ammo );
    return;
}

/*
 * Save items in a clan storage room - adapted for ROM
 * Original by Scryn & Thoric (SMAUG), adapted by Rhys
 */
void save_clan_storeroom( CHAR_DATA *ch )
{
    FILE *fp;
    char filename[256];
    sh_int templvl;
    OBJ_DATA *contents;

    if ( !ch )
    {
        bug ("save_clan_storeroom: Null ch pointer!", 0);
        return;
    }

    if ( !IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
        return;

    sprintf( filename, "%s%d.vault", STORAGE_DIR, ch->in_room->vnum );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_clan_storeroom: fopen", 0 );
        perror( filename );
    }
    else
    {
        templvl = ch->level;
        ch->level = MAX_LEVEL;  /* make sure EQ doesn't get lost */
        contents = ch->in_room->contents;
        fprintf( fp, "#VNUM %d\n", ch->in_room->vnum );
        if (contents)
            fwrite_obj(ch, contents, fp, 0);
        fprintf( fp, "#END\n" );
        ch->level = templvl;
        fclose( fp );
        return;
    }
    return;
}

/*
 * Save a donation pit's contents
 */
void save_pit( OBJ_DATA *pit )
{
    FILE *fp;
    char filename[256];
    CHAR_DATA dummy_ch;
    
    if ( !pit )
    {
        bug ("save_pit: Null pit pointer!", 0);
        return;
    }
    
    if ( pit->pIndexData->vnum != OBJ_VNUM_PIT && 
         pit->pIndexData->vnum != OBJ_VNUM_ORC_PIT )
        return;
    
    sprintf( filename, "%spit_%d.vault", STORAGE_DIR, pit->pIndexData->vnum );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_pit: fopen", 0 );
        perror( filename );
    }
    else
    {
        /* Create a dummy character with max level */
        memset(&dummy_ch, 0, sizeof(CHAR_DATA));
        dummy_ch.level = MAX_LEVEL;
        
        fprintf( fp, "#PITVNUM %d\n", pit->pIndexData->vnum );
        if (pit->contains)
            fwrite_obj_compat(&dummy_ch, pit->contains, fp, 0);
        fprintf( fp, "#END\n" );
        fclose( fp );
    }
}

/*
 * Save all donation pits in the game
 */
void save_all_pits( void )
{
    OBJ_DATA *obj;
    
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->pIndexData->vnum == OBJ_VNUM_PIT || 
             obj->pIndexData->vnum == OBJ_VNUM_ORC_PIT )
        {
            save_pit( obj );
        }
    }
}
