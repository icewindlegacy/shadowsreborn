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
 *           keyring.c - November 13, 2025
 */
/***************************************************************************
 * Keyring [C] 2002 Petr [Dingo] Dvorak <dingo@texoma.net>                 *
 *                                                                         *
 * Written for Anime Planet MUD [animeplanet.genesismuds.com:3333]         *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

// some state definitions so we can handle all the states with one function
// instead repeating the same stuff over and over.
#define KR_OK			0
#define KR_NO_KEYRING		1
#define KR_NO_KEY		2
#define KR_NO_REMKEY		3
#define KR_INVALID_KEYRING	4
#define KR_INVALID_KEY		5
#define KR_INVALID_NAME		6
#define KR_KEY_DUP		7

DECLARE_DO_FUN( do_examine );

// allocates memory for new key on the keyring
KEY_DATA * new_key( void )
{
    KEY_DATA * key;
    
    // allocate memory for the key, calloc 0's the allocated memory so we don't
    // have to initialize the variables.
    key = calloc(1,sizeof(*key));
    VALIDATE(key);

    return key;
}

// frees memory allocated by new_key
void free_key( KEY_DATA * key )
{
    // usual sanity checks
    if (!key || !IS_VALID(key))	
	return;

    // clean up, makes debugging easier :)
    key->next  = NULL;
    key->index = NULL;	// this can NEVER be free'd here !!!
    INVALIDATE(key);

    // free the memory
    free(key);

    return;
}

// adds key to the keyring
int add_key( OBJ_DATA * keyring, OBJ_DATA * key)
{
    KEY_DATA * nkey;

    // the usual sanity checks
    if (!keyring)		return KR_NO_KEYRING;
    if (!key)			return KR_NO_KEY;
    if (!IS_VALID(keyring))	return KR_INVALID_KEYRING;
    if (!IS_VALID(key))		return KR_INVALID_KEY;
    
    // loop thru the keyring and make sure that the key is not already on it
    for ( nkey = keyring->keys; nkey != NULL; nkey = nkey->next )
	if (nkey->index == key->pIndexData)
	    return KR_KEY_DUP;
    
    nkey = new_key();

    // fill in the data
    nkey->index = key->pIndexData;

    // add the key to the keyring
    nkey->next = keyring->keys;
    keyring->keys = nkey;

    // get rid of the 'real' key
    obj_from_char(key);
    extract_obj(key);

    return KR_OK;
}

// removes key from the keyring
int remove_key( CHAR_DATA * ch, OBJ_DATA * keyring, char * name )
{
    KEY_DATA	* key;
    KEY_DATA	* key_next;

    // usual sanity check
    if (!keyring)		return KR_NO_KEYRING;
    if (!keyring->keys)		return KR_NO_KEY;
    if (!name)			return KR_INVALID_NAME;
    if (name[0] == '\0')	return KR_INVALID_NAME;

    // let's find the key
    for ( key = keyring->keys; key != NULL; key = key_next )
    {
	key_next = key->next;

	// check the name
	if (key && key->index->name && !str_prefix(name, key->index->name))
	{
	    OBJ_DATA *phys_key;
	    
	    // create physical key and put it in players inventory
	    phys_key = create_object(key->index,key->index->level);
	    obj_to_char(phys_key, ch);
	
	    // if the key is first on the ring, its easy to drop it    
	    if ( keyring->keys == key )	keyring->keys = key->next;
	    else // we gotta find the previous key
	    {
		KEY_DATA *prev;

		for ( prev = keyring->keys; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == key )
		    {
			// drop the key from the ring;
			prev->next = key->next;
			break;
		    }
		}
	    }
	    // free the key structure
	    free_key(key);
	    
	    // time to bail.
	    return KR_OK;
	}
    }
    // hmm, we are still around ? key was not found.	
    return KR_NO_REMKEY;
}

void save_keyring( FILE * fp, OBJ_DATA * obj)
{
    KEY_DATA *  keys;

    // the usual sanity check
    if (!obj || !obj->keys)
	return;
	
    fprintf(fp, "Keyring  ");
    
    // loop thru the keys on keyring and save the vnums
    for (keys = obj->keys; keys != NULL; keys = keys->next)
    {
	if ( keys->index->vnum )
	    fprintf(fp, "%d ", keys->index->vnum);
    }
    
    // save -1 on the end of the list to terminate it
    fprintf(fp,"-1\n\r");

    return;
}

// load keys from the pfile
void load_keyring( FILE * fp, OBJ_DATA * obj)
{
    int 	vnum = 0;
    KEY_DATA *  nkey;

    do
    {
	// read one vnum from the pfile
	vnum = fread_number(fp);
	if (vnum > 0)
	{
	    // create new key
	    nkey = new_key();
	    
	    // fill in the blanks
	    nkey->index = get_obj_index(vnum);
	    nkey->next = obj->keys;
	    obj->keys = nkey;
	}
    } 
    while (vnum > 0);

    return;
}

// lists the keys on the keyring
void list_keys( CHAR_DATA * ch, OBJ_DATA * keyring )
{
    KEY_DATA	* key;
    char 	  buf[MSL];
    
    // no keys on the keyring
    if ( !keyring->keys )
    {
	send_to_char("The keyring is empty.\n\r",ch);
	return;
    }

    // list the keys
    send_to_char("\n\rThe keyring contains the following keys:\n\r\n\r",ch);
    
    for ( key = keyring->keys; key != NULL; key = key->next)
    {
	buf[0] = '\0';
	
	// imms can see vnums of the keys in the list.
	if (IS_IMMORTAL(ch)) sprintf(buf,"{D[{C%5.5d{D]{x",key->index->vnum);

	// send out the formatted line
	printf_to_char(ch,"    %s %s from %s.\n\r", 
	    buf,
	    key->index->short_descr ? key->index->short_descr : "Unknown",
	    key->index->area->name ? key->index->area->name : "Unknown place"
	    );
	       
    }

    return;
}

// searches players inventory for keyring
OBJ_DATA * find_keyring( CHAR_DATA * ch )
{
    OBJ_DATA *obj;

    // search players inventory for keyring object
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( (can_see_obj( ch, obj ) ) 
	&&   !str_prefix( "keyring", obj->name ) )
		return obj;
    }
    return NULL;
}

void keyring_message(int message, CHAR_DATA * ch, OBJ_DATA * key)
{
    switch (message)
    {
	case KR_OK:			// everything ok .. what are we doing here ?
	    return;
	    break;
	case KR_NO_KEYRING:		// no keyring
	    send_to_char("You don't have a keyring, go buy one.\n\r",ch);
	    break;
	case KR_NO_KEY:			// no key
	    send_to_char("You don't have that key.\n\r",ch);
	    break;
	case KR_NO_REMKEY:		// no key on keyring
	    send_to_char("You don't have that key on your keyring.\n\r",ch);
	    break;
	case KR_KEY_DUP:
	    printf_to_char(ch, "You already have %s on your keyring.\n\r",key->short_descr);
	    break;
	case KR_INVALID_NAME:
	case KR_INVALID_KEYRING:
	case KR_INVALID_KEY:
	default:
	    send_to_char("Something gone horribly wrong, please tell the nearest immortal.\n\r",ch);
	    // add your own debugging code here
	    break;
    }

    return;
}

// the command function
void do_keyring( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *  keyring = NULL;
    OBJ_DATA *  key = NULL;
    char 	arg1[MIL];
    char 	arg2[MIL];
    int		message = KR_OK;

    keyring = find_keyring(ch);

    if (!keyring)
    {
	keyring_message(KR_NO_KEYRING,ch,NULL);
	return;
    }

    // no arguments ? just show list of the keys on the keyring then.
    if (argument[0] == '\0')
    {
	list_keys(ch, keyring);
	return;
    }

    argument = one_argument( argument, arg1 );	// command - add | remove
    argument = one_argument( argument, arg2 );  // key name

    if (arg2[0] == '\0' )
    {
	send_to_char("You must specify the name of the key you wish to add or remove.\n\r", ch);
	return;
    }

    if (is_name( arg1, "add") )
    {
	key = get_obj_carry( ch, arg2, ch );

        if (!key)
	{
	    send_to_char("You don't have that key.\n\r", ch);
	    return;
        }
	
	if (key->item_type != ITEM_KEY)
	{
	    send_to_char("That is not a key!\n\r", ch);
	    return;
	}

	message = add_key(keyring, key);

	if (!message)
	{
            act("$n adds $p on $s keyring.", ch, key, NULL, TO_ROOM);
            act("You added $p on your keyring.", ch, key, NULL, TO_CHAR);
	} else
	{
	    keyring_message(message,ch,key);
	}
    }
    else if (is_name( arg1, "remove" ) )
    {
	message = remove_key(ch, keyring, arg2);
	if (!message)
	{
            act("$n removes a key from $s keyring.", ch, NULL, NULL, TO_ROOM);
            act("You removed a key from your keyring.", ch, NULL, NULL, TO_CHAR);
	} else
	{
	    keyring_message(message,ch,NULL);
	}
    }
    else
    {
	send_to_char("What exactly do you want to do with the keyring ?\n\r", ch);
	return;
    }
    
    return;
}
