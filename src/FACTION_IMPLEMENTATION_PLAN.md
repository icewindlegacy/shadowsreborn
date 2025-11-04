# Faction System Implementation Plan

## Overview
This plan outlines the implementation of a dual-faction system where players are assigned to either the "Human Faction" or "Orcish Faction" based on their race selection, with each faction having its own starting area and mud school.

## Current State Analysis

### Existing Races (Human Faction) ? COMPLETE
- **human** - PC race, 0 points
- **elf** - PC race, 5 points  
- **dwarf** - PC race, 8 points
- **verbeeg** - PC race, 6 points
- **Location**: const.c lines 208-232 (marked with "/* HUMAN FACTION BELOW */" comments)

### New Races (Orcish Faction) ? COMPLETE
- **orc** - PC race, 0 points - ? Added to race_table and pc_race_table
- **troll** - PC race, 6 points - ? Converted from NPC to PC race
- **hobgoblin** - PC race, 8 points - ? Created
- **felar** - PC race, 5 points - ? Created
- **Location**: const.c lines 233-259 (marked with "/* ORC FACTION RACES BELOW */" comments)

### Current Systems Status
- **Race Tables**: ? Races split by faction comments in const.c
- **MAX_PC_RACE**: ? Already incremented to correct value
- **Mud School**: ROOM_VNUM_SCHOOL (3700) - defined in merc.h line 1421
- **Orcish Mud School Area**: ? Built with vnum 400 (temporary location)
- **Orcish Equipment Constants**: ? OBJ_VNUM_ORC_* defined in merc.h (lines 1193-1206)
  - Note: OBJ_VNUM_ORCL_SWORD (line 1195) appears to be a typo - should be OBJ_VNUM_ORC_SWORD
- **Character Creation**: nanny.c handles new character setup, currently hardcoded to ROOM_VNUM_SCHOOL
- **Outfit System**: do_outfit() in act_wiz.c provides school equipment (OBJ_VNUM_SCHOOL_*) - needs faction awareness
- **Clan System**: Separate from factions - used for player organizations at higher levels

## Implementation Strategy

### Phase 1: Data Structure Foundation

#### 1.1 Faction Field Decision
**Decision Point**: Determine whether to:
- **Option A**: Add new `sh_int faction` field to `char_data` structure in merc.h
- **Option B**: Repurpose existing `sh_int clan` field with different semantics
- **Option C**: Use a bit flag or derive faction from race dynamically

**Recommendation**: Option A - Add dedicated faction field for clarity and separation from clan system. This avoids conflicts with existing clan mechanics.

**Implementation**:
- Add `sh_int faction;` to `char_data` structure (around line 1774, near race field)
- Define faction constants in merc.h:
  ```c
  #define FACTION_NONE     0
  #define FACTION_HUMAN    1
  #define FACTION_ORCISH   2
  ```

#### 1.2 Faction Lookup Functions
Create helper functions in lookup.c or appropriate existing file:
- `int get_faction_by_race(int race)` - returns faction based on race
- Uses race_table index to determine faction:
  - Races 1-4 (human, elf, dwarf, verbeeg) = FACTION_HUMAN
  - Races 5-8 (orc, felar, hobgoblin, troll) = FACTION_ORCISH

### Phase 2: Race System Updates ? COMPLETE

#### 2.1 Update Race Constants ? COMPLETE
- `MAX_PC_RACE` already incremented by user
- Race tables properly organized with faction comments

#### 2.2 Add New Races to race_table ? COMPLETE
Located in `const.c` lines 233-259:
- orc (pc_race = TRUE) - line 236
- felar (pc_race = TRUE) - line 242
- hobgoblin (pc_race = TRUE) - line 248
- troll (pc_race = TRUE) - line 254

#### 2.3 Add New Races to pc_race_table ? COMPLETE
Located in `const.c` lines 457-478:
- orc - " Orc ", 0 points, same stats as human
- felar - "Felar", 5 points, similar to elf
- hobgoblin - "Hgbln", 8 points, similar to dwarf
- troll - "Troll", 6 points, similar to verbeeg

### Phase 3: Room and Object Constants

