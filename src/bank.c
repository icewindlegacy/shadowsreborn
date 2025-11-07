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
 *           bank.c - November 3, 2025
 */            
/* **************************************************************
  January 25th, 1998
  Gothar's Bank Code Version 1.2
  1997 Copyright <* FREEWARE *>
  If you use this code:
     1.Keep all the credits in the code.
     2.Use the help entry for bank
     3.Send a bug report,any comments or ideas
       Ian McCormick (aka Gothar)
       mcco0055@algonquinc.on.ca

Here is a listing of what the code does:
     1.Give some RP to your thieves!
       - must go to their fence in the thieves guild.
     2.Deposit gold or silver into your account
     3.Withdraw gold or silver into your account
     4.Check your account
       -shows shares bought, duration effects
        gold & silver in account and on person
     5.Midgaard Shares to enhance spell duration
       -You can buy shares and sell them at reduced rate.
       -Shares are bought buy bank gold only! 
     6.Change silver into gold 
       -thief's haggle skill an asset!
        Haggle skill lowers the fence's profit 
        from the currency change
        No percentage taken by GBS for curreny exchange.
     7.Let's you still us the Changer bit for something
        if you want.  
NOTE: This code leaves the the IS_CHANGER stuff in do_give
      so you still can use Otho's services for money exchange. 
************************************************************** */
/* N.B: READ this code before you ADD anything!!
 *      This is bank.c plus a patch file for other files.
 */
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

/*
 *  Room vnums used for your Bank.
 *  In this case it is the Temple in Midgaard.
 *  Remember to add the actual room to your MUD
 *  These are defined in merc.h - update them there if needed
 */
#ifndef ROOM_VNUM_BANK
#define ROOM_VNUM_BANK	      3008 
#endif
#ifndef ROOM_VNUM_BANK_THIEF
#define ROOM_VNUM_BANK_THIEF  3029 
#endif
#ifndef ROOM_VNUM_ORC_BANK
#define ROOM_VNUM_ORC_BANK  4000
#endif
#ifndef ROOM_VNUM_ORC_BANK_THIEF
#define ROOM_VNUM_ORC_BANK_THIEF  462
#endif
/* pick any room in the thieves guild */

#define SHARE_MAX	5 /* max shares per level */

/* command procedures needed */
DECLARE_DO_FUN( do_help		);

void do_account (CHAR_DATA *ch, char *argument)
{
   long gold = 0, silver = 0, 
        shares = 0;
   int diff, bonus = 0;
   char buf[MAX_STRING_LENGTH];
   char const *class = class_table[ch->class].name;

  /* No NPC's, No pets, No imms,
   * No chainmail, No service! 
   */
   if( IS_NPC(ch) || IS_IMMORTAL(ch) )
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }

   gold = ch->pcdata->gold_bank;
   silver = ch->pcdata->silver_bank;
   shares = ch->pcdata->shares;
   diff = (SHARE_MAX - ch->pcdata->shares_bought);
   if( ch->pcdata->shares / 4 != 0)
      bonus = shares / 4;
   ch->pcdata->duration = bonus;
   if(!str_cmp(class, "thief"))
   {
      sprintf( buf,"Your guild's fence tells you.\n\rYou have in your beltpouch:\n\r"
                   "  Gold:   %10ld\n\r  Silver: %10ld\n\r" 
                   "Under your bedroll:\n\r"
                   "  Gold:   %10ld\n\r  Silver: %10ld\n\r",ch->gold, ch->silver,gold, silver); 
   }
   else
   {
      sprintf( buf,"Your Midgaard Account records show:\n\rYou have in your beltpouch:\n\r"
                   "  Gold:   %10ld\n\r  Silver: %10ld\n\r" 
                   "In your Account:\n\r"
                   "  Gold:   %10ld\n\r  Silver: %10ld\n\r",ch->gold, ch->silver,gold, silver); 
   }
   send_to_char(buf, ch);
   if( ch->level >= 10 && str_cmp(class, "thief") )
   {
      sprintf( buf,"  Shares: %10ld with a +%d Hours of spell duration.\n\r",shares, bonus); 
      send_to_char(buf, ch);
      sprintf( buf,"  Shares left at level %d: %d.\n\r",ch->level, diff);
      send_to_char(buf, ch);
   }
   return;
}


