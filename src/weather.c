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
 *           weather.c - November 13, 2025
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
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

/* Weather code from Yrth, by Paul Bohme/Trithemius (pbohme@concentric.net)
 * Feel free to use/tweak/&c.  Drop me a note if you like it, and if you
 * don't, drop me a note telling me what's not cool.  Honestly enough, 
 * it was the first serious bit of MUD-specific coding I've done so
 * it might stand a bit of improvement here and there from the heat
 * generally brought to bear when something enters the collective
 * consciousness.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Update the weather.
 */

/* PaB: Constants in weather calcs */
#define TOTAL_WEATHER_PROBABILITY 1000 
#define CLEAR_SKY_RANGE  (TOTAL_WEATHER_PROBABILITY / SEASON_MAX)
#define CLOUDY_SKY_RANGE (TOTAL_WEATHER_PROBABILITY / SEASON_MAX)
#define RAINY_SKY_RANGE  (TOTAL_WEATHER_PROBABILITY / SEASON_MAX)
#define LIGHTNING_RANGE  (TOTAL_WEATHER_PROBABILITY / SEASON_MAX)

/* Influence factors 
 * For each season/sector we keep a list of influences.  These are applied
 * to calculations of what the current weather is like.  For example, 
 * springtime is a stormy period of transition, so we increase the chances
 * of rain and lightning accordingly.  Deserts don't see much rain at all,
 * so we seriously crank down the chances of those happening in desert
 * sectors.
 * The following constants will either fully open a weather chance to happening
 * or fully close it off. 
 */
#define BASE_INFLUENCE ((TOTAL_WEATHER_PROBABILITY / SEASON_MAX) / 4)
#define PLUS4       (BASE_INFLUENCE * 4)
#define PLUS3       (BASE_INFLUENCE * 3)
#define PLUS2       (BASE_INFLUENCE * 2)
#define PLUS1       (BASE_INFLUENCE * 1)
#define NOINFLUENCE (BASE_INFLUENCE * 0)
#define MINUS1      (BASE_INFLUENCE * -1)
#define MINUS2      (BASE_INFLUENCE * -2)
#define MINUS3      (BASE_INFLUENCE * -3)
#define MINUS4      (BASE_INFLUENCE * -4)

/* PaB: Influence of seasons on weather changes.
 * Notes: Because we could be dealing with all kinds of 'climates' which
 * aren't properly reflected in the areas - exactly how minute of detail
 * do we want to track here, anyway?? - we should keep things fairly 
 * mediocre.  Will prevent snow in what should be a tropical climate...
 */

typedef struct _weather_influence_data
{
	int clear_sky;
	int cloudy_sky;
	int rainy_sky;
	int lightning_sky;
   int max_probability;
} weather_influence_data;

/* Notes: 
 * Base weather table.  Even chances of all weather types, then seasonal
 * and sector influences are applied.
 */
static weather_influence_data base_weather =
{
 /* clear     cloudy    raining   lightning */
    CLEAR_SKY_RANGE, CLOUDY_SKY_RANGE, RAINY_SKY_RANGE, LIGHTNING_RANGE
};

/* Notes: 
 * In simple terms: (Thanks, Eladriel! - PaB)
 * Winter - its cloudy quite a bit, but doesn't rain often and lightnings even less.
 * Spring - clouds up with some frequency and when it does, it rains lot, but
 * not a lot of lightning.
 * Summer - Sunny a bit, but when it rains, it lightnings frequently.
 * Fall   - Sunny a lot, but doesn't rain and rarely lightnings when cloudy.
 */
static weather_influence_data seasonal_weather_table[] =
{
 /* clear        cloudy       raining      lightning */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, MINUS2     }, /* Winter */
   {MINUS3,      MINUS2,      NOINFLUENCE, NOINFLUENCE}, /* Spring */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, MINUS2     }, /* Summer */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, MINUS2     }  /* Fall */
};


