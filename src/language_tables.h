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
 *           language_tables.h - November 13, 2025
 */
/*
 * Language Translation Tables
 * 
 * This file contains translation tables for converting English text
 * into various fantasy languages based on player language skills.
 * 
 * Each language has a mapping from English letters to language-specific
 * sounds, allowing for partial understanding based on skill level.
 */

#ifndef LANGUAGE_TABLES_H
#define LANGUAGE_TABLES_H

/* Forward declaration */
struct char_data;
typedef struct char_data CHAR_DATA;

/* Language types */
#define LANG_HUMAN    0
#define LANG_ELVISH   1
#define LANG_DWARVISH 2
#define LANG_VERBEEG  3
#define MAX_LANGUAGES 4

/* Language names */
extern const char *language_names[MAX_LANGUAGES];

/* Translation tables - each maps English letters to language sounds */
extern const char *human_translation[26];
extern const char *elvish_translation[26];
extern const char *dwarvish_translation[26];
extern const char *verbeeg_translation[26];

/* Function prototypes */
char *translate_text(const char *text, int language, int skill_level);
char *get_language_name(int language);
int get_language_skill(CHAR_DATA *ch, int language);

/* Skill level thresholds for partial understanding */
#define LANG_NONE     0    /* 0% - completely garbled */
#define LANG_BASIC    25   /* 25% - some words recognizable */
#define LANG_PARTIAL  50   /* 50% - most words partially clear */
#define LANG_GOOD     75   /* 75% - mostly clear with some garbling */
#define LANG_FLUENT   100  /* 100% - completely clear */

#endif