#### 3.1 New Orcish Faction Room Constants ? COMPLETE
Already defined in merc.h:
```c
#define ROOM_VNUM_TEMPLE        3001  /* existing human faction */
#define ROOM_VNUM_SCHOOL        3700  /* existing human faction */
#define ROOM_VNUM_ALTAR         3054  /* existing human faction */
#define ROOM_VNUM_MORGUE        3000  /* existing human faction */

#define ROOM_VNUM_ORCISH_TEMPLE  400  /* new orcish faction */
#define ROOM_VNUM_ORCISH_ALTAR   401  /* new orcish faction */
#define ROOM_VNUM_ORCISH_SCHOOL  402  /* new orcish faction */
#define ROOM_VNUM_ORCISH_MORGUE  403  /* new orcish faction */
```
**Note**: Orcish faction uses vnums 400-403 for temple, altar, school, and morgue.

#### 3.2 New Orcish School Equipment Constants ? COMPLETE
Already defined in merc.h around lines 1193-1206:
```c
#define OBJ_VNUM_ORC_MACE       400
#define OBJ_VNUM_ORC_DAGGER     401
#define OBJ_VNUM_ORC_SWORD      402   /* ? Fixed typo */
#define OBJ_VNUM_ORC_SPEAR      403   /* ? Fixed typo */
#define OBJ_VNUM_ORC_STAFF      404
#define OBJ_VNUM_ORC_AXE        405
#define OBJ_VNUM_ORC_FLAIL      406
#define OBJ_VNUM_ORC_WHIP       407
#define OBJ_VNUM_ORC_POLEARM    408
#define OBJ_VNUM_ORC_VEST       409
#define OBJ_VNUM_ORC_SHIELD     410
#define OBJ_VNUM_ORC_BANNER     411
#define OBJ_VNUM_ORC_MAP        412
```
**Note**: Constants use OBJ_VNUM_ORC_* naming (not OBJ_VNUM_ORCISH_SCHOOL_*).

### Phase 4: Character Creation Logic

#### 4.1 Race Selection Enhancement
In `nanny.c` CON_GET_NEW_RACE case (around line 441-473):
- Update race listing to show faction groupings
- Example: "Human Faction: human, elf, dwarf, verbeeg"
- Example: "Orcish Faction: orc, troll, hobgoblin, felar"

#### 4.2 Faction Assignment
In `nanny.c` CON_GET_NEW_RACE case, after race is validated:
- Call helper function to determine faction from race
- Set `ch->faction = get_faction_by_race(race);`

#### 4.3 Starting Room Assignment
In `nanny.c` CON_READ_MOTD case (around line 806):
- Replace hardcoded `ROOM_VNUM_SCHOOL` with conditional:
  ```c
  if (ch->faction == FACTION_HUMAN)
      char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
  else if (ch->faction == FACTION_ORCISH)
      char_to_room(ch, get_room_index(ROOM_VNUM_ORCISH_SCHOOL));
  else
      char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL)); /* fallback */
  ```

### Phase 5: Outfit System Updates

#### 5.1 Modify do_outfit()
In `act_wiz.c` around line 266-325:
- Add faction check at beginning
- Create separate code paths for each faction
- Use appropriate OBJ_VNUM constants based on faction:
  - Human faction: OBJ_VNUM_SCHOOL_*
  - Orcish faction: OBJ_VNUM_ORC_*
- Keep same logic structure but duplicate equipment creation per faction

### Phase 5.5: Recall Command Updates

#### 5.5.1 Modify do_recall()
In `act_move.c` around line 1860-1945:
- Currently defaults to ROOM_VNUM_TEMPLE (line 1878)
- Add faction check before setting location
- If faction == FACTION_HUMAN: use ROOM_VNUM_TEMPLE
- If faction == FACTION_ORCISH: use ROOM_VNUM_ORCISH_TEMPLE
- Clan recall logic remains unchanged (checks clan_table[ch->clan].recall)

#### 5.5.2 Modify spell_word_of_recall()
In `magic.c` around line 4716-4747:
- Currently uses ROOM_VNUM_TEMPLE (line 4725)
- Add faction check for victim
- If victim->faction == FACTION_HUMAN: use ROOM_VNUM_TEMPLE
- If victim->faction == FACTION_ORCISH: use ROOM_VNUM_ORCISH_TEMPLE