/* Notes: (Thanks, Eladriel! - PaB)
 * Inside and City - baseline of the weather.
 * Field  - sunny a lot, but when it gets cloudy, it rains and often lightnings
 * Forest - sunny a bit, rains when cloudy but doesn't lightning often.
 * Hills  - gets overcast a bit, rains with some frequency but no big lightning
 * Mountains - overcast a lot, rains a lot but not a lot of lightning.
 * Water (swim)-sun and clouds normal, but when cloudy it rains and lightnings.
 * water (no) -sunny a bit and little clouds, but when there, it rains a lot
 *             and lightnings quite a bit too.
 * Tundra - will tell you once I've got it set :P DEC
 * air    - most everything normal, though when it rains, it lightnings like
 *          hell.
 * Desert - Rarely, if ever, gets cloudy.  When it does, it rains and
 *          lightnings to make up for lost time.
 */
static weather_influence_data sector_weather_table[] =
{
 /* clear        cloudy       raining      lightning */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, NOINFLUENCE}, /* SECT_INSIDE */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, NOINFLUENCE}, /* SECT_CITY */
   {NOINFLUENCE, NOINFLUENCE, PLUS1,       PLUS1      }, /* SECT_FIELD */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, MINUS1     }, /* SECT_FOREST */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, MINUS2     }, /* SECT_HILLS */
   {NOINFLUENCE, PLUS2,       PLUS2,       NOINFLUENCE}, /* SECT_MOUNTAIN */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, NOINFLUENCE}, /* SECT_WATER_SWIM */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, NOINFLUENCE}, /* SECT_WATER_NOSWIM */
   {NOINFLUENCE, NOINFLUENCE, NOINFLUENCE, NOINFLUENCE}, /* SECT_UNUSED */
   {NOINFLUENCE, NOINFLUENCE, MINUS3,      NOINFLUENCE}, /* SECT_AIR */
   {PLUS4,       MINUS4,      MINUS4,      PLUS4      }  /* SECT_DESERT */
};

/* Weather conversion table.  Some sectors don't have normal weather
 * patterns, e.g. it doesn't rain in the desert.  This enables the logic
 * to neatly step over unwanted states.
 */
static int weather_conversion_table[SECT_MAX][SKY_MAX] =
{
   {0, 1, 2, 3},   /* SECT_INSIDE */
   {0, 1, 2, 3},   /* SECT_CITY */
   {0, 1, 2, 3},   /* SECT_FIELD */
   {0, 1, 2, 3},   /* SECT_FOREST */
   {0, 1, 2, 3},   /* SECT_HILLS */
   {0, 1, 2, 3},   /* SECT_MOUNTAIN */
   {0, 1, 2, 3},   /* SECT_WATER_SWIM */
   {0, 1, 2, 3},   /* SECT_WATER_NOSWIM */
   {0, 1, 2, 3},   /* SECT_UNUSED */
   {0, 1, 2, 3},   /* SECT_AIR */
   {0, 0, 0, 3}    /* SECT_DESERT */
};

/* Master weather table.  At startup, all of the above rubbish is 
 * coalesced into this table, which we then use when figuring out
 * what the weather is like.
 */
static int master_weather_table[SECT_MAX][SEASON_MAX][SKY_MAX];

/* PaB: String tables by sector.  If we really wanted to be extravagant, 
 * we could fill them in for seasons, as well..
 */
