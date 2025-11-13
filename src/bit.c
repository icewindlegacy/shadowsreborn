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
 *           bit.c - November 13, 2025
 */
/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */



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
#include "tables.h"
#include "lookup.h"


struct flag_stat_type {
    const struct flag_type *structure;
    bool stat;
};



/*****************************************************************************
 Name:        flag_stat_table
 Purpose:    This table catagorizes the tables following the lookup
         functions below into stats and flags.  Flags can be toggled
         but stats can only be assigned.  Update this table when a
         new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] = {
/*  {    structure        stat    }, */
    {area_flags, FALSE},
    {sex_flags, TRUE},
    {exit_flags, FALSE},
    {door_resets, TRUE},
    {room_flags, FALSE},
    {sector_flags, TRUE},
    {type_flags, TRUE},
    {extra_flags, FALSE},
    {wear_flags, FALSE},
    {act_flags, FALSE},
    {affect_flags, FALSE},
    {apply_flags, TRUE},
    {wear_loc_flags, TRUE},
    {wear_loc_strings, TRUE},
    {container_flags, FALSE},

/* ROM specific flags: */

    {form_flags, FALSE},
    {part_flags, FALSE},
    {ac_type, TRUE},
    {size_flags, TRUE},
    {position_flags, TRUE},
    {off_flags, FALSE},
    {imm_flags, FALSE},
    {res_flags, FALSE},
    {vuln_flags, FALSE},
    {weapon_class, TRUE},
    {weapon_type2, FALSE},
    {apply_types, TRUE},
    {0, 0}
};



/*****************************************************************************
 Name:        is_stat( table )
 Purpose:    Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:    flag_value and flag_string.
 Note:        This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat (const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_stat_table[flag].structure; flag++)
    {
        if (flag_stat_table[flag].structure == flag_table
            && flag_stat_table[flag].stat)
            return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 Name:        flag_value( table, flag )
 Purpose:    Returns the value of the flags entered.  Multi-flags accepted.
 Called by:    olc.c and olc_act.c.
 ****************************************************************************/
int flag_value (const struct flag_type *flag_table, char *argument)
{
    char word[MAX_INPUT_LENGTH];
    int bit;
    int marked = 0;
    bool found = FALSE;

    if (is_stat (flag_table))
        return flag_lookup (argument, flag_table);

    /*
     * Accept multiple flags.
     */
    for (;;)
    {
        argument = one_argument (argument, word);

        if (word[0] == '\0')
            break;

        if ((bit = flag_lookup (word, flag_table)) != NO_FLAG)
        {
            SET_BIT (marked, bit);
            found = TRUE;
        }
    }

    if (found)
        return marked;
    else
        return NO_FLAG;
}



/*****************************************************************************
 Name:        flag_string( table, flags/stat )
 Purpose:    Returns string with name(s) of the flags or stat entered.
 Called by:    act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string (const struct flag_type *flag_table, int bits)
{
    static char buf[2][512];
    static int cnt = 0;
    int flag;

    if (++cnt > 1)
        cnt = 0;

    buf[cnt][0] = '\0';

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if (!is_stat (flag_table) && IS_SET (bits, flag_table[flag].bit))
        {
            strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[flag].name);
        }
        else if (flag_table[flag].bit == bits)
        {
            strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[flag].name);
            break;
        }
    }
    return (buf[cnt][0] != '\0') ? buf[cnt] + 1 : "none";
}
