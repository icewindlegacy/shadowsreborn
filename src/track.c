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
 *           track.c - November 3, 2025
 */            
/*
 * Track (Hunt) skill for warriors and thieves.
 * Ported from survival.c and adapted for 6 exits (n,e,s,w,u,d)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

#define PATH_MAX_DIR  6  /* 6 directions: n,e,s,w,u,d */
#define BITS_PER_INT  32
#define MAX_TRACK_ROOMS 1000  /* Reasonable limit for tracking */
#define PATH_IS_FLAG(flag, bit)  ((unsigned)flag[bit/BITS_PER_INT]>>bit%BITS_PER_INT&01)
#define PATH_SET_FLAG(flag, bit) (flag[bit/BITS_PER_INT] |= 1 << bit%BITS_PER_INT)

void do_track(CHAR_DATA *ch, char *argument);
int find_path(CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth);

void debug_print(CHAR_DATA *ch, const char *fmt, ...)
{
#ifdef DEBUG_TRACK
    va_list args;
    char buf[MSL];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (ch)
        send_to_char(buf, ch);
    else
        fprintf(stderr, "%s", buf);
#endif
}

void do_track(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int skill;
    int direction;

    /* Wolfen have innate tracking at 100% */
    if (!IS_NPC(ch) && ch->race == race_lookup("wolfen"))
        skill = 100;
    else
        skill = get_skill(ch, gsn_hunt);
    
    if (skill == 0 || IS_NPC(ch))
    {
        send_to_char("You do not know how to track.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Track whom?\n\r", ch);
        return;
    }

    victim = get_char_world(ch, argument);
    if (!victim)
    {
        send_to_char("No one around by that name.\n\r", ch);
        return;
    }

    if (ch->in_room == victim->in_room)
    {
        act("$N is here!", ch, NULL, victim, TO_CHAR);
        return;
    }

    // Check for Pass Without Trace (if defined)
    #ifdef AFF_PASS_WITHOUT_TRACE
    if (IS_AFFECTED(victim, AFF_PASS_WITHOUT_TRACE))
    {
        send_to_char("You cannot find any trace of that person.\n\r", ch);
        return;
    }
    #endif

    act("$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, skill_table[gsn_hunt].beats);

    // Determine max search depth based on skill (reasonable limits)
    skill = UMIN(skill / 4 + ch->level / 5, 50);  // Cap at 50 rooms max

    direction = find_path(ch, ch->in_room, victim->in_room, skill);

    if (direction == -1)
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "Unable to find a path to %s.\n\r", PERS(victim, ch));
        send_to_char(buf, ch);
    }
    else
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%s is %s from here.\n\r", PERS(victim, ch), dir_name[direction]);
        send_to_char(buf, ch);
    }

    check_improve(ch, gsn_hunt, direction != -1, 1);
}