static char* weather_strings[11][8] =
{
   {
   /* SECT_INSIDE */
      /* cloudless */
      "",
      "",
      /* cloudy */
      "",
      "",
      /* raining */
      "",
      "",
      /* lightning */
      "",
      ""
   },
   {
/* SECT_CITY */
      /* cloudless */
      "The clouds break up over the city.\r\n",
      "",
      /* cloudy */
      "The drumbeats of rain on rooftops dies down.\r\n",
      "Above the rooftops, you see clouds rolling in.\r\n",
      /* raining */
      "The thunder rolling off of nearby buildings fades and is quiet.\r\n",
      "Rain makes a melodic drumbeat as it falls on the rooftops.\r\n",
      /* lightning */
      "",
      "Flashes light up the sky and thunder crashes off the buildings.\r\n"
   },
   {
/* SECT_FIELD */
      /* cloudless */
      "The clear sky opens up over the fields.\r\n",
      "",
      /* cloudy */
      "The fields are no longer being bathed in rainfall.\r\n",
      "The fields darken as clouds roll in, obscuring the sky.\r\n",
      /* raining */
      "The rolls of thunder rolling across the fields fades into silence.\r\n",
      "The thirsty fields drink up the rain as it begins falling.\r\n",
      /* lightning */
      "",
      "Thunder clashes and roars across the fields.  Shouldn't you seek cover..?\r\n"
   },
   {
/* SECT_FOREST */
      /* cloudless */
      "The forest breathes again as the sky clears.\r\n",
      "",
      /* cloudy */
      "The drumbeat of rain on leaves fades..\r\n",
      "The forest hushes as clouds darken the sky.\r\n",
      /* raining */
      "The thunder shaking the forest rumbles one last and is gone.\r\n",
      "The raindrops falling on the forest around you makes a soothing sound.\r\n",
      /* lightning */
      "",
      "The denizens of the forest run for cover as the elements vent their fury!\r\n"
   },
   {
/* SECT_HILLS */
      /* cloudless */
      "The hilltops are no longer obscured by clouds.\r\n",
      "",
      /* cloudy */
      "It stops raining.\r\n",
      "An eerie silence settles over the hills as clouds roll in.\r\n",
      /* raining */
      "The echoes of thunder on distant hills dies down.\r\n",
      "The thirsty fields drink up the rain as it begins falling.\r\n",
      /* lightning */
      "",
      "Lightning flashes and thunder rolls from distant hilltops.\r\n"
   },
   {
/* SECT_MOUNTAIN */
      /* cloudless */
      "The moutaintops reach majestically towards the clear sky.\r\n",
      "",
      /* cloudy */
      "The clouds, lightened of their load of rain, stop pouring it upon Yrth.\r\n",
      "Heavy clouds roll in around the mountain.\r\n",
      /* raining */
      "The booming thunder quiets and dies.\r\n",
      "Rain pours mercilessly down upon you.\r\n",
      /* lightning */
      "",
      "Thunder roars and reverberates from the surrounding peaks.\r\n"
   },
   {
/* SECT_WATER_SWIM */
      /* cloudless */
      "The sky far above seems to be clearing.\r\n",
      "",
      /* cloudy */
      "The chaotic noise of rain from above dies down.\r\n",
      "The water darkens as clouds roll in.\r\n",
      /* raining */
      "The distant sound of thunder fades.\r\n",
      "The rain beats chaotic on the surface above.\r\n",
      /* lightning */
      "",
      "Flashes light up the water around you as the air above storms and thunders.\r\n"
   },
   {
/* SECT_WATER_NOSWIM */
      /* cloudless */
      "The sky above seems to be clearing.\r\n",
      "",
      /* cloudy */
      "The chaotic noise of rain on water dies down.\r\n",
      "The water around you darkens as clouds roll in.\r\n",
      /* raining */
      "The distant sound of thunder fades.\r\n",
      "The rain beats chaotic on the surface above.\r\n",
      /* lightning */
      "",
      "Flashes light up the water around you as the air above storms and thunders.\r\n"
   },
   {
/* SECT_UNUSED */
      /* cloudless */
      "",
      "",
      /* cloudy */
      "",
      "",
      /* raining */
      "",
      "",
      /* lightning */
      "",
      ""
   },
   {
/* SECT_AIR */
      /* cloudless */
      "The clouds around you dissipate.\r\n",
      "",
      /* cloudy */
      "The rain falling around you tapers off..\r\n",
      "Clouds begin rolling in around you.\r\n",
      /* raining */
      "The flashes of lightning and impacts of thunder die off..\r\n",
      "Rain begins forming around you and falling to Yrth.\r\n",
      /* lightning */
      "",
      "Thunder shakes you about as lightning crashes around you!\r\n"
   },
   {
/* SECT_DESERT */
      /* cloudless */
      "The cruel sky opens above you.\r\n",
      "",
      /* cloudy */
      "",
      "",
      /* raining */
      "",
      "",
      /* lightning */
      "",
      "Thunder rolls across the desert as an electrical storm comes to life.\r\n"
   }
};

/* PaB: Which season are we in? 
 * Notes: Simply figures out which season the current month falls into
 * and returns a proper value.
 */
