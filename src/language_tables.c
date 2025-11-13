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
 *           language_tables.c - November 13, 2025
 */
/*
 * Language Translation Tables Implementation
 * 
 * This file implements the translation system for converting English text
 * into various fantasy languages with partial understanding based on skill.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "language_tables.h"

/* Language names */
const char *language_names[MAX_LANGUAGES] = {
    "Human",
    "Elvish", 
    "Dwarvish",
    "Verbeeg"
};

/*
 * Translation tables for each language
 * Each array maps English letters (a-z) to language-specific sounds
 */

/* Human - Clear, straightforward sounds similar to English */
const char *human_translation[26] = {
    "ay",    /* a */
    "sh",    /* b */
    "k",     /* c */
    "d",     /* d */
    "eh",    /* e */
    "f",     /* f */
    "g",     /* g */
    "h",     /* h */
    "ih",    /* i */
    "j",     /* j */
    "k",     /* k */
    "l",     /* l */
    "m",     /* m */
    "n",     /* n */
    "oh",    /* o */
    "p",     /* p */
    "kw",    /* q */
    "r",     /* r */
    "s",     /* s */
    "t",     /* t */
    "uh",    /* u */
    "v",     /* v */
    "w",     /* w */
    "ks",    /* x */
    "y",     /* y */
    "z"      /* z */
};

/* Elvish - Flowing, melodic sounds with many vowels and soft consonants */
const char *elvish_translation[26] = {
    "ae",    /* a */
    "bh",    /* b */
    "ch",    /* c */
    "dh",    /* d */
    "ei",    /* e */
    "fh",    /* f */
    "gh",    /* g */
    "hh",    /* h */
    "ii",    /* i */
    "jh",    /* j */
    "kh",    /* k */
    "lh",    /* l */
    "mh",    /* m */
    "nh",    /* n */
    "oe",    /* o */
    "ph",    /* p */
    "qh",    /* q */
    "rh",    /* r */
    "sh",    /* s */
    "th",    /* t */
    "ui",    /* u */
    "vh",    /* v */
    "wh",    /* w */
    "xh",    /* x */
    "yh",    /* y */
    "zh"     /* z */
};

/* Dwarvish - Harsh, guttural sounds with hard consonants */
const char *dwarvish_translation[26] = {
    "gz",    /* a */
    "v",     /* b */
    "kz",    /* c */
    "dz",    /* d */
    "gr",    /* e */
    "fz",    /* f */
    "gz",    /* g */
    "hz",    /* h */
    "iz",    /* i */
    "jz",    /* j */
    "kz",    /* k */
    "lz",    /* l */
    "mz",    /* m */
    "nz",    /* n */
    "oz",    /* o */
    "pz",    /* p */
    "qz",    /* q */
    "rz",    /* r */
    "sz",    /* s */
    "tz",    /* t */
    "uz",    /* u */
    "vz",    /* v */
    "wz",    /* w */
    "xz",    /* x */
    "yz",    /* y */
    "zz"     /* z */
};

/* Verbeeg - Grunting, guttural giant sounds with heavy consonants */
const char *verbeeg_translation[26] = {
    "uh",    /* a */
    "gu",    /* b */
    "ku",    /* c */
    "du",    /* d */
    "eh",    /* e */
    "fu",    /* f */
    "gu",    /* g */
    "hu",    /* h */
    "ih",    /* i */
    "ju",    /* j */
    "ku",    /* k */
    "lu",    /* l */
    "mu",    /* m */
    "nu",    /* n */
    "oh",    /* o */
    "pu",    /* p */
    "qu",    /* q */
    "ru",    /* r */
    "su",    /* s */
    "tu",    /* t */
    "uh",    /* u */
    "vu",    /* v */
    "wu",    /* w */
    "xu",    /* x */
    "yu",    /* y */
    "zu"     /* z */
};

/*
 * Get the name of a language by index
 */
char *get_language_name(int language)
{
    if (language < 0 || language >= MAX_LANGUAGES)
        return "Unknown";
    
    return (char *)language_names[language];
}

/*
 * Get a character's skill level in a specific language
 */
