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
 *           board.h - November 13, 2025
 */
/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */


#define NOTE_DIR  				"../notes" /* set it to something you like */

#define DEF_NORMAL  0 /* No forced change, but default (any string)   */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

#define MAX_BOARD 	  7

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */
                        
#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
						   /* This only applies in the Body of the note */                        
						   
#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)
						
#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Data about a board */
struct board_data
{
	char *short_name; /* Max 8 chars */
	char *long_name;  /* Explanatory text, should be no more than 40 ? chars */
	
	int read_level; /* minimum level to see board */
	int write_level;/* minimum level to post notes */

	char *names;       /* Default recipient */
	int force_type; /* Default action (DEF_XXX) */
	
	int purge_days; /* Default expiration */

	/* Non-constant data */
		
	NOTE_DATA *note_first; /* pointer to board's first note */
	bool changed; /* currently unused */
		
};

typedef struct board_data BOARD_DATA;


/* External variables */

extern BOARD_DATA boards[MAX_BOARD]; /* Declare */


/* Prototypes */

void finish_note (BOARD_DATA *board, NOTE_DATA *note); /* attach a note to a board */
void free_note   (NOTE_DATA *note); /* deallocate memory used by a note */
void load_boards (void); /* load all boards */
int board_lookup (const char *name); /* Find a board with that name */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note); /* is tha note to ch? */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void save_notes ();

/* for nanny */
void handle_con_note_to 		(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_subject 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_expire 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_finish 	(DESCRIPTOR_DATA *d, char * argument);
void show_note_finish_prompt	(DESCRIPTOR_DATA *d);


/* Commands */

DECLARE_DO_FUN (do_note		);
DECLARE_DO_FUN (do_board	);
