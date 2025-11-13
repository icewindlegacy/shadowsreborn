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
 *           olc_cedit.c - November 13, 2025
 */
/***************************************************************************
 *  Clan Editor for OLC by Om of Shadows Reborn
 *  Allows editing clan data online
 ***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"

/* Show clan editor menu */
bool cedit_show (CHAR_DATA *ch, char *argument)
{
    int cn;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        edit_done(ch);
        return FALSE;
    }
    
    send_to_char ("\n\r", ch);
    sprintf(buf, "Clan Number: %d\n\r", cn);
    send_to_char (buf, ch);
    send_to_char ("----------------------------------------\n\r", ch);
    
    sprintf(buf, "Name:         %s\n\r", 
            clan_table[cn].name ? clan_table[cn].name : "(none)");
    send_to_char (buf, ch);
    
    sprintf(buf, "Who Name:     %s\n\r", 
            clan_table[cn].who_name ? clan_table[cn].who_name : "(none)");
    send_to_char (buf, ch);
    
    pRoom = get_room_index (clan_table[cn].deathroom);
    sprintf(buf, "Deathroom:    %d %s\n\r", 
            clan_table[cn].deathroom,
            pRoom ? pRoom->name : "(invalid)");
    send_to_char (buf, ch);
    
    pRoom = get_room_index (clan_table[cn].recall);
    sprintf(buf, "Recall:       %d %s\n\r", 
            clan_table[cn].recall,
            pRoom ? pRoom->name : "(invalid)");
    send_to_char (buf, ch);
    
    sprintf(buf, "Independent:  %s\n\r", 
            clan_table[cn].independent ? "true" : "false");
    send_to_char (buf, ch);
    
    sprintf(buf, "Min Level:    %d\n\r", clan_table[cn].min_level);
    send_to_char (buf, ch);
    
    send_to_char ("\n\r", ch);
    send_to_char ("Commands:\n\r", ch);
    send_to_char ("  create <number>    - Create new clan at slot number\n\r", ch);
    send_to_char ("  name <name>        - Set clan name\n\r", ch);
    send_to_char ("  who <who entry>    - Set who entry (what shows in do_who)\n\r", ch);
    send_to_char ("  deathroom <vnum>   - Set deathroom (where corpses go)\n\r", ch);
    send_to_char ("  recall <vnum>      - Set recall room (where 'recall clan' goes)\n\r", ch);
    send_to_char ("  indie <true/false> - Set if clan is independent\n\r", ch);
    send_to_char ("  minlevel <level>   - Set minimum level to join\n\r", ch);
    send_to_char ("  done               - Exit editor\n\r", ch);
    send_to_char ("\n\r", ch);
    
    return FALSE;
}

/* Set clan name */
bool cedit_name (CHAR_DATA *ch, char *argument)
{
    int cn;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    if (argument[0] == '\0')
    {
        send_to_char ("Syntax: name <name>\n\r", ch);
        return FALSE;
    }
    
    /* Free old name */
    if (clan_table[cn].name)
        free_string (clan_table[cn].name);
    
    /* Allocate new name */
    clan_table[cn].name = str_dup (argument);
    
    send_to_char ("Clan name set.\n\r", ch);
    return TRUE;
}

/* Set who name */
bool cedit_who (CHAR_DATA *ch, char *argument)
{
    int cn;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    if (argument[0] == '\0')
    {
        send_to_char ("Syntax: who <who entry>\n\r", ch);
        return FALSE;
    }
    
    /* Free old who name */
    if (clan_table[cn].who_name)
        free_string (clan_table[cn].who_name);
    
    /* Allocate new who name */
    clan_table[cn].who_name = str_dup (argument);
    
    send_to_char ("Who entry set.\n\r", ch);
    return TRUE;
}

/* Set deathroom */
bool cedit_deathroom (CHAR_DATA *ch, char *argument)
{
    int cn;
    int vnum;
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char ("Syntax: deathroom <vnum>\n\r", ch);
        return FALSE;
    }
    
    vnum = atoi(argument);
    
    /* Validate room exists */
    pRoom = get_room_index (vnum);
    if (pRoom == NULL)
    {
        send_to_char ("That room vnum does not exist.\n\r", ch);
        return FALSE;
    }
    
    clan_table[cn].deathroom = vnum;
    
    send_to_char ("Deathroom set.\n\r", ch);
    return TRUE;
}

/* Set recall room */
bool cedit_recall (CHAR_DATA *ch, char *argument)
{
    int cn;
    int vnum;
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char ("Syntax: recall <vnum>\n\r", ch);
        return FALSE;
    }
    
    vnum = atoi(argument);
    
    /* Validate room exists */
    pRoom = get_room_index (vnum);
    if (pRoom == NULL)
    {
        send_to_char ("That room vnum does not exist.\n\r", ch);
        return FALSE;
    }
    
    clan_table[cn].recall = vnum;
    
    send_to_char ("Recall room set.\n\r", ch);
    return TRUE;
}

/* Set independent flag */
bool cedit_indie (CHAR_DATA *ch, char *argument)
{
    int cn;
    char arg[MAX_INPUT_LENGTH];
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    argument = one_argument (argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char ("Syntax: indie <true|false>\n\r", ch);
        return FALSE;
    }
    
    if (!strcasecmp (arg, "true") || !strcasecmp (arg, "1") || !strcasecmp (arg, "yes"))
    {
        clan_table[cn].independent = TRUE;
        send_to_char ("Clan set to independent.\n\r", ch);
    }
    else if (!strcasecmp (arg, "false") || !strcasecmp (arg, "0") || !strcasecmp (arg, "no"))
    {
        clan_table[cn].independent = FALSE;
        send_to_char ("Clan set to non-independent (real clan).\n\r", ch);
    }
    else
    {
        send_to_char ("Value must be true or false.\n\r", ch);
        return FALSE;
    }
    
    return TRUE;
}