### Phase 6: Save/Load System

#### 6.1 Save Character
In `save.c` fwrite_char() function:
- Add faction field to save file format
- Format: `Faction <faction_number>\n`

#### 6.2 Load Character  
In `save.c` or `db.c` load function:
- Parse faction field from save file
- Set `ch->faction` appropriately
- Handle migration for existing characters (default to FACTION_HUMAN)

### Phase 7: Area Building (Not Code) ? COMPLETE

#### 7.1 Orcish Mud School Area ? COMPLETE
- User has already built the area file
- Everything set to vnum 400 for temporary location
- Includes recall, temple, mud school, etc.

### Phase 8: Integration and Testing

#### 8.1 Race-Based Systems Review
Check and update if needed:
- NPC reactions to races (is_friend() in handler.c)
- Combat systems that check race
- Social commands that display race information
- Any quest systems that reference specific races

#### 8.2 Optional Enhancements
- Add faction display to who/whois commands
- Create faction-specific help files
- Add faction-based social commands
- Consider faction-based PvP restrictions

## File Modification Summary

### Files Requiring Code Changes:
1. **merc.h**
   - Add faction constants (FACTION_HUMAN, FACTION_ORCISH)
   - Add `faction` field to `char_data` structure
   - ? Room constants already defined (ROOM_VNUM_ORCISH_*)

2. **lookup.c** (or appropriate file)
   - Add get_faction_by_race() helper function

3. **nanny.c**
   - Update race selection prompt (optional)
   - Add faction assignment on race selection
   - Modify starting room assignment logic

4. **act_wiz.c**
   - Modify do_outfit() to be faction-aware

5. **act_move.c**
   - Modify do_recall() to use faction-appropriate temple

6. **magic.c**
   - Modify spell_word_of_recall() to use faction-appropriate temple

7. **save.c**
   - Add faction save/load code

### Files Already Complete ?:
- **const.c** - Races already added and organized
- **merc.h** - Equipment constants already defined
- **Area files** - Orcish mud school area already built

## Testing Checklist

- [ ] New human faction race starts in correct mud school
- [ ] New orcish faction race starts in correct mud school  
- [ ] Faction is correctly assigned based on race selection
- [ ] Outfit command gives appropriate faction equipment
- [ ] Character save/load preserves faction
- [ ] Existing characters migrate correctly (default to human faction)
- [ ] All new races appear in race selection list
- [ ] No crashes or bugs during character creation

## Notes

- The clan system (clan.c) is separate from factions and should not be modified
- Factions are assigned at character creation and determine starting location
- Clans are player organizations that can be joined later in the game
- The orcish mud school area has been built with vnum 400
- Equipment constant typos have been fixed (OBJ_VNUM_ORC_SWORD, OBJ_VNUM_ORC_SPEAR)
- Orcish faction room constants defined: TEMPLE (400), ALTAR (401), SCHOOL (402), MORGUE (403)

## Remaining Work Summary

### Critical Tasks (Must Complete):
1. **Add faction field to char_data structure** (merc.h)
2. **Create faction constants** (FACTION_HUMAN, FACTION_ORCISH)
3. **Create helper function** get_faction_by_race() in lookup.c
4. ~~**Add ROOM_VNUM_ORCISH_SCHOOL constant**~~ ? Already defined (value 402)
5. **Modify nanny.c** to assign faction on race selection
6. **Modify nanny.c** to place in correct mud school based on faction
7. **Modify do_outfit()** to use faction-appropriate equipment
8. **Modify do_recall()** in act_move.c to use faction-appropriate temple
9. **Modify spell_word_of_recall()** in magic.c to use faction-appropriate temple
10. **Add save/load support** for faction field

### Optional Tasks:
- Update race selection prompt to show faction groupings
- Review race-based NPC reactions for faction awareness
- Add faction display to who/whois commands
- Update help files

## Dependencies ? COMPLETE

- ? Race stats defined
- ? Orcish mud school area built
- ? Orcish school equipment objects created
- ? Orcish school mobiles created
