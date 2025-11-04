#!/usr/bin/env python3
"""Add orcish class title tables to const.c"""

# Read the file
with open('const.c', 'r') as f:
    lines = f.readlines()

# Find where to insert (after line 836: "};" that closes warrior title table)
insert_line = 836  # 1-indexed

# Copy mage titles and modify for sorcerer
# Copy cleric titles and modify for shaman  
# Copy thief titles and modify for knave
# Copy warrior titles and modify for berserker

# For now, let's use the same titles as the human classes
# This is simpler and we can refine later if needed

# Read the existing title tables to copy them
# Mage titles start around line 537, cleric at 612, thief at 687, warrior at 762

# Actually, let's insert blank title tables first and fill them in
# Each title table has MAX_LEVEL+1 entries (61 entries: 0-60)

# Insert after line 836
new_titles = '''    {
     {"Man", "Woman"},

     {"Apprentice of Dark Arts", "Apprentice of Dark Arts"},
     {"Dark Student", "Dark Student"},
     {"Scholar of Shadow", "Scholar of Shadow"},
     {"Delver in Dark Spells", "Delveress in Dark Spells"},
     {"Medium of Corruption", "Medium of Corruption"},

     {"Scribe of Shadows", "Scribess of Shadows"},
     {"Dark Seer", "Dark Seeress"},
     {"Shadow Sage", "Shadow Sage"},
     {"Necromancer", "Necromancer"},
     {"Dark Abjurer", "Dark Abjuress"},

     {"Dark Invoker", "Dark Invoker"},
     {"Enchanter of Shadows", "Enchantress of Shadows"},
     {"Conjurer of Darkness", "Conjuress of Darkness"},
     {"Dark Magician", "Dark Witch"},
     {"Creator", "Creator"},

     {"Dark Savant", "Dark Savant"},
     {"Shadow Magus", "Shadow Craftess"},
     {"Dark Wizard", "Dark Wizard"},
     {"Warlock of Shadows", "War Witch of Shadows"},
     {"Sorcerer", "Sorceress"},

     {"Elder Sorcerer", "Elder Sorceress"},
     {"Grand Sorcerer", "Grand Sorceress"},
     {"Great Sorcerer", "Great Sorceress"},
     {"Demon Maker", "Demon Maker"},
     {"Greater Demon Maker", "Greater Demon Maker"},

     {"Maker of Dark Stones", "Maker of Dark Stones"},
     {"Maker of Poisons", "Maker of Poisons"},
     {"Maker of Cursed Scrolls", "Maker of Cursed Scrolls"},
     {"Maker of Dark Wands", "Maker of Dark Wands"},
     {"Maker of Shadow Staves", "Maker of Shadow Staves"},

     {"Demon Summoner", "Demon Summoner"},
     {"Greater Demon Summoner", "Greater Demon Summoner"},
     {"Dragon Charmer", "Dragon Charmer"},
     {"Greater Dragon Charmer", "Greater Dragon Charmer"},
     {"Master of all Dark Magic", "Master of all Dark Magic"},

     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},

     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},

     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},
     {"Master Sorcerer", "Master Sorcerer"},

     {"Sorcerer Hero", "Sorcerer Heroine"},
     {"Avatar of Dark Magic", "Avatar of Dark Magic"},
     {"Angel of Shadows", "Angel of Shadows"},
     {"Demigod of Dark Magic", "Demigoddess of Dark Magic"},
     {"Immortal of Dark Magic", "Immortal of Dark Magic"},
     {"God of Dark Magic", "Goddess of Dark Magic"},
     {"Deity of Dark Magic", "Deity of Dark Magic"},
     {"Supremity of Dark Magic", "Supremity of Dark Magic"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}
     },

    {
     {"Man", "Woman"},

     {"Tribal Believer", "Tribal Believer"},
     {"Spirit Attendant", "Spirit Attendant"},
     {"Tribe Acolyte", "Tribe Acolyte"},
     {"Spirit Novice", "Spirit Novice"},
     {"Tribe Missionary", "Tribe Missionary"},

     {"Spirit Adept", "Spirit Adept"},
     {"Tribal Deacon", "Tribal Deaconess"},
     {"Shaman Vicar", "Shaman Vicaress"},
     {"Tribal Priest", "Tribal Priestess"},
     {"Tribe Minister", "Tribe Lady Minister"},

     {"Tribal Canon", "Tribal Canon"},
     {"Spirit Levite", "Spirit Levitess"},
     {"Shaman Curate", "Shaman Curess"},
     {"Tribal Monk", "Tribal Nun"},
     {"Spirit Healer", "Spirit Healess"},

     {"Shaman Chaplain", "Shaman Chaplain"},
     {"Spirit Expositor", "Spirit Expositress"},
     {"Tribal Bishop", "Tribal Bishop"},
     {"Arch Shaman", "Arch Lady of the Tribes"},
     {"Tribal Patriarch", "Tribal Matriarch"},

     {"Elder Tribal Patriarch", "Elder Tribal Matriarch"},
     {"Grand Tribal Patriarch", "Grand Tribal Matriarch"},
     {"Great Tribal Patriarch", "Great Tribal Matriarch"},
     {"Demon Slayer", "Demon Slayer"},
     {"Greater Demon Slayer", "Greater Demon Slayer"},

     {"Shaman of the Sea", "Shaman of the Sea"},
     {"Shaman of the Earth", "Shaman of the Earth"},
     {"Shaman of the Air", "Shaman of the Air"},
     {"Shaman of the Ether", "Shaman of the Ether"},
     {"Shaman of the Heavens", "Shaman of the Heavens"},

     {"Avatar of an Immortal", "Avatar of an Immortal"},
     {"Avatar of a Deity", "Avatar of a Deity"},
     {"Avatar of a Supremity", "Avatar of a Supremity"},
     {"Avatar of an Implementor", "Avatar of an Implementor"},
     {"Master of all Spirits", "Mistress of all Spirits"},

     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},

     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},

     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},
     {"Master Shaman", "Master Shaman"},

     {"Spirit Hero", "Spirit Heroine"},
     {"Spirit Avatar", "Spirit Avatar"},
     {"Spirit Angel", "Spirit Angel"},
     {"Demigod of Spirits", "Demigoddess of Spirits"},
     {"Immortal Shaman", "Immortal Shaman"},
     {"God of Spirits", "Goddess of Spirits"},
     {"Deity of Spirits", "Deity of Spirits"},
     {"Supreme Master Shaman", "Supreme Mistress Shaman"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}
     },

    {
     {"Man", "Woman"},

     {"Filcher", "Filcheress"},
     {"Scoundrel", "Scoundrel"},
     {"Thug", "Thug"},
     {"Pick-Pocket", "Pick-Pocket"},
     {"Sneak", "Sneak"},

     {"Cutpurse", "Cutpurse"},
     {"Ruffian", "Ruffian"},
     {"Scoundrel", "Scoundrel"},
     {"Rogue", "Rogue"},
     {"Knave", "Knave"},

     {"Villain", "Villain"},
     {"Miscreant", "Miscreant"},
     {"Outlaw", "Outlaw"},
     {"Bandit", "Bandit"},
     {"Desperado", "Desperado"},

     {"Backstabber", "Backstabber"},
     {"Assassin", "Assassin"},
     {"Killer", "Murderess"},
     {"Brute", "Brute"},
     {"Cut-Throat", "Cut-Throat"},

     {"Spy", "Spy"},
     {"Grand Spy", "Grand Spy"},
     {"Master Spy", "Master Spy"},
     {"Master Assassin", "Master Assassin"},
     {"Greater Master Assassin", "Greater Master Assassin"},

     {"Master of Shadows", "Mistress of Shadows"},
     {"Master of Stealth", "Mistress of Stealth"},
     {"Master of Deception", "Mistress of Deception"},
     {"Master of Treachery", "Mistress of Treachery"},
     {"Master of Betrayal", "Mistress of Betrayal"},

     {"Crime Lord", "Crime Mistress"},
     {"Infamous Crime Lord", "Infamous Crime Mistress"},
     {"Greater Crime Lord", "Greater Crime Mistress"},
     {"Master Crime Lord", "Master Crime Mistress"},
     {"Darkfather", "Darkmother"},

     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},

     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},

     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},
     {"Master Knave", "Master Knave"},

     {"Knave Hero", "Knave Heroine"},
     {"Avatar of Deception", "Avatar of Deception"},
     {"Angel of Shadows", "Angel of Shadows"},
     {"Demigod of Knaves", "Demigoddess of Knaves"},
     {"Immortal Knave", "Immortal Knave"},
     {"God of Knaves", "God of Knaves"},
     {"Deity of Knaves", "Deity of Knaves"},
     {"Supreme Master Knave", "Supreme Mistress Knave"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}
     },

    {
     {"Man", "Woman"},

     {"Tribe Pupil", "Tribe Pupil"},
     {"Barbarian Recruit", "Barbarian Recruit"},
     {"Tribe Guard", "Tribe Guard"},
     {"Savage Fighter", "Savage Fighter"},
     {"Tribal Warrior", "Tribal Warrior"},

     {"Berserker", "Berserker"},
     {"Veteran Raider", "Veteran Raider"},
     {"Tribal Swordsman", "Tribal Swordswoman"},
     {"Savage Fencer", "Savage Fenceress"},
     {"Ruthless Combatant", "Ruthless Combatess"},

     {"Tribal Hero", "Tribal Heroine"},
     {"Battle-Mad", "Battle-Mad"},
     {"Raging Swashbuckler", "Raging Swashbuckleress"},
     {"Mercenary Raider", "Mercenaress Raider"},
     {"Savage Swordmaster", "Savage Swordmistress"},

     {"Tribal Lieutenant", "Tribal Lieutenant"},
     {"Champion Raider", "Lady Champion Raider"},
     {"Tribal Dragoon", "Lady Tribal Dragoon"},
     {"Barbarian Cavalier", "Lady Barbarian Cavalier"},
     {"Tribal Knight", "Lady Tribal Knight"},

     {"Grand Tribal Knight", "Grand Tribal Knight"},
     {"Master Tribal Knight", "Master Tribal Knight"},
     {"Savage Paladin", "Savage Paladin"},
     {"Grand Savage Paladin", "Grand Savage Paladin"},
     {"Demon Slayer", "Demon Slayer"},

     {"Greater Demon Slayer", "Greater Demon Slayer"},
     {"Dragon Slayer", "Dragon Slayer"},
     {"Greater Dragon Slayer", "Greater Dragon Slayer"},
     {"Tribal Underlord", "Tribal Underlord"},
     {"Tribal Overlord", "Tribal Overlord"},

     {"Baron of Thunder", "Baroness of Thunder"},
     {"Baron of Storms", "Baroness of Storms"},
     {"Baron of Tornadoes", "Baroness of Tornadoes"},
     {"Baron of Hurricanes", "Baroness of Hurricanes"},
     {"Baron of Meteors", "Baroness of Meteors"},

     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},

     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},

     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},
     {"Master Berserker", "Master Berserker"},

     {"Berserker Hero", "Berserker Heroine"},
     {"Avatar of War", "Avatar of War"},
     {"Angel of Battle", "Angel of Battle"},
     {"Demigod of War", "Demigoddess of War"},
     {"Immortal Warlord", "Immortal Warlord"},
     {"God of War", "God of War"},
     {"Deity of War", "Deity of War"},
     {"Supreme Master of War", "Supreme Mistress of War"},
     {"Creator", "Creator"},
     {"Implementor", "Implementress"}
     }
'''

# Split new_titles into lines and insert them
new_lines = new_titles.split('\n')
# Insert after line 836 (index 835)
lines[835:835] = [line + '\n' for line in new_lines]

# Write back
with open('const.c', 'w') as f:
    f.writelines(lines)

print("Added orcish class title tables successfully!")