// Fixed BFS pathfinding using ROM hash table iteration
int find_path(CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth)
{
    ROOM_INDEX_DATA **queue = NULL;
    ROOM_INDEX_DATA *next_room;
    ROOM_INDEX_DATA *current_room;
    int *visited = NULL;
    int front = 0, back = 0;
    int *depth_arr = NULL;
    int *parent_vnum = NULL;
    int *parent_dir = NULL;
    int i, dir;
    int max_vnum = 0;
    int visited_size;
    int result = -1;

    if (!from || !to || max_depth <= 0)
        return -1;

    // Find maximum vnum - use a safe default for ROM
    max_vnum = 32767;  // Standard ROM vnum range

    if (max_vnum < 1)
        max_vnum = 32767;  // Fallback

    // Calculate size for visited bitfield
    visited_size = (max_vnum / BITS_PER_INT) + 1;

    // Allocate arrays with bounds checking
    visited = calloc(visited_size, sizeof(int));
    queue = calloc(MAX_TRACK_ROOMS, sizeof(ROOM_INDEX_DATA *));
    depth_arr = calloc(max_vnum + 1, sizeof(int));
    parent_vnum = calloc(max_vnum + 1, sizeof(int));
    parent_dir = calloc(max_vnum + 1, sizeof(int));

    if (!visited || !queue || !depth_arr || !parent_vnum || !parent_dir)
    {
        send_to_char("Tracking failed: memory error.\n\r", ch);
        goto cleanup;
    }

    debug_print(ch, "DEBUG: Starting BFS from room %d to room %d, max depth %d\n", 
                from->vnum, to->vnum, max_depth);

    // Initialize BFS
    front = back = 0;
    queue[back++] = from;
    
    // Bounds check for vnum before setting visited flag
    if (from->vnum >= 0 && from->vnum <= max_vnum)
    {
        PATH_SET_FLAG(visited, from->vnum);
        depth_arr[from->vnum] = 0;
        parent_vnum[from->vnum] = -1;  // Mark as root
    }

    while (front < back && back < MAX_TRACK_ROOMS)
    {
        current_room = queue[front++];
        
        if (!current_room || current_room->vnum < 0 || current_room->vnum > max_vnum)
            continue;

        int cur_depth = depth_arr[current_room->vnum];
        
        debug_print(ch, "DEBUG: Visiting room %d at depth %d\n", current_room->vnum, cur_depth);

        // Don't go beyond max depth
        if (cur_depth >= max_depth)
            continue;

        // Check all directions (6 for ROM: n,e,s,w,u,d)
        for (dir = 0; dir < PATH_MAX_DIR; dir++)
        {
            EXIT_DATA *pexit = current_room->exit[dir];
            
            debug_print(ch, "DEBUG: Checking direction %d from room %d\n", dir, current_room->vnum);
            
            if (!pexit)
            {
                debug_print(ch, "DEBUG: No exit data in direction %d from room %d\n", dir, current_room->vnum);
                continue;
            }
            
            // Get destination room
            next_room = pexit->u1.to_room;
            
            if (!next_room)
            {
                debug_print(ch, "DEBUG: Exit %d exists but no destination room from room %d\n", dir, current_room->vnum);
                continue;
            }

            debug_print(ch, "DEBUG: Found exit %s (%d) from room %d to room %d\n", 
                       dir_name[dir], dir, current_room->vnum, next_room->vnum);
            
            // Bounds check
            if (!next_room || next_room->vnum < 0 || next_room->vnum > max_vnum)
                continue;

            // Skip if already visited
            if (PATH_IS_FLAG(visited, next_room->vnum))
            {
                debug_print(ch, "DEBUG: Room %d already visited, skipping\n", next_room->vnum);
                continue;
            }

            // Skip closed/locked doors unless character can pass
            if (IS_SET(pexit->exit_info, EX_CLOSED))
            {
                debug_print(ch, "DEBUG: Exit to room %d is closed, skipping\n", next_room->vnum);
                continue;
            }

            // Mark as visited and add to queue
            PATH_SET_FLAG(visited, next_room->vnum);
            depth_arr[next_room->vnum] = cur_depth + 1;
            parent_vnum[next_room->vnum] = current_room->vnum;
            parent_dir[next_room->vnum] = dir;

            debug_print(ch, "DEBUG: Added room %d to queue at depth %d\n", next_room->vnum, cur_depth + 1);

            // Found target?
            if (next_room == to)
            {
                debug_print(ch, "DEBUG: Found target at depth %d\n", cur_depth + 1);
                
                // Backtrack to find the FIRST direction from starting room
                int trace_vnum = next_room->vnum;
                
                debug_print(ch, "DEBUG: Backtracking from target room %d\n", trace_vnum);
                
                // Follow parent chain back to find the room that's directly connected to start
                while (parent_vnum[trace_vnum] != -1 && parent_vnum[trace_vnum] != from->vnum)
                {
                    debug_print(ch, "DEBUG: Backtrack: room %d came from room %d via dir %d\n", 
                               trace_vnum, parent_vnum[trace_vnum], parent_dir[trace_vnum]);
                    trace_vnum = parent_vnum[trace_vnum];
                }
                
                // Now trace_vnum should be the room directly connected to 'from'
                // Find which direction from 'from' leads to trace_vnum
                for (i = 0; i < PATH_MAX_DIR; i++)
                {
                    if (from->exit[i] && 
                        from->exit[i]->u1.to_room && 
                        from->exit[i]->u1.to_room->vnum == trace_vnum)
                    {
                        debug_print(ch, "DEBUG: First step is direction %d (%s) to room %d\n", 
                                   i, dir_name[i], trace_vnum);
                        result = i;
                        goto cleanup;
                    }
                }
                
                debug_print(ch, "DEBUG: ERROR: Could not find direction from start room to %d\n", trace_vnum);
                result = -1;
                goto cleanup;
            }

            // Add to queue if there's space
            if (back < MAX_TRACK_ROOMS)
                queue[back++] = next_room;
        }
    }

    debug_print(ch, "DEBUG: BFS completed, no path found\n");

cleanup:
    if (visited) free(visited);
    if (queue) free(queue);
    if (depth_arr) free(depth_arr);
    if (parent_vnum) free(parent_vnum);
    if (parent_dir) free(parent_dir);
    
    return result;
}

void do_hunt(CHAR_DATA *ch, char *argument)
{
    /* Alias for do_track to maintain compatibility */
    do_track(ch, argument);
}