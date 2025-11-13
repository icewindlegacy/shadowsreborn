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
 *           wizlist.c - November 13, 2025
 */
/***************************************************************************
 *  Wizlist system - displays immortal staff in a formatted list          *
 *  Based on code by various ROM contributors                              *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
    #include <sys/types.h>
    #if defined(WIN32)
        #include <time.h>
    #else
        #include <sys/time.h>
    #endif
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

WIZ_DATA *wiz_list;

char *    const    wiz_titles    [] =
{
    "Owner            ",
    "Coders           ",
    "Head Builder     ",
    "Senior Builder   ",
    "Admin            ",
    "Lead Questrunner ",
    "Builder          ",
    "Questrunner      ",
    "Trainee          "
};

/*
 * Local functions.
 */
void    change_wizlist    args( (CHAR_DATA *ch, bool add, int level, char *argument));


void save_wizlist(void)
{
    WIZ_DATA *pwiz;
    FILE *fp;
    bool found = FALSE;

    fclose( fpReserve ); 
    if ( ( fp = fopen( WIZ_FILE, "w" ) ) == NULL )
    {
        perror( WIZ_FILE );
    }

    for (pwiz = wiz_list; pwiz != NULL; pwiz = pwiz->next)
    {
        found = TRUE;
        fprintf(fp,"%s %d\n",pwiz->name,pwiz->level);
     }

     fclose(fp);
     fpReserve = fopen( NULL_FILE, "r" );
     if (!found)
        unlink(WIZ_FILE);
}

void load_wizlist(void)
{
    FILE *fp;
    WIZ_DATA *wiz_last;

    if ( ( fp = fopen( WIZ_FILE, "r" ) ) == NULL )
    {
        return;
    }

    wiz_last = NULL;
    for ( ; ; )
    {
        WIZ_DATA *pwiz;
        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }

        pwiz = new_wiz();

        pwiz->name = str_dup(fread_word(fp));
        pwiz->level = fread_number(fp);
        fread_to_eol(fp);

        if (wiz_list == NULL)
            wiz_list = pwiz;
        else
            wiz_last->next = pwiz;
        wiz_last = pwiz;
    }
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char title[MAX_STRING_LENGTH];
    BUFFER *buffer;
    int level;
    WIZ_DATA *pwiz;
    int lngth;
    int amt;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ((arg1[0] != '\0') && (ch->level == MAX_LEVEL))
    {
        if ( !str_prefix( arg1, "add" ) )
        {
            if ( !is_number( arg2 ) || ( arg3[0] == '\0' ) )
            {
                send_to_char( "Syntax: wizlist add <level> <name>\n\r", ch );
                return;
            }
            level = atoi(arg2);
            change_wizlist( ch, TRUE, level, arg3 );
            return;
        }
        if ( !str_prefix( arg1, "delete" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_char( "Syntax: wizlist delete <name>\n\r", ch );
                return;
            }
            change_wizlist( ch, FALSE, 0, arg2 );
            return;
        }
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "       wizlist delete <name>\n\r", ch );
        send_to_char( "       wizlist add <level> <name>\n\r", ch );
        return;
    }

    if (wiz_list == NULL)
    {
        send_to_char("No immortals listed at this time.\n\r",ch);
        return;
    }
    buffer = new_buf();
    sprintf(title,"    Immortal Staff of {CShadows Reborn  {x");
    sprintf(buf,"{x  ___________________________________________________________________________\n\r");
    add_buf(buffer,buf);
    sprintf(buf,"{x /\\_\\%70s\\_\\\n\r", " ");
    add_buf(buffer,buf);
    lngth = (80 - strlen(title))/2;
    for( ; lngth >= 0; lngth--)
    {
        strcat(title, " ");
    }
    sprintf(buf,"|/\\\\_\\{B%74s{x\\_\\\n\r", title);
    add_buf(buffer,buf);
    sprintf(buf,"{x\\_/_|_|%69s|_|\n\r", " ");
    add_buf(buffer,buf);
    for (level = IMPLEMENTOR; level > HERO; level--)
    {
        found = FALSE;
        amt = 0;
        for (pwiz = wiz_list;pwiz != NULL;pwiz = pwiz->next)
        {
            if (pwiz->level == level)
            {
                amt++;
                found = TRUE;
            }
        }
        if (!found)
        {
            if (level == HERO+1)
            {
                sprintf(buf,"{x ___|_|%69s|_|\n\r", " ");
                add_buf(buffer,buf);
            }
            continue;
        }
        sprintf(buf,"{x    |_|{c%37s {B[%d]{x%27s|_|\n\r",
            wiz_titles[IMPLEMENTOR-level], level, " ");
        add_buf(buffer,buf);
        sprintf(buf,"{x    |_|{m%20s***********************{x%26s|_|\n\r",
            " ", " ");
        add_buf(buffer,buf);
        lngth = 0;
        for (pwiz = wiz_list;pwiz != NULL;pwiz = pwiz->next)
        {
            if (pwiz->level == level)
            {
                if (lngth == 0)
                {
                    if (amt > 2)
                    {
                        sprintf(buf, "{x    |_|{%s%12s%-17s ",
                            level >= DEMI ? "B" : "c", " ",
                            pwiz->name);
                        add_buf(buffer, buf);
                        lngth = 1;
                    } else if (amt > 1)
                    {
                        sprintf(buf, "{x    |_|{%s%21s%-17s ",
                            level >= DEMI ? "B" : "c", " ",
                            pwiz->name);
                        add_buf(buffer, buf);
                        lngth = 1;
                    } else
                    {
                        sprintf(buf, "{x    |_|{%s%30s%-39s{x|_|\n\r",
                            level >= DEMI ? "B" : "c", " ",
                            pwiz->name);
                        add_buf(buffer, buf);
                        lngth = 0;
                    }
                } else if (lngth == 1)
                {
                    if (amt > 2)
                    {
                        sprintf(buf, "%-17s ",
                            pwiz->name);
                        add_buf(buffer, buf);
                        lngth = 2;
                    } else
                    {
                        sprintf(buf, "%-30s{x|_|\n\r",
                            pwiz->name);
                        add_buf(buffer, buf);
                        lngth = 0;
                    }
                } else
                {
                    sprintf(buf, "%-21s{x|_|\n\r",
                        pwiz->name);
                    add_buf(buffer, buf);
                    lngth = 0;
                    amt -= 3;
                }
            }
        }
        if (level == HERO+1)
        {
            sprintf(buf,"{x ___|_|%69s|_|\n\r", " ");
        } else
        {
            sprintf(buf,"{x    |_|%69s|_|\n\r", " ");
        }
        add_buf(buffer,buf);
    }
    sprintf(buf,"{x/ \\ |_|%69s|_|\n\r", " ");
    add_buf(buffer,buf);
    sprintf(buf,"{x|\\//_/%70s/_/\n\r", " ");
    add_buf(buffer,buf);
    sprintf(buf,"{x \\/_/______________________________________________________________________/_/\n\r");
    add_buf(buffer,buf);
    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);
    return;
}

