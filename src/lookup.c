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
 *           lookup.c - November 3, 2025
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
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merc.h"
#include "tables.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if (LOWER (name[0]) == LOWER (flag_table[flag].name[0])
            && !str_prefix (name, flag_table[flag].name))
            return flag_table[flag].bit;
    }

    return NO_FLAG;
}


int position_lookup (const char *name)
{
    int pos;

    for (pos = 0; position_table[pos].name != NULL; pos++)
    {
        if (LOWER (name[0]) == LOWER (position_table[pos].name[0])
            && !str_prefix (name, position_table[pos].name))
            return pos;
    }

    return -1;
}

int sex_lookup (const char *name)
{
    int sex;

    for (sex = 0; sex_table[sex].name != NULL; sex++)
    {
        if (LOWER (name[0]) == LOWER (sex_table[sex].name[0])
            && !str_prefix (name, sex_table[sex].name))
            return sex;
    }

    return -1;
}

int size_lookup (const char *name)
{
    int size;

    for (size = 0; size_table[size].name != NULL; size++)
    {
        if (LOWER (name[0]) == LOWER (size_table[size].name[0])
            && !str_prefix (name, size_table[size].name))
            return size;
    }

    return -1;
}

/* returns race number */
int race_lookup (const char *name)
{
    int race;

    for (race = 0; race_table[race].name != NULL; race++)
    {
        if (LOWER (name[0]) == LOWER (race_table[race].name[0])
            && !str_prefix (name, race_table[race].name))
            return race;
    }

    return 0;
}

/* returns faction based on race */
int get_faction_by_race (int race)
{
    /* Races 1-4: human, elf, dwarf, verbeeg = FACTION_HUMAN */
    if (race >= 1 && race <= 12)
        return FACTION_HUMAN;
    
    /* Races 5-8: orc, felar, hobgoblin, troll = FACTION_ORCISH */
    if (race >= 13 && race <= 24)
        return FACTION_ORCISH;
    
    /* Default to none for invalid races */
    return FACTION_NONE;
}

int item_lookup (const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER (name[0]) == LOWER (item_table[type].name[0])
            && !str_prefix (name, item_table[type].name))
            return item_table[type].type;
    }

    return -1;
}

int liq_lookup (const char *name)
{
    int liq;

    for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
        if (LOWER (name[0]) == LOWER (liq_table[liq].liq_name[0])
            && !str_prefix (name, liq_table[liq].liq_name))
            return liq;
    }

    return -1;
}

HELP_DATA *help_lookup (char *keyword)
{
    HELP_DATA *pHelp;
    char temp[MIL], argall[MIL];

    argall[0] = '\0';

    while (keyword[0] != '\0')
    {
        keyword = one_argument (keyword, temp);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, temp);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
        if (is_name (argall, pHelp->keyword))
            return pHelp;

    return NULL;
}

HELP_AREA *had_lookup (char *arg)
{
    HELP_AREA *temp;
    extern HELP_AREA *had_list;

    for (temp = had_list; temp; temp = temp->next)
        if (!str_cmp (arg, temp->filename))
            return temp;

    return NULL;
}
