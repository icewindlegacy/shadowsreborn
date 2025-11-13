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
 *           windgate.c - November 13, 2025
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "windgate.h"

/* Forward declarations */
char * return_true_false(bool value);

/*
 * windgate_table()
 *
 * All windgates =
 */
const struct windgate_type windgate_table[] =
{
    {"Midgaard",   3025,
    {SYMBOL_LINE, SYMBOL_CIRCLE, SYMBOL_LINE},
    NO_MIN_LEVEL,
    FALSE,      FALSE,      -1,     -1},
    
    {"Old Thalos",   5300,
        {SYMBOL_LINE, SYMBOL_MOON, SYMBOL_SUN},
        NO_MIN_LEVEL,
        FALSE,      FALSE,      -1,     -1},
    
    {"Mega City One",   8001,
    {SYMBOL_CIRCLE, SYMBOL_STAR, SYMBOL_PENTAGON},
    NO_MIN_LEVEL,
    FALSE,      FALSE,      -1,     -1},
    
    {"Elemental Canyon",   9201,
    {SYMBOL_MOON, SYMBOL_TRIANGLE, SYMBOL_LINE},
    NO_MIN_LEVEL,
    FALSE,      FALSE,      -1,     -1},

    {"Drow City",   5100,
    {SYMBOL_SQUARE, SYMBOL_CIRCLE, SYMBOL_SQUARE},
    16,
    FALSE,      FALSE,      -1,     -1},
    
    {NULL, -1, {-1, -1, -1}, -1, FALSE, FALSE, -1, -1}
};

/* symbol_table[]
 * Uh, symbols, easier this way
 */

const struct symbol_type symbol_table[] =
{
    {"none",        SYMBOL_NONE,        "with boggled look on your face"},
    {"Fehu",      SYMBOL_SQUARE,      "with a {Rreddish{x glow"},
    {"Uruz",    SYMBOL_TRIANGLE,    "with a {Ggreenish{x glow"},
    {"Thurisaz",      SYMBOL_CIRCLE,      "with a {Wwhite{x glow"},
    {"Ansuz",        SYMBOL_STAR,        "with a {Dgrey{x glow"},
    {"Raidho",    SYMBOL_PENTAGON,    "with a {Ddark{x light"},
    {"Kenaz",        SYMBOL_LINE,        "with a {Ppurple{x glow"},
    {"Gebo",         SYMBOL_SUN,         "with a {Ygolden{x glow"},
    {"Wunjo",        SYMBOL_MOON,        "with a {Bblueish{x glow"},
    {"Hagalaz",        SYMBOL_HAND,        "with a loud hum"},
    {NULL, -1}
};

/*
 * cmd_press()
 *
 * I like stargate. So Pfft on all of you.
 * Ok so this is how it works, the player 'presses' the symbols in order
 * to open up the gate. If it works, the gate activates and creates a portal
 * of types to that location.
 */
void cmd_press(CHAR_DATA * ch, char *argument)
{
    char arg1[MSL];
    char buf[MSL];
    OBJ_DATA * obj;
    bool IS_WINDGATE = FALSE;
    int symbol;
    int i;
    
    argument = one_argument (argument, arg1);
    
    for (obj = ch->in_room->contents; obj; obj = obj->next_content)
    {
        if (obj->item_type == ITEM_WINDGATE)
        {
            IS_WINDGATE = TRUE;
            break;
        }
    }
    
    /* Check if there's a windgate here */
    if (!IS_WINDGATE || obj == NULL)
    {
        stc("You cannot find a windgate here.\n", ch);
        return;
    }
    
    if (arg1[0] == '\0')
    {
        stc("Which symbol do you wish to press?\n", ch);
        return;
    }
    
    /* Ok find the symbol from the list */
    symbol = -1;
    for (i = 0; i < MAX_SYMBOL; i++)
    {
        if (symbol_table[i].name == NULL)
            break;
        if (LOWER (arg1[0]) == LOWER (symbol_table[i].name[0]) && !str_prefix (arg1, symbol_table[i].name))
            symbol = i;
    }
    
    if (symbol == -1)
    {
        stc("That is NOT a rune!\nValid runes are:\n", ch);

        for (i = 0; i < MAX_SYMBOL; i++)
        {
            if (symbol_table[i].name == NULL)
                break;
            sprintf(buf, "%s ", symbol_table[i].name);
            stc(buf, ch);
        }
        stc("\n", ch);
        return;
    }
    
    sprintf(buf, "You press the %s rune and it lights up with %s.\n", 
        symbol_table[symbol].name,
        symbol_table[symbol].press_string);
    stc(buf, ch);
    
    
    fix_lights(ch, obj, symbol);
    
    if (obj->value[4] == 0)
    {
        sprintf(buf, "Currently, the %s, %s and %s lights are lit up.\n", 
            symbol_table[obj->value[0]].name,
            symbol_table[obj->value[1]].name,
            symbol_table[obj->value[2]].name);
        stc(buf, ch);
    }
    
}