void do_deposit (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char const *class = class_table[ch->class].name;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   /* No NPC's, No imms,
    * No chainmail, No service! 
    */
   if( IS_NPC(ch) || IS_IMMORTAL(ch) )
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }
   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
      && str_cmp(class, "thief"))
   {
      send_to_char("You must be in the Bank to deposit.\n\r",ch);
      return;
   }
   else if(ch->in_room != get_room_index(ROOM_VNUM_BANK_THIEF)
           && !str_cmp(class, "thief"))
   {
         send_to_char("You must be at your fence to deposit.\n\r", ch);
         return;
   }
   else  /* In the Bank */
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      if (arg1[0] == '\0' || arg2[0] == '\0')
      {
         send_to_char("How much to squirrel away?\n\r", ch );
         if(!str_cmp(class, "thief"))
         {
            send_to_char("THIEF: Deposit <value> gold\n\r",ch);
            send_to_char("       Deposit <value> silver\n\r",ch);
         }
         else
         {
            send_to_char("GBS: Deposit <value> gold\n\r",ch);
            send_to_char("     Deposit <value> silver\n\r",ch);
         }
         send_to_char("For more information Type 'bank'.\n\r",ch);
         return;
      }
      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
         if(!str_cmp(class, "thief"))
         {
            act("A thief lurks in the shadows here.", ch,NULL,NULL, TO_ROOM);
            send_to_char("GBS: You aren't allowed in Here, you'd better go!\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	          send_to_char( "GBS: To deposit you must give money!\n\r", ch );
                  send_to_char( "     For more information Type 'Bank'.\n\r",ch);
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->gold < amount)
                  {
	             send_to_char("GBS: You don't have that much gold.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank += amount;
                     ch->gold -= amount;
                     act("$n deposits gold into $s account.", ch,NULL,NULL,TO_ROOM);
                     sprintf( buf, "GBS: You have deposited %ld Gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                              amount, ch->pcdata->gold_bank, ch->gold);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->silver < amount)
                  {
	             send_to_char("GBS: You don't have that much silver.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank += amount;
                     ch->silver -= amount;
                     act("$n deposits silver into $s account.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "GBS: You have deposited %ld Silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
         }
      }
      else if(ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF))
      {
         if(str_cmp(class, "thief"))
         {
            act("A raider is Here to kill the Guildmaster.", ch,NULL,NULL, TO_ROOM);
            send_to_char("You aren't allowed in Here, you'd better go!\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	          send_to_char( "THIEF: To deposit you must give money!\n\r", ch );
                  send_to_char( "       For more information Type 'Bank'.\n\r",ch);
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->gold < amount)
                  {
	             send_to_char("THIEF: You don't have that much gold.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank += amount;
                     ch->gold -= amount;
                     act("$n hides gold under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have hidden %ld Gold.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->gold_bank, ch->gold);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->silver < amount)
                  {
	             send_to_char("THIEF: You don't have that much silver.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank += amount;
                     ch->silver -= amount;
                     act("$n hides silver under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have hidden %ld Silver.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("The type of currency must be stated before the amount.\n\r",ch);
            }
         }
      }
      else
      {
         bug( "Do_deposit: Bank doesn't exist.", 0 );
         send_to_char( "Bank doesn't exist.\n\r", ch );
         return;
      } 
   }
   return;
}


void do_withdraw (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char const *class = class_table[ch->class].name;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   /* No NPC's, No imms,
    * No chainmail, No service! 
    */
   if( IS_NPC(ch) || IS_IMMORTAL(ch) )
   {
      send_to_char("Only players need money!\n\r", ch);
      return;
   }
   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
      && str_cmp(class, "thief"))
   {
      send_to_char("You must be in the Bank to withdraw.\n\r",ch);
      return;
   }
   else if(ch->in_room != get_room_index(ROOM_VNUM_BANK_THIEF)
           && !str_cmp(class, "thief"))
   {
         send_to_char("You must be at your fence to withdraw.\n\r", ch);
         return;
   }
   else  /* In the Bank */
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      if (arg1[0] == '\0'|| arg2[0] == '\0')
      {
         send_to_char("How much to withdraw away?\n\r", ch );
         if(!str_cmp(class, "thief"))
         {
            send_to_char("THIEF: Withdraw <value> gold\n\r",ch);
            send_to_char("       Withdraw <value> silver\n\r",ch);
         }
         else
         {
            send_to_char("GBS: Withdraw <value> gold\n\r",ch);
            send_to_char("     Withdraw <value> silver\n\r",ch);
         }
         send_to_char("For more information Type 'Bank'.\n\r",ch);
         return;
      }
      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
         if(!str_cmp(class, "thief"))
         {
            act("A thief lurks in the shadows here.", ch,NULL,NULL, TO_ROOM);
            send_to_char("You aren't allowed in Here, you'd better go!\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
             {
	          send_to_char( "GBS: To withdraw you must give an amount!\n\r", ch );
                  send_to_char( "     For information Type 'Bank'.\n\r",ch);
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->pcdata->gold_bank < amount)
                  {
	             send_to_char("GBS: You don't have that much gold squirreled away.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank -= amount;
                     ch->gold += amount;
                     act("$n withdraws gold from $s account.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "GBS: You have withdrawn %ld Gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->gold_bank, ch->gold);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
	             send_to_char("GBS: You don't have that much silver squirreled away.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act("$n withdraws silver from $s account.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "GBS: You have withdrawn %ld Silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("The type of currency must be stated before the amount.\n\r",ch);
            }
         }
      }
      else if(ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF))
      {
         if(str_cmp(class, "thief"))
         {
            act("A raider is Here to kill the Guildmaster.", ch,NULL,NULL, TO_ROOM);
            send_to_char("You aren't allowed in Here, you'd better go!\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	          send_to_char( "THIEF: To withdraw you must give an amount!\n\r", ch );
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->pcdata->gold_bank < amount)
                  {
	             send_to_char("THIEF: You don't have that much gold squirreled away.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank -= amount;
                     ch->gold += amount;
                     act("$n grabs gold from under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have grabbed %ld Gold.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->gold_bank, ch->gold);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
	             send_to_char("THIEF: You don't have that much silver squirreled away.\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act("$n grabs silver from under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have grabbed %ld Silver.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("The type of currency must be stated before the amount.\n\r",ch);
            }
         }
      }
      else
      {
         bug( "Do_withdraw: Bank doesn't exist.", 0 );
         send_to_char( "Bank doesn't exist.\n\r", ch );
         return;
      } 
   }
   return;
}


void do_change (CHAR_DATA *ch, char *argument)
{
   int amount = 0,change = 0, roll, fence = 0;
   float profit;
   char buf [MAX_STRING_LENGTH];
   char arg [MAX_INPUT_LENGTH];
   char const *class = class_table[ch->class].name;

   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
      && str_cmp(class, "thief"))
   {
      send_to_char("You must be in the Bank to change currency.\n\r",ch);
      return;
   }
   else  /* In the Bank */
   {
      /* No NPC's, No pets, No imms,
       * No chainmail, No service! 
       */
      if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
          || (IS_IMMORTAL(ch)) )
      {
         send_to_char("Only players need to change currency!\n\r", ch);
         return;
      }
      else if(!str_cmp(class, "thief") 
              && ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF))
      {
         argument = one_argument( argument, arg );
         if ( is_number( arg ) )
         {
	    amount = atoi(arg);
	    if ( amount <= 0 )
            {
               send_to_char("THIEF: How much silver do you wish to change?\n\r",ch);
               return;
            }
            else
            {
               if(ch->silver < amount )
               {
                  sprintf(buf, "THIEF: You can only change %ld silver.\n\r",ch->silver);
                  send_to_char( buf, ch);
                  return;
               }
               else
               {
		    /* haggle */
		    roll = number_percent();
		    if (roll < get_skill(ch,gsn_haggle))
                    {
                       profit = (float)roll/100;
                       sprintf(buf,"THIEF: You haggle the fence's percentage down to %d%%.\n\r",(int)profit*100);
	               send_to_char(buf,ch);
	               check_improve(ch,gsn_haggle,TRUE,4);
                    }
                    else
                       profit = 0.10; /* 10% exchange rate at the fence */
                      
                  if( ch->level < 10)  
                     change = amount/100;
                  else
                  {
                    if( profit > 0.10) /* no tax rate above 10% */
                        profit = 0.10;
                     change = amount/100 - (int)((float)amount/100*profit);
                     fence = (int)(amount/100 * profit);
                  }
                  ch->gold += change;
                  ch->silver -= amount;
                  sprintf( buf, "THIEF: You have changed %d Silver.\n\rYou have "
                                "recieved %d Gold.\n\rFence profit: %d Gold\n\r",
                                amount, change, fence);
                  send_to_char( buf, ch);
                  return;
               }
            }
         }
         else
         {
            send_to_char("THIEF: Change syntax:\n\r",ch);
            send_to_char("       Change <value> i.e. change 1200\n\r",ch);
            send_to_char("THIEF: Profitting occurs from currency exchange!\n\r",ch);
            send_to_char("       Profit Max: 10%% of currency changed.\n\r",ch);
            send_to_char("       Honour among thieves and all that!\n\r",ch);
            return;
         }
      }
      else if(ch->in_room == get_room_index(ROOM_VNUM_BANK)
         && str_cmp(class, "thief"))
      {
         argument = one_argument( argument, arg );
         if ( is_number( arg ) )
         {
	    amount = atoi(arg);
	    if ( amount <= 0 )
            {
               send_to_char("GBS: How much silver do you wish to change?\n\r",ch);
               return;
            }
            else
            {
               if(ch->silver < amount )
               {
                  sprintf(buf, "GBS: You can only change %ld silver.\n\r",ch->silver);
                  send_to_char( buf, ch);
                  return;
               }
               else
               {
                  change = amount/100;
                  ch->gold += change;
                  ch->silver -= amount;
                  sprintf( buf, "GBS: You have changed %d Silver.\n\rYou have recieved %d Gold.\n\r", amount, change);
                  send_to_char( buf, ch);
                  return;
               }
            }
         }
         else
         {
            send_to_char("GBS: Change syntax:\n\r",ch);
            send_to_char("     Change <value> i.e. change 120\n\r",ch);
            send_to_char("     So 1200 silver changes into 12 gold.\n\r",ch);
            send_to_char("GBS: Doesn't profit from currency exchange!\n\r",ch);
            return;
         }
      }
      else if(ch->in_room == get_room_index(ROOM_VNUM_BANK)
              && !str_cmp(class, "thief"))
      {
         act("A thief lurks in shadows of the Midgaard Bank.", ch,NULL,NULL, TO_ROOM);
         send_to_char("You aren't allowed in Here, you'd better go!\n\r",ch);
         send_to_char("You must go to your Guild's Fence to change currency.\n\r",ch); 
         return;
      }
      else /* a non-thief at fence */
      {
         act("A noble is here waiting to be robbed.", ch,NULL,NULL, TO_ROOM);
         send_to_char("You aren't allowed in Here, you'd better go!\n\r",ch); 
         return;
      }
   } 
   return;
}


void do_share (CHAR_DATA *ch, char *argument)
{
   long shares = 0, cost = 0;
   BUFFER *output;
   int level, row, money = 0, diff, i;
   int list_level, tax;
   int bonus = 0;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char const *class = class_table[ch->class].name;

   shares = ch->pcdata->shares;
   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
      && str_cmp(class, "thief"))
   {
      send_to_char("You must be in the Bank to find out about shares.\n\r",ch);
      return;
   }
   else if(!str_cmp(class, "thief"))
   {
      send_to_char("Due to a poor economic base -Thieves cannot get shares.\n\r", ch);
      return;
   }
   else  /* In the Bank */
   {
      /* No NPC's, No pets, No imms,
       * No chainmail, No service! 
       */
      argument = one_argument( argument, arg );
         if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
          || (IS_IMMORTAL(ch)) )
      {
         send_to_char("Only players need shares!\n\r", ch);
         return;
      }
      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
         if (arg[0] == '\0' && ch->level >= 10)
         { 
            send_to_char("GBS: Share Costs are all in Gold.\n\r",ch);
            send_to_char("     To Buy Shares: Gold is taken from your Account.\n\r",ch);
            send_to_char("     Share Spell Bonus +1 hour duration per 4 shares.\n\r",ch);
            send_to_char("     ONLY 5 shares can be bought per level.\n\r",ch);
            send_to_char("     ONLY 5 shares can be bought a one time.\n\r",ch);
            send_to_char("GBS: Share <value>\n\r",ch);
            send_to_char("     For the Share prices Type 'share price'.\n\r",ch);
            return;
         }
         else if(!str_cmp(arg, "price"))
         {
            output = new_buf();
            send_to_char("                   GBS -> Share Listing <-\n\r",ch);
            sprintf(buf2,"Level:     Cost:      Level:     Cost:      Level:     Cost:      \n\r");
            send_to_char(buf2, ch);
            cost = 0;
            row = tax = 0;

            if(IS_HERO(ch) || ch->level+2 == LEVEL_HERO)
              list_level = ch->level;  /*if hero list ch->level*/
            else
              list_level = ch->level+2; /*2 level past ch->level*/

            /* list levels for mortals */ 
            for(level=10; level <= list_level; level++)  
            {
               if(++tax % 9 == 0)
                  cost += 120000; /* every 9 levels */
               else
                  cost += 20000;
               sprintf(buf,"%-6d     %-9ld  ",level,cost);
               if(++row % 3 == 0)
                  strcat( buf, "\n\r");
               add_buf( output, buf);
            }
            page_to_char(buf_string(output),ch);
            free_buf(output);
            send_to_char( "\n\r", ch);
            return;
         }
         else if(ch->level < 10)
         {
            send_to_char("GBS: Share buying starts at level 10.\n\r",ch);
            return;
         }
         else
         {
            if( is_number( arg ) )
            {
	       shares = atoi(arg);
	       if ( shares < 0 )
                {
                     shares = abs(shares);
	             send_to_char( "GBS: Bad market - shares don't sell well!\n\r", ch );
                   if( ch->pcdata->shares < shares)
                   {
                      sprintf( buf, "GBS: You only have %d shares.\n\r",ch->pcdata->shares);
                      send_to_char( buf, ch);
                   }
                   else
                   {
                      ch->pcdata->shares -= shares;
                      act("$n sells $s some shares.", ch,NULL,NULL, TO_ROOM);

                      if(ch->pcdata->shares == 0)
                         send_to_char( "GBS: You sell ALL your shares in Midgaard Financial.\n\r",ch);
                      else
                         send_to_char( "GBS: You sell shares in Midgaard Financial.\n\r",ch);

                      money = ( 20000 / (int)((float)shares * 0.10) ); /* cash back */
                      sprintf( buf,"GBS: You get %d gold for %ld %s deposited in your Account.\n\r", 
                                money, shares, shares > 1 ? "shares" : "share");
                      send_to_char( buf, ch);
                      ch->pcdata->gold_bank +=money; /* put gold in the bank */
                      ch->pcdata->shares_bought -= shares;
                      if( ch->pcdata->shares / 4 != 0)
                          bonus = ch->pcdata->shares / 4;
                       ch->pcdata->duration = bonus;
                      return;
                   }   
	       }
	       else if ( shares > SHARE_MAX )
                {
	             sprintf(buf,"GBS: Bad market - don't buy more than %d shares.\n\r",SHARE_MAX);
	             send_to_char( buf, ch );
	             return;
	       }
	       else
                {
                   if(ch->pcdata->shares_bought > SHARE_MAX)
                   {
                      sprintf(buf,"GBS: You have reached your %d share limit per level.\n\r",SHARE_MAX);
                      send_to_char( buf, ch);
                      return;
                   }
                   else
                   {
                     if(ch->pcdata->share_level == ch->level
                        && ch->pcdata->shares_bought <= SHARE_MAX)
                     {
                       cost = 0;
                       tax = 0;
                       for(level=10;level<LEVEL_HERO;level++)
                       { 
                         if(++tax % 9 == 0)
                           cost += 120000; /* every 9 levels */
                         else
                            cost += 20000;
                         if(level == ch->level)
                            break;
                       }
                       diff = (SHARE_MAX - ch->pcdata->shares_bought - shares);
                       sprintf( buf,"GBS: You can buy %d more %s at this level.\n\r", 
                                diff, shares > 1 ? "share" : "shares");
                       send_to_char( buf, ch ); 
                       cost *= shares; /* actual cost to the player */
                       if(ch->pcdata->gold_bank < cost)
                       {
                          sprintf( buf,"GBS: You only have %ld gold in your Account.\n\r"
                                       "     For %ld %s cost %ld.\n\r",ch->pcdata->gold_bank, shares,
                                          shares > 1 ? "shares" : "share", cost);
                          send_to_char( buf, ch);
                          return;
                       }
                       else
                       {
                          if( shares == 1 )
                          {
                             act("$n buys one share in the Midgaard Bakery.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"GBS: You buy a share in the Midgaard Bakery.\n\r");
                          }
                          else if( shares > 1 && shares <= 3)
                          {
                             act("$n buys shares in the Midgaard Merchant's Guild.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"GBS: You buy %ld shares in the Midgaard Merchant's Guild.\n\r", shares);
                          }
                          else /* shares bought = 4 & 5 */
                          {
                             act("$n buys shares in the Midgaard Waterworks.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"GBS: You buy %ld shares in the Midgaard Waterworks.\n\r", shares);
	                    }
                          send_to_char( buf, ch );
                          ch->pcdata->gold_bank -= cost;
                          ch->pcdata->shares += shares;
                          ch->pcdata->shares_bought += shares;
                          do_account(ch, "");
                          if( ch->pcdata->shares / 4 != 0)
                              bonus = ch->pcdata->shares / 4;
                          ch->pcdata->duration = bonus;
 	                  return;
                       }
                     }
                     else
                     {
                        bug( "Do_shares: shares_bought && share_level error.", 0 );
                        bug( "Do_shares: bought > %d && share_level not ch->level.",SHARE_MAX );
                        send_to_char( "Stock market has crashed.\n\r", ch );
                        return;
                     }
                   }
	       }
            }
         } 
      }
      else
      {
         bug( "Do_shares: Bank doesn't exist.", 0 );
         send_to_char( "Bank doesn't exist.\n\r", ch );
         return;
      } 
   }
   return;
}


/* **************** This is the End of bank.c *************** */