/* Set minimum level */
bool cedit_minlevel (CHAR_DATA *ch, char *argument)
{
    int cn;
    int level;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        return FALSE;
    }
    
    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char ("Syntax: minlevel <level>\n\r", ch);
        return FALSE;
    }
    
    level = atoi(argument);
    
    if (level < 0 || level > MAX_LEVEL)
    {
        send_to_char ("Level must be between 0 and MAX_LEVEL.\n\r", ch);
        return FALSE;
    }
    
    clan_table[cn].min_level = level;
    
    send_to_char ("Minimum level set.\n\r", ch);
    return TRUE;
}

/* Create new clan */
bool cedit_create (CHAR_DATA *ch, char *argument)
{
    int cn;
    
    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char ("Syntax: create <clan number>\n\r", ch);
        send_to_char ("Creates a new clan at the specified number (0 to MAX_CLAN-1).\n\r", ch);
        return FALSE;
    }
    
    cn = atoi(argument);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan number. Valid range: 0 to MAX_CLAN-1\n\r", ch);
        return FALSE;
    }
    
    /* Check if clan already exists */
    if (clan_table[cn].name != NULL && clan_table[cn].name[0] != '\0')
    {
        send_to_char ("That clan slot is already in use.\n\r", ch);
        return FALSE;
    }
    
    /* Free any existing strings */
    if (clan_table[cn].name)
        free_string (clan_table[cn].name);
    if (clan_table[cn].who_name)
        free_string (clan_table[cn].who_name);
    
    /* Initialize new clan with defaults */
    clan_table[cn].name = str_dup ("New Clan");
    clan_table[cn].who_name = str_dup ("");
    clan_table[cn].deathroom = ROOM_VNUM_MORGUE;
    clan_table[cn].recall = ROOM_VNUM_TEMPLE;
    clan_table[cn].independent = FALSE;
    clan_table[cn].min_level = 0;
    
    send_to_char ("New clan created.\n\r", ch);
    
    /* Set editor to this clan */
    if (ch->desc)
    {
        ch->desc->pEdit = (void *)(long)cn;
        ch->desc->editor = ED_CLAN;
        cedit_show (ch, "");
    }
    
    return TRUE;
}

/* Command table for cedit */
const struct olc_cmd_type cedit_table[] = {
    {"show", cedit_show},
    {"create", cedit_create},
    {"name", cedit_name},
    {"who", cedit_who},
    {"deathroom", cedit_deathroom},
    {"recall", cedit_recall},
    {"indie", cedit_indie},
    {"independent", cedit_indie},
    {"minlevel", cedit_minlevel},
    {NULL, NULL}
};

/* Clan editor interpreter */
void cedit (CHAR_DATA *ch, char *argument)
{
    int cn;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd;
    
    EDIT_CLAN(ch, cn);
    
    if (cn < 0 || cn >= MAX_CLAN)
    {
        send_to_char ("Invalid clan.\n\r", ch);
        edit_done(ch);
        return;
    }
    
    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);
    
    if (!str_cmp (command, "done"))
    {
        edit_done (ch);
        return;
    }
    
    if (command[0] == '\0')
    {
        cedit_show (ch, argument);
        return;
    }
    
    /* Search Table and Dispatch Command. */
    for (cmd = 0; cedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, cedit_table[cmd].name))
        {
            (*cedit_table[cmd].olc_fun) (ch, argument);
            return;
        }
    }
    
    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}

/* Entry point for clan editor */
void do_cedit (CHAR_DATA *ch, char *argument)
{
    int cn;
    char arg[MAX_STRING_LENGTH];
    
    if (IS_NPC (ch))
        return;
    
    if (!IS_IMMORTAL(ch))
    {
        send_to_char ("You are not authorized to edit clans.\n\r", ch);
        return;
    }
    
    argument = one_argument (argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char ("Syntax: cedit <clan number|clan name|create number>\n\r", ch);
        send_to_char ("  cedit <number>    - Edit existing clan\n\r", ch);
        send_to_char ("  cedit <name>      - Edit clan by name\n\r", ch);
        send_to_char ("  cedit create <#>  - Create new clan at slot number\n\r", ch);
        send_to_char ("Use 'clanlist' to see available clans.\n\r", ch);
        return;
    }
    
    /* Check for create command */
    if (!str_cmp(arg, "create"))
    {
        if (ch->desc == NULL)
            return;
        
        /* Create new clan and enter editor */
        if (!cedit_create(ch, argument))
            return;
        
        /* Editor already set by cedit_create */
        return;
    }
    
    /* Try as number first */
    if (is_number(arg))
    {
        cn = atoi(arg);
        if (cn < 0 || cn >= MAX_CLAN)
        {
            send_to_char ("Invalid clan number. Valid range: 0 to MAX_CLAN-1\n\r", ch);
            return;
        }
    }
    else
    {
        /* Try as name */
        cn = clan_lookup (arg);
        if (cn == 1) /* clan_lookup returns 1 (independent) if not found */
        {
            /* Check if it's actually independent or not found */
            if (clan_table[1].name && !str_prefix(arg, clan_table[1].name))
                cn = 1;
            else
            {
                send_to_char ("That clan does not exist.\n\r", ch);
                return;
            }
        }
    }
    
    if (ch->desc == NULL)
        return;
    
    ch->desc->pEdit = (void *)(long)cn;
    ch->desc->editor = ED_CLAN;
    
    cedit_show (ch, "");
    return;
}

