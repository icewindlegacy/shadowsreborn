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
 *           olc_skedit.c - November 3, 2025
 */            
/***************************************************************************
 *  Skill Editor for OLC by Om of Shadows Reborn
 *  Allows editing which classes get skills/spells at what level
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"

/* Helper function to set skill level for a class */
bool skedit_set_level (CHAR_DATA *ch, int class_num, char *argument)
{
    int level;
    int sn;
    
    EDIT_SKILL(ch, sn);
    
    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char ("You must specify a level.\n\r", ch);
        send_to_char ("Use 0 to remove the skill from this class, or LEVEL_IMMORTAL+ to make it unavailable to players.\n\r", ch);
        send_to_char ("Levels above LEVEL_IMMORTAL allow immortals to test skills from other classes.\n\r", ch);
        return FALSE;
    }
    
    level = atoi(argument);
    
    if (level < 0 || level > MAX_LEVEL)
    {
        send_to_char ("Level must be between 0 and MAX_LEVEL.\n\r", ch);
        return FALSE;
    }
    
    skill_table[sn].skill_level[class_num] = level;
    
    if (level >= LEVEL_IMMORTAL)
    {
        if (level == LEVEL_IMMORTAL)
            send_to_char ("Skill removed from this class (players cannot learn it).\n\r", ch);
        else
            send_to_char ("Skill set to high level - only immortals can learn it for testing.\n\r", ch);
    }
    else if (level == 0)
        send_to_char ("Skill available at level 0 for this class.\n\r", ch);
    else
        send_to_char ("Skill will be gained at the specified level.\n\r", ch);
    
    return TRUE;
}

/* Show skill editor menu */
bool skedit_show (CHAR_DATA *ch, char *argument)
{
    int sn;
    int i;
    char buf[MAX_STRING_LENGTH];
    
    EDIT_SKILL(ch, sn);
    
    if (sn < 0 || sn >= MAX_SKILL || skill_table[sn].name == NULL)
    {
        send_to_char ("Invalid skill.\n\r", ch);
        edit_done(ch);
        return FALSE;
    }
    
    send_to_char ("\n\r", ch);
    sprintf(buf, "Skill: %s\n\r", skill_table[sn].name);
    send_to_char (buf, ch);
    send_to_char ("\n\r", ch);
    
    /* Show current levels for each class */
    sprintf(buf, "%-12s Level\n\r", "Class");
    send_to_char (buf, ch);
    send_to_char ("----------------------------------------\n\r", ch);
    
    for (i = 0; i < MAX_CLASS; i++)
    {
        int level = skill_table[sn].skill_level[i];
        char level_buf[32];
        const char *level_str;
        
        if (level >= LEVEL_IMMORTAL)
        {
            if (level == LEVEL_IMMORTAL)
                level_str = "unavailable";
            else
            {
                sprintf(level_buf, "%d (immortal)", level);
                level_str = level_buf;
            }
        }
        else
        {
            sprintf(level_buf, "%d", level);
            level_str = level_buf;
        }
        
        sprintf(buf, "%-12s %s\n\r", class_table[i].name, level_str);
        send_to_char (buf, ch);
    }
    
    send_to_char ("\n\r", ch);
    send_to_char ("Commands: <class> <level> - Set level for class\n\r", ch);
    send_to_char ("          done - Exit editor\n\r", ch);
    send_to_char ("\n\r", ch);
    send_to_char ("Level range: 0 to MAX_LEVEL\n\r", ch);
    send_to_char ("  - 0: Available at level 0\n\r", ch);
    send_to_char ("  - 1-52: Available at that level for players\n\r", ch);
    send_to_char ("  - LEVEL_IMMORTAL: Unavailable to players\n\r", ch);
    send_to_char ("  - LEVEL_IMMORTAL+: Only immortals can learn (for testing)\n\r", ch);
    send_to_char ("\n\r", ch);
    send_to_char ("Class names: mage, cleric, thief, warrior, sorcerer, shaman, knave, berserker\n\r", ch);
    send_to_char ("            (or: mag, cle, thi, war, sor, sha, kna, ber)\n\r", ch);
    send_to_char ("\n\r", ch);
    
    return FALSE;
}

/* Class command handlers */
bool skedit_mage (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 0, argument);
}

bool skedit_cleric (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 1, argument);
}

bool skedit_thief (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 2, argument);
}

bool skedit_warrior (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 3, argument);
}

bool skedit_sorcerer (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 4, argument);
}

bool skedit_shaman (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 5, argument);
}

bool skedit_knave (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 6, argument);
}

bool skedit_berserker (CHAR_DATA *ch, char *argument)
{
    return skedit_set_level(ch, 7, argument);
}

/* Command table for skedit */
const struct olc_cmd_type skedit_table[] = {
    {"show", skedit_show},
    {"mage", skedit_mage},
    {"mag", skedit_mage},
    {"cleric", skedit_cleric},
    {"cle", skedit_cleric},
    {"thief", skedit_thief},
    {"thi", skedit_thief},
    {"warrior", skedit_warrior},
    {"war", skedit_warrior},
    {"sorcerer", skedit_sorcerer},
    {"sor", skedit_sorcerer},
    {"shaman", skedit_shaman},
    {"sha", skedit_shaman},
    {"knave", skedit_knave},
    {"kna", skedit_knave},
    {"berserker", skedit_berserker},
    {"ber", skedit_berserker},
    {NULL, NULL}
};

/* Skill editor interpreter */
void skedit (CHAR_DATA *ch, char *argument)
{
    int sn;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd;
    
    EDIT_SKILL(ch, sn);
    
    if (sn < 0 || sn >= MAX_SKILL || skill_table[sn].name == NULL)
    {
        send_to_char ("Invalid skill.\n\r", ch);
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
        skedit_show (ch, argument);
        return;
    }
    
    /* Search Table and Dispatch Command. */
    for (cmd = 0; skedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix (command, skedit_table[cmd].name))
        {
            (*skedit_table[cmd].olc_fun) (ch, argument);
            return;
        }
    }
    
    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}

/* Entry point for skill editor */
void do_skedit (CHAR_DATA *ch, char *argument)
{
    int sn;
    char arg[MAX_STRING_LENGTH];
    
    if (IS_NPC (ch))
        return;
    
    if (!IS_IMMORTAL(ch))
    {
        send_to_char ("You are not authorized to edit skills.\n\r", ch);
        return;
    }
    
    argument = one_argument (argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char ("Syntax: skedit <skill name>\n\r", ch);
        return;
    }
    
    sn = skill_lookup (arg);
    
    if (sn == -1)
    {
        send_to_char ("That skill does not exist.\n\r", ch);
        return;
    }
    
    if (ch->desc == NULL)
        return;
    
    ch->desc->pEdit = (void *)(long)sn;
    ch->desc->editor = ED_SKILL;
    
    skedit_show (ch, "");
    return;
}