void update_wizlist(CHAR_DATA *ch, int level)
{
    WIZ_DATA *prev;
    WIZ_DATA *curr;

    if (IS_NPC(ch))
    {
        return;
    }
    prev = NULL;
    for ( curr = wiz_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( ch->name, curr->name ) )
        {
            if ( prev == NULL )
                wiz_list   = wiz_list->next;
            else
                prev->next = curr->next;

            free_wiz(curr);
            save_wizlist();
        }
    }
    if (level <= HERO)
    {
        return;
    }
    curr = new_wiz();
    curr->name = str_dup(ch->name);
    curr->level = level;
    curr->next = wiz_list;
    wiz_list = curr;
    save_wizlist();
    return;
}

void change_wizlist(CHAR_DATA *ch, bool add, int level, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    WIZ_DATA *prev;
    WIZ_DATA *curr;
     
    one_argument( argument, arg );
    if (arg[0] == '\0')
    {
        send_to_char( "Syntax:\n\r", ch );
        if ( !add )
            send_to_char( "    wizlist delete <name>\n\r", ch );
        else
            send_to_char( "    wizlist add <level> <name>\n\r", ch );
        return;
    }
    if ( add )
    {
        if ( ( level <= HERO ) || ( level > MAX_LEVEL ) )
        {
            send_to_char( "Syntax:\n\r", ch );
            send_to_char( "    wizlist add <level> <name>\n\r", ch );
            return;
        }
    }
    if ( !add )
    {
        prev = NULL;
        for ( curr = wiz_list; curr != NULL; prev = curr, curr = curr->next )
        {
            if ( !str_cmp( capitalize( arg ), curr->name ) )
            {
                if ( prev == NULL )
                    wiz_list   = wiz_list->next;
                else
                    prev->next = curr->next;

                free_wiz(curr);
                save_wizlist();
            }
        }
    } else
    {
        curr = new_wiz();
        curr->name = str_dup( capitalize( arg ) );
        curr->level = level;
        curr->next = wiz_list;
        wiz_list = curr;
        save_wizlist();
    }
    return;
}

