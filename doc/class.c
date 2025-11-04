/***************************************************************************
 *  Online Class/Skill Level Management                                    *
 *  (c) 1996 Erwin S. Andreasen <erwin@andreasen.org>                      *
 *  Allows online modification of skill/spell levels for classes           *
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
#include <time.h>
#include "merc.h"

/* Save this class */
void save_class (int num)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int lev, i;
    
    sprintf (buf, "%s%s", CLASS_DIR, class_table[num].name);
    
    if (!(fp = fopen (buf, "w")))
    {
        bugf ("Could not open file %s in order to save class %s.", buf, class_table[num].name);
        return;
    }
    
    
    for (lev = 0; lev < LEVEL_HERO; lev++)
        for (i = 0; i < MAX_SKILL; i++)
        {
            if (!skill_table[i].name || !skill_table[i].name[0])
                continue;
                
            if (skill_table[i].skill_level[num] == lev)
                fprintf (fp, "%d %s\n", lev, skill_table[i].name);
        }
    
    fprintf (fp, "-1"); /* EOF -1 */
    fclose (fp);
}

void save_classes()
{
    int i;
    
    for (i = 0; i < MAX_CLASS; i++)
        save_class (i);
}

/* Load a class */
void load_class (int num)
{
    char buf[MAX_STRING_LENGTH];
    int level,n;
    FILE *fp;
    
    sprintf (buf, "%s%s", CLASS_DIR, class_table[num].name);
    
    if (!(fp = fopen (buf, "r")))
    {
        bugf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
        return;
    }
    
    fscanf (fp, "%d", &level);
    
    while (level != -1)
    {
        fscanf (fp, " %[^\n]\n", buf); /* read name of skill into buf */
        
        n = skill_lookup (buf); /* find index */
        
        if (n == -1)
        {
            char buf2[200];
            sprintf (buf2, "Class %s: unknown spell %s", class_table[num].name, buf);
            bug (buf2, 0);
        }
        else
            /* Cast away const to allow runtime modification */
            ((SKILL_DATA *)skill_table)[n].skill_level[num] = level;

        fscanf (fp, "%d", &level);
    }
    
    fclose (fp);
}
    
void load_classes ()
{
    int i;

    for (i = 0; i < MAX_CLASS; i++)
    {
        int j;
        /* Cast away const to allow runtime modification */
        for (j = 0; j < MAX_SKILL; j++)
            ((SKILL_DATA *)skill_table)[j].skill_level[i] = LEVEL_HERO;
        
        load_class (i);
    }
}

void do_skill (CHAR_DATA *ch ,char * argument)
{
    char class_name[MAX_INPUT_LENGTH], skill_name[MAX_INPUT_LENGTH];
    int sn, level, class_no;
    
    argument = one_argument (argument, class_name);
    argument = one_argument (argument, skill_name);
    
    if (!argument[0])
    {
        send_to_char ("Syntax is: SKILL <class> <skill> <level>.\n\r",ch);
        return;
    }
    
    level = atoi (argument);
    
    if (!is_number(argument) || level < 0 || level > LEVEL_HERO)
    {
        printf_to_char (ch, "Level range is from 0 to %d.\n\r", LEVEL_HERO);
        return;
    }
    
    
    if ( (sn = skill_lookup (skill_name)) == -1)
    {
        printf_to_char (ch, "There is no such spell/skill as '%s'.\n\r", skill_name);
        return;
    }
    
    for (class_no = 0; class_no < MAX_CLASS; class_no++)
        if (!str_cmp(class_name, class_table[class_no].who_name))
            break;
    
    if (class_no == MAX_CLASS)
    {
        printf_to_char (ch, "No class named '%s' exists. Use the 3-letter WHO names (Psi, Mag etc.)\n\r", class_name);
        return;
    }
    
    /* Cast away const to allow runtime modification */
    ((SKILL_DATA *)skill_table)[sn].skill_level[class_no] = level;
    
    printf_to_char (ch, "Skill '%s' level for class '%s' set to %d.\n\r", 
                   skill_name, class_name, level);
    
    save_classes();
}