/*
 * fix_lights()
 *
 *  Ugh, handle the lighting of the windgate
 */
void fix_lights(CHAR_DATA * ch, OBJ_DATA * obj, int symbol)
{
    if (obj->value[0] == 0)
    {
        obj->value[0] = symbol;
        return;
    }
    if (obj->value[1] == 0)
    {
        obj->value[1] = symbol;
        return;
    }
    if (obj->value[2] == 0)
    {
        obj->value[2] = symbol;
        /* Last symbol, check to see if it activates the gate! */
        check_valid_sequence(ch, obj);
        return;
    }
    /* Ok, so no possible symbol availible, lets reset the thing */
    reset_windgate(ch, obj);
    return;
}

/*
 * reset_windgate()
 *
 * Simple utility to reset the gate. Used on invalid sequence, and on boot
 */
void reset_windgate(CHAR_DATA * ch, OBJ_DATA * obj)
{
    obj->value[0] = 0;
    obj->value[1] = 0;
    obj->value[2] = 0;
    obj->value[3] = 0;
    obj->value[4] = 0;
    stc("\n\nAll the lights on the gate fade.\n\n", ch);
    return;
}

/*
 * check_valid_sequence()
 *
 * checks to see if the windgate's values match up with a valid location, if so
 * open the gate up, and reset the symbols
 */
void check_valid_sequence(CHAR_DATA * ch, OBJ_DATA * obj)
{
    int i;
    char buf[MSL];

    for (i = 0; i <= MAX_GATES; i++)
    {
        if (windgate_table[i].name == NULL)
            return;
        
        if (windgate_table[i].symbol[0] == obj->value[0] &&
            windgate_table[i].symbol[1] == obj->value[1] &&
            windgate_table[i].symbol[2] == obj->value[2] &&
            ch->level >= windgate_table[i].min_level     &&
            !windgate_table[i].disabled)
        {   /* Ok checks out, open the gate */
            reset_windgate(ch, obj);
            sprintf(buf, "A strange swirling vortex opens up in the center of the runic circle, leading to %s.\n\r",
                windgate_table[i].name);
            stc(buf, ch);
            obj->value[4] = windgate_table[i].vnum;
            return;
        }
    }

    stc("All the lights on the circle fade.\n\r", ch);
    reset_windgate(ch, obj);
    return;
}
            
    

/*
 * cmd_windgate()
 *
 * Immortal utility to see all windgates
 */
void cmd_windgate(CHAR_DATA * ch, char *argument)
{
    int i;
    char buf[MSL];
    
    stc(" Current Runic Circles in the universe...\n\n", ch);
    stc("Circle Name        |  Rune 1     Rune 2     Rune 3     | Min | Disabl | Wild  | X/Y\n", ch);
    stc("-------------------|-----------------------------------|-----|-------|-------|------\n", ch);
    for (i = 0; i <= MAX_GATES; i++)
    {
        if (windgate_table[i].name == NULL)
            return;
        
        sprintf(buf, "%-18s | %-11s %-10s %-10s | %-3d | %-5s | %-5s | %d/%d\n",
            windgate_table[i].name,
            symbol_table[windgate_table[i].symbol[0]].name,
            symbol_table[windgate_table[i].symbol[1]].name,
            symbol_table[windgate_table[i].symbol[2]].name,
            windgate_table[i].min_level,
            return_true_false(windgate_table[i].disabled),
            return_true_false(windgate_table[i].to_wild),
            windgate_table[i].x,
            windgate_table[i].y);
        stc(buf, ch);
    }
    stc("That is all the runic circles we have!\n\r", ch);
    return;
}

char * return_true_false(bool value)
{
    return value ? "TRUE" : "FALSE";
}

char * return_gatesymbol(int symbol)
{
    return symbol_table[symbol].name;
}
