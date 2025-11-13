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
 *           mount.c - November 13, 2025
 */
/*
 * Mount system implementation
 * Based on the ROM 2.4 horses patch
 */

#include <stddef.h>
#include "merc.h"

/*
 * Mount success function - determines if a character can successfully mount
 */
int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
    int chance;

    if (!IS_NPC(mount))
        return FALSE;

    if (!IS_SET(mount->act, ACT_MOUNT))
        return FALSE;

    /* If checking while already mounted, mount->mount != NULL is expected */
    /* Only check this when initially mounting (canattack == TRUE) */
    if (canattack && mount->mount != NULL)
        return FALSE;

    if (mount->position < POS_STANDING)
        return FALSE;

    if (canattack && mount->fighting != NULL)
        return FALSE;

    /* Immortals always succeed */
    if (IS_IMMORTAL(ch))
        return TRUE;

    chance = get_skill(ch, gsn_riding);
    if (chance == 0)
        return FALSE;

    /* At 100% skill, always succeed */
    if (chance >= 100)
        return TRUE;

    if (IS_AFFECTED(ch, AFF_SLOW))
        chance /= 2;

    if (IS_AFFECTED(ch, AFF_HASTE))
        chance *= 2;

    return (number_percent() < chance);
}

/*
 * Mount command - allows a character to mount a creature
 */
void do_mount(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mount;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Mount what?\n\r", ch);
        return;
    }

    if (MOUNTED(ch))
    {
        send_to_char("You are already mounted.\n\r", ch);
        return;
    }

    if (RIDDEN(ch))
    {
        send_to_char("You are being ridden.\n\r", ch);
        return;
    }

    if (ch->position < POS_STANDING)
    {
        send_to_char("You must be standing to mount.\n\r", ch);
        return;
    }

    if ((mount = get_char_room(ch, arg)) == NULL)
    {
        send_to_char("You don't see that here.\n\r", ch);
        return;
    }

    if (mount == ch)
    {
        send_to_char("You can't mount yourself.\n\r", ch);
        return;
    }

    if (!IS_NPC(mount))
    {
        send_to_char("You can only mount NPCs.\n\r", ch);
        return;
    }

    if (!IS_SET(mount->act, ACT_MOUNT))
    {
        send_to_char("You can't mount that.\n\r", ch);
        return;
    }

    if (mount->mount != NULL)
    {
        send_to_char("That is already being ridden.\n\r", ch);
        return;
    }

    if (mount->position < POS_STANDING)
    {
        send_to_char("That is not standing.\n\r", ch);
        return;
    }

    if (mount->fighting != NULL)
    {
        send_to_char("That is fighting.\n\r", ch);
        return;
    }

    if (!mount_success(ch, mount, TRUE))
    {
        act("You try to mount $N but fail.", ch, NULL, mount, TO_CHAR);
        act("$n tries to mount you but fails.", ch, NULL, mount, TO_VICT);
        act("$n tries to mount $N but fails.", ch, NULL, mount, TO_NOTVICT);
        check_improve(ch, gsn_riding, FALSE, 4);
        return;
    }

    ch->mount = mount;
    mount->mount = ch;
    ch->riding = TRUE;
    mount->riding = TRUE;

    act("You mount $N.", ch, NULL, mount, TO_CHAR);
    act("$n mounts you.", ch, NULL, mount, TO_VICT);
    act("$n mounts $N.", ch, NULL, mount, TO_NOTVICT);

    check_improve(ch, gsn_riding, TRUE, 4);
}

/*
 * Dismount command - allows a character to dismount
 */
void do_dismount(CHAR_DATA *ch, char *argument)
{
    if (!MOUNTED(ch))
    {
        send_to_char("You are not mounted.\n\r", ch);
        return;
    }

    if (ch->fighting != NULL)
    {
        send_to_char("You can't dismount while fighting.\n\r", ch);
        return;
    }

    act("You dismount from $N.", ch, NULL, ch->mount, TO_CHAR);
    act("$n dismounts from you.", ch, NULL, ch->mount, TO_VICT);
    act("$n dismounts from $N.", ch, NULL, ch->mount, TO_NOTVICT);

    ch->mount->mount = NULL;
    ch->mount->riding = FALSE;
    ch->mount = NULL;
    ch->riding = FALSE;
}

/*
 * Buy mount command - allows purchasing mounts from mount shops
 */
void do_buy_mount(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mount;
    ROOM_INDEX_DATA *pRoomIndexNext;
    ROOM_INDEX_DATA *in_room;
    int cost, roll;

    smash_tilde(argument);

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Buy what mount?\n\r", ch);
        return;
    }

    /* Check if room vnum + 1 exists (where mounts are stored) */
    pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
    if (pRoomIndexNext == NULL)
    {
        bug("Do_buy_mount: bad mount shop at vnum %d.", ch->in_room->vnum);
        send_to_char("Sorry, you can't buy that here.\n\r", ch);
        return;
    }

    /* Look for mount in the next room */
    in_room = ch->in_room;
    ch->in_room = pRoomIndexNext;
    mount = get_char_room(ch, arg);
    ch->in_room = in_room;

    if (mount == NULL || !IS_SET(mount->act, ACT_MOUNT))
    {
        send_to_char("Sorry, you can't buy that here.\n\r", ch);
        return;
    }

    if (ch->mount != NULL)
    {
        send_to_char("You already own a mount.\n\r", ch);
        return;
    }

    cost = 10 * mount->level * mount->level;

    if ((ch->silver + 100 * ch->gold) < cost)
    {
        send_to_char("You can't afford it.\n\r", ch);
        return;
    }

    if (ch->level < mount->level)
    {
        send_to_char("You're not powerful enough to master this mount.\n\r", ch);
        return;
    }

    /* haggle */
    roll = number_percent();
    if (roll < get_skill(ch, gsn_haggle))
    {
        cost -= cost / 2 * roll / 100;
        sprintf(buf, "You haggle the price down to %d coins.\n\r", cost);
        send_to_char(buf, ch);
        check_improve(ch, gsn_haggle, TRUE, 4);
    }

    deduct_cost(ch, cost);

    /* Create a new mount instance (don't use the template) */
    mount = create_mobile(mount->pIndexData);
    SET_BIT(mount->act, ACT_MOUNT);

    argument = one_argument(argument, arg);
    if (arg[0] != '\0')
    {
        sprintf(buf, "%s %s", mount->name, arg);
        free_string(mount->name);
        mount->name = str_dup(buf);
    }

    sprintf(buf, "%sA neck tag says 'I belong to %s'.\n\r",
            mount->description, ch->name);
    free_string(mount->description);
    mount->description = str_dup(buf);

    char_to_room(mount, ch->in_room);
    ch->mount = mount;
    mount->mount = ch;
    ch->riding = TRUE;
    mount->riding = TRUE;

    send_to_char("Enjoy your mount.\n\r", ch);
    act("$n bought $N as a mount.", ch, NULL, mount, TO_ROOM);
}
