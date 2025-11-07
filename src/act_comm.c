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
 *           act_comm.c - November 3, 2025
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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/* Auction system function declarations */
void talk_auction args ((char *argument));
int parsebet args ((const int currentbet, const char *argument));
#include "recycle.h"
#include "tables.h"
#include <ctype.h> /* for isalpha() and isspace() -- JR */

static OBJ_DATA *get_commstone(CHAR_DATA *ch);
static bool commstone_can_hear(CHAR_DATA *listener, int channel, int freq);
static void commstone_transmit(CHAR_DATA *ch, int channel, char *argument);
static int commstone_find_slot(OBJ_DATA *stone, int freq);
static void commstone_build_prefix(char *buffer, size_t length, OBJ_DATA *stone, int freq, int fallback_channel, CHAR_DATA *listener);

/* RT code to delete yourself */

void do_delet (CHAR_DATA * ch, char *argument)
{
    send_to_char ("You must type the full command to delete yourself.\n\r",
                  ch);
}

void do_delete (CHAR_DATA * ch, char *argument)
{
    char strsave[MAX_INPUT_LENGTH];

    if (IS_NPC (ch))
        return;

    if (ch->pcdata->confirm_delete)
    {
        if (argument[0] != '\0')
        {
            send_to_char ("Delete status removed.\n\r", ch);
            ch->pcdata->confirm_delete = FALSE;
            return;
        }
        else
        {
            sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
            wiznet ("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
            stop_fighting (ch, TRUE);
            do_function (ch, &do_quit, "");
            unlink (strsave);
            return;
        }
    }

    if (argument[0] != '\0')
    {
        send_to_char ("Just type delete. No argument.\n\r", ch);
        return;
    }

    send_to_char ("Type delete again to confirm this command.\n\r", ch);
    send_to_char ("WARNING: this command is irreversible.\n\r", ch);
    send_to_char
        ("Typing delete with an argument will undo delete status.\n\r", ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet ("$N is contemplating deletion.", ch, NULL, 0, 0, get_trust (ch));
}


/* RT code to display channel status */

void do_channels (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    send_to_char ("   channel     status\n\r", ch);
    send_to_char ("---------------------\n\r", ch);

    send_to_char ("{dgossip{x         ", ch);
    if (!IS_SET (ch->comm, COMM_NOGOSSIP))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{aauction{x        ", ch);
    if (!IS_SET (ch->comm, COMM_NOAUCTION))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF\n\r", ch);

    send_to_char ("{emusic{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOMUSIC))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{qQ{x/{fA{x            ", ch);
    if (!IS_SET (ch->comm, COMM_NOQUESTION))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{hQuote{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOQUOTE))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{tgrats{x          ", ch);
    if (!IS_SET (ch->comm, COMM_NOGRATS))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    if (IS_IMMORTAL (ch))
    {
        send_to_char ("{igod channel{x    ", ch);
        if (!IS_SET (ch->comm, COMM_NOWIZ))
            send_to_char ("{GON{x\n\r", ch);
        else
            send_to_char ("{ROFF{x\n\r", ch);
    }

    send_to_char("{ghint{x           ",ch);
if (!IS_SET(ch->comm,COMM_NOHINT))
      send_to_char("{GON{x\n\r",ch);
    else
      send_to_char("{ROFF{x\n\r",ch);

    send_to_char ("{tshouts{x         ", ch);
    if (!IS_SET (ch->comm, COMM_SHOUTSOFF))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{ktells{x          ", ch);
    if (!IS_SET (ch->comm, COMM_DEAF))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    send_to_char ("{tquiet mode{x     ", ch);
    if (IS_SET (ch->comm, COMM_QUIET))
        send_to_char ("{GON{x\n\r", ch);
    else
        send_to_char ("{ROFF{x\n\r", ch);

    if (IS_SET (ch->comm, COMM_AFK))
        send_to_char ("You are AFK.\n\r", ch);

    if (IS_SET (ch->comm, COMM_SNOOP_PROOF))
        send_to_char ("You are immune to snooping.\n\r", ch);

    if (ch->lines != PAGELEN)
    {
        if (ch->lines)
        {
            sprintf (buf, "You display %d lines of scroll.\n\r",
                     ch->lines + 2);
            send_to_char (buf, ch);
        }
        else
            send_to_char ("Scroll buffering is off.\n\r", ch);
    }

    if (ch->prompt != NULL)
    {
        sprintf (buf, "Your current prompt is: %s\n\r", ch->prompt);
        send_to_char (buf, ch);
    }

    if (IS_SET (ch->comm, COMM_NOSHOUT))
        send_to_char ("You cannot shout.\n\r", ch);

    if (IS_SET (ch->comm, COMM_NOTELL))
        send_to_char ("You cannot use tell.\n\r", ch);

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
        send_to_char ("You cannot use channels.\n\r", ch);

    if (IS_SET (ch->comm, COMM_NOEMOTE))
        send_to_char ("You cannot show emotions.\n\r", ch);

}

static void commstone_show_status(CHAR_DATA *ch, OBJ_DATA *stone)
{
    int i;

    if (ch == NULL || stone == NULL)
        return;

    send_to_char("Commstone configuration:\n\r", ch);

    for (i = 0; i < COMMSTONE_MAX_CHANNELS; ++i)
    {
        int freq = URANGE(0, stone->value[i], COMMSTONE_MAX_FREQUENCY);
        if (stone->value[i] != freq)
            stone->value[i] = freq;

        printf_to_char(ch, "  Comm%d: %d\n\r", i + 1, freq);
    }

    printf_to_char(ch, "  Remaining charges: %d\n\r", stone->value[4]);
}

static OBJ_DATA *get_commstone(CHAR_DATA *ch)
{
    OBJ_DATA *stone;

    if (ch == NULL)
        return NULL;

    stone = get_eq_char(ch, WEAR_COMM);
    if (stone != NULL && stone->item_type == ITEM_COMM)
        return stone;

    return NULL;
}

static int commstone_find_slot(OBJ_DATA *stone, int freq)
{
    int i;

    if (stone == NULL)
        return -1;

    freq = URANGE(0, freq, COMMSTONE_MAX_FREQUENCY);

    for (i = 0; i < COMMSTONE_MAX_CHANNELS; ++i)
    {
        if (stone->value[i] == freq)
            return i;
    }

    return -1;
}

static void commstone_build_prefix(char *buffer, size_t length, OBJ_DATA *stone, int freq, int fallback_channel, CHAR_DATA *listener)
{
    int slot = commstone_find_slot(stone, freq);
    char color_code[64] = "";
    int channel_num;

    if (slot >= 0)
        channel_num = slot + 1;
    else
    {
        if (fallback_channel < 0)
            fallback_channel = 0;
        channel_num = fallback_channel + 1;
    }

    /* Add color codes based on listener's preferences */
    if (listener && !IS_NPC(listener) && IS_SET(listener->act, PLR_COLOUR))
    {
        int *color_pref;
        
        switch (channel_num)
        {
            case 1: color_pref = listener->pcdata->comm1; break;
            case 2: color_pref = listener->pcdata->comm2; break;
            case 3: color_pref = listener->pcdata->comm3; break;
            case 4: color_pref = listener->pcdata->comm4; break;
            default: color_pref = NULL; break;
        }
        
        if (color_pref != NULL)
        {
            /* Build ANSI color code: \e[brightness;3color_numm */
            if (color_pref[2])  /* Beep enabled */
                sprintf(color_code, "\e[%d;3%dm\a", color_pref[0], color_pref[1]);
            else
                sprintf(color_code, "\e[%d;3%dm", color_pref[0], color_pref[1]);
        }
    }

    snprintf(buffer, length, "%s[Freq %d on Comm%d]\e[0m", color_code, freq, channel_num);
}

static bool commstone_can_hear(CHAR_DATA *listener, int channel, int freq)
{
    OBJ_DATA *stone;

    if (listener == NULL)
        return FALSE;

    if (!IS_NPC(listener))
    {
        if (IS_SET(listener->comm, COMM_QUIET))
            return FALSE;
        if (IS_SET(listener->comm, COMM_NOGOSSIP))
            return FALSE;
        if (IS_SET(listener->comm, COMM_NOCHANNELS))
            return FALSE;
    }

    stone = get_commstone(listener);
    if (stone == NULL)
        return FALSE;

    freq = URANGE(0, freq, COMMSTONE_MAX_FREQUENCY);

    return commstone_find_slot(stone, freq) >= 0;
}

static void commstone_spend_charge(CHAR_DATA *ch, OBJ_DATA *stone)
{
    if (stone == NULL)
        return;

    if (stone->value[4] > 0)
    {
        stone->value[4]--;
        if (stone->value[4] <= 0)
        {
            stone->value[4] = 0;
            if (ch != NULL)
                send_to_char("Your commstone crackles and falls silent; it can no longer transmit.\n\r", ch);
        }
    }
}

static void commstone_transmit(CHAR_DATA *ch, int channel, char *argument)
{
    OBJ_DATA *stone;
    DESCRIPTOR_DATA *d;
    char original_argument[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int freq;
    bool transmitted = FALSE;
    bool is_social = FALSE;
    int social_index = -1;
    char sender_prefix[64];
    char log_buf[MAX_STRING_LENGTH];

    if (ch == NULL)
        return;

    if (channel < 0 || channel >= COMMSTONE_MAX_CHANNELS)
        return;

    sprintf(log_buf, "COMMSTONE DEBUG: do_comm%d called, channel parameter=%d", channel + 1, channel);
    log_string(log_buf);

    stone = get_commstone(ch);

    if (stone == NULL)
    {
        send_to_char("You need to be wearing a commstone to do that.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        commstone_show_status(ch, stone);
        return;
    }

    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_QUIET))
    {
        send_to_char("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
    {
        send_to_char("The gods have revoked your channel privileges.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && stone->value[4] <= 0)
    {
        send_to_char("Your commstone has no charges left to transmit.\n\r", ch);
        return;
    }

    freq = URANGE(0, stone->value[channel], COMMSTONE_MAX_FREQUENCY);
    stone->value[channel] = freq;

    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOGOSSIP))
        REMOVE_BIT(ch->comm, COMM_NOGOSSIP);

    commstone_build_prefix(sender_prefix, sizeof(sender_prefix), stone, freq,
                           channel, ch);

    strncpy(original_argument, argument, sizeof(original_argument) - 1);
    original_argument[sizeof(original_argument) - 1] = '\0';

    argument = one_argument(argument, command);

    /* Check for emote */
    bool is_emote = FALSE;
    if (!str_prefix(command, "emote"))
    {
        is_emote = TRUE;
        /* Skip past "emote" to get the emote text */
        while (*argument == ' ')
            ++argument;
    }
    else if (command[0] != '\0')
    {
        int idx;
        for (idx = 0; social_table[idx].name[0] != '\0'; idx++)
        {
            if (command[0] == social_table[idx].name[0]
                && !str_prefix(command, social_table[idx].name))
            {
                is_social = TRUE;
                social_index = idx;
                break;
            }
        }
    }

    if (is_emote)
    {
        char message[MAX_STRING_LENGTH];
        char buf[MAX_STRING_LENGTH];

        if (argument[0] == '\0')
        {
            send_to_char("Emote what?\n\r", ch);
            return;
        }

        strncpy(message, argument, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';

        if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
            makedrunk(message, ch);

        sprintf(buf, "%s %s %s", sender_prefix, ch->name, message);
        send_to_char(buf, ch);
        send_to_char("\n\r", ch);
        transmitted = TRUE;

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *vch = d->original ? d->original : d->character;
            OBJ_DATA *listener_stone;
            char listener_prefix[64];

            if (d->connected != CON_PLAYING || vch == ch)
                continue;

            if (!commstone_can_hear(vch, channel, freq))
                continue;

            listener_stone = get_commstone(vch);
            commstone_build_prefix(listener_prefix, sizeof(listener_prefix),
                                   listener_stone, freq, channel, vch);

            sprintf(buf, "%s %s %s\n\r", listener_prefix, ch->name, message);
            send_to_char(buf, vch);
        }

        if (transmitted)
            commstone_spend_charge(ch, stone);

        return;
    }
    else if (is_social && social_index >= 0)
    {
        CHAR_DATA *victim = NULL;
        char buf[MAX_STRING_LENGTH];

        while (*argument == ' ')
            ++argument;
        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
            sprintf(buf, "%s %s", sender_prefix,
                    social_table[social_index].char_no_arg);
            act_new(buf, ch, NULL, NULL, TO_CHAR, POS_DEAD);
            transmitted = TRUE;

            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch = d->original ? d->original : d->character;
                OBJ_DATA *listener_stone;
                char listener_prefix[64];

                if (d->connected != CON_PLAYING || vch == ch)
                    continue;

                if (!commstone_can_hear(vch, channel, freq))
                    continue;

                listener_stone = get_commstone(vch);
                commstone_build_prefix(listener_prefix, sizeof(listener_prefix),
                                       listener_stone, freq, channel, vch);

                sprintf(buf, "%s %s", listener_prefix,
                        social_table[social_index].others_no_arg);
                act_new(buf, ch, NULL, vch, TO_VICT, POS_DEAD);
            }
        }
        else if ((victim = get_char_world(ch, arg)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        else if (victim == ch)
        {
            sprintf(buf, "%s %s", sender_prefix,
                    social_table[social_index].char_auto);
            act_new(buf, ch, NULL, NULL, TO_CHAR, POS_DEAD);
            transmitted = TRUE;

            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch = d->original ? d->original : d->character;
                OBJ_DATA *listener_stone;
                char listener_prefix[64];

                if (d->connected != CON_PLAYING || vch == ch)
                    continue;

                if (!commstone_can_hear(vch, channel, freq))
                    continue;

                listener_stone = get_commstone(vch);
                commstone_build_prefix(listener_prefix, sizeof(listener_prefix),
                                       listener_stone, freq, channel, vch);

                sprintf(buf, "%s %s", listener_prefix,
                        social_table[social_index].others_auto);
                act_new(buf, ch, NULL, vch, TO_VICT, POS_DEAD);
            }
        }
        else
        {
            OBJ_DATA *listener_stone;
            char listener_prefix[64];

            sprintf(buf, "%s %s", sender_prefix,
                    social_table[social_index].char_found);
            act_new(buf, ch, NULL, victim, TO_CHAR, POS_DEAD);
            transmitted = TRUE;

            if (commstone_can_hear(victim, channel, freq))
            {
                listener_stone = get_commstone(victim);
                commstone_build_prefix(listener_prefix, sizeof(listener_prefix),
                                       listener_stone, freq, channel, victim);
                sprintf(buf, "%s %s", listener_prefix,
                        social_table[social_index].vict_found);
                act_new(buf, ch, NULL, victim, TO_VICT, POS_DEAD);
            }
            else
            {
                sprintf(buf, "%s %s", sender_prefix,
                        social_table[social_index].vict_found);
                act_new(buf, ch, NULL, victim, TO_VICT, POS_DEAD);
            }

            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch = d->original ? d->original : d->character;
                OBJ_DATA *stone_listener;
                char recv_prefix[64];

                if (d->connected != CON_PLAYING || vch == ch || vch == victim)
                    continue;

                if (!commstone_can_hear(vch, channel, freq))
                    continue;

                stone_listener = get_commstone(vch);
                commstone_build_prefix(recv_prefix, sizeof(recv_prefix),
                                       stone_listener, freq, channel, vch);

                sprintf(buf, "%s %s", recv_prefix,
                        social_table[social_index].others_found);
                act_new(buf, ch, NULL, vch, TO_VICT, POS_DEAD);
            }
        }

        if (transmitted)
            commstone_spend_charge(ch, stone);

        return;
    }

    {
        char message[MAX_STRING_LENGTH];

        strncpy(message, original_argument, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';

        if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
            makedrunk(message, ch);

        printf_to_char(ch, "%s You transmit: '%s'\n\r", sender_prefix,
                       message);
        transmitted = TRUE;

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *vch = d->original ? d->original : d->character;
            OBJ_DATA *listener_stone;
            char listener_prefix[64];

            if (d->connected != CON_PLAYING || vch == ch)
                continue;

            if (!commstone_can_hear(vch, channel, freq))
                continue;

            listener_stone = get_commstone(vch);
            commstone_build_prefix(listener_prefix, sizeof(listener_prefix),
                                   listener_stone, freq, channel, vch);

            if (d->character == NULL)
                continue;

            printf_to_char(vch, "%s %s transmits: '%s'\n\r",
                           listener_prefix, PERS(ch, vch), message);
        }
    }

    if (transmitted)
        commstone_spend_charge(ch, stone);
}

void do_tune (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *stone;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int channel;
    int freq;
    int skill;

    if (ch == NULL)
        return;

    stone = get_commstone(ch);

    if (stone == NULL)
    {
        send_to_char("You need to be wearing a commstone to tune it.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        commstone_show_status(ch, stone);
        return;
    }

    if (!is_number(arg1) || !is_number(arg2))
    {
        send_to_char("Syntax: tune <1-4> <0-99999>\n\r", ch);
        return;
    }

    channel = atoi(arg1);
    if (channel < 1 || channel > COMMSTONE_MAX_CHANNELS)
    {
        send_to_char("Channel must be between 1 and 4.\n\r", ch);
        return;
    }

    freq = atoi(arg2);
    if (freq < 0 || freq > COMMSTONE_MAX_FREQUENCY)
    {
        send_to_char("Frequency must be between 0 and 99999.\n\r", ch);
        return;
    }

    skill = get_skill(ch, gsn_tune);

    if (!IS_NPC(ch) && skill <= 0)
    {
        send_to_char("You have no idea how to tune a commstone yet.\n\r", ch);
        return;
    }

    if (!IS_NPC(ch) && skill <= 20)
    {
        send_to_char("You lack the finesse to tune a commstone reliably. Practice more.\n\r", ch);
        return;
    }

    stone->value[channel - 1] = URANGE(0, freq, COMMSTONE_MAX_FREQUENCY);
    printf_to_char(ch, "You adjust Comm%d to frequency %d.\n\r", channel, stone->value[channel - 1]);
}

void do_comm1 (CHAR_DATA *ch, char *argument)
{
    commstone_transmit(ch, 0, argument);
}

void do_comm2 (CHAR_DATA *ch, char *argument)
{
    commstone_transmit(ch, 1, argument);
}

void do_comm3 (CHAR_DATA *ch, char *argument)
{
    commstone_transmit(ch, 2, argument);
}

void do_comm4 (CHAR_DATA *ch, char *argument)
{
    commstone_transmit(ch, 3, argument);
}

/* RT deaf blocks out all shouts */

void do_deaf (CHAR_DATA * ch, char *argument)
{

    if (IS_SET (ch->comm, COMM_DEAF))
    {
        send_to_char ("You can now hear tells again.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_DEAF);
    }
    else
    {
        send_to_char ("From now on, you won't hear tells.\n\r", ch);
        SET_BIT (ch->comm, COMM_DEAF);
    }
}

/* RT quiet blocks out all communication */

void do_quiet (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("Quiet mode removed.\n\r", ch);
        REMOVE_BIT (ch->comm, COMM_QUIET);
    }
    else
    {
        send_to_char ("From now on, you will only hear says and emotes.\n\r",
                      ch);
        SET_BIT (ch->comm, COMM_QUIET);
    }
}

/* afk command */

void do_afk (CHAR_DATA * ch, char *argument)
{
    if (IS_SET (ch->comm, COMM_AFK))
    {
        printf_to_char(ch,"AFK mode removed%s\n\r", buf_string(ch->pcdata->buffer)[0] == '\0' ? " (you didn't miss any 'tells')." : ". You have some 'tells' stored (type 'replay' to see them).");

        REMOVE_BIT (ch->comm, COMM_AFK);
    }
    else
    {
        send_to_char ("You are now in AFK mode.\n\r", ch);
        SET_BIT (ch->comm, COMM_AFK);
    }
}

void do_replay (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
    {
        send_to_char ("You can't replay.\n\r", ch);
        return;
    }

    if (buf_string (ch->pcdata->buffer)[0] == '\0')
    {
        send_to_char ("You have no tells to replay.\n\r", ch);
        return;
    }

    page_to_char (buf_string (ch->pcdata->buffer), ch);
    clear_buf (ch->pcdata->buffer);
}

/* Automated auction system by Erwin S. Andreasen */
void do_auction (CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

    if (IS_NPC(ch))
        return;

    if (arg1[0] == '\0')
    {
        if (auction->item != NULL)
        {
            if (auction->bet > 0)
                sprintf (buf, "Current bet on this item is %d gold.\n\r",auction->bet);
            else
                sprintf (buf, "No bets on this item have been received.\n\r");
            send_to_char (buf,ch);
            send_to_char (auction->item->description, ch);
            send_to_char ("\n\r", ch);
            return;
        }
        else
        {
            send_to_char ("Auction WHAT?\n\r",ch);
            return;
        }
    }

    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    {
        if (auction->item == NULL)
        {
            send_to_char ("There is no auction going on you can stop.\n\r",ch);
            return;
        }
        else
        {
            sprintf (buf,"Sale of %s has been stopped by God. Item confiscated.",
                            auction->item->short_descr);
            talk_auction (buf);
            obj_to_char (auction->item, ch);
            auction->item = NULL;
            if (auction->buyer != NULL)
            {
                auction->buyer->gold += auction->bet;
                send_to_char ("Your money has been returned.\n\r",auction->buyer);
            }
            return;
        }
    }

    if (!str_cmp(arg1,"bid") )
    {
        if (auction->item != NULL)
        {
            int newbet;

            if (argument[0] == '\0')
            {
                send_to_char ("Bid how much?\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);

            if (newbet < (auction->bet + 100))
            {
                send_to_char ("You must at least bid 100 coins over the current bet.\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("You don't have that much money!\n\r",ch);
                return;
            }

            if (auction->buyer != NULL)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet;
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION;

            sprintf (buf,"A bid of %d gold has been received on %s.\n\r",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;
        }
        else
        {
            send_to_char ("There isn't anything being auctioned right now.\n\r",ch);
            return;
        }
    }

    obj = get_obj_list (ch, arg1, ch->carrying);

    if (obj == NULL)
    {
        send_to_char ("You aren't carrying that.\n\r",ch);
        return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {
    default:
        send_to_char ("You cannot auction that type of item.\n\r", ch);
        return;

    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_SCROLL:
    case ITEM_MANUAL:
        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0;
        auction->buyer = NULL;
        auction->seller = ch;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf (buf, "A new item has been received: %s.", obj->short_descr);
        talk_auction (buf);
        return;
    }
    else
    {
        act ("Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
        return;
    }
}

/* RT chat replaced with ROM gossip */
void do_gossip (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOGOSSIP))
        {
            send_to_char ("Gossip channel is now ON.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOGOSSIP);
        }
        else
        {
            send_to_char ("Gossip channel is now OFF.\n\r", ch);
            SET_BIT (ch->comm, COMM_NOGOSSIP);
        }
        return;
    }

    send_to_char("The commstone network has replaced the old gossip channel. Use 1-4 to transmit through your tuned commstone.\n\r", ch);
    return;

    /* Check if first argument is a social command */
    char original_argument[MAX_INPUT_LENGTH];
    strcpy(original_argument, argument);
    argument = one_argument(argument, command);
    
    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
        && !str_prefix( command,social_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        /* Handle social command on gossip channel */
        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_QUIET))
        {
            send_to_char("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOGOSSIP))
        {
            send_to_char("But you have the gossip channel turned off!\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
        {
            send_to_char("The gods have revoked your channel priviliges.\n\r", ch);
            return;
        }

        switch (ch->position)
        {
        case POS_DEAD:
            send_to_char("Lie still; you are DEAD!\n\r", ch);
            return;
        case POS_INCAP:
        case POS_MORTAL:
            send_to_char("You are hurt far too bad for that.\n\r", ch);
            return;
        case POS_STUNNED:
            send_to_char("You are too stunned for that.\n\r", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOGOSSIP);

        one_argument(argument, arg);
        victim = NULL;
        if (arg[0] == '\0')
        {
            sprintf(buf, "{mGossip: {M%s{x", social_table[cmd].char_no_arg );
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf, "{mGossip: {M%s{x", social_table[cmd].others_no_arg );
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOGOSSIP) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else if ((victim = get_char_world(ch,arg)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        else if (victim == ch)
        {
            sprintf(buf,"{mGossip: {M%s{x", social_table[cmd].char_auto);
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf,"{mGossip: {M%s{x", social_table[cmd].others_auto);
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOGOSSIP) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else
        {
            sprintf(buf,"{mGossip: {M%s{x", social_table[cmd].char_found);
            act_new(buf,ch,NULL,victim,TO_CHAR,POS_DEAD);
            sprintf(buf,"{mGossip: {M%s{x", social_table[cmd].vict_found);
            act_new(buf,ch,NULL,victim,TO_VICT,POS_DEAD);
            
            sprintf(buf,"{mGossip: {M%s{x", social_table[cmd].others_found);
            for (counter = 0; buf[counter+1] != '\0'; counter++)
            {
                if (buf[counter] == '$' && buf[counter + 1] == 'N')
                {
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,victim->name);
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'E')
                {
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count ++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'M')
                {
                    buf[counter] = '%';
                    buf[counter + 1] = 's';
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"him");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"her");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'S')
                {
                    switch (victim->sex)
                    {
                    default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"its");
                    for (count = 0;buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"his");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"hers");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
            }
            for (d=descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    d->character != victim &&
                    !IS_SET(vch->comm, COMM_NOGOSSIP) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        return;
    }

    /* Regular gossip message */
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
        send_to_char
            ("The gods have revoked your channel priviliges.\n\r", ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOGOSSIP);

    /* Make the words drunk if needed */
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        makedrunk(original_argument,ch);

    sprintf (buf, "{dYou gossip '{9%s{d'{x\n\r", original_argument);
    send_to_char (buf, ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_NOGOSSIP) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act_new ("{d$n gossips '{9$t{d'{x",
                     ch, original_argument, d->character, TO_VICT, POS_SLEEPING);
        }
    }
}

void do_grats (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOGRATS))
        {
            send_to_char ("Grats channel is now ON.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOGRATS);
        }
        else
        {
            send_to_char ("Grats channel is now OFF.\n\r", ch);
            SET_BIT (ch->comm, COMM_NOGRATS);
        }
        return;
    }

    /* Check if first argument is a social command */
    char original_argument[MAX_INPUT_LENGTH];
    strcpy(original_argument, argument);
    argument = one_argument(argument, command);
    
    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
        && !str_prefix( command,social_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        /* Handle social command on grats channel */
        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_QUIET))
        {
            send_to_char("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOGRATS))
        {
            send_to_char("But you have the grats channel turned off!\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
        {
            send_to_char("The gods have revoked your channel priviliges.\n\r", ch);
            return;
        }

        switch (ch->position)
        {
        case POS_DEAD:
            send_to_char("Lie still; you are DEAD!\n\r", ch);
            return;
        case POS_INCAP:
        case POS_MORTAL:
            send_to_char("You are hurt far too bad for that.\n\r", ch);
            return;
        case POS_STUNNED:
            send_to_char("You are too stunned for that.\n\r", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOGRATS);

        one_argument(argument, arg);
        victim = NULL;
        if (arg[0] == '\0')
        {
            sprintf(buf, "Grats: %s", social_table[cmd].char_no_arg );
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf, "Grats: %s", social_table[cmd].others_no_arg );
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOGRATS) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else if ((victim = get_char_world(ch,arg)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        else if (victim == ch)
        {
            sprintf(buf,"Grats: %s", social_table[cmd].char_auto);
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf,"Grats: %s", social_table[cmd].others_auto);
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOGRATS) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else
        {
            sprintf(buf,"Grats: %s", social_table[cmd].char_found);
            act_new(buf,ch,NULL,victim,TO_CHAR,POS_DEAD);
            sprintf(buf,"Grats: %s", social_table[cmd].vict_found);
            act_new(buf,ch,NULL,victim,TO_VICT,POS_DEAD);
            
            sprintf(buf,"Grats: %s", social_table[cmd].others_found);
            for (counter = 0; buf[counter+1] != '\0'; counter++)
            {
                if (buf[counter] == '$' && buf[counter + 1] == 'N')
                {
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,victim->name);
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'E')
                {
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count ++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'M')
                {
                    buf[counter] = '%';
                    buf[counter + 1] = 's';
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"him");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"her");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'S')
                {
                    switch (victim->sex)
                    {
                    default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"its");
                    for (count = 0;buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"his");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"hers");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
            }
            for (d=descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    d->character != victim &&
                    !IS_SET(vch->comm, COMM_NOGRATS) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        return;
    }

    /* Regular grats message */
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
        send_to_char
            ("The gods have revoked your channel priviliges.\n\r", ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOGRATS);

    /* Make the words drunk if needed */
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        makedrunk(original_argument,ch);

    sprintf (buf, "{tYou grats '%s'{x\n\r", original_argument);
    send_to_char (buf, ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_NOGRATS) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act_new ("{t$n grats '$t'{x",
                     ch, original_argument, d->character, TO_VICT, POS_SLEEPING);
        }
    }
}

void do_quote (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUOTE))
        {
            send_to_char ("{hQuote channel is now ON.{x\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUOTE);
        }
        else
        {
            send_to_char ("{hQuote channel is now OFF.{x\n\r", ch);
            SET_BIT (ch->comm, COMM_NOQUOTE);
        }
    }
    else
    {                            /* quote message sent, turn quote on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            send_to_char ("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            send_to_char
                ("The gods have revoked your channel priviliges.\n\r", ch);
            return;

        }

        REMOVE_BIT (ch->comm, COMM_NOQUOTE);

        sprintf (buf, "{hYou quote '{H%s{h'{x\n\r", argument);
        send_to_char (buf, ch);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOQUOTE) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("{h$n quotes '{H$t{h'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}

/* RT question channel */
void do_question (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUESTION))
        {
            send_to_char ("Q/A channel is now ON.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUESTION);
        }
        else
        {
            send_to_char ("Q/A channel is now OFF.\n\r", ch);
            SET_BIT (ch->comm, COMM_NOQUESTION);
        }
        return;
    }

    /* Check if first argument is a social command */
    char original_argument[MAX_INPUT_LENGTH];
    strcpy(original_argument, argument);
    argument = one_argument(argument, command);
    
    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
        && !str_prefix( command,social_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        /* Handle social command on question channel */
        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_QUIET))
        {
            send_to_char("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOQUESTION))
        {
            send_to_char("But you have the question channel turned off!\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
        {
            send_to_char("The gods have revoked your channel priviliges.\n\r", ch);
            return;
        }

        switch (ch->position)
        {
        case POS_DEAD:
            send_to_char("Lie still; you are DEAD!\n\r", ch);
            return;
        case POS_INCAP:
        case POS_MORTAL:
            send_to_char("You are hurt far too bad for that.\n\r", ch);
            return;
        case POS_STUNNED:
            send_to_char("You are too stunned for that.\n\r", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOQUESTION);

        one_argument(argument, arg);
        victim = NULL;
        if (arg[0] == '\0')
        {
            sprintf(buf, "{YQuestion: {W%s{x", social_table[cmd].char_no_arg );
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf, "{YQuestion: {W%s{x", social_table[cmd].others_no_arg );
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else if ((victim = get_char_world(ch,arg)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        else if (victim == ch)
        {
            sprintf(buf,"{YQuestion: {W%s{x", social_table[cmd].char_auto);
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf,"{YQuestion: {W%s{x", social_table[cmd].others_auto);
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else
        {
            sprintf(buf,"{YQuestion: {W%s{x", social_table[cmd].char_found);
            act_new(buf,ch,NULL,victim,TO_CHAR,POS_DEAD);
            sprintf(buf,"{YQuestion: {W%s{x", social_table[cmd].vict_found);
            act_new(buf,ch,NULL,victim,TO_VICT,POS_DEAD);
            
            sprintf(buf,"{YQuestion: {W%s{x", social_table[cmd].others_found);
            for (counter = 0; buf[counter+1] != '\0'; counter++)
            {
                if (buf[counter] == '$' && buf[counter + 1] == 'N')
                {
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,victim->name);
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'E')
                {
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count ++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'M')
                {
                    buf[counter] = '%';
                    buf[counter + 1] = 's';
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"him");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"her");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'S')
                {
                    switch (victim->sex)
                    {
                    default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"its");
                    for (count = 0;buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"his");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"hers");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
            }
            for (d=descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    d->character != victim &&
                    !IS_SET(vch->comm, COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        return;
    }

    /* Regular question message */
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
        send_to_char
            ("The gods have revoked your channel priviliges.\n\r", ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOQUESTION);

    /* Make the words drunk if needed */
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        makedrunk(original_argument,ch);

    sprintf (buf, "{qYou question '{Q%s{q'{x\n\r", original_argument);
    send_to_char (buf, ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_NOQUESTION) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act_new ("{q$n questions '{Q$t{q'{x",
                     ch, original_argument, d->character, TO_VICT, POS_SLEEPING);
        }
    }
}

/* RT answer channel - uses same line as questions */
void do_answer (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOQUESTION))
        {
            send_to_char ("Q/A channel is now ON.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOQUESTION);
        }
        else
        {
            send_to_char ("Q/A channel is now OFF.\n\r", ch);
            SET_BIT (ch->comm, COMM_NOQUESTION);
        }
        return;
    }

    /* Check if first argument is a social command */
    char original_argument[MAX_INPUT_LENGTH];
    strcpy(original_argument, argument);
    argument = one_argument(argument, command);
    
    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
        && !str_prefix( command,social_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        /* Handle social command on answer channel */
        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_QUIET))
        {
            send_to_char("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOQUESTION))
        {
            send_to_char("But you have the answer channel turned off!\n\r", ch);
            return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
        {
            send_to_char("The gods have revoked your channel priviliges.\n\r", ch);
            return;
        }

        switch (ch->position)
        {
        case POS_DEAD:
            send_to_char("Lie still; you are DEAD!\n\r", ch);
            return;
        case POS_INCAP:
        case POS_MORTAL:
            send_to_char("You are hurt far too bad for that.\n\r", ch);
            return;
        case POS_STUNNED:
            send_to_char("You are too stunned for that.\n\r", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOQUESTION);

        one_argument(argument, arg);
        victim = NULL;
        if (arg[0] == '\0')
        {
            sprintf(buf, "{YAnswer: {W%s{x", social_table[cmd].char_no_arg );
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf, "{YAnswer: {W%s{x", social_table[cmd].others_no_arg );
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else if ((victim = get_char_world(ch,arg)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        else if (victim == ch)
        {
            sprintf(buf,"{YAnswer: {W%s{x", social_table[cmd].char_auto);
            act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
            sprintf(buf,"{YAnswer: {W%s{x", social_table[cmd].others_auto);
            for (d = descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    !IS_SET(vch->comm,COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        else
        {
            sprintf(buf,"{YAnswer: {W%s{x", social_table[cmd].char_found);
            act_new(buf,ch,NULL,victim,TO_CHAR,POS_DEAD);
            sprintf(buf,"{YAnswer: {W%s{x", social_table[cmd].vict_found);
            act_new(buf,ch,NULL,victim,TO_VICT,POS_DEAD);
            
            sprintf(buf,"{YAnswer: {W%s{x", social_table[cmd].others_found);
            for (counter = 0; buf[counter+1] != '\0'; counter++)
            {
                if (buf[counter] == '$' && buf[counter + 1] == 'N')
                {
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,victim->name);
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'E')
                {
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count ++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'M')
                {
                    buf[counter] = '%';
                    buf[counter + 1] = 's';
                    switch (victim->sex)
                    {
                    default:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"it");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"him");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"her");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
                else if (buf[counter] == '$' && buf[counter + 1] == 'S')
                {
                    switch (victim->sex)
                    {
                    default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"its");
                    for (count = 0;buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                    case 1:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"his");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    case 2:
                        strcpy(buf2,buf);
                        buf2[counter] = '\0';
                        strcat(buf2,"hers");
                        for (count = 0; buf[count] != '\0'; count++)
                        {
                            buf[count] = buf[count+counter+2];
                        }
                        strcat(buf2,buf);
                        strcpy(buf,buf2);
                        break;
                    }
                }
            }
            for (d=descriptor_list; d != NULL; d = d->next)
            {
                CHAR_DATA *vch;
                vch = d->original ? d->original : d->character;
                if (d->connected == CON_PLAYING &&
                    d->character != ch &&
                    d->character != victim &&
                    !IS_SET(vch->comm, COMM_NOQUESTION) &&
                    !IS_SET(vch->comm,COMM_QUIET))
                {
                    act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
                }
            }
        }
        return;
    }

    /* Regular answer message */
    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOCHANNELS))
    {
        send_to_char
            ("The gods have revoked your channel priviliges.\n\r", ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOQUESTION);

    /* Make the words drunk if needed */
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
        makedrunk(original_argument,ch);

    sprintf (buf, "{fYou answer '{F%s{f'{x\n\r", original_argument);
    send_to_char (buf, ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_NOQUESTION) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act_new ("{f$n answers '{F$t{f'{x",
                     ch, original_argument, d->character, TO_VICT, POS_SLEEPING);
        }
    }
}

/* RT music channel */
void do_music (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOMUSIC))
        {
            send_to_char ("Music channel is now ON.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOMUSIC);
        }
        else
        {
            send_to_char ("Music channel is now OFF.\n\r", ch);
            SET_BIT (ch->comm, COMM_NOMUSIC);
        }
    }
    else
    {                            /* music sent, turn music on if it isn't already */

        if (IS_SET (ch->comm, COMM_QUIET))
        {
            send_to_char ("You must turn off quiet mode first.\n\r", ch);
            return;
        }

        if (IS_SET (ch->comm, COMM_NOCHANNELS))
        {
            send_to_char
                ("The gods have revoked your channel priviliges.\n\r", ch);
            return;
        }

        REMOVE_BIT (ch->comm, COMM_NOMUSIC);

        sprintf (buf, "{eYou MUSIC: '{E%s{e'{x\n\r", argument);
        send_to_char (buf, ch);
        sprintf (buf, "$n MUSIC: '%s'", argument);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *victim;

            victim = d->original ? d->original : d->character;

            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET (victim->comm, COMM_NOMUSIC) &&
                !IS_SET (victim->comm, COMM_QUIET))
            {
                act_new ("{e$n MUSIC: '{E$t{e'{x",
                         ch, argument, d->character, TO_VICT, POS_SLEEPING);
            }
        }
    }
}


void do_immtalk (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_NOWIZ))
        {
            send_to_char ("Immortal channel is now ON\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_NOWIZ);
        }
        else
        {
            send_to_char ("Immortal channel is now OFF\n\r", ch);
            SET_BIT (ch->comm, COMM_NOWIZ);
        }
        return;
    }

    REMOVE_BIT (ch->comm, COMM_NOWIZ);

    act_new ("{i[{I$n{i]: $t{x", ch, argument, NULL, TO_CHAR, POS_DEAD);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING &&
            IS_IMMORTAL (d->character) &&
            !IS_SET (d->character->comm, COMM_NOWIZ))
        {
            act_new ("{i[{I$n{i]: $t{x", ch, argument, d->character, TO_VICT,
                     POS_DEAD);
        }
    }

    return;
}

void do_hint( CHAR_DATA *ch, char *argument)
{
if(IS_NPC(ch))
  return;

if(IS_SET(ch->comm,COMM_NOHINT))
  {
  REMOVE_BIT(ch->comm, COMM_NOHINT);
  send_to_char("Hint channel is now ON.\n\r",ch);
  return;
  }
else
  {
  SET_BIT(ch->comm, COMM_NOHINT);
  send_to_char("Hint channel is now OFF.\n\r",ch);
  return;
  }
}

void do_say (CHAR_DATA * ch, char *argument)
{
    if (argument[0] == '\0')
    {
        send_to_char ("Say what?\n\r", ch);
        return;
    }

    act ("{6$n says '{7$T{6'{x", ch, NULL, argument, TO_ROOM);
    act ("{6You say '{7$T{6'{x", ch, NULL, argument, TO_CHAR);

    if (!IS_NPC (ch))
    {
        CHAR_DATA *mob, *mob_next;
        for (mob = ch->in_room->people; mob != NULL; mob = mob_next)
        {
            mob_next = mob->next_in_room;
            if (IS_NPC (mob) && HAS_TRIGGER (mob, TRIG_SPEECH)
                && mob->position == mob->pIndexData->default_pos)
                mp_act_trigger (argument, mob, ch, NULL, NULL, TRIG_SPEECH);
        }
    }
    return;
}



void do_shout (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
        if (IS_SET (ch->comm, COMM_SHOUTSOFF))
        {
            send_to_char ("You can hear shouts again.\n\r", ch);
            REMOVE_BIT (ch->comm, COMM_SHOUTSOFF);
        }
        else
        {
            send_to_char ("You will no longer hear shouts.\n\r", ch);
            SET_BIT (ch->comm, COMM_SHOUTSOFF);
        }
        return;
    }

    if (IS_SET (ch->comm, COMM_NOSHOUT))
    {
        send_to_char ("You can't shout.\n\r", ch);
        return;
    }

    REMOVE_BIT (ch->comm, COMM_SHOUTSOFF);

    WAIT_STATE (ch, 12);

    act ("You shout '$T'", ch, NULL, argument, TO_CHAR);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if (d->connected == CON_PLAYING &&
            d->character != ch &&
            !IS_SET (victim->comm, COMM_SHOUTSOFF) &&
            !IS_SET (victim->comm, COMM_QUIET))
        {
            act ("$n shouts '$t'", ch, argument, d->character, TO_VICT);
        }
    }

    return;
}



void do_tell (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_SET (ch->comm, COMM_NOTELL) || IS_SET (ch->comm, COMM_DEAF))
    {
        send_to_char ("Your message didn't get through.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_QUIET))
    {
        send_to_char ("You must turn off quiet mode first.\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_DEAF))
    {
        send_to_char ("You must turn off deaf mode first.\n\r", ch);
        return;
    }

    argument = one_argument (argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        send_to_char ("Tell whom what?\n\r", ch);
        return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ((victim = get_char_world (ch, arg)) == NULL
        || (IS_NPC (victim) && victim->in_room != ch->in_room))
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim->desc == NULL && !IS_NPC (victim))
    {
        act ("$N seems to have misplaced $S link...try again later.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    if (!(IS_IMMORTAL (ch) && ch->level > LEVEL_IMMORTAL)
        && !IS_AWAKE (victim))
    {
        act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (
        (IS_SET (victim->comm, COMM_QUIET)
         || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch))
    {
        act ("$E is not receiving tells.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_SET (victim->comm, COMM_AFK))
    {
        if (IS_NPC (victim))
        {
            act ("$E is AFK, and not receiving tells.", ch, NULL, victim,
                 TO_CHAR);
            return;
        }

        act ("$E is AFK, but your tell will go through when $E returns.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

	if (victim->desc->connected >= CON_NOTE_TO && victim->desc->connected <= CON_NOTE_FINISH)
	{
		act ("$E is writing a note, but your tell will go through when $E returns.",
				ch, NULL, victim, TO_CHAR);
		sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS (ch, victim), argument);
		buf[0] = UPPER (buf[0]);
		add_buf (victim->pcdata->buffer, buf);
		return;
	}

    act ("{kYou tell $N '{K$t{k'{x", ch, argument, victim, TO_CHAR);
    act_new ("{k$n tells you '{K$t{k'{x", ch, argument, victim, TO_VICT,
             POS_DEAD);
    victim->reply = ch;

    if (!IS_NPC (ch) && IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_SPEECH))
        mp_act_trigger (argument, victim, ch, NULL, NULL, TRIG_SPEECH);

    return;
}



void do_reply (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET (ch->comm, COMM_NOTELL))
    {
        send_to_char ("Your message didn't get through.\n\r", ch);
        return;
    }

    if ((victim = ch->reply) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (victim->desc == NULL && !IS_NPC (victim))
    {
        act ("$N seems to have misplaced $S link...try again later.",
             ch, NULL, victim, TO_CHAR);
        sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    if (!IS_IMMORTAL (ch) && !IS_AWAKE (victim))
    {
        act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (
        (IS_SET (victim->comm, COMM_QUIET)
         || IS_SET (victim->comm, COMM_DEAF)) && !IS_IMMORTAL (ch)
        && !IS_IMMORTAL (victim))
    {
        act_new ("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
                 POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL (victim) && !IS_AWAKE (ch))
    {
        send_to_char ("In your dreams, or what?\n\r", ch);
        return;
    }

    if (IS_SET (victim->comm, COMM_AFK))
    {
        if (IS_NPC (victim))
        {
            act_new ("$E is AFK, and not receiving tells.",
                     ch, NULL, victim, TO_CHAR, POS_DEAD);
            return;
        }

        act_new ("$E is AFK, but your tell will go through when $E returns.",
                 ch, NULL, victim, TO_CHAR, POS_DEAD);
        sprintf (buf, "{k%s tells you '{K%s{k'{x\n\r", PERS (ch, victim),
                 argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        return;
    }

    act_new ("{kYou tell $N '{K$t{k'{x", ch, argument, victim, TO_CHAR,
             POS_DEAD);
    act_new ("{k$n tells you '{K$t{k'{x", ch, argument, victim, TO_VICT,
             POS_DEAD);
    victim->reply = ch;

    return;
}



void do_yell (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (IS_SET (ch->comm, COMM_NOSHOUT))
    {
        send_to_char ("You can't yell.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char ("Yell what?\n\r", ch);
        return;
    }


    act ("You yell '$t'", ch, argument, NULL, TO_CHAR);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected == CON_PLAYING
            && d->character != ch
            && d->character->in_room != NULL
            && d->character->in_room->area == ch->in_room->area
            && !IS_SET (d->character->comm, COMM_QUIET))
        {
            act ("$n yells '$t'", ch, argument, d->character, TO_VICT);
        }
    }

    return;
}


void do_emote (CHAR_DATA * ch, char *argument)
{
    if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
        send_to_char ("You can't show your emotions.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char ("Emote what?\n\r", ch);
        return;
    }

    /* little hack to fix the ',{' bug posted to rom list
     * around 4/16/01 -- JR
     */
    if (!(isalpha(argument[0])) || (isspace(argument[0])))
    {
	send_to_char ("Moron!\n\r", ch);
	return;
    }

    MOBtrigger = FALSE;
    act ("$n $T", ch, NULL, argument, TO_ROOM);
    act ("$n $T", ch, NULL, argument, TO_CHAR);
    MOBtrigger = TRUE;
    return;
}


void do_pmote (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
        send_to_char ("You can't show your emotions.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char ("Emote what?\n\r", ch);
        return;
    }

    /* little hack to fix the ',{' bug posted to rom list
     * around 4/16/01 -- JR
     */
    if (!(isalpha(argument[0])) || (isspace(argument[0])))
    {
	send_to_char ("Moron!\n\r", ch);
	return;
    }

    act ("$n $t", ch, argument, NULL, TO_CHAR);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;

        if ((letter = strstr (argument, vch->name)) == NULL)
        {
            MOBtrigger = FALSE;
            act ("$N $t", vch, argument, ch, TO_CHAR);
            MOBtrigger = TRUE;
            continue;
        }

        strcpy (temp, argument);
        temp[strlen (argument) - strlen (letter)] = '\0';
        last[0] = '\0';
        name = vch->name;

        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen (vch->name))
            {
                strcat (temp, "r");
                continue;
            }

            if (*letter == 's' && matches == strlen (vch->name))
            {
                matches = 0;
                continue;
            }

            if (matches == strlen (vch->name))
            {
                matches = 0;
            }

            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen (vch->name))
                {
                    strcat (temp, "you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat (last, letter, 1);
                continue;
            }

            matches = 0;
            strcat (temp, last);
            strncat (temp, letter, 1);
            last[0] = '\0';
            name = vch->name;
        }

        MOBtrigger = FALSE;
        act ("$N $t", vch, temp, ch, TO_CHAR);
        MOBtrigger = TRUE;
    }

    return;
}


/*
 * All the posing stuff.
 */
struct pose_table_type {
    char *message[2 * MAX_CLASS];
};

const struct pose_table_type pose_table[] = {
    {
     {
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."}
     },

    {
     {
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."}
     },

    {
     {
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."}
     },

    {
     {
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."}
     },

    {
     {
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."}
     },

    {
     {
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."}
     },

    {
     {
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
      "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."}
     },

    {
     {
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."}
     },

    {
     {
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."}
     },

    {
     {
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."}
     },

    {
     {
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."}
     },

    {
     {
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."}
     },

    {
     {
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."}
     },

    {
     {
      "The heavens and grass change colour as you smile.",
      "The heavens and grass change colour as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."}
     },

    {
     {
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."}
     },

    {
     {
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."}
     },

    {
     {
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great god Om gives you a staff.",
      "The great god Om gives $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."}
     }
};



void do_pose (CHAR_DATA * ch, char *argument)
{
    int level;
    int pose;

    if (IS_NPC (ch))
        return;

    level =
        UMIN (ch->level, sizeof (pose_table) / sizeof (pose_table[0]) - 1);
    pose = number_range (0, level);

    act (pose_table[pose].message[2 * ch->class + 0], ch, NULL, NULL,
         TO_CHAR);
    act (pose_table[pose].message[2 * ch->class + 1], ch, NULL, NULL,
         TO_ROOM);

    return;
}



void do_bug (CHAR_DATA * ch, char *argument)
{
    append_file (ch, BUG_FILE, argument);
    send_to_char ("Bug logged.\n\r", ch);
    return;
}

void do_typo (CHAR_DATA * ch, char *argument)
{
    append_file (ch, TYPO_FILE, argument);
    send_to_char ("Typo logged.\n\r", ch);
    return;
}

void do_rent (CHAR_DATA * ch, char *argument)
{
    send_to_char ("There is no rent here.  Just save and quit.\n\r", ch);
    return;
}


void do_qui (CHAR_DATA * ch, char *argument)
{
    send_to_char ("If you want to QUIT, you have to spell it out.\n\r", ch);
    return;
}



void do_quit (CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if (IS_NPC (ch))
        return;

    if (ch->position == POS_FIGHTING)
    {
        send_to_char ("No way! You are fighting.\n\r", ch);
        return;
    }

    if (ch->position < POS_STUNNED)
    {
        send_to_char ("You're not DEAD yet.\n\r", ch);
        return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
        return;
    }
    //send_to_char ("Alas, all good things must come to an end.\n\r", ch);
    char *message;
        
    switch (number_range (0, 6))
    {
        default:
            message = "Alas, all good things must come to an end.\n\r";
            break;
        case 0:
            message = "Fantasy is hardly an escape from reality. It's a way of understanding it. - Lloyd Alexander\n\r";        
            break;
        case 1:
            message = "Fantasy is escapist, and that is its glory. If a soldier is imprisioned by the enemy, don't we consider it his duty to escape? - J.R.R. Tolkien\n\r";      
            break;
        case 2:       
            message = "Stories of imagination tend to upset those without one. - Sir Terry Pratchett\n\r";
            break;
        case 3:       
            message = "Come away, O human child!\n\rTo the waters and the wild\n\rWith a faery, hand in hand,\n\rFor the world's more full of \n\rweeping than you can understand. - W.B. Yeats\n\r";
            break;
        case 4:
            message = "They can keep their heaven. When I die, I?d sooner go to Middle-earth. - George R.R. Martin\n\r";
            break;
        case 5:
            message = "Do not meddle in the affairs of wizards, for they are subtle and quick to anger. - J.R.R. Tolkien\n\r";
            break;
        case 6:
            message = "Beautiful and full of monsters? All the best stories are. - Laini Taylor\n\r";
            break;
    }

    send_to_char (message, ch);
    act ("$n has left the game.", ch, NULL, NULL, TO_ROOM);
    sprintf (log_buf, "%s has quit.", ch->name);
    log_string (log_buf);
    recent_add( ch );
    
    /* Track session time for mudinfo average calculation */
    if (ch->pcdata->login_time > 0)
    {
        total_player_seconds += (current_time - ch->pcdata->login_time);
    }
    
    wiznet ("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0,
            get_trust (ch));
            info( ch, 0, "{W[{YINFO{W]:{x %s just logged out!", ch->name );

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj (ch);

	/* Free note that might be there somehow */
	if (ch->pcdata->in_progress)
		free_note (ch->pcdata->in_progress);

    id = ch->id;
    d = ch->desc;
    extract_char (ch, TRUE);
    if (d != NULL)
        close_socket (d);

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
        CHAR_DATA *tch;

        d_next = d->next;
        tch = d->original ? d->original : d->character;
        if (tch && tch->id == id)
        {
            extract_char (tch, TRUE);
            close_socket (d);
        }
    }

    return;
}



void do_save (CHAR_DATA * ch, char *argument)
{
    if (IS_NPC (ch))
        return;

    save_char_obj (ch);
    send_to_char ("Saving. Remember that ROM has automatic saving now.\n\r",
                  ch);
    WAIT_STATE (ch, 4 * PULSE_VIOLENCE);
    return;
}



void do_follow (CHAR_DATA * ch, char *argument)
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Follow whom?\n\r", ch);
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL)
    {
        act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        if (ch->master == NULL)
        {
            send_to_char ("You already follow yourself.\n\r", ch);
            return;
        }
        stop_follower (ch);
        return;
    }

    if (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOFOLLOW)
        && !IS_IMMORTAL (ch))
    {
        act ("$N doesn't seem to want any followers.\n\r", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    REMOVE_BIT (ch->act, PLR_NOFOLLOW);

    if (ch->master != NULL)
        stop_follower (ch);

    add_follower (ch, victim);
    return;
}


void add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
    if (ch->master != NULL)
    {
        bug ("Add_follower: non-null master.", 0);
        return;
    }

    ch->master = master;
    ch->leader = NULL;

    if (can_see (master, ch))
        act ("$n now follows you.", ch, NULL, master, TO_VICT);

    act ("You now follow $N.", ch, NULL, master, TO_CHAR);

    return;
}



void stop_follower (CHAR_DATA * ch)
{
    if (ch->master == NULL)
    {
        bug ("Stop_follower: null master.", 0);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        STR_REMOVE_BIT (ch->affected_by, AFF_CHARM);
        affect_strip (ch, gsn_charm_person);
    }

    if (can_see (ch->master, ch) && ch->in_room != NULL)
    {
        act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
        act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    }
    if (ch->master->pet == ch)
        ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets (CHAR_DATA * ch)
{
    CHAR_DATA *pet;

    if (IS_NPC(ch))
        return;

    if ((pet = ch->pet) != NULL)
    {
        stop_follower (pet);
        if (pet->in_room != NULL)
            act ("$N slowly fades away.", ch, NULL, pet, TO_NOTVICT);
        extract_char (pet, TRUE);
    }
    ch->pet = NULL;

    if ( ch->mount
    && (ch->mount->in_room == ch->in_room || ch->mount->in_room==NULL) )
    {
        pet = ch->mount;
        do_dismount(ch, "");
        if (pet->in_room != NULL)
            act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
        else
            log_string("void nuke_pets: Extracting null pet");
        ch->mount = NULL;
        ch->riding = FALSE;            
        extract_char(pet, TRUE);
    }
    else if (ch->mount)
    {
        ch->mount->mount = NULL;
        ch->mount->riding = FALSE;
    }
    ch->mount = NULL;

    return;
}



void die_follower (CHAR_DATA * ch)
{
    CHAR_DATA *fch;

    if (ch->master != NULL)
    {
        if (ch->master->pet == ch)
            ch->master->pet = NULL;
        stop_follower (ch);
    }

    ch->leader = NULL;

    for (fch = char_list; fch != NULL; fch = fch->next)
    {
        if (fch->master == ch)
            stop_follower (fch);
        if (fch->leader == ch)
            fch->leader = fch;
    }

    return;
}



void do_order (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument (argument, arg);
    one_argument (argument, arg2);

    if (!str_cmp (arg2, "delete") || !str_cmp (arg2, "mob"))
    {
        send_to_char ("That will NOT be done.\n\r", ch);
        return;
    }

    if (arg[0] == '\0' || argument[0] == '\0')
    {
        send_to_char ("Order whom to do what?\n\r", ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        send_to_char ("You feel like taking, not giving, orders.\n\r", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        fAll = TRUE;
        victim = NULL;
    }
    else
    {
        fAll = FALSE;

        if (!strcmp(arg, "mount"))
        {
            if (!ch->mount)
            {
                send_to_char("Your don't have a mount.\n\r", ch);
                return;
            }

            if (ch->mount->in_room != ch->in_room)
            {
                send_to_char("Your mount isn't here!\n\r", ch);
                return;
            }
            else
            {
                victim = ch->mount;
            }
        }
        else if ((victim = get_char_room (ch, arg)) == NULL)
        {
            send_to_char ("They aren't here.\n\r", ch);
            return;
        }

        if (victim == ch)
        {
            send_to_char ("Aye aye, right away!\n\r", ch);
            return;
        }

        if ( victim->mount == ch )
        { 
            if ( !mount_success(ch, victim, FALSE) )
            {
                act("$N ignores your orders.", ch, NULL, victim, TO_CHAR);
                return;
            }
            else
            {
                sprintf(buf,"%s orders you to \'%s\'.", ch->name, argument);
                send_to_char(buf, victim);
                interpret( victim, argument );
                return;
            }                
        }
        else if (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch
            || (IS_IMMORTAL (victim) && victim->trust >= ch->trust))
        {
            send_to_char ("Do it yourself!\n\r", ch);
            return;
        }
    }

    found = FALSE;
    for (och = ch->in_room->people; och != NULL; och = och_next)
    {
        och_next = och->next_in_room;

        if (IS_AFFECTED (och, AFF_CHARM)
            && och->master == ch && (fAll || och == victim))
        {
            found = TRUE;
            sprintf (buf, "$n orders you to '%s'.", argument);
            act (buf, ch, NULL, och, TO_VICT);
            interpret (och, argument);
        }
    }

    if (found)
    {
        WAIT_STATE (ch, PULSE_VIOLENCE);
        send_to_char ("Ok.\n\r", ch);
    }
    else
        send_to_char ("You have no followers here.\n\r", ch);
    return;
}



void do_group (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
    {
        CHAR_DATA *gch;
        CHAR_DATA *leader;

        leader = (ch->leader != NULL) ? ch->leader : ch;
        sprintf (buf, "%s's group:\n\r", PERS (leader, ch));
        send_to_char (buf, ch);

        for (gch = char_list; gch != NULL; gch = gch->next)
        {
            if (is_same_group (gch, ch))
            {
                sprintf (buf,
                         "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
                         gch->level,
                         IS_NPC (gch) ? "Mob" : class_table[gch->
                                                            class].who_name,
                         capitalize (PERS (gch, ch)), gch->hit, gch->max_hit,
                         gch->mana, gch->max_mana, gch->move, gch->max_move,
                         gch->exp);
                send_to_char (buf, ch);
            }
        }
        return;
    }

    if ((victim = get_char_room (ch, arg)) == NULL)
    {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
        send_to_char ("But you are following someone else!\n\r", ch);
        return;
    }

    if (victim->master != ch && ch != victim)
    {
        act_new ("$N isn't following you.", ch, NULL, victim, TO_CHAR,
                 POS_SLEEPING);
        return;
    }

    if (IS_AFFECTED (victim, AFF_CHARM))
    {
        send_to_char ("You can't remove charmed mobs from your group.\n\r",
                      ch);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM))
    {
        act_new ("You like your master too much to leave $m!",
                 ch, NULL, victim, TO_VICT, POS_SLEEPING);
        return;
    }

    if (is_same_group (victim, ch) && ch != victim)
    {
        victim->leader = NULL;
        act_new ("$n removes $N from $s group.",
                 ch, NULL, victim, TO_NOTVICT, POS_RESTING);
        act_new ("$n removes you from $s group.",
                 ch, NULL, victim, TO_VICT, POS_SLEEPING);
        act_new ("You remove $N from your group.",
                 ch, NULL, victim, TO_CHAR, POS_SLEEPING);
        return;
    }

    victim->leader = ch;
    act_new ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT,
             POS_RESTING);
    act_new ("You join $n's group.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
    act_new ("$N joins your group.", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split (CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument (argument, arg1);
    one_argument (argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char ("Split how much?\n\r", ch);
        return;
    }

    amount_silver = atoi (arg1);

    if (arg2[0] != '\0')
        amount_gold = atoi (arg2);

    if (amount_gold < 0 || amount_silver < 0)
    {
        send_to_char ("Your group wouldn't like that.\n\r", ch);
        return;
    }

    if (amount_gold == 0 && amount_silver == 0)
    {
        send_to_char ("You hand out zero coins, but no one notices.\n\r", ch);
        return;
    }

    if (ch->gold < amount_gold || ch->silver < amount_silver)
    {
        send_to_char ("You don't have that much to split.\n\r", ch);
        return;
    }

    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (is_same_group (gch, ch) && !IS_AFFECTED (gch, AFF_CHARM))
            members++;
    }

    if (members < 2)
    {
        send_to_char ("Just keep it all.\n\r", ch);
        return;
    }

    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    if (share_gold == 0 && share_silver == 0)
    {
        send_to_char ("Don't even bother, cheapskate.\n\r", ch);
        return;
    }

    ch->silver -= amount_silver;
    ch->silver += share_silver + extra_silver;
    ch->gold -= amount_gold;
    ch->gold += share_gold + extra_gold;

    if (share_silver > 0)
    {
        sprintf (buf,
                 "You split %d silver coins. Your share is %d silver.\n\r",
                 amount_silver, share_silver + extra_silver);
        send_to_char (buf, ch);
    }

    if (share_gold > 0)
    {
        sprintf (buf,
                 "You split %d gold coins. Your share is %d gold.\n\r",
                 amount_gold, share_gold + extra_gold);
        send_to_char (buf, ch);
    }

    if (share_gold == 0)
    {
        sprintf (buf, "$n splits %d silver coins. Your share is %d silver.",
                 amount_silver, share_silver);
    }
    else if (share_silver == 0)
    {
        sprintf (buf, "$n splits %d gold coins. Your share is %d gold.",
                 amount_gold, share_gold);
    }
    else
    {
        sprintf (buf,
                 "$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
                 amount_silver, amount_gold, share_silver, share_gold);
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
        if (gch != ch && is_same_group (gch, ch)
            && !IS_AFFECTED (gch, AFF_CHARM))
        {
            act (buf, ch, NULL, gch, TO_VICT);
            gch->gold += share_gold;
            gch->silver += share_silver;
        }
    }

    return;
}



void do_gtell (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *gch;

    if (argument[0] == '\0')
    {
        send_to_char ("Tell your group what?\n\r", ch);
        return;
    }

    if (IS_SET (ch->comm, COMM_NOTELL))
    {
        send_to_char ("Your message didn't get through!\n\r", ch);
        return;
    }

    for (gch = char_list; gch != NULL; gch = gch->next)
    {
        if (is_same_group (gch, ch))
            act_new ("$n tells the group '$t'",
                     ch, argument, gch, TO_VICT, POS_SLEEPING);
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
    if (ach == NULL || bch == NULL)
        return FALSE;

    if (ach->leader != NULL)
        ach = ach->leader;
    if (bch->leader != NULL)
        bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
    {
        send_to_char_bw ("ColoUr is not ON, Way Moron!\n\r", ch);
        return;
    }

    argument = one_argument (argument, arg);

    if (!*arg)
    {
        if (!IS_SET (ch->act, PLR_COLOUR))
        {
            SET_BIT (ch->act, PLR_COLOUR);
            send_to_char ("ColoUr is now ON, Way Cool!\n\r"
                          "Further syntax:\n\r   colour {c<{xfield{c> <{xcolour{c>{x\n\r"
                          "   colour {c<{xfield{c>{x {cbeep{x|{cnobeep{x\n\r"
                          "Type help {ccolour{x and {ccolour2{x for details.\n\r"
                          "ColoUr is brought to you by Lope, ant@solace.mh.se.\n\r",
                          ch);
        }
        else
        {
            send_to_char_bw ("ColoUr is now OFF, <sigh>\n\r", ch);
            REMOVE_BIT (ch->act, PLR_COLOUR);
        }
        return;
    }

    if (!str_cmp (arg, "default"))
    {
        default_colour (ch);
        send_to_char_bw ("ColoUr setting set to default values.\n\r", ch);
        return;
    }

    if (!str_cmp (arg, "all"))
    {
        all_colour (ch, argument);
        return;
    }

    /*
     * Yes, I know this is ugly and unnessessary repetition, but its old
     * and I can't justify the time to make it pretty. -Lope
     */
    if (!str_cmp (arg, "text"))
    {
    ALTER_COLOUR (text)}
    else if (!str_cmp (arg, "auction"))
    {
    ALTER_COLOUR (auction)}
    else if (!str_cmp (arg, "auction_text"))
    {
    ALTER_COLOUR (auction_text)}
    else if (!str_cmp (arg, "gossip"))
    {
    ALTER_COLOUR (gossip)}
    else if (!str_cmp (arg, "gossip_text"))
    {
    ALTER_COLOUR (gossip_text)}
    else if (!str_cmp (arg, "music"))
    {
    ALTER_COLOUR (music)}
    else if (!str_cmp (arg, "music_text"))
    {
    ALTER_COLOUR (music_text)}
    else if (!str_cmp (arg, "question"))
    {
    ALTER_COLOUR (question)}
    else if (!str_cmp (arg, "question_text"))
    {
    ALTER_COLOUR (question_text)}
    else if (!str_cmp (arg, "answer"))
    {
    ALTER_COLOUR (answer)}
    else if (!str_cmp (arg, "answer_text"))
    {
    ALTER_COLOUR (answer_text)}
    else if (!str_cmp (arg, "quote"))
    {
    ALTER_COLOUR (quote)}
    else if (!str_cmp (arg, "quote_text"))
    {
    ALTER_COLOUR (quote_text)}
    else if (!str_cmp (arg, "immtalk_text"))
    {
    ALTER_COLOUR (immtalk_text)}
    else if (!str_cmp (arg, "immtalk_type"))
    {
    ALTER_COLOUR (immtalk_type)}
    else if (!str_cmp (arg, "info"))
    {
    ALTER_COLOUR (info)}
    else if (!str_cmp (arg, "say"))
    {
    ALTER_COLOUR (say)}
    else if (!str_cmp (arg, "say_text"))
    {
    ALTER_COLOUR (say_text)}
    else if (!str_cmp (arg, "tell"))
    {
    ALTER_COLOUR (tell)}
    else if (!str_cmp (arg, "tell_text"))
    {
    ALTER_COLOUR (tell_text)}
    else if (!str_cmp (arg, "reply"))
    {
    ALTER_COLOUR (reply)}
    else if (!str_cmp (arg, "reply_text"))
    {
    ALTER_COLOUR (reply_text)}
    else if (!str_cmp (arg, "gtell_text"))
    {
    ALTER_COLOUR (gtell_text)}
    else if (!str_cmp (arg, "gtell_type"))
    {
    ALTER_COLOUR (gtell_type)}
    else if (!str_cmp (arg, "wiznet"))
    {
    ALTER_COLOUR (wiznet)}
    else if (!str_cmp (arg, "room_title"))
    {
    ALTER_COLOUR (room_title)}
    else if (!str_cmp (arg, "room_text"))
    {
    ALTER_COLOUR (room_text)}
    else if (!str_cmp (arg, "room_exits"))
    {
    ALTER_COLOUR (room_exits)}
    else if (!str_cmp (arg, "room_things"))
    {
    ALTER_COLOUR (room_things)}
    else if (!str_cmp (arg, "prompt"))
    {
    ALTER_COLOUR (prompt)}
    else if (!str_cmp (arg, "fight_death"))
    {
    ALTER_COLOUR (fight_death)}
    else if (!str_cmp (arg, "fight_yhit"))
    {
    ALTER_COLOUR (fight_yhit)}
    else if (!str_cmp (arg, "fight_ohit"))
    {
    ALTER_COLOUR (fight_ohit)}
    else if (!str_cmp (arg, "fight_thit"))
    {
    ALTER_COLOUR (fight_thit)}
    else if (!str_cmp (arg, "fight_skill"))
    {
    ALTER_COLOUR (fight_skill)}
    else if (!str_cmp (arg, "comm1"))
    {
    ALTER_COLOUR (comm1)}
    else if (!str_cmp (arg, "comm2"))
    {
    ALTER_COLOUR (comm2)}
    else if (!str_cmp (arg, "comm3"))
    {
    ALTER_COLOUR (comm3)}
    else if (!str_cmp (arg, "comm4"))
    {
    ALTER_COLOUR (comm4)}
    else
    {
        send_to_char_bw ("Unrecognised Colour Parameter Not Set.\n\r", ch);
        return;
    }

    send_to_char_bw ("New Colour Parameter Set.\n\r", ch);
    return;
}

void info( CHAR_DATA * ch, int level, char * message, ...)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA * d;
    va_list args;

    va_start(args,message);
    vsnprintf(buf, MAX_STRING_LENGTH, message, args);
    va_end(args);

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING && 
             d->character != ch &&
             get_trust(d->character) >= level )
           send_to_char(buf, d->character);
    }
}

void do_info(CHAR_DATA * ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_NOINFO))
    {
        send_to_char("Info channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOINFO);
    }
    else
    {
        send_to_char("Info channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOINFO);
    }
}

/* Drunken speech system - by Apex, modified by the Maniac, further mods by Kohl Desenee */
char * makedrunk (char *string, CHAR_DATA * ch)
{
/* This structure defines all changes for a character */
  struct struckdrunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[1024];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;
  char *string_start = string;  /* Save original pointer before loop increments it */

  /* Check how drunk a person is... */
  if (IS_NPC(ch))
        drunklevel = 0;
  else
        drunklevel = ch->pcdata->condition[COND_DRUNK];

  if (drunklevel > 0)
    {
      do
        {
          temp = toupper (*string);
          if ((temp >= 'A') && (temp <= 'Z'))
            {
              if (drunklevel > drunk[temp - 'A'].min_drunk_level)
                {
                  randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
                  strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
                  pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
                }
              else
                buf[pos++] = *string;
            }
          else
            {
              if ((temp >= '0') && (temp <= '9'))
                {
                  temp = '0' + number_range (0, 9);
                  buf[pos++] = temp;
                }
              else
                buf[pos++] = *string;
            }
        }
      while (*string++);
      buf[pos] = '\0';
      strcpy(string_start, buf);  /* Use saved pointer */
      return(string_start);
    }
    
  return (string_start);
}