int get_language_skill(CHAR_DATA *ch, int language)
{
    int sn;
    
    if (IS_NPC(ch))
        return 0;
    
    switch (language) {
        case LANG_HUMAN:
            sn = skill_lookup("human");
            break;
        case LANG_ELVISH:
            sn = skill_lookup("elvish");
            break;
        case LANG_DWARVISH:
            sn = skill_lookup("dwarvish");
            break;
        case LANG_VERBEEG:
            sn = skill_lookup("verbeeg");
            break;
        default:
            return 0;
    }
    
    if (sn == -1)
        return 0;
    
    return ch->pcdata->learned[sn];
}

/*
 * Translate text based on language and skill level
 * Returns a newly allocated string that must be freed by caller
 */
char *translate_text(const char *text, int language, int skill_level)
{
    char *result;
    char *ptr;
    int i, len;
    const char **translation_table;
    
    if (!text || language < 0 || language >= MAX_LANGUAGES)
        return str_dup(text);
    
    /* Select the appropriate translation table */
    switch (language) {
        case LANG_HUMAN:
            translation_table = (const char **)human_translation;
            break;
        case LANG_ELVISH:
            translation_table = (const char **)elvish_translation;
            break;
        case LANG_DWARVISH:
            translation_table = (const char **)dwarvish_translation;
            break;
        case LANG_VERBEEG:
            translation_table = (const char **)verbeeg_translation;
            break;
        default:
            return str_dup(text);
    }
    
    /* Calculate maximum possible length (each letter could become 2 chars) */
    len = strlen(text);
    result = alloc_mem(len * 2 + 1);
    ptr = result;
    
    /* Translate each character */
    for (i = 0; text[i] != '\0'; i++) {
        char c = LOWER(text[i]);
        bool is_upper = (text[i] >= 'A' && text[i] <= 'Z');
        const char *translation;
        
        if (c >= 'a' && c <= 'z') {
            translation = translation_table[c - 'a'];
            
            /* Apply translation based on skill level - progressive intelligibility */
            if (skill_level >= LANG_FLUENT) {
                /* 100% - show original text */
                *ptr++ = text[i];
            } else if (skill_level >= LANG_GOOD) {
                /* 75% - mostly original with some translated letters */
                if (number_percent() < 75) {
                    *ptr++ = text[i];
                } else {
                    /* Copy translated text, preserving first letter case */
                    if (is_upper && translation[0] != '\0') {
                        *ptr++ = UPPER(translation[0]);
                        strcpy(ptr, translation + 1);
                        ptr += strlen(translation + 1);
                    } else {
                        strcpy(ptr, translation);
                        ptr += strlen(translation);
                    }
                }
            } else if (skill_level >= LANG_PARTIAL) {
                /* 50% - mix of original and translated */
                if (number_percent() < 50) {
                    *ptr++ = text[i];
                } else {
                    /* Copy translated text, preserving first letter case */
                    if (is_upper && translation[0] != '\0') {
                        *ptr++ = UPPER(translation[0]);
                        strcpy(ptr, translation + 1);
                        ptr += strlen(translation + 1);
                    } else {
                        strcpy(ptr, translation);
                        ptr += strlen(translation);
                    }
                }
            } else if (skill_level >= LANG_BASIC) {
                /* 25% - mostly translated with some original letters */
                if (number_percent() < 25) {
                    *ptr++ = text[i];
                } else {
                    /* Copy translated text, preserving first letter case */
                    if (is_upper && translation[0] != '\0') {
                        *ptr++ = UPPER(translation[0]);
                        strcpy(ptr, translation + 1);
                        ptr += strlen(translation + 1);
                    } else {
                        strcpy(ptr, translation);
                        ptr += strlen(translation);
                    }
                }
            } else {
                /* 0% - fully translated (completely garbled from English perspective) */
                if (is_upper && translation[0] != '\0') {
                    *ptr++ = UPPER(translation[0]);
                    strcpy(ptr, translation + 1);
                    ptr += strlen(translation + 1);
                } else {
                    strcpy(ptr, translation);
                    ptr += strlen(translation);
                }
            }
        } else {
            /* Non-alphabetic characters pass through unchanged */
            *ptr++ = text[i];
        }
    }
    
    *ptr = '\0';
    return result;
}