int calc_season(void)
{
   int season = 0;

   /* How far along in the year are we, measured in days? */
   /* PaB: Am doing this in days to minimize roundoff impact */
   int day = time_info.month * DAYS_PER_MONTH + time_info.day;

   if(day < (DAYS_PER_YEAR / 8))
   {
      season = SEASON_WINTER;
   }
   else if(day < (DAYS_PER_YEAR / 8) * 3)
   {
      season = SEASON_SPRING;
   }
   else if(day < (DAYS_PER_YEAR / 8) * 5)
   {
      season = SEASON_SUMMER;
   }
   else if(day < (DAYS_PER_YEAR / 8) * 7)
   {
      season = SEASON_FALL;
   }
   else
   {
      season = SEASON_WINTER;
   }

   return season;
}

void update_daylight(char buf[SECT_MAX][MAX_STRING_LENGTH])
{
   int sect;

   ++time_info.hour;

   for(sect = 0; sect < SECT_MAX; sect++)
   {
      switch(time_info.hour)
      {
         case  HOUR_DAY_BEGIN:
            weather_info[sect].sunlight = SUN_LIGHT;
            strcat(buf[sect], "The day has begun.\n\r");
            break;

         case  HOUR_SUNRISE:
            weather_info[sect].sunlight = SUN_RISE;
            strcat(buf[sect], "The sun rises in the east.\n\r");
            break;

         case HOUR_SUNSET:
            weather_info[sect].sunlight = SUN_SET;
            strcat(buf[sect], "The sun slowly disappears in the west.\n\r");
            break;

         case HOUR_NIGHT_BEGIN:
            weather_info[sect].sunlight = SUN_DARK;
            strcat(buf[sect], "The night has begun.\n\r");
            break;
      }
   }
   if(time_info.hour == HOUR_MIDNIGHT)
   {
      time_info.hour = 0;
      time_info.day++;
   }

   if(time_info.day >= DAYS_PER_MONTH)
   {
      time_info.day = 0;
      time_info.month++;
   }

   if(time_info.month >= MONTHS_PER_YEAR)
   {
      time_info.month = 0;
      time_info.year++;
   }
}

