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
 *           windgate.h - November 3, 2025
 */            
/*
 * windgate.h
 */

/* Symbol defines for usage of the Windgates */
#define SYMBOL_NONE         0
#define SYMBOL_SQUARE       1
#define SYMBOL_TRIANGLE     2
#define SYMBOL_RECTANGLE    3
#define SYMBOL_CIRCLE       4
#define SYMBOL_STAR         5
#define SYMBOL_PENTAGON     6
#define SYMBOL_LINE         7
#define SYMBOL_SUN          8
#define SYMBOL_MOON         9
#define SYMBOL_HAND         10
#define MAX_SYMBOL          11

#define MAX_GATES           10

/* Worded define for easy reading */
#define NO_MIN_LEVEL        1

/* Table structure */
struct windgate_type
{
    char    * name;     /* Name to where it goes        */
    int     vnum;       /* What vnum their location is  */
    int     symbol[3];  /* the 4 symbols used travel    */
    int     min_level;  /* Min level to activate it     */
    bool    disabled;   /* is it disabled?              */
    bool    to_wild;    /* Does it go to the wildnerness*/
    int     x;          /* if to_wild, what X           */
    int     y;          /* if to_wild, what Y           */
};

struct symbol_type
{
    char    * name;
    int     symbol;
    char    * press_string;
};

extern const struct windgate_type windgate_table [];
extern const struct symbol_type symbol_table [];

/* Stuff */
void cmd_press              (CHAR_DATA * ch, char *argument);
void fix_lights             (CHAR_DATA * ch, OBJ_DATA * obj, int symbol);
void reset_windgate         (CHAR_DATA * ch, OBJ_DATA * obj);
void check_valid_sequence   (CHAR_DATA * ch, OBJ_DATA * obj);
void cmd_windgate           (CHAR_DATA * ch, char *argument);
char * return_gatesymbol    (int symbol);
