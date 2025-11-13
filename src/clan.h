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
 *           clan.h - November 13, 2025
 */
/* **************************************************************
  December 27th, 1997
  Gothar's Clan Systems Version 1.0
  1997 Copyright <* FREEWARE *>
  If you use this code:
     1.Keep all the credits in the code.
     2.Use the help entry for bank
     3.Send a bug report,any comments or ideas
       Ian McCormick (aka Gothar)
       mcco0055@algonquinc.on.ca
************************************************************** */

#ifndef CLAN_H
#define CLAN_H

int clan_lookup		 (const char *name);

struct clan_type
{
    char 	*name;       /* name */
    char 	*who_name;   /* who entry */
    sh_int 	deathroom;   /* death-transfer room */
    sh_int  recall;      /* recall room */
    bool	independent; /* true for loners */
    sh_int  min_level;   /* min level to petition */
    bool    war_declared[MAX_CLAN];  /* Wars this clan has declared */
};

extern          struct  clan_type       clan_table[MAX_CLAN];
void save_clan_wars args((void));
void load_clan_wars args((void));

#endif /* CLAN_H */