void weather_update(void)
{
   char buf[SECT_MAX][MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *d;
   int season;
   int diff;
   int sect;
   int sky;
   int i;
   int changed = 0;
   int dir = 0;

   for(i = 0; i < SECT_MAX; i++)
   {
      buf[i][0] = '\0';
   }

   /* Day/Night */
   update_daylight(buf);

   season = calc_season();

   for(sect = 0; sect < SECT_MAX; sect++)
   {
      changed = 0;
      sky = weather_info[sect].sky;

      weather_info[sect].mmhg += weather_info[sect].change;

      /* See what it's like at the moment */

#define CHANGE_FACTOR ((random() % 25) + 15)

      if(weather_info[sect].sky == 0)
      {
         if(weather_info[sect].mmhg > master_weather_table[sect][season][weather_info[sect].sky])
         {
            sky = weather_conversion_table[sect][weather_info[sect].sky + 1];
            changed = 1;
            dir = 0;
         }
      }
      else if(weather_info[sect].sky == SKY_MAX - 1)
      {
         if(weather_info[sect].mmhg < master_weather_table[sect][season][weather_info[sect].sky - 1])
         {
            sky = weather_conversion_table[sect][weather_info[sect].sky - 1];
            changed = 1;
            dir = -1;
         }
      }
      else
      {
         if(weather_info[sect].mmhg > master_weather_table[sect][season][weather_info[sect].sky + 1])
         {
            sky = weather_conversion_table[sect][weather_info[sect].sky + 1];
            changed = 1;
         }
         else if(weather_info[sect].mmhg < master_weather_table[sect][season][weather_info[sect].sky - 1])
         {
            sky = weather_conversion_table[sect][weather_info[sect].sky - 1];
            changed = 1;
            dir = -1;
         }
      }

      /* Sky changed, reset counters */
      if(changed)
      {
         /* Figure how much to change the weather */
         if(sky == 0)
         {
            /* Is always going to go up */
            weather_info[sect].change =  CHANGE_FACTOR;

            /* reset mmhg in the middle of the current sky factor */
            /*weather_info[sect].mmhg = master_weather_table[sect][season][sky] / 2;*/
            /* PaB: Cheat for a little more sunshine. ;-) */
            weather_info[sect].mmhg = 0;
         }
         else if(sky == SKY_MAX - 1) /* Right now, SKY_LIGHTNING */
         {
            /* Is always going to come back down */
            weather_info[sect].change =  -1 * CHANGE_FACTOR;

            /* reset mmhg in the middle of the current sky factor */
            weather_info[sect].mmhg = (master_weather_table[sect][season][sky] - 
                                       master_weather_table[sect][season][sky - 1]) / 2 + 
                                       master_weather_table[sect][season][sky];
         }
         else
         {
            /* Might go up or down */
            diff = ((random() % 100) < 50) ? -1 : 1;
            weather_info[sect].change =  diff * CHANGE_FACTOR;

            /* reset mmhg in the middle of the current sky factor */
            weather_info[sect].mmhg = (master_weather_table[sect][season][sky] - 
                                       master_weather_table[sect][season][sky - 1]) / 2 + 
                                       master_weather_table[sect][season][sky + dir];
         }
      }

      /* Generate a proper change message */
      if(changed)
      {
         if(sky < weather_info[sect].sky)
         {
            strcat(buf[sect], weather_strings[sect][sky * 2]);
         }
         else
         {
            strcat(buf[sect], weather_strings[sect][sky * 2 + 1]);
         }
         weather_info[sect].sky = sky;
      }

   }

   for(d = descriptor_list; d != NULL; d = d->next)
   {
      if(d->connected == CON_PLAYING &&
         IS_OUTSIDE(d->character)    &&
         IS_AWAKE(d->character)      &&
         d->character                &&
         d->character->in_room       &&
         buf[d->character->in_room->sector_type][0] != '\0')
      {
         send_to_char(buf[d->character->in_room->sector_type], d->character);
      }
   }

   /* See if Divine Intervention is really going to piss on someone's
    * parade.
    */
   for(d = descriptor_list; d != NULL; d = d->next)
   {
      if(d->connected == CON_PLAYING &&
         IS_OUTSIDE(d->character)    &&
         d->character                &&
         d->character->in_room)
      {
         if(weather_info[d->character->in_room->sector_type].sky == SKY_LIGHTNING)
         {
            if((random() % 10000) == 0)
            {
               send_to_char("{YZZAAAAAAAPP! {WYou are nailed by a bolt from the sky..{n\n\r", d->character);
               damage(d->character, d->character,
                  number_range(500, 1000), TYPE_UNDEFINED, DAM_LIGHTNING, FALSE, FALSE);
            }
         }
      }
   }

   lightning();
   return;
}

void init_weather(void)
{
   int sect, seas;

   for(sect = 0; sect < SECT_MAX; sect++)
   {
      weather_info[sect].change = CHANGE_FACTOR;
	   weather_info[sect].mmhg = 0; /* Start at 0 for cloudless sky */
      weather_info[sect].sky = SKY_CLOUDLESS;

      for(seas = 0; seas < SEASON_MAX; seas++)
      {
         int cur = 0;

         /* Each value is an accumulation of the preceeding */
         cur += UMAX(base_weather.clear_sky + 
                     seasonal_weather_table[seas].clear_sky + 
                     sector_weather_table[sect].clear_sky, 0);
         master_weather_table[sect][seas][SKY_CLOUDLESS] = cur;

         cur += UMAX(base_weather.cloudy_sky + 
                     seasonal_weather_table[seas].cloudy_sky + 
                     sector_weather_table[sect].cloudy_sky, 0);
         master_weather_table[sect][seas][SKY_CLOUDY] = cur;

         cur += UMAX(base_weather.rainy_sky + 
                     seasonal_weather_table[seas].rainy_sky + 
                     sector_weather_table[sect].rainy_sky, 0);
         master_weather_table[sect][seas][SKY_RAINING] = cur;

         cur += UMAX(base_weather.lightning_sky + 
                     seasonal_weather_table[seas].lightning_sky + 
                     sector_weather_table[sect].lightning_sky, 0);
         master_weather_table[sect][seas][SKY_LIGHTNING] = cur;
      }
   }

   weather_update();
}
