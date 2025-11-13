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
 *           bounty.c - November 13, 2025
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
****************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

/* Global variables */
BOUNTY_DATA	*bounty_list;

/* Command functions needed */
DECLARE_DO_FUN(do_say	);

/* Function definitions */
int     clan_lookup     args( (const char *name) );

/* And some local defines */
#define BOUNTY_PLACE_CHARGE	3
#define BOUNTY_REMOVE_CHARGE	100
#define BOUNTY_ADD_CHARGE	5
#define MIN_BOUNTY		10

void save_bounties()
{
  FILE *fp;
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  fclose(fpReserve);
  if ( ( fp = fopen( BOUNTY_FILE, "w" ) ) == NULL )
  {
    perror( BOUNTY_FILE );
  }

  for( bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    found = TRUE;
    fprintf(fp,"%-12s %d\n",bounty->name,bounty->amount);
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
  if(!found)
    unlink(BOUNTY_FILE);
}

void load_bounties()
{
  FILE *fp;
  BOUNTY_DATA *blist;
  char line[MAX_STRING_LENGTH];
  char name[MAX_INPUT_LENGTH];
  int amount;

  if ( ( fp = fopen( BOUNTY_FILE, "r" ) ) == NULL )
	return;

  blist = NULL;
  while ( fgets(line, sizeof(line), fp) != NULL )
  {
    BOUNTY_DATA *bounty;
    
    /* Skip empty lines */
    if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
      continue;
    
    /* Parse the line: name amount */
    if (sscanf(line, "%255s %d", name, &amount) != 2)
      continue;
    
    bounty = new_bounty();
    bounty->name = str_dup(name);
    bounty->amount = amount;

    if(bounty_list == NULL)
	bounty_list = bounty;
    else
	blist->next = bounty;
    blist = bounty;
  }
  
  fclose(fp);
}

bool is_bountied(CHAR_DATA *ch)
{
  BOUNTY_DATA *bounty;
  bool found = FALSE;

  if(IS_NPC(ch))
    return FALSE;

  for(bounty = bounty_list; bounty != NULL; bounty = bounty->next)
  {
    if( !str_cmp(ch->name,bounty->name) )
      found = TRUE;
  }

  return found;
}

void do_bounty(CHAR_DATA *ch, char *argument)
{
  FILE *fp;
  CHAR_DATA *vch;
  BOUNTY_DATA *bounty;
  BOUNTY_DATA *prev=NULL;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char strsave[MAX_STRING_LENGTH];
  char *word;
  bool found = FALSE;
  bool clanned = FALSE;
  int cash, amount, surcharge;

  if( IS_NPC(ch) )
    return;

  /* Check for the bounty collector */
  for(vch = ch->in_room->people; vch ; vch = vch->next_in_room)
  {
    if(IS_NPC(vch) && IS_SET(vch->act, ACT_BOUNTY))
      break;
  }

  if(vch == NULL)
  {
    send_to_char("You cannot do that here.\n\r",ch);
    return;
  }

  /* Only clanned chars/pkillers can place bounties. */
  if( !is_clan(ch) )
  {
    do_say(vch,"You must be in a clan to place or collect a bounty.");
    return;
  }

  argument = one_argument(argument, arg);
  one_argument(argument, arg2);

  if(arg[0] == '\0')
  {
    do_say(vch,"These are the bounties I will honor:");
    send_to_char("  Name              Bounty\n\r",ch);
    send_to_char("  ============      ===============\n\r",ch);
    for(bounty = bounty_list;bounty != NULL;bounty=bounty->next)
    {
      sprintf(buf,"  %-12s      %d\n\r",bounty->name,bounty->amount);
      send_to_char(buf,ch);
    }

    return;
  }

  if( arg2[0] == '\0' 
  ||  (str_cmp(arg,"remove") && str_cmp(arg,"collect") && !is_number(arg2)) )
  {
    send_to_char("Syntax: bounty\n\r",ch);
    send_to_char("        bounty remove <player>\n\r",ch);
    send_to_char("        bounty collect <player>\n\r",ch);
    send_to_char("        bounty <player> <amount>\n\r",ch);
    return;
  }

  /* For collecting bounties. */
  if( !str_cmp(arg,"collect") )
  {
    OBJ_DATA *obj;

    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(capitalize(arg2),capitalize(bounty->name)) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    for(obj = ch->carrying; obj != NULL; obj = obj->next)
    {
      if( is_name(arg2,obj->name)
      &&  (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD) )
	break;
    }

    if( obj == NULL )
    {
      do_say(vch,"I'm afraid I'll need proof of the death.");
      return;
    }

    extract_obj(obj);
    do_say(vch,"Very well, here is your payment.");
    ch->gold += bounty->amount;

    /*
     * OK, we've got a valid bounty and they've
     * been paid.  Time to remove the bounty.
     */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;

    free_bounty(bounty);
    save_bounties();
    return;
  }

  /* For buying off bounties. */
  if( !str_cmp(arg,"remove") )
  {
    for(bounty = bounty_list; bounty != NULL; prev=bounty, bounty=bounty->next)
    {
      if( !str_cmp(arg2,bounty->name) )
	break;
    }

    if( bounty == NULL )
    {
      do_say(vch,"That individual is not in the books.");
      return;
    }

    surcharge = BOUNTY_REMOVE_CHARGE;
    amount = bounty->amount+(bounty->amount*surcharge/100);
    cash = ch->gold + (ch->silver/100);

    if(cash < amount)
    {
      do_say(vch,"You cannot afford to remove that bounty.");
      return;
    }

    if(ch->silver < (amount*100))
    {
      amount -= (ch->silver/100);
      ch->silver %= 100;
    }
    else
    {
      ch->silver -= amount*100;
      amount = 0;
    }

    if(ch->gold < amount)
    {
      amount -= ch->gold;
      ch->gold = 0;
    }
    else
    {
      ch->gold -= amount;
      amount = 0;
    }

    /*
     * OK, we've got a valid bounty and they can
     * pay.  Time to remove the bounty.
     */
    if(prev == NULL)
      bounty_list = bounty_list->next;
    else
      prev->next = bounty->next;

    free_bounty(bounty);
    do_say(vch,"The name has been removed from the books.");
    save_bounties();
    return;
  }

  /*
   * This code allows you to bounty those
   * offline as well as those not visible.
   */

  fclose(fpReserve);
  sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(arg));
  if ( ( fp = fopen( strsave, "r" ) ) == NULL )
  {
    do_say(vch,"I am afraid there is no such person.");
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  strcpy(arg,capitalize(arg));

  while( !found )
  {
    char *clanName;

    word = feof(fp) ? "End" : fread_word( fp );

    if( !str_cmp(word, "Clan") )
    {
      found = TRUE;
      clanName = fread_string(fp);
      clanned = clan_lookup(clanName) ? TRUE : FALSE;
      free_string(clanName);
    }
    else if( !str_cmp(word, "End") )
    {
      found = TRUE;
    }
    else
    {
      fread_to_eol( fp );
    }
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  /* Only clanned chars can be bountied */
  if( !clanned )
  {
    do_say(vch,"You cannot place bounties on non-clanned characters.");
    return;
  }

  /* Minimum bounty, change as desired */
  if( (amount = atoi(arg2)) < MIN_BOUNTY)
  {
    sprintf(buf,"I do not accept bounties under %d gold.",MIN_BOUNTY);
    do_say(vch,buf);
    return;
  }

  /*
   * Char's cash total in gold, bounties
   * are always set in gold. Notice the
   */
  cash = ch->gold + (ch->silver/100);

  /*
   * Does the bounty already exist?
   * Let's find out.
   */
  for(bounty = bounty_list;bounty != NULL;bounty=bounty->next)
  {
    if( !str_cmp(bounty->name,arg) )
      break;
  }

  /*
   * Either create a new bounty
   * or add to an existing one.
   * Notice the charge added to
   * place a bounty and the
   * additional one to add to a
   * bounty.  Don't want to make
   * it too cheap.  Again, change
   * as desired.
   */

  if(bounty == NULL)
  {
    surcharge = BOUNTY_PLACE_CHARGE;
    if(cash < (amount+(amount*surcharge/100)))
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty = new_bounty();
    bounty->name = str_dup(arg);
    bounty->amount = amount;
    bounty->next = bounty_list;
    bounty_list = bounty;
    amount += amount*surcharge/100;
  }
  else
  {
    sprintf(buf,"There is a %d%% surcharge to add to an existing bounty.",BOUNTY_ADD_CHARGE);
    do_say(vch,buf);
    surcharge = BOUNTY_PLACE_CHARGE + BOUNTY_ADD_CHARGE;
    amount += amount*surcharge/100;
    if(cash < amount)
    {
      do_say(vch,"You cannot afford to place that bounty.");
      return;
    }
    bounty->amount += amount;
  }

  save_bounties();

  /*
   * Now make them pay, use up silver first
   * then go to gold.
   */
  if(ch->silver < (amount*100))
  {
    amount -= (ch->silver/100);
    ch->silver %= 100;
  }
  else
  {
    ch->silver -= amount*100;
    amount = 0;
  }

  if(ch->gold < amount)
  {
    amount -= ch->gold;
    ch->gold = 0;
  }
  else
  {
    ch->gold -= amount;
    amount = 0;
  }

  do_say(vch,"Your bounty has been recorded in the books.");
}