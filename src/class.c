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
 *           class.c - November 3, 2025
 */            
/* Online setting of skill/spell levels, 
 * (c) 1996 Erwin S. Andreasen <erwin@andreasen.org>
 *
 */

 #include "include.h" /* This includes all the usual include files */

 /* 
 
   Class table levels loading/saving
   
 */
 
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
     
     
     for (lev = 0; lev < LEVEL_IMMORTAL; lev++)
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
    int base_class = -1;
    
    /* Map orcish classes to their base human classes */
    switch (num)
    {
        case 4: base_class = 0; break; /* sorcerer -> mage */
        case 5: base_class = 1; break; /* shaman -> cleric */
        case 6: base_class = 2; break; /* knave -> thief */
        case 7: base_class = 3; break; /* berserker -> warrior */
    }
    
    sprintf (buf, "%s%s", CLASS_DIR, class_table[num].name);
    
    if (!(fp = fopen (buf, "r")))
    {
        /* If orcish class file doesn't exist, try base class file */
        if (base_class >= 0)
        {
            sprintf (buf, "%s%s", CLASS_DIR, class_table[base_class].name);
            if (!(fp = fopen (buf, "r")))
            {
                bugf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
                return;
            }
        }
        else
        {
            bugf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
            return;
        }
    }
     
     fscanf (fp, "%d", &level);
     
     while (level != -1)
     {
         fscanf (fp, " %[^\n]\n", buf); /* read name of skill into buf */
         
         n = skill_lookup (buf); /* find index */
         
         if (n == -1)
         {
            char buf2[200];
            snprintf (buf2, sizeof(buf2), "Class %s: unknown spell %.100s", class_table[num].name, buf);
             bug (buf2, 0);
         }
         else
             skill_table[n].skill_level[num] = level;
 
         fscanf (fp, "%d", &level);
     }
     
     fclose (fp);
 }
     
 void load_classes ()
 {
     int i,j;
 
     for (i = 0; i < MAX_CLASS; i++)
     {
         for (j = 0; j < MAX_SKILL; j++)
             skill_table[j].skill_level[i] = LEVEL_IMMORTAL;
         
         load_class (i);
     }
 }
 
 
 
 void do_skill (CHAR_DATA *ch ,char * argument)
 {
     char class_name[MIL], skill_name[MIL];
     int sn, level, class_no;
     
     argument = one_argument (argument, class_name);
     argument = one_argument (argument, skill_name);
     
     if (!argument[0])
     {
         send_to_char ("Syntax is: ISKILL <class> <skill> <level>.\n\r",ch);
         return;
     }
     
     level = atoi (argument);
     
     if (!is_number(argument) || level < 0 || level > LEVEL_IMMORTAL)
     {
         printf_to_char (ch, "Level range is from 0 to %d.\n\r", LEVEL_IMMORTAL);
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
     
     skill_table[sn].skill_level[class_no] = level;
     
     printf_to_char (ch, "OK, %ss will now gain %s at level %d%s.\n\r", class_table[class_no].name,
                                 skill_table[sn].name, level, level == LEVEL_IMMORTAL ? " (i.e. never)" : "");
     
     
     save_classes();
 }
 
 