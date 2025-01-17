inv_source_text="""
# Protos appearing at random encounters have a "9" added to the front.
# This sets random treasure very low, to discourage random treasure farming.

# Marvis, Sylvvi's Assistant (14021)
33
100,COINS BY VALUE,3,600,0
100,(light hammer,red villager garb,black leather gloves,black leather boots)
100,(armor oil,armor oil)

# Marvis inventory
1033
100,GEMS OR JEWELRY,0,0,4

# Ogre Basher, B1/F6  (formerly Lubash)
57
20,CGI,10,1,1  100,COINS BY VALUE,1,10000,0
100,WEAPON HILL GIANT 1
100,chain shirt

# Gnoll leader, F2 (14066)
58
100,CGI,3,1,1
20,gnoll double axe   40,WEAPON GNOLL ONE-HANDED   60,WEAPON TWO-HANDED   100,WEAPON TWO-HANDED 0
40,CHAIN SHIRT   60,ARMOR LIGHT   100,ARMOR LIGHT 0
100,(gnoll shield 2,gnoll sergeants helmet)

# Gnoll, Gnoll Berzerker (14067, 14078) 
59
100,CGI,1,3,1
75,WEAPON GNOLL ONE-HANDED   98,WEAPON ONE-HANDED   100,WEAPON ONE-HANDED 0
75,LEATHER ARMOR  98,ARMOR LIGHT   100,ARMOR LIGHT 0
25,gnoll shield 1   50,gnoll shield 3   75,gnoll shield 4   100,battered gnoll shield
50,gnoll helmet   100,gnoll helmet 2

# Gnoll, Gnoll Berzerker (14067, 14078) 
9059
100,COINS BY VALUE,2,600,0
75,WEAPON GNOLL ONE-HANDED   100,WEAPON ONE-HANDED
75,LEATHER ARMOR  100,ARMOR LIGHT
25,gnoll shield 1   50,gnoll shield 3   75,gnoll shield 4   100,battered gnoll shield
50,gnoll helmet   100,gnoll helmet 2


# --------------------------------------------------------------------------------------------------
# Brigands and Bandits
#
# Inventory ID kept the same for all of them,
# except for Grank and his Bandits that were changed to match their proto.
#
# proto  Temple            Cove              Notes, portraits
# --------------------------------------------------------------------------------------------------
# 14069  Brigand Leader    Brigand Leader    Moathouse interior, B1 --> R3
#                                            was: 6800 (Raul the grim)
#                                            now: 7200 (Mandy)  
# 14070  Brigand           Brigand Sneak     Moathouse brigands, R1 --> F1/R1
#                                            was: 6810 (helm w/yellow hair, now Janni)
#                                            now: 6870 (murderous thief from city vignette)
#
# 14074  Guardsman         - unused -        6780, armored man w/helmet, red background.
# 14075  Sergeant          - unused -        6780, armored man w/helmet, red background.
# 14076  Sergeant          - unused -        6770, armored man no-helmet, red background.
# 14077  Lieutenant        - unused -        6790, selucus no-helmet, red background.
#
# 14205  Brigand Prisoner  Brigand           Wicked
#
# 14255  Bandit Leader     Bandit Leader     Grank, now Todd Ambush, C4 --> F4
# 14357  Granks Bandit     Bandit            Grank, now Todd Ambush, R8 --> F1/R1
#
# 14310  Brigand Leader    Brigand Leader    Broken Tower, 6820 unchanged, same levels.
# 14311  Brigand Lieut     Brigand Deputy    Broken Tower, was:6830, now:6800 (Raul the grim)
# 14312  Brigand Footman   Brigand           Broken Tower, was:6830, now:6870 (murderous thief), F2/R1 --> F1
# 14313  Brigand Archer    Brigand Archer    Broken Tower, 6830 unchanged, same levels.
# 14314  Brigand Crossbow  Brigand Crossbow  Broken Tower, 6830 unchanged, same levels.
#
# Portrait 6830 is the guy with the chain-hood and the red face.
# --------------------------------------------------------------------------------------------------

# Brigand Leader, R3 (14069)
63
100,CGI,3,1,1
40,WEAPON ROGUE  100,WEAPON ROGUE 0  
20,ARMOR LIGHT  40,CHAIN SHIRT  100,CHAIN SHIRT 0
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
50,black cloak  100,red cloak
100,(hoodless circlet,GLOVES,SIMPLE BOOTS)

# Brigand Sneak, F1/R1 (14070)
65
100,CGI,1,3,1
40,WEAPON ROGUE  100,WEAPON ROGUE 0  
50,LEATHER ARMOR   80,ARMOR LIGHT   100,ARMOR LIGHT 0
100,(LIGHT HELM,GLOVES,SIMPLE BOOTS)

# Brigand Sneak, F1/R1 (14070)
9065
100,COINS BY VALUE,2,400,0
100,WEAPON ROGUE  
50,LEATHER ARMOR   100,ARMOR LIGHT
100,(LIGHT HELM,GLOVES,SIMPLE BOOTS)

# Skeleton Gnoll (14081,14082)
70
20,gnoll single axe  40,gnoll heavy mace  60,gnoll morningstar  80,gnoll double axe  98,spiked chain  100,WEAPON MELEE 0
48,battered gnoll shield   50,SHIELD 0

# Skeleton Gnoll (14081,14082)
9070
20,gnoll single axe  40,gnoll heavy mace  60,gnoll morningstar  80,gnoll double axe  100,spiked chain
50,battered gnoll shield

# Lizardfolk (14084, 14085)
71
100,CGI,1,3,1
50,lizardman club   98,lizardman greatclub   100,WEAPON TWO-HANDED 0

# Lizardfolk (14084, 14085)
9071
100,COINS BY VALUE,2,400,0
50,lizardman club   100,lizardman greatclub

# Bugbear (14093)
72
100,CGI,2,3,1
75,WEAPON BUGBEAR   98,WEAPON MELEE   100,WEAPON MELEE 0
75,LEATHER ARMOR   98,ARMOR LIGHT   100,ARMOR LIGHT 0
75,bugbear-gnoll shield   98,SHIELD   100,SHIELD 0

# Bugbear (14093)
9072
100,COINS BY VALUE,3,600,0
75,WEAPON BUGBEAR   100,WEAPON MELEE
75,LEATHER ARMOR   100,ARMOR LIGHT
75,bugbear-gnoll shield   100,SHIELD

# Romag Ghost (14154)
92
100,(fancy brown robes,black leather boots)

# Priest of Earth, Hartsch, C9 (14154, 123)
93
100,COINS BY VALUE,3,6000,0
100,masterwork heavy mace
100,full plate +1
100,LARGE SHIELD
100,amulet of wisdom +2
100,(fancy brown robes,black leather boots)
100,shield of faith_3
100,barkskin_3
100,resist energy acid

# Alrem Ghost (14155)
94
100,(fancy crimson robes,black leather boots)

# Goblin Sergeant, F2/R1 (14183)
97
100,CGI,3,3,1
40,WEAPON LIGHT   100,WEAPON LIGHT 0
20,ARMOR LIGHT   40,CHAIN SHIRT   80,CHAIN SHIRT 0   100,ARMOR LIGHT 0
40,SHIELD NO TOWER   100,SHIELD NO TOWER 0
5,RING OR AMULET 1
5,CLOAK OF RESISTANCE 1

# Goblin (14186, 14187)
98
100,CGI,1,9,1
75,light mace   98,WEAPON LIGHT   100,WEAPON HALFLING MELEE 0
75,LEATHER ARMOR   98,ARMOR LIGHT   100,ARMOR LIGHT 0
75,SMALL SHIELD   98,SHIELD   100,SHIELD 0

# Goblin (14186, 14187)
9098
100,COINS BY VALUE,2,40,0
75,light mace   100,WEAPON LIGHT
75,LEATHER ARMOR   100,ARMOR LIGHT
75,SMALL SHIELD   100,SHIELD

# Hobgoblin (14188, 14189)
99
100,CGI,1,6,1
75,longsword   98,WEAPON MELEE   100,WEAPON MELEE 0
75,javelin
75,STUDDED LEATHER   98,ARMOR LIGHT   100,ARMOR LIGHT 0
75,SMALL SHIELD   98,SHIELD   100,SHIELD 0

# Hobgoblin (14188, 14189)
9099
100,COINS BY VALUE,2,400,0
75,longsword   100,WEAPON MELEE
75,javelin
75,STUDDED LEATHER   100,ARMOR LIGHT
75,SMALL SHIELD   100,SHIELD

# Belsornig Ghost (14193)
100
100,(fancy moss-green robes,black leather boots)

# Kelno Ghost (14194)
101
100,(fancy ivory robes,black leather boots)

# Troll Chief, B3 (14195)  Oohlgrist, unused right now
102
100,CGI,8,1,1
40,WEAPON MARTIAL TWO-HANDED  100,WEAPON MARTIAL TWO-HANDED 0
40,bone armor  100,ARMOR LIGHT 0
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1

# Priest of Water, C9 (14197)
# toggles Shield of Faith +3 (shield of faith_3)
# toggles Barkskin +3 (barkskin_3)
103
100,COINS BY VALUE,3,6000,0
100,masterwork heavy mace
100,full plate +1
100,LARGE SHIELD
100,amulet of wisdom +2
100,(fancy moss-green robes,black leather boots)
100,resist energy cold

# Priest of Fire, C9 (14211)
# toggles Shield of Faith +3 (shield of faith_3)
# toggles Barkskin +3 (barkskin_3)
105
100,COINS BY VALUE,3,6000,0
100,masterwork heavy mace
100,full plate +1
100,LARGE SHIELD
100,amulet of wisdom +2
100,(fancy crimson robes,black leather boots)
100,resist energy fire

# Priest of Air, C9 (14212)
# toggles Shield of Faith +3 (shield of faith_3)
# toggles Barkskin +3 (barkskin_3)
106
100,COINS BY VALUE,3,6000,0
100,masterwork heavy mace
100,full plate +1
100,LARGE SHIELD
100,amulet of wisdom +2
100,(fancy ivory robes,black leather boots)
100,resist energy electricity

# Hill Giants
#  14217 - used for random encounters (inv = 107)
#  14218 - used in Roso ambush, faction = 9 (inv = 107)
#  14219 - used for temple exterior, and giant/drow quest, F4 (inv = 314)
#  14220 - used for luxwood dungeon, B2 (inv = 310)
#  14221 - model looks like caveman, others have armor and helm
#  14864 - used for random farm encounter (inv = 864)

# Hill Giant (14217, 14218, 14221)
107
20,CGI,7,1,1  100,COINS BY VALUE,3,70000,0
100,WEAPON HILL GIANT
5,hill giant rock,0,0,3

# Hill Giant (14217, 14218, 14221)
9107
100,COINS BY VALUE,4,2000,0
100,WEAPON HILL GIANT

# Scorpp (14272)
108
100,gold key_2
100,CGI,13,3,1
100,WEAPON HILL GIANT 2
100,chain shirt

# Scorpp's Mom (14699)
699
100,CGI,13,3,1
100,WEAPON HILL GIANT 3
100,chain shirt

# Forest Troll (Troll Hydra Keeper), B1/F5 (14224)
109
20,CGI,11,3,1  100,COINS BY VALUE,1,11000,0
100,LONGSPEAR 0
100,bone armor
100,POTION MELEE

# Thrommel (14234)
117
100,unholy longsword +4
100,prince thrommels plate armor
100,large steel shield_2
100,prince thrommels golden amulet
100,prince thrommels golden belt
100,prince thrommels golden ring
100,brown leather boots
100,barkskin_4

# Ettin (14238)
120
20,CGI,6,3,1  100,COINS BY VALUE,3,6000,0
75,ettin club   100,greathammer
100,ettin club_2

# Ettin (14238)
9120
100,COINS BY VALUE,3,600,0
75,ettin club   100,greathammer
100,ettin club_2

# Ettin, Beneath, Air Electricity Enchanter area(14238)
1201
20,CGI,6,3,1  100,COINS BY VALUE,3,6000,0
100,masterwork greathammer
100,ettin club_2
100,chain shirt
100,shield spell

# Ettin, B1 with drow Noble group (14238)
1202
20,CGI,6,3,1  100,COINS BY VALUE,3,6000,0
100,masterwork greathammer
100,ettin club_2
100,chain shirt
100,shield spell
100,shield of faith_3

# Minotaur (14241)
121
100,CGI,5,1,1
50,POTION MELEE MAJOR

# Harpy (14243)
122
100,CGI,3,3,1

# Ogre Chief, B4 (14248)  Cave Chief, unused right now
124
100,CGI,7,1,1
75,WEAPON HILL GIANT 0   100,WEAPON HILL GIANT 1
75,HIDE ARMOR 0   100,HIDE ARMOR 1
10,RING OF PROTECTION 1
10,AMULET 1
10,CLOAK OF RESISTANCE 1

# Ogre (14249, 14448)  Cave Ogre, Random
125
100,CGI,3,3,1
50,ogre club   98,ogre club_2   100,WEAPON MARTIAL TWO-HANDED 0
25,POTION MELEE

# Ogre (14249, 14448)  Cave Ogre, Random
9125
100,COINS BY VALUE,3,600,0
50,ogre club   100,ogre club_2

# Feldrin, Bandit Commander, F4/R9 (14253)
126
100,COINS BY VALUE,3,6000,0
100,longsword +3
100,leather armor +2
100,cloak of resistance +3 blue
100,brown leather boots

# Brunk, Bandit Lieutenant, F8/R4 (14254)
127
100,COINS BY VALUE,2,6000,0
100,poison dart,0,0,12
100,chain shirt
100,LARGE SHIELD 1
100,ring of protection +2
100,brown leather boots

# Darley, Sorceress, W13 (14268, 171)
131
20,CGI,13,3,1  100,COINS BY VALUE,1,13000,0
100,masterwork warhammer
100,little red dress
100,(black leather gloves,black leather boots)
100,mage armor
100,shield spell

# Great Ogre Chieftain, B12 (14354)  (formerly Greater Temple Ogre Chieftain)
354
100,CGI,12,1,1
100,WEAPON HILL GIANT 3
100,CHAIN SHIRT 3
100,RING OF PROTECTION 1

# Assassin, R10 (14303)
136
100,COINS BY VALUE,1,10000,0
100,GEMS,2,6,0
50,(shortsword +2,shortsword +2)   100,(rapier +2,rapier +2)
100,ARMOR LIGHT 2
100,(BOOTS AND GLOVES)
100,RING OF PROTECTION 1
100,AMULET 1
100,CLOAK OF RESISTANCE 1
50,black cloak   75,red cloak   100,blue cloak

# Brigand Leader, F5/R1 (14310)
142
20,CGI,6,1,1  100,COINS BY VALUE,1,6000,0
100,(LONGSWORD 1,WEAPON MARTIAL LIGHT)
100,(CHAIN SHIRT 1,LARGE SHIELD 1)
100,(chainmail gloves,chainmail boots)

# Brigand Lieutenant, F3/R1 (14311) 
143
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
20,WEAPON MARTIAL ONE-HANDED   100,WEAPON MARTIAL ONE-HANDED 0
20,COMPOSITE LONGBOW STR16   100,COMPOSITE LONGBOW STR16 0
20,CHAIN SHIRT   100,CHAIN SHIRT 0
100,(plate gloves,plate boots)

# Brigand, F1 (14312)
144
100,CGI,1,3,1
80,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0
80,scale mail  100,ARMOR 0
80,LARGE SHIELD  100,LARGE SHIELD 0
50,LIGHT HELM
100,black leather boots

# Brigand, F1, battlefield (14312)
1441
100,CGI,1,3,1
80,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0
80,scale mail  100,ARMOR 0
80,LARGE SHIELD  100,LARGE SHIELD 0
50,LIGHT HELM
100,black leather boots
25,scroll of grease  50,scroll of bulls strength  75,scroll of glitterdust  100,scroll of scorching ray

# Brigand Archer, F1 (14313)
145
100,CGI,1,3,1
80,LONGBOW   100,LONGBOW 0
100,WEAPON MARTIAL LIGHT
80,ARMOR LIGHT  100,ARMOR LIGHT 0
100,buckler
100,chain helmet
100,(GLOVES,SIMPLE BOOTS)

# Brigand Crossbowman, F1 (14314)
146
100,CGI,1,3,1
80,LIGHT CROSSBOW   100,LIGHT CROSSBOW 0
100,WEAPON MARTIAL LIGHT
80,ARMOR LIGHT  100,ARMOR LIGHT 0
100,chain helmet
100,(GLOVES,SIMPLE BOOTS)

# Giant Gar, THIS IS USED FOR ANY CREATURE THAT HAS NO INVENTORY, or EMPTY INVENTORY 14329)
147
0,copper

# Balor (14286)
155
0,balor vorpal sword

# Human Cleric, Ikian (14332, 201, tracking=332)
149
100,COINS BY VALUE,1,10000,0
100,masterwork light mace
100,full plate +1
100,steel shield +2
100,plate boots
100,shield of faith_3

# Half-elven Wizard, Vincent (14333, 716)
150
100,COINS BY VALUE,1,10000,0
100,masterwork dagger
100,light blue robes
100,BOOTS
100,mage armor
100,shield of faith_3

# Dwarven Fighter, Ogavick (14334, 2)
151
100,COINS BY VALUE,1,10000,0
100,greathammer +2
100,dwarven stone armor +2
100,(chain helmet,chainmail boots,chainmail gloves)
100,shield of faith_3

# Halfling Rogue, Non (14335, 2)
152
100,COINS BY VALUE,1,10000,0
100,shortsword +2
100,LEATHER ARMOR
100,(brown leather gloves,brown leather boots)
100,mage armor
100,shield of faith_3

# Elven Ranger, Neukoln (14336, 2)
153
100,COINS BY VALUE,1,10000,0
100,longsword +2
100,dagger
100,chain shirt +2
100,(brown leather gloves,brown leather boots)
100,shield of faith_3

# Earth Temple Guard, now Ghost (14337)
156
100,masterwork longsword
100,(brown robes,black leather boots)

# Ogre Shaman, C3 (14353)
163
20,CGI,6,3,1  100,COINS BY VALUE,1,6000,0
40,WEAPON HILL GIANT   100,WEAPON HILL GIANT 0
40,HIDE ARMOR   100,HIDE ARMOR 0
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1

# Balor (14358)
165
0,balor vorpal sword

# Glabrezu (14359)
166
0,copper

# Hezrou (14360)
167
0,copper

# Vrock (14361)
168
0,copper

# Villager (14001)
174
100,COINS BY VALUE,2,600,0
30,FARMER GARB  90,VILLAGER GARB  100,FANCY GARB
100,SIMPLE BOOTS

# Elf, Luxwood (14201,14202)
1741
100,COINS BY VALUE,2,600,0
50,GARB  100,ARMOR LIGHT
25,LONGBOW  50,LONGSWORD  75,dagger  100,SHORTBOW
100,SIMPLE BOOTS

# Elf, Luxwood, prisoner (14201,14202)
1742
10,COINS BY VALUE,2,600,0
10,rusty dagger
25,FARMER GARB  50,VILLAGER GARB  75,SKIRTLESS GARB
50,SIMPLE BOOTS

# Elf, Luxwood, slaughtered exterior(14201,14202)
1743
25,COINS BY VALUE,2,600,0
10,WONDROUS ITEM
25,MUNDANE PERSONAL,1,2,0
25,FOOD,1,2,0
25,HERBS AND SPICES,1,2,0
25,dagger
25,FARMER GARB  50,VILLAGER GARB  75,SKIRTLESS GARB
25,HAT
50,SIMPLE BOOTS

# Elf, Luxwood, Ruins, after Gila is dead (14201,14202)
1744
25,COINS BY VALUE,2,600,0
10,WONDROUS ITEM
25,MUNDANE PERSONAL,1,2,0
25,FOOD,1,2,0
25,HERBS AND SPICES,1,2,0
25,dagger
25,FARMER GARB  50,VILLAGER GARB  100,SKIRTLESS GARB
25,HAT
100,SIMPLE BOOTS

# Villager (14000)
177
100,COINS BY VALUE,2,600,0
30,FARMER GARB  90,VILLAGER GARB  100,FANCY GARB
100,SIMPLE BOOTS

# Village Boy (14001)
178
100,COINS BY VALUE,2,60,0
50,FARMER GARB  100,VILLAGER GARB   
100,SIMPLE BOOTS

# Village Girl (14002)
179
100,COINS BY VALUE,2,60,0
50,FARMER GARB  100,VILLAGER GARB   
100,SIMPLE BOOTS

# Pirate, B1/R1 (14290)
199
20,CGI,2,1,1  100,COINS BY VALUE,1,2000,0
25,shortsword  50,cutlass  75,WEAPON LIGHT  100,WEAPON LIGHT 0
33,LEATHER ARMOR  66,STUDDED LEATHER  100,ARMOR LIGHT
100,buckler
100,blue bandana
100,BOOTS

# Pirate, B1/R1 (14290)
9199
100,COINS BY VALUE,2,400,0
25,shortsword  50,cutlass  100,WEAPON LIGHT
33,LEATHER ARMOR  66,STUDDED LEATHER  100,ARMOR LIGHT
100,buckler
100,blue bandana
100,BOOTS

# Gnome Daggermaster F4/R7 (14200)
200
100,COINS BY VALUE,1,11000,0
100,DAGGER 0
100,LEATHER ARMOR 0
25,RING OF PROTECTION 1
25,DEXTERITY 1
100,masterwork thieves tools
100,BOOTS

# Lamia (14342)
236
100,CGI,7,1,1

# Bandit Bowman, F9 (14423)
239
100,COINS BY VALUE,2,4000,0
100,COMPOSITE LONGBOW STR16 0
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
100,LEATHER ARMOR
100,buckler
100,(brown monk robes,leather helm,brown leather boots)
100,archers brew

# Bugbear Mercenary, F6/R1 (14347) (was G.T. Bug Lt.)
251
20,CGI,7,1,1  100,COINS BY VALUE,1,10000,0
100,GREATSWORD 0
100,banded mail
20,RING OF PROTECTION 1
20,cloak of resistance +1 red
100,(plate boots,plate gloves)

# Grank, F4 (14255)
255
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
75,BATTLEAXE 0  100,BATTLEAXE 1
75,CHAINMAIL 0  100,CHAINMAIL 1
20,LARGE SHIELD  100,LARGE SHIELD 0
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 red  100,red cloak
100,barbarian helm
100,combat boots

# Bugbear Mercenary Leader, F11 (14346) (was G.T. Bug Leader)
256
20,CGI,10,1,1  100,COINS BY VALUE,1,10000,0
100,RANSEUR 2
100,FULL PLATE 1
50,RING OF PROTECTION 1
50,cloak of resistance +1 unseen
25,STRENGTH 1
100,(plate boots,plate gloves)
100,potion of heal

# Bugbear Archer, F1 (14172)
261
100,CGI,3,3,1
40,(bugbear longbow,quiver of arrows,quiver of arrows)  80,COMPOSITE LONGBOW STR14  100,COMPOSITE LONGBOW STR14 0
75,WEAPON BUGBEAR ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED
75,LEATHER ARMOR  98,ARMOR LIGHT  100,ARMOR LIGHT 0

# Bugbear Warrior, F1/R1 flanker (14173)
262
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
75,WEAPON BUGBEAR ONE-HANDED  98,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0
75,STUDDED LEATHER  98,ARMOR LIGHT  100,ARMOR LIGHT 0
75,GNOLL SHIELD  98,LARGE SHIELD  100,LARGE SHIELD 0

# Bugbear Berzerker, B1 (14171)
263
100,CGI,3,3,1
60,WEAPON BUGBEAR TWO-HANDED  80,WEAPON MARTIAL TWO-HANDED  100,WEAPON MARTIAL TWO-HANDED 0
60,STUDDED LEATHER  80,ARMOR LIGHT  100,ARMOR LIGHT 0

# Bugbear Leader, F2 (14174)
264
100,CGI,5,1,1
20,WEAPON BUGBEAR ONE-HANDED  40,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0
20,CHAIN SHIRT  40,ARMOR LIGHT  100,CHAIN SHIRT 0
20,GNOLL SHIELD  40,LARGE SHIELD  100,LARGE SHIELD 0

# Female Bugbear (14213, 14214, 14215, 14216)
265
100,CGI,2,3,1
50,SHORTBOW
60,WEAPON BUGBEAR ONE-HANDED  98,WEAPON ONE-HANDED  100,WEAPON ONE-HANDED 0
60,STUDDED LEATHER  98,ARMOR LIGHT  100,ARMOR LIGHT 0

# Bugbear Gang Archer, F7 (14176)
266
100,COINS BY VALUE,1,10000,0
100,masterwork composite longbow str 18
50,quiver of arrows +2
100,(quiver of arrows,quiver of arrows,quiver of arrows)
100,WEAPON MARTIAL ONE-HANDED
100,ARMOR LIGHT 0
100,BUCKLER 0
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1
25,potion of haste

# Bugbear Gang Rager, B7 (14175)
267
100,COINS BY VALUE,1,10000,0
100,WEAPON MARTIAL TWO-HANDED 0
100,ARMOR LIGHT 0
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1

# Bugbear Gang Fighter, F4/R3 flanker (14177)
268
100,COINS BY VALUE,1,10000,0
100,WEAPON MARTIAL ONE-HANDED 0
100,ARMOR LIGHT 0
50,bugbear round temple shield  100,bugbear notched temple shield
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1

# Bugbear Gang Leader, F9 (14174)
269
20,CGI,12,1,1  100,COINS BY VALUE,1,12000,0
50,LONGSWORD 0  100,BATTLEAXE 0
50,CHAIN SHIRT 0  100,ARMOR LIGHT 0
50,bugbear round temple shield  100,bugbear notched temple shield
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1

# Ogre Shaman, C5 (14356)
278
20,CGI,8,3,1
50,WEAPON HILL GIANT 0  50,WEAPON HILL GIANT 1
50,HIDE ARMOR 0  50,HIDE ARMOR 1
25,RING OF PROTECTION 1
25,AMULET 1
25,CLOAK OF RESISTANCE 1

# King Behemoth Frog (14445)
285
100,CGI,6,1,1

# Giant Frog, battlefield (14445)
2851
100,ancient pendant,0,0,1
100,scroll of sleep,0,0,3
100,scroll of magic missile,0,0,3
100,scroll of mage armor,0,0,1
100,scroll of eagles splendor,0,0,1
100,potion of restoration,0,0,2
100,(WIZARD ROBES,WIZARD HAT,BOOTS,scholars kit)

# Skeleton (14107)
292
20,CGI,1,3,1
25,WEAPON RANGED  50,(WEAPON ONE-HANDED,SHIELD)  75,WEAPON ONE-HANDED  98,WEAPON TWO-HANDED  99,WEAPON RANGED 0  100,WEAPON 0 

# Skeleton (14107)
9292
25,WEAPON RANGED  50,(WEAPON ONE-HANDED,SHIELD)  75,WEAPON ONE-HANDED  100,WEAPON TWO-HANDED 

#------------------------------------------------------------------------------
# Original vanilla game Inventory Source numbers end here, at 296.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# New inventory Source numbers, created by marc, start here, at 297.
#
# Numbers 297-450 are reserved for creature protos from the original game
# that never had an inventory number, or for creature protos that had an
# identical inventory number as another proto, but need a unique inventory.
#
# Example: All four goblins from the original game used #98, but when one of
# them were re-purposed as a Goblin Archer, it needed different equipment
# suiting an archer, so #299 was created for him.
#
# The inventory source number does not need to correspond to the proto number.
# Any number can be chosen between 297-450.
#------------------------------------------------------------------------------

# Merrow (14108)
297
100,CGI,3,3,1
50,merrow longspear   73,ogre club   75,WEAPON MARTIAL TWO-HANDED 0
100,LEATHER ARMOR

# Merrow (14108)
9297
50,merrow longspear   100,ogre club

# Toll Thug, F6 (originally Skole's Goon) (14315)
295
100,COINS BY VALUE,1,6000,0
100,masterwork bastard sword
100,chainmail
100,(black monk robes,blue cloak,generic helm,black leather boots)
100,barkskin_3

# Goblin, R1 Weapon Finesse (14184)
298
100,CGI,1,3,1
50,dagger   90,WEAPON HALFLING LIGHT   100,WEAPON HALFLING LIGHT 0
50,LEATHER ARMOR   90,ARMOR LIGHT   100,ARMOR LIGHT 0
45,SMALL SHIELD   50,SHIELD 0

# Goblin, F1 Archer (14185)
299
100,CGI,1,3,1
80,SHORTBOW   100,SHORTBOW 0
98,WEAPON LIGHT   100,WEAPON LIGHT 0
40,LEATHER ARMOR   80,ARMOR LIGHT   100,ARMOR LIGHT 0

# Goblin, F8 Archer, Luxwood (14185)
# Uses F1 proto, with stat changes to simulate an F8 Archer.
# Wears no armor or buckler, to offset increased dexterity.
# Uses special +6 Shortbow, to simulate increased levels.
# See Readme in World Builder Mobs folder for complete explanation.
2991
20,CGI,8,1,1  100,COINS BY VALUE,1,8000,0
100,goblin shortbow
100,quiver of arrows,0,0,5
100,barkskin_3

# Goblin, F1 Archer (14185)
9299
100,COINS BY VALUE,3,60,0
100,SHORTBOW
100,WEAPON LIGHT
50,LEATHER ARMOR   100,ARMOR LIGHT

# Hobgoblin Sergeant, F3 (14189)
300
100,CGI,3,3,1
40,WEAPON MARTIAL TWO-HANDED   100,WEAPON MARTIAL TWO-HANDED 0
100,WEAPON LIGHT
40,ARMOR HEAVY   100,ARMOR HEAVY 0
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1

# Troll (14262)
301
20,CGI,5,1,1
50,GEMS OR JEWELRY,0,0,1

# Troll, spearchucker (14262)
3011
20,CGI,5,1,1
50,GEMS OR JEWELRY,0,0,1
100,troll throwing spear,0,0,10

# Troll (14262)
9301
10,COINS BY VALUE,3,60,0

# Efreeti (14340)
302
100,CGI,7,1,1

# Lizardfolk Patriarch, B3 (14086)
304
100,CGI,3,3,1
40,greatclub   90,masterwork greatclub   100,greatclub +1
100,bone armor
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1

# Lizardfolk Shaman, D3  (14087)
305
100,CGI,3,3,1
50,trident   90,masterwork trident   100,trident +1
50,bone armor   100,shell armor
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(scroll of chill metal,scroll of summon natures ally ii)

# Gelatenous Cube (14137)
306
100,CGI,3,1,1

# Sea Hag (14279)
307
100,CGI,4,1,1

# Sea Hag (14279)
9307
50,COINS BY VALUE,3,600,0

# Werewolf (14344)
308
100,CGI,3,3,1

# Werewolf (14344)
9308
50,COINS BY VALUE,3,600,0

# Ghast Giant, Luxwood (14136)
309
100,MAGIC MINOR
100,scroll of raise dead
100,HERBS AND SPICES,1,4,1

# Hill Giant, Luxwood dungeon, B2 (14220)
310
20,CGI,9,3,1   100,COINS BY VALUE,3,90000,0
100,ancient figurine
100,hill giant spiked chain
100,studded leather armor

# Tuelk, Orc Guard, F10/R3 (14206, 131)
311
20,CGI,13,3,1  100,COINS BY VALUE,1,13000,0
100,shortsword +2
100,black monk robes
100,BUCKLER 2
100,RING OF PROTECTION 2
100,cloak of resistance +2 unseen
100,DEXTERITY 1
100,brown leather boots
100,mage armor
100,barkskin_3

# Pintark, Orc Guard, F13 (14207, 132)
312
20,CGI,13,3,1  100,COINS BY VALUE,1,13000,0
100,longspear +2
100,full plate +2
100,RING OF PROTECTION 2
100,cloak of resistance +2 unseen
100,STRENGTH 1
100,generic helm
100,black leather boots
100,shield spell
100,barkskin_3

# Wicked, R9/F4/B1 (14205)
313
20,CGI,13,3,1  100,COINS BY VALUE,1,13000,0
100,dagger of frost +2
100,studded leather +3
100,RING OF PROTECTION 2
100,DEXTERITY 3
100,(generic helm,black leather boots)
100,shield spell
100,barkskin_4

# Hill Giant, Temple Exterior, F4 (14219)
314
20,CGI,11,3,1  100,COINS BY VALUE,3,11000,0
100,WEAPON HILL GIANT 2
100,chain shirt

# Goblin Brawler, F4 (14187)
315
100,CGI,3,3,1
100,WEAPON MARTIAL LIGHT 0
100,ARMOR LIGHT
50,small wooden shield  100,large wooden shield_4

# Gnoll Warrior, B1/F4 (14079)
316
20,CGI,6,1,1  100,COINS BY VALUE,1,6000,0
25,FALCHION 0  50,GLAIVE 0  75,GREATAXE 0  100,masterwork greatclub
50,breastplate  100,chainmail
100,(generic helm,black leather boots)

# Orc Slave, male, F1 (14160)
316
10,CGI,1,3,1
25,(club,dagger,handaxe)
50,FARMER GARB  75,patchwork armor  100,LEATHER ARMOR

# Orc Slave, male, F1, Pyrohydra Arena (14160)
3161
10,CGI,1,3,1
100,longspear
100,(white monk robes,patchwork armor,buckler)
100,(HELM,black leather boots)

# Orc Slave, female (14161)
317
10,CGI,1,3,1
25,(club,dagger,masterwork great cleaver)
50,FARMER GARB  75,VILLAGER GARB

# Grank's Bandit, F1/R1 (14357)
357
100,CGI,2,3,1
40,WEAPON MARTIAL TWO-HANDED  100,WEAPON MARTIAL TWO-HANDED 0
50,CHAIN SHIRT  100,ARMOR LIGHT
100,(chainmail gloves,chainmail boots,HELM,black cloak)

# Hill Giantess, B1 (14162)
402
100,COINS BY VALUE,1,80000,0
100,GEMS OR JEWELRY,1,3,1
100,WEAPON HILL GIANT
100,studded leather armor

# Hill Giant Chief, B1/F7 (14163)
403
100,CGI,13,1,1
100,WEAPON HILL GIANT 1
100,studded leather armor
100,cloak of resistance +2 fur

# Frost Giantess, B1 (14164)
404
100,COINS BY VALUE,3,10000,0
100,GEMS OR JEWELRY,1,4,1
100,WEAPON TWO-HANDED
100,(frost giant chainmail,large wooden shield_2,chainmail gloves, chainmail boots)

# Frost Giant Chief, B1/F7 (14165)
405
100,CGI,14,1,1
100,frost brand	
100,(frost giant chainmail,chainmail gloves, chainmail boots)
100,steel shield +2
100,cloak of resistance +2 white
100,barbarian helm

# Fire Giantess, B1 (14166)
406
100,COINS BY VALUE,1,11000,0
100,GEMS OR JEWELRY,1,6,1
100,WEAPON TWO-HANDED
100,fire giantess armor
100,(black leather boots,red padded gloves)

# Fire Giant Chief, B1/F7 (14167)
407
100,CGI,15,1,1
100,fire giant greatsword +1
100,fire giant half-plate
100,cloak of resistance +2 red
100,red long coat
100,(plate boots,red padded gloves)

# Gnome Druid, D13 (14168)
408
100,COINS BY VALUE,3,60,0
100,scimitar +3
100,studded leather armor
100,cloak of resistance +3 green
100,(green robes,black leather boots)
100,shield of faith_3
100,barkskin_4
100,resist energy fire
100,resist energy cold

# Drow Noble Cleric, C15 (14178)
409
100,COINS BY VALUE,1,16000,0
100,LIGHT MACE 2
100,mithral full plate +4
100,mithral large shield +3
100,cloak of resistance +3 orange
100,ioun stone pink
100,ioun stone pale blue
100,(drow mask,black leather boots,black leather gloves)
100,shield of faith_4
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Drow Noble Wizard, W13 (14179)
410
100,COINS BY VALUE,1,16000,0
100,masterwork quarterstaff
100,cloak of resistance +3 red
100,ioun stone pink
100,ioun stone scarlet and blue
100,(drow mask,black robes,black leather boots,black leather gloves)
100,mage armor
100,shield spell
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Drow Noble leader, B1/R3/F16 (14180)
411
100,COINS BY VALUE,1,19000,0
100,unholy light mace +4
100,drow elven chain +3
100,mithral buckler +3
100,cloak of resistance +4 white
100,ioun stone pink
100,ioun stone deep red
100,(drow mask,black leather boots,black leather gloves)
100,shield of faith_4

#------------------------------------------------------------------------------
# Inventory numbers 451 - 999 are reserved for new creatures protos added
# by marc. They will have proto numbers starting from 14451.
#
# The aim here is to match-up the inventory source number with the last three
# digits of the proto number for that creature. So Gnoll Shaman (proto #14712)
# will have an inventory source number #712, etc.
#------------------------------------------------------------------------------

# Southland Nomad, F9/R3 (14492)
490
100,COINS BY VALUE,1,12000,0
100,SCIMITAR 2
100,chain shirt
100,BUCKLER 2
50,DEXTERITY 2
100,(white robes,BOOTS)

# Southland Nomad, F9/R3 (14492)
4901
100,COINS BY VALUE,1,12000,0
100,(composite shortbow str 14 +2,masterwork dagger)
100,quiver of arrows,0,0,3
100,chain shirt
100,BUCKLER 2
50,DEXTERITY 2
100,(brown robes,BOOTS)

# Cavalier, F9/R3 (14491)
491
100,COINS BY VALUE,1,12000,0
100,LONGSWORD 2
100,(FULL PLATE,plate boots,plate gloves)
50,SHIELD 1  100,BUCKLER 1
50,STRENGTH 2

# Cavalier, F9/R3 (14491)
4911
100,COINS BY VALUE,1,12000,0
100,(composite longbow str 18 +2,masterwork shortsword)
100,quiver of arrows,0,0,3
100,ARMOR MEDIUM 2
100,BUCKLER 2
50,DEXTERITY 2
100,(GLOVES,BOOTS)

# Sea Dog, F9/R3 (14492)
492
100,COINS BY VALUE,1,12000,0
100,SHORTSWORD 2
100,LEATHER ARMOR
100,BUCKLER 2
50,DEXTERITY 2
100,(BANDANA,brown leather boots)

# Sea Dog, F9/R3 (14492)
4921
100,COINS BY VALUE,1,12000,0
100,(composite shortbow str 14 +2,masterwork dagger)
100,quiver of arrows,0,0,3
100,LEATHER ARMOR
100,BUCKLER 2
50,DEXTERITY 2
100,(BANDANA,BOOTS)

# Talsar Knight, F9/R3 (14493)
493
100,COINS BY VALUE,1,12000,0
100,LONGSWORD 2
100,(FULL PLATE,plate boots,plate gloves)
50,SHIELD 1  100,BUCKLER 1
50,STRENGTH 2

# Talsar Knight, F9/R3 (14493)
4931
100,COINS BY VALUE,1,12000,0
100,(composite longbow str 18 +2,masterwork shortsword)
100,quiver of arrows,0,0,3
100,ARMOR MEDIUM 2
100,BUCKLER 2
50,DEXTERITY 2
100,(GLOVES,BOOTS)

# Ancient Golem (14495)
495
100,(full plate,great helm,plate boots,plate gloves)

# Drow Guard, B1/R3/F11 (14498)
498
100,COINS BY VALUE,1,15000,0
100,SHORTSWORD 2
50,drow elven chain +2  100,mithral full plate +2  
100,mithral buckler +2
100,(drow mask,black leather boots,black leather gloves)
100,shield of faith_3

# Drow Guard, Archer, B1/R3/F11 (14498)
4981
100,COINS BY VALUE,1,15000,0
100,COMPOSITE LONGBOW STR16 1
100,drow elven chain +1
100,mithral buckler +1
100,ioun stone deep red
100,(drow mask,black leather boots,black leather gloves)
100,shield of faith_3

# Drow Witch, W11 (14499)
499
100,COINS BY VALUE,1,11000,0
100,masterwork dagger
100,RING OF PROTECTION 2
100,cloak of resistance +2 unseen
100,ioun stone pink
100,ioun stone scarlet and blue
100,(drow mask,black wizard robes,black leather boots,black leather gloves)
100,mage armor
100,shield spell
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Drow Guard Leader, B1/R3/F15 (14500)
500
100,COINS BY VALUE,1,19000,0
100,unholy shortsword +4
100,drow elven chain +3
100,mithral buckler +3
100,(drow mask,black leather boots,black leather gloves)
100,shield of faith_4
100,barkskin_3

# MERCHANTS

# Shop Merchants
# 501  Vivian Tosh, gems
# 502  Landy
# 503  Brollo, magic
# 504  Quimby, metal
# 505  Omar, wood
# 506  Corva, leather
# 507  Faye, cloth
# 508  Leopold, metal
# 509  Bollion, mw metal armor (outside)
# 510  Kharduli, mw metal weapons (outside)
# 511  Hild, mw wood bows (inside, then outside)
# 512  Groulmorlig, mw wood non-bows
# 513  Roscoe, mw flesh armor

# Marketplace Merchants
# 514  Bone merchant, mw flesh weapons
# 515  Arcane Wonders, magic
# 516   [open]
# 517  Produce
# 518  Grains
# 519  Beekeeper
# 520  Carpets
# 521   [open]
# 522  Money changer, gems
# 523  Gypsy spa treatments
# 524  Gypsy hair stylist
# 525   [animal dealer]
# 526  Fabrics, cloth
# 527   [fabric reverse]
# 528   [cart near fabric]
# 529  Occidental exotics, masterwork
# 530  Occidental exotics, normal
# 531  Hermonger
# 532   [carter]
# 533  Beast tender
# 534   [open]
# 535   [open]

# Shop Merchants
# 536  Church, magic
# 537  Wizard Guild, assistant guildmaster
# 538  Wizard Guild, potion dealer
# 539  Elvish thief (outside)
# 540  Elvish wizard, magic
# 541  Elvish smith, metal
# 542  Nautical Shop (outside)
# 543  Grocer
# 544  Potter
# 545  Clothing Chest at Tosh Trading Co., cloth

# Inns and Taverns
# 546  Inn, Mellok
# 547  Inn, Roxie
# 548  Inn, Moxie
# 549  Tavern, Uriah
# 550  Flute, Lipton
# 551  Book dealer
# 552   [open]
# 553   [open]

# Outside Merchants, not in Marketplace
# 554  Halfling Tinker
# 555  Druid, Vernox
# 556  Rahlen
# 557  Zahara
# 558   [open]
# 559   [open]

# Crossroads Merchants
# 560  Antique dealer, Corona
# 561  Provisioner, fence
# 562  Gem merchant, Kelvin
# 563   [open]
# 564   [open]
# 565   [open]
# 566   [open]
# 567   [open]
# 568   [open]
# 569   [open]

# Other Merchants, not in Paladin's Cove
# 570  Sylvvi, at Temm's
# 571   [open]
# 572   [open]
# 573   [open]
# 574   [open]
# 575   [open]
# 576   [open]
# 577   [open]
# 578   [open]
# 579   [open]
# 580  Dwarf Innkeeper, Rivermist Inn
# 581  Dwarf Innkeeper, Rivermist Inn
# 582  Dwarf Innkeeper, Rivermist Inn
# 583  Dwarf Innkeeper, Rivermist Inn
# 584  Dwarf Innkeeper, Rivermist Inn
# 585  Dwarf Innkeeper, Rivermist Inn
# 586  Dwarf Innkeeper, Rivermist Inn
# 587   [open]
# 588   [open]
# 589   [open]

# Temple Merchants
# 590  Kelrom, magic
# 591  Eyvva, gems
# 592  Thondorli, metal
# 593   [open]
# 594   [open]
# 595   [open]
# 596   [open]
# 597   [open]
# 598   [open]
# 599   Drow Goddess

# Vivian Tosh, jewel merchant, F5 (14501)
501
100,COINS BY VALUE,0,0,25000
100,GEMS BY VALUE,0,0,500
100,(CHAIN SHIRT 1,black leather boots)
100,(eyeglasses,holy water)

# Vivian inventory
1501
100,thieves tools,0,0,2
100,(healers kit,scholars kit,eyeglasses,spyglass)
100,(drum,flute,horn,mandolin)
100,holy water,0,0,2
100,flask of oil,0,0,2
100,antitoxin,0,0,2
100,lockslip grease,0,0,2
100,armor oil,0,0,2
100,smokestick,0,0,2
100,(rope 50 feet,chain 10 feet,iron spike,ten-foot pole)
100,quiver of arrows,0,0,10
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20
100,pouch of bullets,0,0,20
100,HELM,1,4,1
100,BOOTS,1,4,1
100,ARMORED BOOTS,1,2,0
100,GLOVES,1,4,1
100,ARMORED GLOVES,1,2,0
100,FARMER GARB,1,4,1
100,VILLAGER GARB,1,4,1
100,CROWN,1,2,0

# Landy, masterwork merchant, buys nothing, F3 (14502)
502
100,COINS BY VALUE,3,600,0
100,(black leather armor,black leather boots,white monk robes)
100,(masterwork thieves tools,potion of cure moderate wounds)

# Landy inventory
1502
100,WEAPON 0,2,4,0
50,ARMOR 0,1,2,0
50,SHIELD 0,1,2,0

# Brollo Woodwink, magic merchant, W5 (14503)
503
100,COINS BY VALUE,3,600,0
100,(ALL ROBES,black leather boots)   
25,HAT  50,WIZARD HAT
100,(scholars kit,potion of cure serious wounds)
100,scroll of lightning bolt
100,mage armor

# Brollo inventory, chapter one
1503
100,potion of shield
100,potion of mirror image
100,potion of cure critical wounds
100,potion of death ward
100,potion of dimension door
100,potion of freedom of movement
100,potion of greater invisibility
100,potion of lesser globe of invulnerability
100,potion of restoration
100,potion of stoneskin
100,potion of greater heroism
100,potion of heal
100,potion of protection from undead
100,potion of protection from outsiders
100,potion of protection from elementals
100,(halfling kama,halfling siangham,halfling quarterstaff)
100,(halfling rapier,halfling shortsword,halfling spiked chain)

# Brollo inventory, chapter two
2503
100,potion of mass bulls strength,0,0,2
100,potion of mass cats grace,0,0,2
100,potion of mass bears endurance,0,0,2
100,potion of mass foxs cunning,0,0,2
100,potion of mass owls wisdom,0,0,2
100,potion of mass eagles splendor,0,0,2
100,potion of mass haste,0,0,2
100,potion of mass cure light wounds,0,0,2
100,potion of mass cure moderate wounds,0,0,2
100,potion of shield
100,potion of mirror image
100,potion of cure critical wounds
100,potion of death ward
100,potion of dimension door
100,potion of freedom of movement
100,potion of greater invisibility
100,potion of lesser globe of invulnerability
100,potion of restoration
100,potion of stoneskin
100,potion of greater heroism
100,potion of heal
100,potion of protection from undead
100,potion of protection from outsiders
100,potion of protection from elementals
100,(halfling kama,halfling siangham,halfling quarterstaff)
100,(halfling rapier,halfling shortsword,halfling spiked chain)

# Poor Box (1053, 1054, 1055)
1053
90,ragged clothes,1,4,0
50,FARMER GARB,1,3,0
25,VILLAGER GARB,1,2,0
10,FANCY GARB
75,BOOTS,1,2,0
50,GLOVES
10,WIZARD HAT  50,HAT
25,ALL ROBES
25,CLOAK
5,fancy ivory robes  10,fancy brown robes  15,fancy crimson robes  20,fancy moss-green robes  25,fancy black robes
50,WEAPON RUSTY,1,2,0
10,WEAPON
1,WEAPON 0
10,SHIELD
1,SHIELD 0
5,ARMOR LIGHT  10,patchwork armor
1,ARMOR 0
5,broom  10,rake  15,shovel  20,rolling pin
10,rope 50 feet  20,chain 10 feet  30,iron spike  40,ten-foot pole
10,rope 50 feet  20,chain 10 feet  30,iron spike  40,ten-foot pole
10,rope 50 feet  20,chain 10 feet  30,iron spike  40,ten-foot pole
10,rope 50 feet  20,chain 10 feet  30,iron spike  40,ten-foot pole
1,old sealed letter  2,silver star badge  3,gold crown badge  4,eye of flame cloak  5,ladys ring inscribed from jay
1,holy water  2,jaers spheres of fire  3,eyeglasses  4,masterwork thieves tools  6,MW MUSICAL INSTRUMENT  7,smokestick  8,healers kit  9,scholars kit
1,helm of read magic  2,cloak of elvenkind  3,boots of elvenkind  4,extraplanar chest miniature
 
# Quimby, metal weapons merchant, F4 (14504)
504
100,COINS BY VALUE,3,600,0
100,(half-plate,masterwork falchion,black robes,combat boots)
100,(armor oil,armor oil,potion of cure serious wounds)

# Quimby inventory 
1504
100,(handaxe,throwing axe,bastard sword,bastard sword,battleaxe,battleaxe,butterfly sword,dwarven war axe)
100,(dagger,dagger,throwing dagger,throwing dagger,dart,dart)
100,(falchion,falchion,heavy flail,flail,glaive,gnome hooked hammer)
100,(greataxe,greataxe,greathammer,greathammer,greatsword,greatsword,guisarme,halberd,light hammer,kama,kama)
100,(longsword,longsword,heavy mace,heavy mace,light mace,light mace,morningstar,morningstar)
100,(orc double axe,heavy pick,rapier,rapier,ranseur,scythe,scimitar,scimitar,shortsword,shortsword)
100,(siangham,siangham,sickle,sickle,spiked chain,trident,warhammer,warhammer)

# Omar, wood merchant, F3 (14505)
505
100,COINS BY VALUE,3,600,0
100,(chain shirt,masterwork composite longbow str 16,longsword)
100,quiver of arrows,0,0,2
100,(white wizard robes,red bandana,brown leather boots)
100,(antitoxin,holy water,potion of cure moderate wounds)

# Omar inventory
1505
100,longbow,0,0,2
100,(composite longbow str 18,composite longbow str 16,composite longbow str 14,composite longbow str 12)
100,shortbow,0,0,2
100,(composite shortbow str 14,composite shortbow str 12)
100,(heavy crossbow,heavy crossbow,light crossbow,light crossbow)
100,(repeating light crossbow,repeating light crossbow)
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20
100,javelin,0,0,4
100,sling,0,0,3
100,pouch of bullets,0,0,20
100,(club,greatclub,maul,maul,quarterstaff,quarterstaff_2)
100,(spear,spear,longspear,longspear,shortspear,shortspear,tonfa,tonfa)
100,wooden tower shield,0,0,2
100,(large wooden shield,large wooden shield_2,large wooden shield_3,large wooden shield_4)
100,small wooden shield,0,0,2

# Corva, leather merchant, R3 (14506)
506
100,COINS BY VALUE,3,600,0
100,(little black dress,black leather gloves,black leather boots)
100,(MUSICAL INSTRUMENT,thieves tools)

# Corva inventory
1506
100,brown leather armor,0,0,2
100,black leather armor,0,0,2
100,grey leather armor,0,0,2
100,tan leather armor,0,0,2
100,studded leather armor,0,0,2
100,leather scale armor,0,0,2
100,hide armor,0,0,2
100,(red padded armor,tan padded armor)
100,(leather cap,leather helm)
100,druidic helm,0,0,2
100,black leather gloves,0,0,2
100,brown leather gloves,0,0,2
100,black leather boots,0,0,2
100,brown leather boots,0,0,2
100,fine leather boots,0,0,2
100,green leather boots,0,0,2
100,white leather boots,0,0,2
100,buccaneer boots,0,0,2
100,combat boots,0,0,2

# Faye Tali, cloth merchant (14507)
507
100,COINS BY VALUE,3,600,0
50,VILLAGER GARB  100,FANCY GARB   
100,BOOTS
25,HAT
25,CLOAK

# Faye inventory
1507
100,(masterwork padded armor,masterwork sling)

# Leopold, metal armor merchant, F4 (14508)
508
100,COINS BY VALUE,3,600,0
100,(full plate,gilded plate boots)
100,(armor oil,armor oil,potion of cure serious wounds)

# Leopold inventory
1508
100,full plate
100,half-plate
100,banded mail
100,splint mail
100,breastplate,0,0,2
100,chainmail
100,fine chainmail
100,lamellar armor,0,0,2
100,scale mail,0,0,2
100,chain shirt,0,0,2
100,metal tower shield,0,0,2
100,(large steel shield,large steel shield_2,large steel shield_3,large steel shield_4)
100,small steel shield,0,0,2
100,buckler,0,0,2
100,generic helm,0,0,2
100,chain helmet,0,0,2
100,great helm,0,0,2
100,(plumed helm,gold plumed helm)
100,barbarian helm,0,0,2
100,simple circlet,0,0,2
100,hoodless circlet,0,0,2
100,chainmail gloves,0,0,2
100,chainmail boots,0,0,2
100,plate gloves,0,0,2
100,plate boots,0,0,2

# Bollion K'Zob, masterwork metal armor merchant, F4 (14509)
509
100,COINS BY VALUE,3,600,0
100,(light hammer,red villager garb,black leather gloves,black leather boots)
100,(armor oil,armor oil,potion of cure serious wounds)

# Bollion inventory, mw 10%
1509
1,masterwork dwarven stone armor
1,dwarven plate
1,adamantine breastplate
1,adamantine chain shirt
10,masterwork full plate
10,masterwork half-plate
10,masterwork banded mail
10,masterwork splint mail
10,masterwork breastplate
10,masterwork lamellar armor
10,masterwork chainmail
10,masterwork fine chainmail
10,masterwork scale mail
10,masterwork chain shirt
10,masterwork metal tower shield
10,masterwork large steel shield
10,masterwork small steel shield
10,masterwork buckler

# Bollion inventory, mw 100%, after quest #24 Adamantine Mine
2509
100,adamantine breastplate
100,adamantine chain shirt
100,dwarven plate
100,masterwork dwarven stone armor
100,masterwork full plate
100,masterwork half-plate
100,masterwork banded mail
100,masterwork splint mail
100,masterwork breastplate
100,masterwork lamellar armor
100,masterwork chainmail
100,masterwork fine chainmail
100,masterwork scale mail
100,masterwork chain shirt
100,masterwork metal tower shield
100,masterwork large steel shield
100,masterwork small steel shield
100,masterwork buckler

# Karduli K'Zob, masterwork metal weapons merchant, F4 (14510)
510
100,COINS BY VALUE,3,600,0
100,(masterwork glaive,chain shirt,black leather boots)
100,POTION MELEE

# Karduli inventory, mw 10%
1510
10,masterwork bastard sword
10,masterwork battleaxe
10,masterwork butterfly sword
10,masterwork dagger
10,masterwork throwing dagger
10,masterwork dart
10,masterwork dwarven war axe
10,masterwork falchion
10,masterwork heavy flail
10,masterwork flail
10,masterwork glaive
10,masterwork gnome hooked hammer
10,masterwork greataxe
10,masterwork greathammer
10,masterwork greatsword
10,masterwork guisarme
10,masterwork handaxe
10,masterwork throwing axe
10,masterwork halberd
10,masterwork light hammer
10,masterwork kama
10,masterwork longsword
10,masterwork heavy mace
10,masterwork light mace
10,masterwork morningstar
10,masterwork orc double axe
10,masterwork heavy pick
10,masterwork ranseur
10,masterwork rapier
10,masterwork scimitar
10,masterwork scythe
10,masterwork shortsword
10,masterwork siangham
10,masterwork sickle
10,masterwork spiked chain
10,masterwork trident
10,masterwork warhammer

# Karduli inventory, mw 100%, after quest #5 Werehouse Robberies
2510
100,masterwork bastard sword
100,masterwork battleaxe
100,masterwork butterfly sword
100,masterwork dagger
100,masterwork throwing dagger
100,masterwork dart
100,masterwork dwarven war axe
100,masterwork falchion
100,masterwork heavy flail
100,masterwork flail
100,masterwork glaive
100,masterwork gnome hooked hammer
100,masterwork greataxe
100,masterwork greathammer
100,masterwork greatsword
100,masterwork guisarme
100,masterwork handaxe
100,masterwork throwing axe
100,masterwork halberd
100,masterwork light hammer
100,masterwork kama
100,masterwork longsword
100,masterwork heavy mace
100,masterwork light mace
100,masterwork morningstar
100,masterwork orc double axe
100,masterwork heavy pick
100,masterwork ranseur
100,masterwork rapier
100,masterwork scimitar
100,masterwork scythe
100,masterwork shortsword
100,masterwork siangham
100,masterwork sickle
100,masterwork spiked chain
100,masterwork trident
100,masterwork warhammer

# Hild, masterwork wood bow merchant, in house, F2 (14511)
511
100,COINS BY VALUE,3,600,0
100,masterwork longbow
100,quiver of arrows,0,0,3
100,(green skirtless garb,orange cloak,brown leather boots)

# Hild, masterwork wood bow merchant, in marketplace, F2 (14511)
5111
100,COINS BY VALUE,3,600,0
100,masterwork longbow
100,quiver of arrows,0,0,3
100,(green skirtless garb,orange cloak,brown leather boots)

# Hild inventory, mw 10%
1511
10,masterwork longbow
100,masterwork shortbow
10,masterwork composite longbow str 12
10,masterwork composite longbow str 14
10,masterwork composite longbow str 16
10,masterwork composite longbow str 18
10,masterwork composite shortbow str 12
10,masterwork composite shortbow str 14
10,masterwork heavy crossbow
10,masterwork light crossbow
10,masterwork repeating light crossbow
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20

# Hild inventory, mw 100%, after quest #6 She's Good with Wood
2511
100,masterwork longbow
100,masterwork shortbow
100,masterwork composite longbow str 12
100,masterwork composite longbow str 14
100,masterwork composite longbow str 16
100,masterwork composite longbow str 18
100,masterwork composite shortbow str 12
100,masterwork composite shortbow str 14
100,masterwork heavy crossbow
100,masterwork light crossbow
100,masterwork repeating light crossbow
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20

# Groulmorlig Honorhelm, masterwork wood merchant, R3/W3 (14512)
512
100,COINS BY VALUE,3,600,0
75,RING OF PROTECTION 1
75,AMULET 1
50,CLOAK OF RESISTANCE 1
5,INTELLIGENCE 1
100,(red villager garb,black leather boots)
100,mage armor

# Groulmorlig inventory, mw 10%
1512
100,masterwork club
100,masterwork greatclub
100,masterwork javelin
100,masterwork maul
100,masterwork quarterstaff
100,masterwork spear
100,masterwork longspear
100,masterwork shortspear
100,masterwork tonfa
100,darkwood tower shield
100,darkwood large shield
100,darkwood buckler
100,wooden elvish shield
100,masterwork bark armor
100,masterwork wooden tower shield
100,masterwork large wooden shield
100,masterwork small wooden shield

# Roscoe Bloodfoot, masterwork leather merchant, tanner, F2 (14513)
513
100,COINS BY VALUE,3,600,0
100,(LEATHER ARMOR,brown leather boots)

# Roscoe inventory, mw 100%
1513
100,masterwork leather armor,0,0,2
100,masterwork studded leather,0,0,2
100,masterwork leather scale armor,0,0,2
100,masterwork hide armor,0,0,2
100,barbarian armor,0,0,2
100,masterwork sling,0,0,2

# Barnibus Nox, masterwork flesh merchant, C3 (14514)
514
100,COINS BY VALUE,3,600,0
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,troll bone armor
60,LARGE SHIELD 0  100,large wooden shield_2
100,(violet robes,hoodless circlet,black leather boots)

# Barnibus inventory, 10% mw
1514
10,masterwork bone armor
10,masterwork shell armor
100,shell armor,0,0,2
100,bone armor,0,0,2
100,bone helm,0,0,2
100,goblin leg bone,0,0,2
100,hide armor,0,0,2
100,leather scale armor,0,0,2
100,studded leather armor,0,0,2
100,brown leather armor
100,black leather armor
100,grey leather armor
100,tan leather armor
100,(leather cap,leather helm)
100,druidic helm,0,0,2
100,black leather gloves,0,0,2
100,brown leather gloves,0,0,2
100,black leather boots,0,0,2
100,brown leather boots,0,0,2
100,fine leather boots,0,0,2
100,green leather boots,0,0,2
100,white leather boots,0,0,2

# Barnibus inventory, 100% mw, chapter 2, after PC asks about it
2514
100,dragonhide plate
100,masterwork shell armor
100,masterwork bone armor,0,0,2
100,shell armor,0,0,2
100,bone armor,0,0,2
100,bone helm,0,0,2
100,goblin leg bone,0,0,2
100,hide armor,0,0,2
100,leather scale armor,0,0,2
100,studded leather armor,0,0,2
100,brown leather armor
100,black leather armor
100,grey leather armor
100,tan leather armor
100,(leather cap,leather helm)
100,druidic helm,0,0,2
100,black leather gloves,0,0,2
100,brown leather gloves,0,0,2
100,black leather boots,0,0,2
100,brown leather boots,0,0,2
100,fine leather boots,0,0,2
100,green leather boots,0,0,2
100,white leather boots,0,0,2

# Hannah, magic item merchant, W3 (14515)
515
100,COINS BY VALUE,3,600,0
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,(VILLAGER GARB,black leather boots)
100,mage armor

# Hannah inventory, chapter 1
1515
100,WONDROUS ITEM MINOR,1,4,1
50,WONDROUS ITEM MEDIUM,0,0,1
100,RING MINOR,1,2,0
50,RING MEDIUM,0,0,1
100,WEAPON 1,1,2,0
50,WEAPON 2,0,0,1
100,ARMOR 1,1,2,0
50,ARMOR 2,0,0,1

# Hannah inventory, chapter 2
2515
100,WONDROUS ITEM MINOR,1,4,1
100,WONDROUS ITEM MEDIUM,1,2,0
100,RING MINOR,1,2,0
100,RING MEDIUM,1,2,0
100,WEAPON 1,1,2,0
100,WEAPON 2,1,2,0
100,ARMOR 1,1,2,0
100,ARMOR 2,1,2,0

# Hannah inventory, chapter 2 bonus inventory
7515
100,WONDROUS ITEM MEDIUM,0,0,1
50,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
50,RING MAJOR,0,0,1
100,WEAPON 2,0,0,1
50,WEAPON 3,0,0,1
100,ARMOR 2,0,0,1
50,ARMOR 3,0,0,1

# Hannah inventory, chapter 3
3515
100,WONDROUS ITEM MEDIUM,1,4,1
100,WONDROUS ITEM MAJOR,1,2,0
100,RING MEDIUM,1,2,0
100,RING MAJOR,1,2,0
100,WEAPON 3,1,2,0
100,WEAPON 4,0,0,1
100,ARMOR 3,1,2,0
100,SHIELD 4,0,0,1

# Hannah inventory, chapter 4
4515
100,WONDROUS ITEM MEDIUM,1,4,1
100,WONDROUS ITEM MAJOR,1,2,0
100,RING MEDIUM,1,2,0
100,RING MAJOR,1,2,0
100,WEAPON 3,1,2,0
100,WEAPON 4,0,0,1
100,ARMOR 3,1,2,0
100,SHIELD 4,0,0,1

# Produce vendor, 0-level (14517) 
517
100,COINS BY VALUE,3,600,0
100,(green robes,SIMPLE BOOTS)

# Produce Vendor inventory
1517
100,(apple,bananas,cherries,coconut,dates,dried figs,grapes,pear,pineapple,raisins)
100,(beets,cabbage,carrots,chili pepers,garlic,green beans,green peas,mushrooms,olives,onion)

# Grain Vendor, 0-level (14518) 
518
100,COINS BY VALUE,3,600,0
100,(green robes,hoodless circlet,SIMPLE BOOTS)

# Grain Vendor inventory
1518
100,(barley flour 10 lbs.,buckwheat flour 10 lbs.,rye flour 10 lbs.,wheat flour 10 lbs.)
100,(barley 10 lbs.,buckwheat 10 lbs.,chick peas 10 lbs.,lentils 10 lbs.)
100,(oats 10 lbs.,rice 10 lbs.,rye 10 lbs.,wheat 10 lbs.)
100,(almonds,cashews,hazelnuts,walnuts)

# Arric, beekeeper, 0-level (14519) 
519
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,straw cap,SIMPLE BOOTS)

# Arric inventory
1519
100,honey,0,0,4
100,honeycomb,0,0,2
100,bee propolis,0,0,2
100,royal jelly,0,0,2
100,jug of mead,0,0,2

# Aru Savadi, carpet merchant, F3 (14520)
520
100,COINS BY VALUE,3,600,0
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,(fancy ivory robes,chain shirt,red bandana,black leather boots)

# Aru Savadi inventory
1520
100,falchion,0,0,2
100,masterwork falchion,0,0,1
100,falchion +1,0,0,1
100,WEAPON 0,0,0,1
100,WEAPON 0,0,0,1
100,WEAPON 1,0,0,1
100,SHIELD 1,0,0,1

# Florin Pascal, money changer/gems/jewels, R5/B13 (14519) 
522
100,COINS BY VALUE,3,6000,0
100,BRACERS 3
100,RING OF PROTECTION 3
100,AMULET 3
100,cloak of resistance +3 unseen
100,DEXTERITY 3
100,(eyeglasses,masterwork flute,masterwork thieves tools)
100,(blue noble garb,black leather boots)

# Florin inventory (also Gypsy Leader inventory)
1522
100,(diamond,jacinth)
100,(emerald,sapphire)
100,(black pearl,aquamarine)
100,(white pearl,amber)
100,(rhodochrosite,malachite)
100,(carnelian,blue jasper)
100,potion of restoration,0,0,4
100,potion of neutralize poison,0,0,4
100,potion of protection from evil,0,0,4
100,potion of resist acid 30,0,0,6
100,potion of resist cold 30,0,0,6
100,potion of resist electricity 30,0,0,6
100,potion of resist fire 30,0,0,6

# Gisella, spa treatments, R3 (14523)
523
100,COINS BY VALUE,3,600,0
100,(blue gypsy dress,black leather boots)

# De Leon, hairdresser, 0-level (14524)
524
100,COINS BY VALUE,3,600,0
100,(red villager garb,brown leather boots)

# Freyja, fabric merchant, 0-level (14526)
526
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,kilt,black leather boots)

# Freyja inventory
1526
100,bolt of sackcloth,0,0,2
100,bolt of linen,0,0,2
100,bolt of wool,0,0,2
100,bolt of red velvet
100,bolt of blue velvet
100,bolt of green velvet
100,bolt of black velvet
100,bolt of lace,0,0,1
100,bolt of silk,0,0,1
100,vial of red dye
100,vial of green dye
100,vial of indigo dye
100,vial of purple dye

# Kamshika, occidental merchant, F6 (14529)
529
100,COINS BY VALUE,3,600,0
100,CHAIN SHIRT 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 unseen
5,DEXTERITY 1
100,(black monk robes,black leather boots)

# Kamshika inventory
1529
100,masterwork butterfly sword,0,0,2
100,masterwork kama,0,0,2
100,masterwork quarterstaff,0,0,2
100,masterwork siangham,0,0,2
100,masterwork tonfa,0,0,2
100,masterwork war fan,0,0,2
10,masterwork falchion
10,masterwork jian
10,masterwork katana
10,masterwork katar
10,masterwork kukri
10,masterwork masakari
10,masterwork naginata
10,masterwork nodachi
10,masterwork scimitar
10,masterwork shuriken
10,masterwork tetsubo
10,masterwork tanto
10,masterwork wakizashi
10,masterwork samurai armor

# Koyo, occidental merchant, F5 (14530)
530
100,COINS BY VALUE,3,600,0
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 unseen
100,(samurai armor,black long coat,coolie hat,black leather boots)

# Koyo inventory
1530
100,butterfly sword,0,0,2
100,kama,0,0,2
100,quarterstaff,0,0,2
100,siangham,0,0,2
100,tonfa,0,0,2
100,war fan,0,0,2
100,(falchion,gladius,jian,katana,katar,kukri,masakari,naginata,nodachi)
100,(scimitar,tanto,tetsubo,wakizashi)
100,shuriken,0,0,10
100,samurai armor
100,coolie hat

# Juniper, herbmonger, D3 (14531)
531
100,COINS BY VALUE,3,600,0
100,(green villager garb,black leather boots)
100,(scroll of chill metal,scroll of summon natures ally ii)

# Juniper inventory
1531
100,bundles of herb and spice,0,0,2
100,(basil,coriander,juniper,oregano,parsley,rosemary,sage,taragon,thyme)
100,garlic
100,(acorns,acorns)
100,(cinnamon,cloves,cumin,ginger,licorice root,nutmeg,paprika,pepper,saffron,salt,turmeric,vanilla)
100,(fennel seeds,poppy seeds,pumpkin seeds,sesame seeds)
100,(almond oil,olive oil,sesame oil,walnut oil)

# Merdu k'zob, beast tender, 0-level (14533)
533
100,COINS BY VALUE,3,600,0
100,shovel
100,(ochre villager garb,leather helm,brown leather boots)

# Merdu inventory
1533
100,dung,0,0,4
100,beast urine,0,0,4
100,shovel,0,0,1
100,masterwork shovel,0,0,1

# Virgilia, church, C1 (14536)
536
100,COINS BY VALUE,3,600,0
100,(black farmer garb,black cloak,hoodless circlet,black leather boots)

# Virgilia inventory
1536
100,holy water,0,0,2
100,keoghtoms ointment,0,0,2
100,potion of aid,0,0,2
100,potion of bears endurance,0,0,2
100,potion of bulls strength,0,0,2
100,potion of cure light wounds,0,0,4
100,potion of cure moderate wounds,0,0,4
100,potion of cure serious wounds,0,0,2
100,potion of delay poison,0,0,2
100,potion of eagles splendor,0,0,2
100,potion of lesser restoration,0,0,2
100,potion of neutralize poison,0,0,2
100,potion of owls wisdom,0,0,2
100,potion of protection from acid 60
100,potion of protection from acid 108
100,potion of protection from cold 60
100,potion of protection from cold 108
100,potion of protection from electricity 60
100,potion of protection from electricity 108
100,potion of protection from fire 60
100,potion of protection from fire 108
100,potion of protection from sonic 108
100,potion of protection from sonic 60
100,potion of protection from chaos
100,potion of protection from evil
100,potion of protection from earth
100,potion of protection from elementals
100,potion of protection from outsiders
100,potion of protection from undead
100,potion of resist acid 10
100,potion of resist acid 20
100,potion of resist acid 30
100,potion of resist acid 30
100,potion of resist cold 10
100,potion of resist cold 20
100,potion of resist cold 30
100,potion of resist cold 30
100,potion of resist electricity 10
100,potion of resist electricity 20
100,potion of resist electricity 30
100,potion of resist electricity 30
100,potion of resist fire 10
100,potion of resist fire 20
100,potion of resist fire 30
100,potion of resist fire 30
100,potion of resist sonic 10
100,potion of resist sonic 20
100,potion of resist sonic 30
100,potion of resist sonic 30
100,potion of remove blindness/deafness,0,0,2
100,potion of remove curse,0,0,2
100,potion of remove disease,0,0,2
100,potion of sanctuary,0,0,2
100,potion of shield of faith +2,0,0,2
100,potion of shield of faith +3,0,0,2
100,potion of shield of faith +4,0,0,2
100,potion of shield of faith +5,0,0,2
100,scroll of cure light wounds,0,0,2
100,scroll of cure light wounds mass,0,0,2
100,scroll of cure moderate wounds,0,0,2
100,scroll of cure serious wounds,0,0,2
100,scroll of cure critical wounds,0,0,2
100,scroll of death ward,0,0,2
100,scroll of delay poison,0,0,2
100,scroll of dispel magic,0,0,2
100,scroll of neutralize poison,0,0,2
100,scroll of protection from chaos,0,0,2
100,scroll of protection from evil,0,0,2
100,scroll of raise dead,0,0,1
100,scroll of remove curse,0,0,1
100,scroll of remove blindness/deafness,0,0,2
100,scroll of remove disease,0,0,2

# Thormund, wizard's guild deputy guildmaster, W11 (14537) 
537
100,COINS BY VALUE,3,600,0
100,cloak of resistance +1 unseen
100,(eyeglasses,scholars kit)
100,(red robes,black leather boots)
100,scroll of disintegrate
100,mage armor

# Thormund, inventory
5371
100,wizard scrolls level one
100,scroll of burning hands
20,scroll of cause fear
20,scroll of charm person
20,scroll of chill touch
20,scroll of color spray
20,scroll of confusion lesser
20,scroll of cure light wounds
20,scroll of detect secret doors
20,scroll of detect undead
20,scroll of endure elements
20,scroll of enlarge person
20,scroll of expeditious retreat
20,scroll of grease
20,scroll of identify
20,scroll of mage armor
20,scroll of magic missile
20,scroll of magic weapon
20,scroll of obscuring mist
20,scroll of protection from chaos
20,scroll of protection from evil
20,scroll of protection from good
20,scroll of protection from law
20,scroll of ray of enfeeblement
20,scroll of reduce person
20,scroll of shield
20,scroll of shocking grasp
20,scroll of sleep
20,scroll of summon monster i
20,scroll of true strike

5372
100,wizard scrolls level two
100,scroll of bears endurance
20,scroll of blindness/deafness
20,scroll of blur
20,scroll of bulls strength
20,scroll of cats grace
20,scroll of daze monster
20,scroll of eagles splendor
20,scroll of false life
20,scroll of fog cloud
20,scroll of foxs cunning
20,scroll of ghoul touch
20,scroll of glitterdust
20,scroll of gust of wind
20,scroll of invisibility
20,scroll of knock
20,scroll of melfs acid arrow
20,scroll of mirror image
20,scroll of owls wisdom
20,scroll of protection from arrows
20,scroll of resist energy
20,scroll of scare
20,scroll of scorching ray
20,scroll of see invisibility
20,scroll of summon monster ii
20,scroll of tashas hideous laughter
20,scroll of web,0,0,2

5373
100,wizard scrolls level three
100,scroll of blink
20,scroll of clairaudience/clairvoyance
20,scroll of deep slumber
20,scroll of dispel magic
20,scroll of displacement
20,scroll of fireball
20,scroll of gaseous form
20,scroll of gentle repose
20,scroll of halt undead
20,scroll of haste
20,scroll of heroism
20,scroll of hold person
20,scroll of invisibility sphere
20,scroll of keen edge
20,scroll of lightning bolt
20,scroll of magic weapon greater
20,scroll of protection from energy
20,scroll of rage
20,scroll of sleet storm
20,scroll of slow
20,scroll of stinking cloud
20,scroll of summon monster iii
20,scroll of vampiric touch
20,scroll of wind wall,0,0,2

5374
100,wizard scrolls level four
100,scroll of animate dead
20,scroll of bestow curse
20,scroll of charm monster
20,scroll of confusion
20,scroll of contagion
20,scroll of crushing despair
20,scroll of cure critical wounds
20,scroll of dimension door
20,scroll of dimensional anchor
20,scroll of fear
20,scroll of fire shield
20,scroll of freedom of movement
20,scroll of globe of invulnerability lesser
20,scroll of ice storm
20,scroll of invisibility greater
20,scroll of neutralize poison 
20,scroll of otilukes resilient sphere
20,scroll of phantasmal killer
20,scroll of remove curse
20,scroll of repel vermin
20,scroll of shout
20,scroll of solid fog
20,scroll of stoneskin
20,scroll of summon monster iv
20,scroll of summon monster iv

5375
100,wizard scrolls level five
100,scroll of animal growth
20,scroll of blight
20,scroll of break enchantment
20,scroll of cloudkill
20,scroll of cone of cold
20,scroll of dismissal
20,scroll of dominate person
20,scroll of extraplanar chest
20,scroll of feeblemind
20,scroll of hold monster
20,scroll of mind fog
20,scroll of mordenkainens faithful hound
20,scroll of summon monster v
20,scroll of teleport
20,scroll of teleport

5376
100,wizard scrolls level six
100,scroll of analyze dweomer
20,scroll of banishment
20,scroll of bears endurance mass
20,scroll of bulls strength mass
20,scroll of cats grace mass
20,scroll of chain lightning
20,scroll of circle of death
20,scroll of disintegrate
20,scroll of dispel magic greater
20,scroll of eagles splendor mass
20,scroll of eyebite
20,scroll of foxs cunning mass
20,scroll of owls wisdom mass
20,scroll of heroism greater
20,scroll of mislead
20,scroll of summon monster vi
20,scroll of true seeing
20,scroll of undeath to death
20,scroll of undeath to death

5377
100,wizard scrolls level seven
100,scroll of banishment
20,scroll of delayed blast fireball
20,scroll of destruction
20,scroll of finger of death
20,scroll of hold person mass
20,scroll of insanity
20,scroll of power word blind
20,scroll of prismatic spray
20,scroll of summon monster vii

5378
100,wizard scrolls level eight
100,scroll of charm monster mass
20,scroll of horrid wilting
20,scroll of polar ray
20,scroll of power word stun
20,scroll of summon monster viii

5379
100,wizard scrolls level nine
100,scroll of dominate monster
20,scroll of hold monster mass
20,scroll of meteor swarm
20,scroll of power word kill
20,scroll of wail of the banshee
20,scroll of weird
20,scroll of summon monster ix

# Lula, potion vendor, W3 (14538)
538
100,COINS BY VALUE,3,600,0
100,quarterstaff
60,BRACERS 0
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(WIZARD CLOTHING,BOOTS)
100,mage armor

# Lula inventory
1538
100,potion of aid
100,potion of aid
100,potion of bears endurance
100,potion of bears endurance
100,potion of blur
100,potion of blur
100,potion of bulls strength
100,potion of bulls strength
100,potion of cats grace
100,potion of cats grace
100,potion of displacement
100,potion of displacement
100,potion of eagles splendor
100,potion of eagles splendor
100,potion of enlarge person
100,potion of enlarge person
100,potion of foxs cunning
100,potion of foxs cunning
100,potion of gaseous form
100,potion of gaseous form
100,potion of haste
100,potion of haste
100,potion of heroism
100,potion of heroism
100,potion of invisibility
100,potion of invisibility
100,potion of mage armor
100,potion of mage armor
100,potion of owls wisdom
100,potion of owls wisdom
100,potion of protection from acid 60
100,potion of protection from acid 108
100,potion of protection from cold 60
100,potion of protection from cold 108
100,potion of protection from electricity 60
100,potion of protection from electricity 108
100,potion of protection from fire 60
100,potion of protection from fire 108
100,potion of protection from sonic 108
100,potion of protection from sonic 60
100,potion of protection from arrows
100,potion of protection from arrows
100,potion of protection from chaos
100,potion of protection from evil
100,potion of protection from good
100,potion of protection from law
100,potion of protection from earth
100,potion of protection from elementals
100,potion of protection from outsiders
100,potion of protection from undead
100,potion of reduce person
100,potion of reduce person
100,potion of resist acid 10
100,potion of resist acid 20
100,potion of resist acid 30
100,potion of resist acid 30
100,potion of resist cold 10
100,potion of resist cold 20
100,potion of resist cold 30
100,potion of resist cold 30
100,potion of resist electricity 10
100,potion of resist electricity 20
100,potion of resist electricity 30
100,potion of resist electricity 30
100,potion of resist fire 10
100,potion of resist fire 20
100,potion of resist fire 30
100,potion of resist fire 30
100,potion of resist sonic 10
100,potion of resist sonic 20
100,potion of resist sonic 30
100,potion of resist sonic 30

# Roswilda, thief vendor, R4 (14539)
539
100,COINS BY VALUE,3,600,0
25,LEATHER ARMOR 0  100,LEATHER ARMOR 1
10,RING OF PROTECTION 1
10,AMULET 1
10,DEXTERITY 1
10,cloak of resistance +1 unseen
100,BOOTS

# Roswilda inventory
1539
100,(thieves tools,masterwork thieves tools,lockslip grease,lockslip grease,flask of oil,flask of oil,eyeglasses,spyglass)
100,(rope 50 feet,rope 200 feet,iron spike,iron spike,mallet,ten-foot pole)
100,(cloak of elvenkind,boots of elvenkind)
100,(masterwork dagger,masterwork throwing dagger,masterwork shortsword,masterwork buckler)
100,(scroll of dispel magic,scroll of expeditious retreat,scroll of knock,scroll of knock,scroll of identify,scroll of mirror image)
100,(potion of cats grace,potion of eagles splendor,potion of gaseous form,potion of glibness,potion of hiding,potion of haste,potion of invisibility,potion of sneaking)
100,(blue bandana,green bandana,white bandana,red bandana)
100,(gloves of dexterity +2,WONDROUS ITEM MINOR)

# Drasindra, Magic item dealer, W7 (14540)
540
100,COINS BY VALUE,2,6000,0
100,GEMS,2,4,1
75,AMULET 1
75,RING OF PROTECTION 1
75,cloak of resistance +1 unseen
100,green robes
100,(black leather gloves,black leather boots)
100,scroll of mirror image
100,scroll of deep slumber
100,mage armor

# Drasindra inventory, chapter 1
1540
100,SCROLL ARCANE MINOR,2,4,0
100,SCROLL ARCANE MEDIUM,1,4,1
100,WAND ARCANE MINOR,1,2,1
50,WAND ARCANE MEDIUM,0,0,1
100,WONDROUS ITEM MINOR,1,3,1
50,WONDROUS ITEM MEDIUM,0,0,1
100,RING MINOR,0,0,1
50,RING MEDIUM,0,0,1

# Drasindra inventory, chapter 2
2540
100,SCROLL ARCANE MEDIUM,1,6,1
100,WAND ARCANE MAJOR,1,3,1
100,WONDROUS ITEM MINOR,1,3,1
100,WONDROUS ITEM MEDIUM,0,0,1
100,RING MINOR,0,0,1
100,RING MEDIUM,0,0,1

# Drasindra inventory, chapter 2, bonus inventory
7540
100,SCROLL ARCANE MEDIUM,1,6,1
100,SCROLL ARCANE MAJOR,0,0,1
100,WONDROUS ITEM MEDIUM,0,0,1
50,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
50,RING MAJOR,0,0,1

# Drasindra inventory, chapter 3
3540
100,SCROLL ARCANE MEDIUM,1,8,1
100,SCROLL ARCANE MAJOR,1,4,1
100,WAND ARCANE MAJOR,1,4,1
100,WONDROUS ITEM MEDIUM,1,2,1
75,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
75,RING MAJOR,0,0,1

# Drasindra inventory, chapter 4
4560
100,SCROLL ARCANE MEDIUM,1,8,1
100,SCROLL ARCANE MAJOR,1,4,1
100,WAND ARCANE MAJOR,1,4,1
100,WONDROUS ITEM MEDIUM,1,2,1
75,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
75,RING MAJOR,0,0,1

# Daerwyg, elven smith, F4 (14541)
541
100,COINS BY VALUE,3,600,0
75,LONGSWORD 0  100,LONGSWORD 1
75,elven chain  100,elven chain +1
10,AMULET 1
10,RING OF PROTECTION 1
10,cloak of resistance +1 unseen
100,(black leather gloves,black leather boots)
100,(armor oil,armor oil,potion of cure serious wounds)

# elvish smith, 10% mithral
1541
10,mithral full plate
10,mithral breastplate
10,mithral chain shirt
10,elven chain
10,mithral tower shield
10,mithral large shield
100,mithral buckler

# elvish smith, 100% mithral, after quest #82 Catch a Falling Star
2541
100,mithral full plate
100,mithral breastplate
100,elven chain,0,0,2
100,mithral chain shirt,0,0,2
100,mithral tower shield,0,0,2
100,mithral large shield,0,0,2
100,mithral buckler,0,0,2
60,mithral full plate +1  85,mithral full plate +2  100,mithral full plate +3
60,mithral breastplate +1  85,mithral breastplate +2  100,mithral breastplate +3
60,elven chain +1  85,elven chain +2  100,elven chain +3
60,mithral chain shirt +1  85,mithral chain shirt +2  100,mithral chain shirt +3
60,mithral buckler +1  85,mithral buckler +2  100,mithral buckler +3  
60,mithral tower shield +1  85,mithral tower shield +2  100,mithral tower shield +3
100,mithral large shield +1
60,mithral large shield +1  85,mithral large shield +2  100,mithral large shield +3

# elvish smith, 100% mithral, chapter 3
3541
100,mithral full plate
100,mithral breastplate
100,elven chain,0,0,2
100,mithral chain shirt,0,0,2
100,mithral tower shield,0,0,2
100,mithral large shield,0,0,2
100,mithral buckler,0,0,2
50,mithral full plate +1  80,mithral full plate +2  100,mithral full plate +3
50,mithral breastplate +1  80,mithral breastplate +2  100,mithral breastplate +3
50,elven chain +1  80,elven chain +2  100,elven chain +3
50,mithral chain shirt +1  80,mithral chain shirt +2  100,mithral chain shirt +3
50,mithral buckler +1  80,mithral buckler +2  100,mithral buckler +3  
50,mithral tower shield +1  80,mithral tower shield +2  100,mithral tower shield +3
100,mithral large shield +2
50,mithral large shield +1  80,mithral large shield +2  100,mithral large shield +3

# elvish smith, 100% mithral, chapter 4
4541
100,mithral full plate
100,mithral breastplate
100,elven chain,0,0,2
100,mithral chain shirt,0,0,2
100,mithral tower shield,0,0,2
100,mithral large shield,0,0,2
100,mithral buckler,0,0,2
35,mithral full plate +1  70,mithral full plate +2  100,mithral full plate +3
35,mithral breastplate +1  70,mithral breastplate +2  100,mithral breastplate +3
35,elven chain +1  70,elven chain +2  100,elven chain +3
35,mithral chain shirt +1  70,mithral chain shirt +2  100,mithral chain shirt +3
35,mithral buckler +1  70,mithral buckler +2  100,mithral buckler +3  
35,mithral tower shield +1  70,mithral tower shield +2  100,mithral tower shield +3
100,mithral large shield +3
35,mithral large shield +1  70,mithral large shield +2  100,mithral large shield +3

# Nigel Weatherspoon, Nautical Shop Owner, R7 (14542)
542
100,COINS BY VALUE,3,600,0
100,(ochre villager garb,black leather boots)
100,mage armor

# Nautical Shop inventory, buys anything
1542
100,(cutlass,masterwork cutlass)
100,flask of oil,0,0,2
100,rope 50 feet,0,0,2
100,(black long coat,blue long coat,red long coat,white long coat)
100,(bicorne,jaunty hat,blue bandana,red bandana,green bandana,white bandana)

# Talbot, Grocer, 0-level (14543)
543
100,COINS BY VALUE,3,600,0
100,handaxe
100,(red villager garb,black leather boots)

# Talbot inventory
1543
100,bread,0,0,2
100,cheese,0,0,2
100,gnome pie,0,0,2
100,mutton,0,0,2
100,(apple,apple,apple,raisins)
100,(beets,cabbage,carrots,garlic,green beans,green peas,mushrooms,onion)
100,(garlic,salt)
100,honey,0,0,2
100,(buckwheat flour 10 lbs.,wheat flour 10 lbs.)
100,(buckwheat 10 lbs.,wheat 10 lbs.)

# Iris, Potter, 0-level (14544)
544
100,COINS BY VALUE,3,600,0
100,(ochre skirtless garb,black leather boots)

# Iris inventory
1544
100,empty bottle,0,0,4
100,empty jar,0,0,4
100,empty vial,0,0,4
100,eyeglasses
100,spyglass

# Iris misdelivered chest
2544
100,bolt of silk,0,0,8

# Clothing Chest at Tosh
545

# Clothing Chest at Tosh inventory
1545
100,(blue noble garb,red noble garb,purple noble garb,black noble garb,green noble garb,white noble garb)
100,(blue villager garb,green villager garb,ochre villager garb,red villager garb)
100,(blue skirtless garb,green skirtless garb,ochre skirtless garb,red skirtless garb)
100,(black mystic garb,blue mystic garb,green mystic garb,orange mystic garb,pink mystic garb,purple mystic garb,red mystic garb,teal mystic garb,white mystic garb,yellow mystic garb)
100,(blue gypsy dress,green gypsy dress,purple gypsy dress,red gypsy dress)
100,(black evening dress,green evening dress,red evening dress,white evening dress,little black dress,little red dress)
100,(kilt,kilt)
100,(black robes,blue robes,light blue robes,brown robes,green robes,orange robes,red robes,violet robes,white robes,yellow robes)
100,(black monk robes,blue monk robes,brown monk robes,orange monk robes,red monk robes,white monk robes)
100,(black wizard robes,blue wizard robes,red wizard robes,white wizard robes)
100,(black long coat,blue long coat,red long coat,white long coat)
100,(black cloak,blue cloak,light blue cloak,fur cloak,green cloak,orange cloak,red cloak,violet cloak,white cloak,yellow cloak)
100,(banded black hat,black wizard hat,blue wizard hat,brown wizard hat,fire wizard hat,ice wizard hat,white wizard hat)
100,(black fancy hat,blue fancy hat,red fancy hat)
100,(coolie hat,straw cap,bicorne,jaunty hat,blue bandana,red bandana)
100,(black leather boots,brown leather boots,red padded boots,tan padded boots,fine leather boots,green leather boots,white leather boots,buccaneer boots,combat boots,monk boots)
100,(black leather gloves,brown leather gloves,red padded gloves,tan padded gloves)
100,hoodless circlet,0,0,2

# Mellok Morningstar, innkeeper, F1 (14546)
546
100,COINS BY VALUE,3,600,0
100,(red noble garb,black leather boots)

# Mellok inventory, buys nothing
1546
100,ale,0,0,4
100,stout,1,4,0
100,cider,1,4,0
100,mead,1,4,0
100,wine,1,4,0
100,rum,1,4,0
100,jug of mead,0,0,1
100,bottle of wine,0,0,1
100,bottle of rum,0,0,1
100,bottle of goodberry wine,0,0,1
100,potion of cure light wounds,1,2,0
100,potion of bears endurance,0,1,0

# Roxie Morningstar, wench (14547)
547
100,COINS BY VALUE,3,600,0
100,(little red dress,black leather boots)

# Moxie Morningstar, wench, F3 (14548)
548
100,COINS BY VALUE,3,600,0
100,(blue skirtless garb,black leather boots)

# Uriah, barkeep, R4 (14549)
549
100,COINS BY VALUE,3,600,0
100,brown leather armor
10,RING OF PROTECTION 1
10,AMULET 1
10,DEXTERITY 1
10,cloak of resistance +1 unseen
100,BOOTS

# Lipton, flute and lyre, 0-level (14550)
550
100,COINS BY VALUE,3,600,0
100,(red noble garb,black leather boots)

# Lipton inventory, buys nothing
1550
100,masterwork drum
100,masterwork flute
100,masterwork horn
100,masterwork mandolin

# Tycos Bushberry, R15 (14551)
551
100,COINS BY VALUE,3,6000,0
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
100,(VILLAGER GARB,BOOTS)
100,potion of heal
100,mage armor

# Tycos Bushberry, R15 (14551)
5511
100,(VILLAGER GARB,BOOTS)

# Tycos Bushberry, R15, with Fire Giants (14551)
5518
100,COINS BY VALUE,0,0,15000
100,SHORTSWORD 2
100,mithral chain shirt
100,mithral large shield
100,cloak of resistance +2 red
100,black leather boots
100,shield of faith_3
100,potion of greater invisibility

# Tali Woodwink, tinker, 0-level (14554)
554
100,COINS BY VALUE,3,600,0
100,eyeglasses
100,light hammer
100,(ochre skirtless garb,black leather boots)

# Tali inventory, buys metal
1554
100,(halfling rapier,halfling shortsword,halfling spiked chain,halfling siangham,halfling quarterstaff)
100,masterwork light hammer
100,eyeglasses
100,masterwork thieves tools

# Vernox Birch, druid matriarch, D12 (14555)
555
100,COINS BY VALUE,3,60,0
100,leather armor +2
100,ring of protection +2
100,amulet of natural armor +2
100,cloak of resistance +1 blue
100,(green robes,black leather boots)

# Vernox inventory, buys magic
1555
100,(amulet of natural armor +1, amulet of mighty fists +1)
100,(amulet of wisdom +2, amulet of health +2)
100,keoghtoms ointment,0,0,2
100,potion of cure light wounds,0,0,4
100,potion of cure moderate wounds,0,0,4
100,potion of cure serious wounds,0,0,2
100,potion of delay poison,0,0,2
100,potion of neutralize poison,0,0,2
100,potion of remove disease,0,0,2
100,potion of lesser restoration,0,0,2
100,potion of bulls strength,0,0,2
100,potion of bears endurance,0,0,2
100,potion of cats grace,0,0,2
100,potion of owls wisdom,0,0,2
100,potion of barkskin +2,0,0,2
100,potion of barkskin +3,0,0,2
100,potion of barkskin +4,0,0,2
100,potion of barkskin +5,0,0,2
100,potion of protection from acid 60
100,potion of protection from acid 108
100,potion of protection from cold 60
100,potion of protection from cold 108
100,potion of protection from electricity 60
100,potion of protection from electricity 108
100,potion of protection from fire 60
100,potion of protection from fire 108
100,potion of protection from sonic 108
100,potion of protection from sonic 60
100,potion of protection from earth
100,potion of protection from elementals
100,potion of resist acid 10
100,potion of resist acid 20
100,potion of resist acid 30
100,potion of resist acid 30
100,potion of resist cold 10
100,potion of resist cold 20
100,potion of resist cold 30
100,potion of resist cold 30
100,potion of resist electricity 10
100,potion of resist electricity 20
100,potion of resist electricity 30
100,potion of resist electricity 30
100,potion of resist fire 10
100,potion of resist fire 20
100,potion of resist fire 30
100,potion of resist fire 30
100,potion of resist sonic 10
100,potion of resist sonic 20
100,potion of resist sonic 30
100,potion of resist sonic 30

# Rahlon, F10/B1/R5 (14556)
556
100,COINS BY VALUE,3,600,0
100,cloak of resistance +3 unseen
100,(red villager garb,black leather boots)
100,potion of heal
100,mage armor
100,shield spell
100,resist energy fire

# Zahara, W13/R5 (14557)
557
100,COINS BY VALUE,3,600,0
100,cloak of resistance +3 unseen
100,(eyeglasses,scholars kit)
100,(blue villager garb,brown leather boots)
100,potion of heal
100,mage armor
100,shield spell
100,resist energy fire

# Corona, magic item merchant, W3 (14560)
560
100,COINS BY VALUE,3,600,0
100,eyeglasses
100,(VILLAGER GARB,black leather boots)
100,mage armor

# Corona inventory, chapter 1
1560
100,WONDROUS ITEM MINOR,1,3,3
50,WONDROUS ITEM MEDIUM,0,0,1
100,RING MINOR,0,0,1
50,RING MEDIUM,0,0,1
100,WEAPON 1,0,0,1
50,WEAPON 2,0,0,1
100,ARMOR 1,0,0,1
50,ARMOR 2,0,0,1

# Corona inventory, chapter 2
2560
100,WONDROUS ITEM MINOR,1,3,3
100,WONDROUS ITEM MEDIUM,0,0,1
100,RING MINOR,0,0,1
100,RING MEDIUM,0,0,1
100,WEAPON 1,0,0,1
100,WEAPON 2,0,0,1
100,ARMOR 1,0,0,1
100,ARMOR 2,0,0,1

# Corona inventory, chapter 3
3560
100,WONDROUS ITEM MEDIUM,1,3,1
100,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
100,RING MAJOR,0,0,1
100,WEAPON 3,0,0,1
100,WEAPON 4,0,0,1
100,ARMOR 3,0,0,1
100,SHIELD 4,0,0,1

# Corona inventory, chapter 4
4560
100,WONDROUS ITEM MEDIUM,1,3,1
100,WONDROUS ITEM MAJOR,0,0,1
100,RING MEDIUM,0,0,1
100,RING MAJOR,0,0,1
100,WEAPON 3,0,0,1
100,WEAPON 4,0,0,1
100,ARMOR 3,0,0,1
100,SHIELD 4,0,0,1

# Provisioner, crossroads chapter 1, 0-level (14561)
561
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,black leather boots)
100,eyeglasses

# Provisioner, crossroads chapter 2, 0-level (14561)
5612
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,black leather boots)
100,eyeglasses

# Provisioner, crossroads chapter 3, 0-level (14561)
5613
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,black leather boots)
100,eyeglasses

# Provisioner, crossroads chapter 4, 0-level (14561)
5614
100,COINS BY VALUE,3,600,0
100,(VILLAGER GARB,black leather boots)
100,eyeglasses

# Kelvin Garnet, Gem Merchant, 0-level (14562)
562
100,COINS BY VALUE,3,6000,0
100,(red noble garb,jaunty hat,brown leather boots)

# Kelvin inventory
1562
100,eyeglasses
100,ioun stone dusty rose
100,(ioun stone deep red,ioun stone incandescent blue,ioun stone pale blue)
100,(ioun stone pink,ioun stone pink and green,ioun stone scarlet and blue)
100,GEMS OR JEWELRY,2,10,10

# Provisioner inventory
1561
100,(healers kit,scholars kit,eyeglasses,spyglass)
100,thieves tools
100,masterwork thieves tools
100,(drum,flute,horn,mandolin)
100,holy water,0,0,2
100,keoghtoms ointment,0,0,2
100,potion of cure light wounds,0,0,2
100,potion of cure moderate wounds,0,0,2
100,potion of cure serious wounds,0,0,2
100,potion of neutralize poison,0,0,2
100,potion of remove blindness/deafness,0,0,2
100,potion of protection from cold 60
100,potion of protection from cold 108
100,potion of resist cold 10
100,potion of resist cold 20
100,potion of resist cold 30
100,potion of resist cold 30
100,antitoxin,0,0,2
100,flask of oil,0,0,2
100,lockslip grease,0,0,2
100,armor oil,0,0,2
100,smokestick,0,0,2
100,quiver of arrows,0,0,10
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20
100,pouch of bullets,0,0,20
100,masterwork sling
100,light crossbow
100,shortbow
100,mining pick,0,0,2
100,heavy pick,0,0,2
100,(mutton,cheese,bread,honey,apple,almonds,dates,raisins,dried fish,salt pork)
100,(keg of ale,keg of wine)
100,HELM,0,0,2
100,BOOTS,0,0,2
100,LEATHER ARMOR,0,0,2
100,ARMORED BOOTS,0,0,2
100,GLOVES,0,0,2
100,ARMORED GLOVES,0,0,2
100,FARMER GARB,0,0,2
100,VILLAGER GARB,0,0,2
100,ROBES,0,0,2

# Trenn Bandit Archer, F6 (14564)
564
100,COINS BY VALUE,2,600,0
100,composite longbow str 16
100,quiver of arrows,0,0,3
100,club
100,(CHAINMAIL,buckler,brown leather boots,brown leather gloves)
25,RING OR AMULET 1
100,(fur cloak,hoodless circlet)

# Rhondi Bandit Archer, F6 (14565)
565
100,COINS BY VALUE,2,600,0
100,composite longbow str 16
100,quiver of arrows,0,0,3
100,club
100,(CHAINMAIL,buckler,generic helm,black leather boots,black leather gloves)
25,RING OR AMULET 1
100,(black cloak,hoodless circlet)

# Trenn Vet, F11 (14566)
566
100,COINS BY VALUE,2,6000,0
50,GREATHAMMER 0  100,GLAIVE 0
50,FULL PLATE  100,FULL PLATE 1
50,RING OF PROTECTION 1
50,AMULET 1
50,fur cloak  100,cloak of resistance +1 fur
25,STRENGTH 2
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Vet, F11 (14567)
567
100,COINS BY VALUE,2,6000,0
50,LONGSWORD 0  100,WARHAMMER 0
50,FULL PLATE  100,FULL PLATE 1
50,RING OF PROTECTION 1
50,AMULET 1
50,red cloak  100,cloak of resistance +1 red
100,(hoodless circlet,plate gloves,plate boots)
25,STRENGTH 2

# Trenn Knight, F16 (14568)
568
100,COINS BY VALUE,2,6000,0
100,LONGSPEAR 2
100,(FULL PLATE 2,BUCKLER 2)  
100,RING OR AMULET 2
100,cloak of resistance +2 fur
50,STRENGTH 2
100,(great helm,plate gloves,plate boots)

# Rhondi Knight, F16 (14569)
569
100,COINS BY VALUE,2,6000,0
100,LONGSWORD 2
100,(FULL PLATE 2,LARGE SHIELD 2)  
100,RING OR AMULET 2
100,cloak of resistance +2 light blue
50,STRENGTH 2
100,(great helm,plate gloves,plate boots)

# Sylvvi, Sorceress, temm exterior, W7 (14570)
570
100,COINS BY VALUE,0,0,700
100,quarterstaff
100,(FARMER GARB,red cloak,hoodless circlet,BOOTS)
100,mage armor

# Sylvvi, Sorceress, temm upstairs, W7 (14570)
5701
100,COINS BY VALUE,0,0,700
100,quarterstaff
100,(FARMER GARB,red cloak,hoodless circlet,BOOTS)
100,mage armor

# Sylvvi inventory, outside
1570
100,ring of protection +1
100,ring of protection +2
100,bracers of armor +3
100,amulet of health +4

# Sylvvi inventory, inside, chapter 2
2570
100,ring of blinking
100,ring of protection +2
100,bracers of armor +3
100,amulet of health +4
100,WEAPON 3
100,WEAPON 3
50,longsword +4
50,club of striking				# +2, striking (4120)
50,axiomatic warhammer +2		# +2, lawful
10,greatsword +4 frost
10,battleaxe +3 fire and ice
10,greataxe +4 electric
10,warhammer +4 thundering
10,greathammer +4 chiller
10,glaive +4 searing
10,ranseur +4 defender
10,dagger +4 flaming
10,composite longbow str 14 +4 keen
10,composite longbow str 18 +4 shocking
10,longsword +3 blessed by rao
10,heavy mace +3 blessed by cuthbert

# Sylvvi inventory, inside, chapter 3
3570
100,ring of invisibility
100,ring of protection +3
100,bracers of armor +4
100,amulet of health +6
100,greatsword +4 frost
100,WEAPON M 3,2,4,0
100,WEAPON M 4,1,4,0
25,battleaxe +3 fire and ice
25,greataxe +4 electric
25,warhammer +4 thundering
25,greathammer +4 chiller
25,glaive +4 searing
25,ranseur +4 defender
25,dagger +4 flaming
25,composite longbow str 14 +4 keen
25,composite longbow str 18 +4 shocking
25,longsword +3 blessed by rao
25,heavy mace +3 blessed by cuthbert
25,greatsword +5
25,battleaxe +5
25,greataxe +5
25,warhammer +5
25,greathammer +5
25,glaive +5
25,ranseur +5
25,longbow +5
25,composite longbow str 14 +5
25,composite longbow str 18 +5

# Sylvvi inventory, inside, chapter 4
4570
100,ring of invisibility
100,ring of protection +3
100,ring of protection +4
100,bracers of armor +6
100,WEAPON M 3,2,4,2
100,WEAPON M 4,2,4,1
100,WEAPON M 5,2,4,0

# Dwarven Innkeeper, Happy, F3 (14580)
580
100,COINS BY VALUE,3,600,0
100,(NOBLE GARB,BOOTS)

# Dwarven Cook, Bashful, F3 (14581)
581
100,COINS BY VALUE,3,600,0
100,masterwork great cleaver
50,LEATHER ARMOR
100,(VILLAGER GARB,BOOTS)

# Dwarven Housekeeper, Sneezy, F3 (14582)
582
100,COINS BY VALUE,3,600,0
100,masterwork broom
50,LEATHER ARMOR
100,(VILLAGER GARB,BOOTS)

# Dwarven Barkeep, Dopey, F3 (14583)
583
100,COINS BY VALUE,3,600,0
50,LEATHER ARMOR
100,(VILLAGER GARB,BOOTS)

# Dwarven Brewer, Doc, F3 (14584)
584
100,COINS BY VALUE,3,600,0
50,LEATHER ARMOR
100,(NOBLE GARB,BOOTS)

# Dwarven Assistant, Grumpy, F3 (14585)
585
100,COINS BY VALUE,3,600,0
50,LEATHER ARMOR
100,(VILLAGER GARB,BOOTS)

# Dwarven Assistant, Grumpy, at potters house, F3 (14585)
5851
100,COINS BY VALUE,3,600,0
50,LEATHER ARMOR
100,(VILLAGER GARB,BOOTS)

# Dwarven Wizard, Sleepy, W13 (14586)
586
100,COINS BY VALUE,3,6000,0
100,BRACERS 2
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
100,INTELLIGENCE 2
100,(eyeglasses,scholars kit)
100,(green robes,black leather boots)
100,potion of heal
100,mage armor

# Temple Leader, Kelrom Bel'al, C15, initial location (14590)
590
100,COINS BY VALUE,1,15000,0
100,cudgel of retribution
100,FULL PLATE 3
100,LARGE SHIELD 3
100,cloak of resistance +2 black
100,WISDOM 2
100,STRENGTH 2
100,(fancy black robes,hoodless circlet,plate gloves,brown leather boots)
100,scroll of heal
100,shield of faith_3
100,resist energy cold
100,resist energy electricity
100,resist energy fire

# Temple Leader, Kelrom Bel'al, C15, ambush location after pc destroys temple (14590)
5901
100,COINS BY VALUE,1,15000,0
100,heavy mace +3
100,FULL PLATE 3
100,LARGE SHIELD 3
100,cloak of resistance +2 black
100,WISDOM 2
100,STRENGTH 2
100,(fancy black robes,hoodless circlet,plate gloves,brown leather boots)
100,scroll of heal
100,shield of faith_3
100,resist energy cold
100,resist energy electricity
100,resist energy fire

# Temple Leader inventory
1590
100,WONDROUS ITEM MAJOR,1,3,0
100,RING MAJOR,0,0,1
100,WEAPON M 3,0,0,2
100,WEAPON M 4,0,0,1
100,WEAPON M 5,0,0,1
100,ARMOR 3,0,0,1
100,SHIELD 3,0,0,1

# Temple Leader Bonus inventory
7590
100,WONDROUS ITEM MAJOR,1,3,0
100,RING MAJOR,0,0,1
100,WEAPON M 3,0,0,2
100,WEAPON M 4,0,0,1
100,WEAPON M 5,0,0,1
100,ARMOR 3,0,0,1
100,SHIELD 3,0,0,1

# Gypsy Leader, Eyvva, F8/R3 (14591)
591
100,silver key
100,COINS BY VALUE,1,11000,0
100,red gypsy dress
100,(shortsword +2,shortsword +2)
100,ring of protection +2
100,cloak of resistance +1 black
100,gloves of dexterity +4
100,(hoodless circlet,black leather boots)
100,potion of heal
100,mage armor
100,barkskin_4

# Gypsy Leader inventory
1591
100,masterwork dagger,0,0,2

# Gypsy Leader, food chest
5917
100,cheese,0,0,2
100,bread,0,0,2
100,mutton,0,0,2
100,chicken,0,0,2
100,stew,0,0,2
100,grapes,0,0,2
100,apple,0,0,2
100,ale,0,0,4
100,stout,0,0,4
100,mead,0,0,4
100,cider,0,0,2
100,wine,0,0,2
100,rum,0,0,2

# Gypsy Leader, equipment chest
5918
100,thieves tools,0,0,2
100,(healers kit,scholars kit)
100,(drum,flute,horn,mandolin)
100,holy water,0,0,2
100,flask of oil,0,0,2
100,antitoxin,0,0,2
100,lockslip grease,0,0,2
100,armor oil,0,0,2
100,smokestick,0,0,2
100,(rope 50 feet,chain 10 feet,iron spike,ten-foot pole)
100,quiver of arrows,0,0,10
100,quiver of arrows,0,0,10
100,quiver of arrows,0,0,10
100,quarrel of bolts,0,0,20
100,pouch of bullets,0,0,20
100,HELM,0,0,1
100,BOOTS,0,0,2
100,ARMORED BOOTS,0,0,2
100,GLOVES,0,0,2
100,ARMORED GLOVES,0,0,2
100,FARMER GARB,0,0,2
100,VILLAGER GARB,0,0,2
100,hoodless circlet,0,0,2

# Derro Leader, F15 (14592)
592
100,brass key
20,CGI,10,3,1  100,COINS BY VALUE,3,15000,0
100,full plate +2
100,glaive +3
100,ring of protection +2
100,cloak of resistance +1 white
100,belt of giant strength +4
100,(hoodless circlet,black leather boots)
100,potion of heal
100,barkskin_4

# Derro Leader inventory
1592
100,adamantine breastplate
100,adamantine chain shirt
100,dwarven plate
100,masterwork dwarven stone armor
100,mithral full plate
100,mithral breastplate
100,mithral chain shirt
100,elven chain
100,mithral tower shield
100,mithral buckler
100,mithral large shield,0,0,2
20,mithral full plate +1  40,mithral full plate +2  60,mithral full plate +3
20,mithral breastplate +1  40,mithral breastplate +2  60,mithral breastplate +3
20,elven chain +1  40,elven chain +2  60,elven chain +3
20,mithral chain shirt +1  40,mithral chain shirt +2  60,mithral chain shirt +3
20,mithral buckler +1  40,mithral buckler +2  60,mithral buckler +3  
20,mithral large shield +1  40,mithral large shield +2  60,mithral large shield +3
20,mithral large shield +1  40,mithral large shield +2  60,mithral large shield +3
20,mithral tower shield +1  40,mithral tower shield +2  60,mithral tower shield +3

# Trenn Bandit, F3/R1 (14596)
596
100,COINS BY VALUE,2,600,0
33,greatsword  66,greathammer  100,greataxe
100,chain shirt
100,(fur cloak,generic helm,black leather boots)

# Rhondi Bandit, F3/R1 (14597)
597
100,COINS BY VALUE,2,600,0
33,longsword  66,warhammer  100,club
100,chain shirt
100,(black cloak,generic helm,black leather boots)

# Drow Goddess (14599)
599
100,(shortsword,dagger)
100,drow goddess garb
100,(black leather boots,black leather gloves)
100,shield spell

# Guthnar, boatbuilder (14600)
600
100,COINS BY VALUE,3,600,0
100,light hammer
100,(green farmer garb,red bandana,brown leather boots)

# Rasmine, boatbuilder's daughter (14601)
601
100,COINS BY VALUE,3,600,0
100,greathammer
100,(red skirtless garb,black leather boots)

# Captain Marlon Hull, F9 (14602)
602
100,COINS BY VALUE,3,6000,0
100,GREATSWORD 1
100,CHAIN SHIRT 1
100,RING OF PROTECTION 1
100,cloak of resistance +1 blue
50,STRENGTH 1
100,(black long coat,bicorne,black leather gloves,black leather boots)

# Captain Jonas Borrison, night at tavern F9 (14603)
603
100,COINS BY VALUE,3,6000,0
100,GREATAXE 1
100,CHAIN SHIRT 1
100,RING OF PROTECTION 1
100,cloak of resistance +1 unseen
50,STRENGTH 1
100,(red long coat,black leather gloves,black leather boots)

# Captain Jonas Borrison, night at inn, F9 (14603)
6031
100,COINS BY VALUE,3,6000,0
100,GREATAXE 1
100,CHAIN SHIRT 1
100,RING OF PROTECTION 1
100,cloak of resistance +1 unseen
50,STRENGTH 1
100,(red long coat,hoodless circlet,brown leather boots)

# Captain Jorrla Kinsila, W5/R5 (14604)
604
100,COINS BY VALUE,3,6000,0
100,composite shortbow str 14 +1
100,quiver of arrows,0,0,3
100,RAPIER 0
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
75,DEXTERITY 1
100,(green skirtless garb,buccaneer boots)
100,scroll of hold person
100,mage armor

# Father Alterius, day, C9 (14605)
605
100,COINS BY VALUE,3,6000,0
100,light mace +1
100,FULL PLATE 1
100,LARGE SHIELD 1
100,RING OF PROTECTION 1
100,cloak of resistance +1 unseen
50,WISDOM 1
100,(red robes,plate boots)

# Father Alterius, night, C9 (14605)
6051
100,COINS BY VALUE,3,6000,0
100,FULL PLATE 1
100,RING OF PROTECTION 1
100,cloak of resistance +1 unseen
50,WISDOM 1
100,red monk robes

# Parvo, C4 (14606)
606
100,COINS BY VALUE,3,600,0
75,FULL PLATE 0  100,FULL PLATE 1
10,RING OR AMULET 1
10,cloak of resistance +1 light blue
100,(brown robes,light blue cloak,hoodless circlet,black leather boots)

# Brother Foote, C6 (14607)
607
100,COINS BY VALUE,3,600,0
100,CHAIN SHIRT 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 unseen
100,(white robes,brown leather boots)

# Sister Pia, C6 (14608)
608
100,COINS BY VALUE,3,600,0
100,CHAIN SHIRT 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 blue
100,(white monk robes,blue cloak,black leather boots)

# Goblin Janitor, R3 (14609)
609
100,COINS BY VALUE,2,400,0
100,broom

# Goblin Groundskeeper, R3 (14610)
610
100,COINS BY VALUE,2,400,0
100,rake

# Goblin Mucker, R3 (14611)
611
100,COINS BY VALUE,2,400,0
100,shovel

# Ghost (14612)
612
100,black evening dress

# Ghost (14612)
6121
100,air elemental power gem
100,earth elemental power gem
100,fire elemental power gem
100,water elemental power gem
100,GEMS OR JEWELRY,2,4,4

# Ghost (14612)
613
100,red noble garb

# Kirkland Van Kirk, Patrol Sergeant, F6/R3 (14614)
614
100,COINS BY VALUE,2,600,0
100,SHORTSWORD 1
100,CHAIN SHIRT 1
100,BUCKLER 1
100,RING OR AMULET 1
50,DEXTERITY 1
100,(white cloak,hoodless circlet,black leather gloves,black leather boots)

# Remington Tosh, R7 (14615)
615
100,COINS BY VALUE,3,6000,0
100,(green noble garb,black cloak,hoodless circlet,black leather boots)
100,(armor oil,armor oil,potion of cure serious wounds)

# Remington, chest
2615
100,CGI,10,1,1

# Gale Bushberry, F3 (14616)
616
100,COINS BY VALUE,3,1000,0
100,(SKIRTLESS GARB,black leather boots)
100,potion of cure moderate wounds

# Gale Bushberry, Rivermist Inn version, F3 (14616)
6161
100,COINS BY VALUE,3,1000,0
100,(SKIRTLESS GARB,black leather boots)
100,potion of cure moderate wounds

# Oelrich, Flute and Lyre Inn, F3 (14617)
617
100,COINS BY VALUE,3,400,0
100,(VILLAGER GARB,black leather boots)
100,potion of cure moderate wounds

# Oelrich, with Rasmine, F3 (14617)
6171
100,COINS BY VALUE,3,400,0
100,(VILLAGER GARB,black leather boots)
100,potion of cure moderate wounds

# Scutty Gartooth, C9/P3 (14618)
618
100,COINS BY VALUE,3,600,0
100,chain shirt +3
100,amulet of natural armor +3
100,(VILLAGER GARB,black leather boots)
100,potion of cure serious wounds,0,0,2

# Sir Witkos, F11 (14619)
619
100,COINS BY VALUE,3,600,0
100,longsword +3
100,full plate +2
100,ring of protection +1
100,cloak of resistance +1 unseen
100,gauntlets of ogre power
100,ring of freedom of movement
100,brown leather boots
100,potion of cure serious wounds,0,0,1

# Vorlo Honorhelm, Guildmaster of Craftsmen, F12 (14620)
620
100,COINS BY VALUE,3,6000,0
100,dwarven stone armor +2
100,BUCKLER 1
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
100,STRENGTH 1
100,(orange cloak,hoodless circlet,black leather boots)
100,potion of heal

# Bex Paravishra, Guildmaster of Trade, Ranger 11 (14621)
621
100,COINS BY VALUE,3,6000,0
100,CHAIN SHIRT 2
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
0,DEXTERITY 1
100,(beige robes,brown leather boots)
100,potion of heal

# Gil of Luxwood, Guildmaster of Magic, W13 (14622) 
622
100,COINS BY VALUE,3,6000,0
100,BRACERS 2
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
100,INTELLIGENCE 2
100,(eyeglasses,scholars kit)
100,(black mystic garb,hoodless circlet,black leather boots)
100,potion of heal
100,mage armor

# Ilevium Vendetta, Guildmaster of Metal, C12 (14623)
623
100,COINS BY VALUE,3,6000,0
100,FULL PLATE 2
100,RING OF PROTECTION 2
0,AMULET 1
100,cloak of resistance +1 orange
100,WISDOM 1
100,(orange cloak,hoodless circlet,plate boots)
100,potion of heal

# Ilevium Vendetta, after becoming leader (14623)
6231
100,COINS BY VALUE,3,6000,0
100,FULL PLATE 3
100,AMULET 3
100,cloak of resistance +3 violet
100,WISDOM 1
100,(gold crown,plate boots)
100,potion of heal
100,shield of faith_4
100,resist energy fire
100,resist energy cold
100,resist energy acid


# Myella Von Ruggenport, Guildmaster of Stone, R9 (14624)
# Weapons created when she joins group
624
100,COINS BY VALUE,3,6000,0
100,myellas gypsy armor
100,ring of protection +1
100,amulet of natural armor +1
100,cloak of resistance +2 unseen
100,gloves of dexterity +2
100,black leather boots
100,potion of heal
100,mage armor

# Thornwell Peck, Guildmaster of Seamen, F4/R7 (14625)
625
100,COINS BY VALUE,3,6000,0
100,CHAIN SHIRT 2
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 unseen
100,DEXTERITY 1
100,(black long coat,black leather boots)
100,potion of heal

# Velanya Bushberry, Guildmaster of The Watch, F11 (14626)
626
100,COINS BY VALUE,3,6000,0
100,black full plate
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 white
100,STRENGTH 1
100,plate boots
100,potion of heal

# Octavius, Guildhall Administrator, F1 (14627)
627
100,COINS BY VALUE,3,600,0
100,quarterstaff
100,(NOBLE GARB,black leather boots)
100,potion of barkskin +4

# Lucern, C3 (14628)
628
100,COINS BY VALUE,0,0,2300
100,masterwork morningstar
100,(half-plate,large steel shield,plate boots)
100,potion of cure light wounds_2,0,0,4
100,potion of cure moderate wounds_2,0,0,2
100,potion of cure serious wounds_2,0,0,1

# Evian Riss, crypt, W5 (14629)
629
100,COINS BY VALUE,2,600,0
100,scroll of expeditious retreat
100,scroll of identify,0,0,2
100,scroll of knock,0,0,2
100,(VILLAGER GARB,BOOTS)
100,mage armor

# Evian Riss, Inn, W5 (14629)
6291
100,COINS BY VALUE,2,6000,0
100,scroll of expeditious retreat
100,scroll of identify,0,0,2
100,scroll of knock,0,0,2
100,(VILLAGER GARB,BOOTS)
100,mage armor

# Evian Riss, back at crypt, W5 (14629)
6292
100,COINS BY VALUE,2,6000,0
100,scroll of expeditious retreat
100,scroll of identify,0,0,2
100,scroll of knock,0,0,2
100,(VILLAGER GARB,BOOTS)
100,mage armor

# Torv, B3 (14630)
630
100,COINS BY VALUE,3,600,0
100,(masterwork greatsword,greatclub)
100,(scale mail,fine leather boots)
100,ring of protection +1
100,potion of cure moderate wounds,0,0,2

# Kaleela, Bard 3 (14631)
631
100,COINS BY VALUE,0,0,1500
100,dagger
100,(masterwork mandolin,flute,drum)
100,(black farmer garb,black leather boots)

# Pilzner Cuthbert, Ranger 4 (14632)
632
100,COINS BY VALUE,0,0,3500
100,(studded leather +1,buckler)
100,(masterwork composite longbow str 12,longsword)
100,quiver of arrows,0,0,2
100,(red padded gloves,combat boots)
100,(antitoxin,potion of cure moderate wounds_2)

# Waldon, crypt exterior, F3 (14633)
633
100,COINS BY VALUE,0,0,1900
100,club
100,chainmail
100,BOOTS

# Waldon, Inn, F3 (14633)
6331
100,COINS BY VALUE,0,0,1900
100,masterwork warhammer
100,(chainmail,large wooden shield_2)
100,BOOTS

# Hollan, M4 (14634)
634
100,COINS BY VALUE,0,0,700
100,amulet of mighty fists +2
100,(white monk robes,monk wrap,monk boots)

# Thok, Tavern, F2 (14635)
635
100,COINS BY VALUE,0,0,2300
100,masterwork maul
100,(banded mail,plate boots,black leather gloves)

# Pierce, Tavern, F2 (14636)
636
100,COINS BY VALUE,0,0,150
100,masterwork longspear
100,(breastplate,plate boots)

# Keega, Centaur at Luxwood (14637)
637
100,potion of heal_2,0,0,1
100,mother ferns healing potion,0,0,2
100,potion of cure serious wounds_2,0,0,1
100,potion of restoration_2
100,masterwork sling
100,(pouch of bullets,pouch of bullets)
100,greatsword +3
100,chain shirt

# Fralla No-Thumbs, R5 (14638)
638
100,COINS BY VALUE,0,0,750
100,(potion of cats grace,potion of cure moderate wounds)
100,masterwork thieves tools
100,dagger +2
100,chain shirt
100,(black leather gloves,BOOTS)

# Fralla No-Thumbs, on docks at night, R5 (14638)
6381
100,COINS BY VALUE,0,0,750
100,(potion of cats grace,potion of cure moderate wounds)
100,(emerald_2,sapphire_2,jasper ring_2,jade pendant_2,gold crown_2)
100,masterwork thieves tools
100,dagger +2
100,chain shirt
100,(black leather gloves,BOOTS)

# Detlef Horlacher, F1/R8 (14639)
639
100,COINS BY VALUE,0,0,15000
100,masterwork wakizashi
100,padded armor +2
100,ring of protection +1
100,gloves of dexterity +2
100,(masterwork thieves tools,lockslip grease)
100,black leather boots

# Erig, F8/R3 (14640)
640
100,masterwork dagger
100,black leather armor
100,(black cloak,hoodless circlet,black leather boots)

# Erig, F8/R3 (14640)
6401
100,COINS BY VALUE,0,0,3730
100,masterwork dagger
100,black leather armor
100,(black cloak,hoodless circlet,black leather boots)

# Erig, F8/R3 (14640)
6402
100,COINS BY VALUE,0,0,13100
100,shortsword +3
100,black leather armor
100,darkwood buckler
100,RING OR AMULET 2
100,(watch cloak,hoodless circlet,black leather boots)

# Lake, B2/C9 (14641)
641
100,COINS BY VALUE,3,6000,0
100,holy ranseur +3
100,FULL PLATE 3
100,RING OF PROTECTION 1
100,amulet of natural armor +4
100,cloak of resistance +2 white
100,ioun stone pink
100,ioun stone deep red
100,ioun stone incandescent blue
100,(hoodless circlet,black leather gloves,plate boots)
100,mother ferns healing potion,0,0,5
100,keoghtoms ointment,0,0,2
100,scroll of raise dead,0,0,1

# Cloud Giantess (14642)
# no crown, it makes her look bald.
642
100,COINS BY VALUE,3,6000,0
100,cloud giant morningstar
100,ring of the clouds
100,white evening dress
100,silver necklace
100,monk boots

# Trenn Commander, F6 (14650)
650
100,COINS BY VALUE,0,0,27600
100,GREATSWORD 1
100,BREASTPLATE 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 fur  100,fur cloak
5,STRENGTH 1
100,(hoodless circlet,GLOVES,black leather boots)
100,potion of cure moderate wounds,0,0,3

# Trenn Commander, chapter 1, F6 (14650)
6501
100,COINS BY VALUE,0,0,27600
100,GREATSWORD 1
100,BREASTPLATE 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 fur  100,fur cloak
5,STRENGTH 1
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Commander, chapter 2, F6 (14650)
6502
100,COINS BY VALUE,0,0,27600
100,GREATSWORD 1
100,BREASTPLATE 1
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 fur  100,fur cloak
5,STRENGTH 1
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Commander B1, with Hill Giants, F15 (14650)
6503
100,COINS BY VALUE,0,16000,0
100,GREATSWORD 3
100,BREASTPLATE 3
100,cloak of resistance +1 fur
100,STRENGTH 1
100,(hoodless circlet,GLOVES,black leather boots)
100,barkskin_3
100,shield of faith_3

# Trenn Commander, after dying in battle, F6 (14650)
6509
100,greatsword
100,breastplate
100,fur cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Lieutenant, F4/R1 (14651)
651
100,COINS BY VALUE,0,0,3700
50,SHORTSWORD 1  100,SHORTSWORD 0
75,SHORTSWORD 1  100,SHORTSWORD 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 fur  100,fur cloak
100,(hoodless circlet,GLOVES,black leather boots)
100,black pearl,0,0,3
100,white pearl,0,0,3

# Trenn Lieutenant, chapter 1, F4/R1 (14651)
6511
100,COINS BY VALUE,0,0,3700
50,SHORTSWORD 1  100,SHORTSWORD 0
75,SHORTSWORD 1  100,SHORTSWORD 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 fur  100,fur cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Lieutenant, chapter 2, F4/R1 (14651)
6512
100,COINS BY VALUE,0,0,3700
50,SHORTSWORD 1  100,SHORTSWORD 0
75,SHORTSWORD 1  100,SHORTSWORD 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 fur  100,fur cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Lieutenant, chapter 3, F4/R1 (14651)
6513
100,COINS BY VALUE,0,0,3700
50,SHORTSWORD 1  100,SHORTSWORD 0
75,SHORTSWORD 1  100,SHORTSWORD 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 black  100,black cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Lieutenant, chapter 4, F4/R1 (14651)
6514
100,COINS BY VALUE,0,0,3700
50,SHORTSWORD 1  100,SHORTSWORD 0
75,SHORTSWORD 1  100,SHORTSWORD 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Trenn Lieutenant, after dying in battle, F4/R1 (14651)
6519
100,shortsword,0,0,2
100,chain shirt
100,fur cloak
100,(hoodless circlet,GLOVES,black leather boots)

# Liossa, Trenn Sorceress, battelfield, W6 (14652)
652
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(red gypsy dress,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,sack of berries
100,mage armor
100,shield spell

# Liossa, Trenn Sorceress, chapter 1, W6 (14652)
6521
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(red gypsy dress,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Liossa, Trenn Sorceress, chapter 2, W6 (14652)
6522
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(red gypsy dress,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Liossa, Trenn Sorceress, chapter 3, W6 (14652)
6523
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(blue gypsy dress,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Liossa, Trenn Sorceress, chapter 4, W6 (14652)
6524
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(purple gypsy dress,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Liossa, Drow ally, chapter 4, W11 (14652)
6525
100,COINS BY VALUE,0,0,7300
100,LONGBOW 1
100,AMULET 1
100,RING OF PROTECTION 3
100,CLOAK OF RESISTANCE 2
100,(purple gypsy dress,hoodless circlet)
100,BOOTS
100,mage armor
100,shield spell

# Rhondi Commander, battlefield, F6 (14653)
653
100,COINS BY VALUE,0,0,6800
100,LONGSWORD 1
100,FULL PLATE 1
75,LARGE SHIELD 1  100,LARGE SHIELD
50,RING OF PROTECTION 1
50,cloak of resistance +1 white  100,white cloak
5,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)
100,potion of cure light wounds,0,0,5

# Rhondi Commander, chapter 1, F6 (14653)
6531
100,COINS BY VALUE,0,0,6800
100,LONGSWORD 1
100,FULL PLATE 1
75,LARGE SHIELD 1  100,LARGE SHIELD
50,RING OF PROTECTION 1
50,cloak of resistance +1 white  100,white cloak
5,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Commander, chapter 2, F6 (14653)
6532
100,COINS BY VALUE,0,0,6800
100,LONGSWORD 1
100,FULL PLATE 1
75,LARGE SHIELD 1  100,LARGE SHIELD
50,RING OF PROTECTION 1
50,cloak of resistance +1 white  100,white cloak
5,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Commander, B1, with Hill Giants, F6 (14653)
6533
100,LONGSWORD 3
100,FULL PLATE 1
100,LARGE SHIELD 2
100,cloak of resistance +1 white
100,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)
100,barkskin_3
100,shield of faith_3

# Rhondi Commander, battlefield visiting Trenn camp, F6 (14653)
6538
100,LONGSWORD 1
100,FULL PLATE 1
75,LARGE SHIELD 1  100,LARGE SHIELD
50,RING OF PROTECTION 1
50,cloak of resistance +1 white  100,white cloak
5,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Commander, after dying in battle, F6 (14653)
6539
100,(LONGSWORD,full plate,LARGE SHIELD)
100,(white cloak,hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, R5 (14654)
654
100,COINS BY VALUE,0,0,28700
50,RAPIER 1  100,RAPIER 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,BUCKLER 1  100,BUCKLER 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, chapter 1, R5 (14654)
6541
100,COINS BY VALUE,0,0,28700
50,RAPIER 1  100,RAPIER 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,BUCKLER 1  100,BUCKLER 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, chapter 2, R5 (14654)
6542
100,COINS BY VALUE,0,0,28700
50,RAPIER 1  100,RAPIER 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,BUCKLER 1  100,BUCKLER 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, chapter 3, R5 (14654)
6543
100,COINS BY VALUE,0,0,28700
50,RAPIER 1  100,RAPIER 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,BUCKLER 1  100,BUCKLER 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 black  100,black cloak
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, chapter 4, R5 (14654)
6544
100,COINS BY VALUE,0,0,28700
50,RAPIER 1  100,RAPIER 0
50,CHAIN SHIRT 1  100,CHAIN SHIRT 0
25,BUCKLER 1  100,BUCKLER 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 violet  100,violet cloak
100,(hoodless circlet,plate gloves,plate boots)

# Rhondi Lieutenant, after dying in battle, R5 (14654)
6549
100,(rapier,chain shirt,buckler)
100,(white cloak,hoodless circlet,plate gloves,plate boots)

# Wexton, Rhondi Wizard, camp version, W6 (14655)
655
100,COINS BY VALUE,0,0,13700
50,dagger +1  100,masterwork dagger
25,AMULET 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 red  100,red cloak
100,(orange robes,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Wexton, Rhondi Wizard, chapter 1, W6 (14655)
6551
100,COINS BY VALUE,0,0,13700
50,dagger +1  100,masterwork dagger
25,AMULET 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 red  100,red cloak
100,(orange robes,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Wexton, Rhondi Wizard, chapter 2, W6 (14655)
6552
100,COINS BY VALUE,0,0,13700
50,dagger +1  100,masterwork dagger
25,AMULET 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 red  100,red cloak
100,(orange robes,hoodless circlet)
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Wexton, Rhondi Wizard, chapter 3, W6 (14655)
6553
100,COINS BY VALUE,0,0,13700
50,dagger +1  100,masterwork dagger
25,AMULET 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 black  100,black cloak
100,orange robes
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Wexton, Rhondi Wizard, chapter 4, W6 (14655)
6554
100,COINS BY VALUE,0,0,13700
50,dagger +1  100,masterwork dagger
25,AMULET 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,orange robes
100,BOOTS
100,scroll of mirror image
100,scroll of fireball,0,0,2
100,scroll of magic missile,0,0,4
100,mage armor

# Wexton, Rhondi Wizard, wounded/dying in battle, W6 (14655)
6559
100,cinnamon
100,dagger
100,(orange robes,white cloak)
100,BOOTS
100,potion of cure light wounds,0,0,3
100,mage armor

# Mabon, battelfield, C6 (14656)
656
100,COINS BY VALUE,0,0,1300
100,quarterstaff +1
100,full plate +1
100,AMULET 1
50,cloak of resistance +1 white  100,white cloak
100,malachite,0,0,20
100,(brown robes,black leather boots)

# Mabon, chapter 1, C6 (14656)
6561
100,COINS BY VALUE,0,0,1300
100,quarterstaff +1
100,full plate +1
100,AMULET 1
50,cloak of resistance +1 white  100,white cloak
100,(brown robes,black leather boots)

# Mabon, chapter 2, C6 (14656)
6562
100,COINS BY VALUE,0,0,1300
100,quarterstaff +1
100,full plate +1
100,AMULET 1
50,cloak of resistance +1 white  100,white cloak
100,(brown robes,black leather boots)

# Mabon, relics in hut in chest for quest #125 (1005)
6569
100,(artifact tools,artifact trinkets,artifact shards,artifact pieces)

# Jubu-Nara, troll scout (14657)
657
100,ring of fire resistance minor

# Rhondi Cleric, 4d8/C5 (14658)
658
100,COINS BY VALUE,3,6000,0
100,vial of goop,0,0,2
100,potion of glibness
50,heavy mace +1  100,masterwork heavy mace
50,FULL PLATE 1  100,FULL PLATE 0
25,LARGE SHIELD 1  100,large steel shield_3
5,RING OR AMULET 1
25,cloak of resistance +1 unseen
100,(plate gloves,plate boots)

# Rhondi Cleric, after dying on battlefield, 4d8/C5 (14658)
6589
100,masterwork heavy mace
100,FULL PLATE 0
100,large steel shield_3
100,(plate gloves,plate boots)

# Artemis, Ranger 3 (14659)
659
100,COINS BY VALUE,1,50000,0
100,tan padded armor
100,quiver of arrows,0,0,2
100,(green cloak,hoodless circlet,combat boots)
100,(antitoxin,potion of cure moderate wounds_2)

# Jenera, chapter 1, F2 (14660)
660
100,COINS BY VALUE,3,600,0
40,masterwork dwarven war axe  100,dwarven war axe
100,(VILLAGER GARB,BOOTS)

# Jenera, chapter 2, F2 (14660)
6602
100,COINS BY VALUE,3,600,0
40,masterwork dwarven war axe  100,dwarven war axe
100,(VILLAGER GARB,generic helm,BOOTS)

# Jenera, chapter 3, F2 (14660)
6603
100,COINS BY VALUE,3,600,0
40,masterwork dwarven war axe  100,dwarven war axe
100,(VILLAGER GARB,generic helm,BOOTS)

# Jenera, chapter 4, F2 (14660)
6604
100,COINS BY VALUE,3,600,0
40,masterwork dwarven war axe  100,dwarven war axe
100,(VILLAGER GARB,generic helm,BOOTS)

# Cyndra, Cook, 0-level (14661)
661
100,COINS BY VALUE,3,600,0
100,dagger
100,(VILLAGER GARB,black leather boots)

# Salamin, battlefield, F3 (14662)
662
100,COINS BY VALUE,3,600,0
100,masterwork sickle
100,(FARMER GARB,SIMPLE BOOTS)

# Salamin, chapters 1, F3 (14662)
6621
100,COINS BY VALUE,3,600,0
100,masterwork sickle
100,(FARMER GARB,SIMPLE BOOTS)

# Salamin, chapters 2, F3 (14662)
6622
100,COINS BY VALUE,3,600,0
100,masterwork sickle
100,(FARMER GARB,SIMPLE BOOTS)

# Salamin, chapters 3, F3 (14662)
6623
100,COINS BY VALUE,3,600,0
100,masterwork sickle
100,(FARMER GARB,SIMPLE BOOTS)

# Salamin, chapters 4, F3 (14662)
6624
100,COINS BY VALUE,3,600,0
100,masterwork sickle
100,(FARMER GARB,SIMPLE BOOTS)

# Furt, Goblin Boss, R6 (14663)
# Burt, Furt's brother, R6 (14663)
# Gnome Assassin, B1/R5 (14072)
663
100,COINS BY VALUE,1,20000,0
100,WEAPON HALFLING LIGHT 0
100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 unseen
10,DEXTERITY 1

# Whiskers, Wererat, tavern, R5 (14664)
664
100,COINS BY VALUE,4,6000,0
100,GEMS,2,12,0
100,RAPIER 0
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 blue  100,blue cloak

# Whiskers, Wererat, east shorline, R5 (14664)
6641
100,COINS BY VALUE,4,6000,0
100,GEMS,2,12,0
100,RAPIER 0
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 blue  100,blue cloak

# Whiskers, Wererat, jail cell, R5 (14664)
6642
100,FARMER GARB

# Angus Lang, F5, Paladin's Cove shoreline (14665)
665
100,CGI,5,1,1
100,LONGSWORD 0
100,CHAIN SHIRT 0
100,buckler
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 red  100,red cloak
100,combat boots

# Angus Lang, F12, Smuggler Ship (14692)
6651
100,COINS BY VALUE,1,12000,0
100,GEMS,2,4,0
100,JEWELRY,1,6,0
100,LONGSWORD 2
100,CHAIN SHIRT 2
100,BUCKLER 0
100,RING OF PROTECTION 1
100,cloak of resistance +1 violet
100,DEXTERITY 1
100,(hoodless circlet,combat boots)

# Mr. Temm, 0-level (14666)
666
100,COINS BY VALUE,0,0,3700
100,(green noble garb,black leather boots)

# Cook, 0-level (14667)
667
100,COINS BY VALUE,0,0,1000
100,rusty dagger
100,(GARB SIMPLE,black leather boots)

# Maid, 0-level (14668)
668
100,COINS BY VALUE,0,0,1000
100,club
100,(GARB SIMPLE,black leather boots)

# The Collector, C9 (14669)
669
100,COINS BY VALUE,1,9000,0
100,FULL PLATE 1
100,BUCKLER 1
100,DAGGER 0
50,cloak of resistance +1 unseen
50,WISDOM 1
100,(red wizard robes,black leather boots)
100,(jeweled crown,scarab necklace,platinum ring,ruby ring)
100,potion of heal
100,shield of faith_3

# Tivanya, Elf Sorceress, W7 (14670)
670
100,COINS BY VALUE,0,0,2500
100,QUARTERSTAFF 1
75,AMULET 1
75,RING OF PROTECTION 1
75,CLOAK OF RESISTANCE 1
10,INTELLIGENCE
100,(green gypsy dress,hoodless circlet)
100,BOOTS
100,mage armor
100,shield spell

# Sylvvi, Sorceress, S3 (14671)
671
100,COINS BY VALUE,0,0,300
100,quarterstaff
100,(FARMER GARB,BOOTS)

# Todd Migbrudal, F2/R2 (14673)
673
100,COINS BY VALUE,0,0,13700
75,LEATHER ARMOR 0  100,LEATHER ARMOR 1
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 unseen
100,(GLOVES,black leather boots)
100,potion of cure serious wounds,0,0,2

# Ria Tiempa, Barmaid, R6/W1 (14674)
674
100,COINS BY VALUE,0,0,35000
100,ring of protection +1
100,ria tiempas key
100,(SKIRTLESS GARB,green bandana,black leather boots)
100,(oregano,pumpkin seeds)
100,mage armor
100,shield spell

# Tomeo, Servering Man, F8 (14675)
675
100,COINS BY VALUE,0,0,17000
100,dagger
100,STRENGTH 1
100,ioun stone deep red
100,(ochre villager garb,black leather boots)
100,mage armor
100,shield spell

# Trinity, Rogue aspect, R3 (14676)
676
100,shortsword
100,studded leather armor
100,black leather boots

# Old Man, F8 (14677)
677
100,FARMER GARB
100,old mans key

# Shekelesh, inn version, F5 (14678)
678
100,COINS BY VALUE,0,0,3800
100,masterwork scimitar
50,chain shirt  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,AMULET 1
75,white cloak  100,cloak of resistance +1 white
100,ioun stone deep red
100,(red robes,black leather boots)

# Shekelesh, ambush version, F5 (14678)
6781
100,COINS BY VALUE,0,0,3800
100,masterwork scimitar
50,chain shirt  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,AMULET 1
75,white cloak  100,cloak of resistance +1 white
100,ioun stone deep red
100,(red robes,black leather boots)

# Shady Character, ally, R5 (14679)
679
100,COINS BY VALUE,0,0,15000
100,masterwork shortsword
100,chain shirt
100,green cloak
100,black leather boots

# Shady Character, tavern, R5 (14679)
6791
100,COINS BY VALUE,0,0,15000
100,masterwork shortsword
100,chain shirt
100,green cloak
100,black leather boots

# Shady Character, tavern, R5 (14679)
6792
100,COINS BY VALUE,0,0,15000
100,masterwork shortsword
100,chain shirt
100,green cloak
100,black leather boots

# Shardana, C6 (14680)
680
100,COINS BY VALUE,3,6000,0
100,sickle +1
100,FULL PLATE 1
50,green cloak  100,cloak of resistance +1 green
100,(brown monk robes,plate boots)
100,potion of cure critical wounds
100,shield of faith_3

# Knight, F4 (14681)
681
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
75,LONGSWORD 0  100,LONGSWORD 1
75,FULL PLATE  100,FULL PLATE 1
100,large steel shield_2
10,RING OF PROTECTION 1
10,cloak of resistance +1 white  100,white cloak
100,(plate gloves,plate boots)
100,potion of cure serious wounds

# Troll Reanimator, C6 (14682)
682
100,COINS BY VALUE,3,6000,0
100,scythe +1
100,FULL PLATE 0
50,green cloak  100,cloak of resistance +1 green
100,potion of cure critical wounds
100,shield of faith_3

# Bugbear Deserter Leader, pool cave
6821
100,CGI,5,1,1
100,ranseur

# Bugbear Deserter Leader, arena cave
6822
100,CGI,5,1,1
100,ranseur

# Farlamond, R15 (14683)
683
100,COINS BY VALUE,0,0,15000
100,LEATHER ARMOR 1
100,black leather boots

# Farlamond, R15, with Frost Giants (14683)
6831
100,COINS BY VALUE,0,0,15000
100,masterwork dagger
100,chain shirt
100,mithral large shield +1
100,black leather boots
100,shield of faith_3

# Draskin Mancy, Director of Archaic Society, W13/R5 (14684)
684
100,COINS BY VALUE,3,6000,0
100,light hammer
100,RING OF PROTECTION 2
100,AMULET 2
100,cloak of resistance +3 unseen
100,INTELLIGENCE 2
100,(eyeglasses,scholars kit)
100,(red skirtless garb,black leather boots)
100,potion of heal
100,draskin mancys key
100,mage armor
100,shield spell

# Roso, F10/B1/R5 (14685)   TBD, used Bushberry as template
685
100,COINS BY VALUE,3,6000,0
100,SHORTSWORD 2
100,LEATHER ARMOR 3
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +1 red
100,STRENGTH 1
100,(hoodless circlet,brown leather boots,black leather gloves)
100,potion of heal

# Roso, F10/B1/R5, with drow (14685)
6851
100,COINS BY VALUE,3,6000,0
100,unholy shortsword +4
100,mithral chain shirt +3 
100,cloak of resistance +3 red
100,(hoodless circlet,brown leather boots,black leather gloves)
100,potion of heal
100,shield spell
100,shield of faith_3

# Asher, Chief Historian, W5 (14686)
686
100,COINS BY VALUE,3,6000,0
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 unseen
100,(brown robes,BOOTS)
100,scroll of magic missile,0,0,2
100,scroll of scorching ray,0,0,2
100,(scroll of lightning bolt,scroll of fireball)
100,mage armor

# Asher, Chief Historian, W5 (14686)
6861
100,COINS BY VALUE,3,6000,0
100,masterwork quarterstaff
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 unseen
100,(brown robes,BOOTS)
100,(scroll of deep slumber,scroll of lightning bolt,scroll of fireball)
100,scroll of magic missile,0,0,1
100,mage armor

# Zolroth, not used (14687)
687
100,COINS BY VALUE,0,0,7
100,scythe
100,(blue robes,black cloak,hoodless circlet,black leather boots)
100,shield of faith_4

# Zolroth, crossroads exterior (14687)
6871
100,COINS BY VALUE,0,0,7
100,scythe
100,(blue robes,black cloak,hoodless circlet,black leather boots)
100,shield of faith_4

# Zolroth, crypt exterior (14687)
6872
100,COINS BY VALUE,0,0,7
100,scythe
100,(blue robes,black cloak,hoodless circlet,black leather boots)
100,shield of faith_4

# Zolroth, luxwood (14687)
6873
100,COINS BY VALUE,0,0,7
100,scythe
100,(blue robes,black cloak,hoodless circlet,black leather boots)
100,shield of faith_4

# Zolroth, crypt interior (14687)
6874
100,COINS BY VALUE,0,0,7
100,scythe
100,(blue robes,black cloak,hoodless circlet,black leather boots)
100,shield of faith_4
100,resist energy fire

# Zolroth, crossroads chapter 2 exterior, mancy quest (14687)
6875
100,COINS BY VALUE,0,0,7
100,scythe +3
100,chain shirt +3
100,cloak of resistance +3 white
100,(blue robes,hoodless circlet,black leather boots)
100,shield of faith_4
100,resist energy fire

# Zolroth, Archaic Society, mancy quest (14687)
6876
100,COINS BY VALUE,0,0,7
100,scythe +3
100,chain shirt +3
100,cloak of resistance +3 white
100,(blue robes,hoodless circlet,black leather boots)
100,shield of faith_4
100,resist energy fire

# Tiamara (14688)
688
100,COINS BY VALUE,0,0,777
100,quarterstaff
100,(white evening dress, brown leather boots)

# Farmer, random farm hill giant attack (14689)
689
100,COINS BY VALUE,2,60,0
100,FARMER GARB
50,straw cap  100,coolie hat
100,SIMPLE BOOTS

# Gerjuli Burks, W7 (14690)
690
100,COINS BY VALUE,3,6000,0
100,RING OF PROTECTION 1
100,AMULET 1
100,cloak of resistance +2 unseen
100,(red noble garb,red bandana,combat boots)
100,potion of heal
100,mage armor

# Gelwea, W13 (14691)
691
100,longspear +3
100,potion of heal

# Nigel Weatherspoon, Galwea, R7 (14693)
693
100,COINS BY VALUE,3,600,0
100,boots of teleportation
100,(ochre villager garb,black leather boots)

# Toll Collector, F8/C1 (14694)
694
100,COINS BY VALUE,1,9000,0
50,FULL PLATE 0  100,FULL PLATE 1
100,scythe +1
50,RING OF PROTECTION 1
50,AMULET 1
25,cloak of resistance +1 unseen
10,STRENGTH 1
100,(brown robes,hoodless circlet,black leather gloves,black leather boots)
100,potion of cure serious wounds

# Kizi, Troll Chieftess, C7 (14695)
695
100,COINS BY VALUE,1,14000,0
100,hill giant club +1
100,troll bone armor
25,cloak of resistance +1 unseen
100,spider brooch
100,shield of faith_3

# Gila Lot-Loreth, C12 (14696)
696
100,COINS BY VALUE,3,6000,0
100,FULL PLATE 2
100,RING OF PROTECTION 2
100,cloak of resistance +2 black
100,WISDOM 2
100,(green robes,hoodless circlet,BOOTS)
100,potion of heal

# Gila Lot-Loreth, C12 (14696)
6961
100,COINS BY VALUE,3,6000,0
100,masterwork quarterstaff
100,FULL PLATE 2
100,cloak of resistance +2 red
100,WISDOM 2
100,(hoodless circlet,BOOTS)
100,potion of heal
100,shield of faith_4
100,resist energy fire

# Dwarf Leader, mithral map, F12 (14697)
697
100,COINS BY VALUE,1,12000,0
100,glaive +2
100,dwarven stone armor +2
100,BUCKLER 1
100,RING OF PROTECTION 2
100,cloak of resistance +1 unseen
100,STRENGTH 1
100,(plumed helm,gold plumed helm)
100,(hoodless circlet,black leather boots)

# Elf Leader, mithral map, Ranger 12 (14698)
698
100,COINS BY VALUE,1,12000,0
100,composite longbow str 16 +2
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
#100,masterwork longsword
100,elven chain +2
100,RING OF PROTECTION 2
100,cloak of resistance +1 green
100,DEXTERITY 1
100,(hoodless circlet,brown leather boots)
100,potion of barkskin +5
100,potion of heal

# Trenn Soldier (14700)
700
100,COINS BY VALUE,2,600,0
50,greatsword  75,greathammer  100,greataxe
100,(scale mail,black leather boots)
100,initiative stone

# Trenn Soldier, archer, 14,16,12,10,10,10 (14701)
7001
100,COINS BY VALUE,2,600,0
100,composite longbow str 14
100,quiver of arrows,0,0,3
100,(LEATHER ARMOR,buckler,brown leather boots,brown leather gloves)
100,orange cloak
100,club

# Trenn Soldier, light armored (14701)
7002
100,COINS BY VALUE,2,600,0
50,longsword  100,warhammer
100,(LEATHER ARMOR,buckler,brown leather boots,brown leather gloves)

# Trenn Soldier, sergeant 18,14,14,12,12,12 (14700)
7003
100,COINS BY VALUE,2,600,0
50,masterwork greatsword  75,masterwork greathammer  100,masterwork greataxe
100,chain shirt
100,(fur cloak,generic helm,black leather boots)

# Trenn Soldier, wounded on battlefield (14700)
7009
50,greatsword  75,greathammer  100,greataxe
100,(scale mail,black leather boots)

# Rhondi Soldier (14701)
701
100,COINS BY VALUE,2,600,0
50,longsword  100,warhammer
100,(chainmail,large steel shield_3,generic helm,black leather boots,chainmail gloves)
100,blue cloak
100,(club,hoodless circlet)
100,initiative stone

# Rhondi Soldier, archer, 12,18,12,10,10,10 (14701)
7011
100,COINS BY VALUE,2,600,0
100,LONGBOW
100,(brown leather armor,buckler,generic helm,black leather boots,black leather gloves)
100,black cloak
100,(club,hoodless circlet)

# Rhondi Soldier, light armor 14701)
7012
100,COINS BY VALUE,2,600,0
50,longsword  100,warhammer
100,(chain shirt,small wooden shield,black leather boots,black leather gloves)

# Rhondi Soldier, sergeant, 16,16,16,12,12,12 (14701)
7013
100,COINS BY VALUE,2,600,0
100,masterwork longsword
100,(breastplate,large steel shield_4,generic helm,plate boots,plate gloves)
100,light blue cloak
100,(club,hoodless circlet)

# Rhondi Soldier, bodyguard, 18,12,18,10,14,10 (14701)
7018
100,COINS BY VALUE,2,600,0
100,masterwork glaive
100,(banded mail,great helm,plate boots,plate gloves)
100,orange cloak
100,(club,hoodless circlet)

# Rhondi Soldier, wounded on battlefield (14701)
7019
50,longsword  100,warhammer
50,potion of cure light wounds
100,(chainmail,large steel shield_3,generic helm,black leather boots,chainmail gloves)
100,blue cloak

# Abjurer, W1/Rgr1 (14702)
702
100,CGI,2,3,1
80,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0
80,WEAPON MARTIAL LIGHT  100,WEAPON MARTIAL LIGHT 0
100,chain shirt
100,orange cloak
25,LIGHT HELM  75,hoodless circlet
100,BOOTS AND GLOVES

# Abjurer, W9/Rgr1 (14762)
762
100,masterwork shortsword
100,cloak of resistance +2 orange
100,monk wrap
25,LIGHT HELM  75,hoodless circlet
100,BOOTS AND GLOVES
100,mage armor
100,shield spell
100,barkskin_3
	
# Conjurer, W1/R1 (14703)
703
100,CGI,2,3,1
30,WEAPON ROGUE  50,WEAPON ROGUE 0  80,spear  100,masterwork spear
40,BRACERS 0
100,blue robes
35,WIZARD HAT NEUTRAL  70,hoodless circlet  
100,BOOTS

# Conjurer, W9/R1 (14763)
763
100,BOOTS
100,mage armor
100,shield spell
100,barkskin_3

# Diviner, W1/F1 (14704)
704
100,CGI,2,3,1
60,WEAPON MARTIAL TWO-HANDED  100,WEAPON MARTIAL TWO-HANDED 0
100,scale mail
100,yellow robes
25,LIGHT HELM  75,hoodless circlet
100,BOOTS AND GLOVES

# Diviner, W9/F1 (14764)
764
100,greathammer +2
100,FULL PLATE 2
100,yellow robes
100,BOOTS AND GLOVES
100,shield spell
100,barkskin_3
100,shield of faith_2

# Enchanter, W1/Bard1 (14705)
705
100,CGI,2,3,1
60,longspear  100,masterwork longspear
40,BRACERS 0
100,light blue robes
25,WIZARD HAT NEUTRAL  50,jaunty hat  75,hoodless circlet  
100,BOOTS

# Enchanter, W9/Bard1 (14765)
765
100,masterwork longspear
100,ioun stone pink and green
100,cloak of resistance +1 light blue
25,WIZARD HAT NEUTRAL  50,jaunty hat  75,hoodless circlet  
100,BOOTS
100,mage armor
100,shield spell
100,barkskin_3

# Evoker, W1/D1 (14706)
706
100,CGI,2,3,1
100,sorcerers note
100,quarterstaff
40,BRACERS 0
100,red robes
25,WIZARD HAT NEUTRAL  75,druidic helm  
100,BOOTS

# Evoker, W9/D1 (14766)
766
100,masterwork quarterstaff
100,RING OF PROTECTION 1
100,ring of fire resistance major
100,(eyeglasses,scholars kit)
100,red robes
25,WIZARD HAT NEUTRAL  75,druidic helm  
100,BOOTS
100,mage armor
100,shield spell
100,barkskin_3

# Illusionist, W1/M1 (14707)
707
100,CGI,2,3,1
40,BRACERS 0
100,fancy ivory robes
50,hoodless circlet  
100,BOOTS

# Illusionist, W7/M6 (14767)
767
100,RING OF PROTECTION 2
100,(fancy ivory robes,hoodless circlet,black leather gloves)
100,BOOTS
100,scroll of raise dead
100,mage armor
100,shield spell
100,barkskin_3

# Illusionist, W7/M8 (14767)
7671
100,masterwork quarterstaff
100,(fancy ivory robes,black leather gloves)
100,BOOTS

# Necromancer, W1/C1 (14708)
708
100,CGI,2,3,1
60,longspear  100,masterwork longspear
40,BRACERS 0
100,little black dress
100,BOOTS

# Necromancer, W9/C1 (14768)
768
100,cloak of resistance +3 black
100,masterwork dagger
100,little black dress
100,BOOTS
100,mage armor
100,shield spell
100,barkskin_3
100,shield of faith_2

# Transmuter, W1/B1 (14709)
709
100,CGI,2,3,1
60,WEAPON MARTIAL TWO-HANDED  100,WEAPON MARTIAL TWO-HANDED 0
50,barbarian armor  100,hide armor
100,green cloak
50,barbarian helm  100,hoodless circlet
100,BOOTS AND GLOVES

# Transmuter, W11/Barb1 (14709)
769
100,WEAPON MARTIAL TWO-HANDED 0
100,cloak of resistance +3 black
100,green robes
50,barbarian helm  100,hoodless circlet
100,BOOTS AND GLOVES
100,mage armor
100,shield spell
100,barkskin_3

# Goblin Wizard, W3 (14710, 710)
710
100,CGI,3,2,1
60,small wooden elvish shield
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(scroll of scorching ray,scroll of sleep)
100,scroll of magic missile,0,0,2
100,mage armor

# Bugbear Witch, W3 (14711, 710)
711
100,CGI,3,2,1
60,wooden elvish shield
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(scroll of scorching ray,scroll of sleep)
100,scroll of magic missile,0,0,2
100,mage armor

# Gnoll Shaman, D3 (14712)
712
100,CGI,3,2,1
33,club   66,shortspear   100,sickle
40,hide armor   100,HIDE ARMOR 0
40,DRUID SHIELD   100,DRUID SHIELD 0
5,RING OR AMULET 1
5,CLOAK OF RESISTANCE 1
100,(scroll of chill metal,scroll of summon natures ally ii)

# Zombie Adventurer (14713)
713
20,CGI,1,9,1
50,scale mail  100,LEATHER ARMOR
20,buckler  40,small wooden shield  60,LARGE SHIELD

# Shadow Wizard, W9 (14714)
714
100,mage armor

# Shadow Wizard, Temple Level 1, W9 (14714)
7141
100,ioun stone scarlet and blue
100,mage armor
100,shield spell
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Bugbear Witch, W9 (14715, 710)
715
100,COINS BY VALUE,3,600,0
100,masterwork club
50,RING OF PROTECTION 1
50,cloak of resistance +1 unseen
20,INTELLIGENCE 1
20,ring of fire resistance minor
100,mage armor
100,shield spell
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Wizard 1, W1 (14716)
716
100,CGI,1,3,1
20,wooden elvish shield  
100,(green robes,BOOTS)
100,(scroll of burning hands,scroll of magic missile)
100,mage armor

# Wizard 2, W2 (14717)
717
100,CGI,2,3,1
40,wooden elvish shield  
100,(green robes,white cloak,BOOTS)
100,(scroll of burning hands,scroll of magic missile,scroll of sleep)
100,mage armor

# Wizard 3, W3 (14718)
718
100,CGI,3,3,1
60,wooden elvish shield
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,(brown robes,BOOTS)
100,(scroll of burning hands,scroll of scorching ray)
100,scroll of magic missile,0,0,2
100,mage armor

# Wizard 4, W4 (14719)
719
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
80,wooden elvish shield
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 orange
100,(brown robes,orange cloak,BOOTS)
100,(scroll of blindness/deafness,scroll of burning hands,scroll of scorching ray)
100,scroll of magic missile,0,0,2
100,mage armor
100,shield spell

# Patrolman, F1/R1 (14720)
720
100,COINS BY VALUE,2,600,0
100,(WEAPON ONE-HANDED,WEAPON LIGHT)
100,quiver of arrows,0,0,3
60,LEATHER ARMOR  90,CHAIN SHIRT  100,ARMOR LIGHT SPECIAL
100,buckler
50,generic helm  100,hoodless circlet
100,patrol cloak
50,(black leather gloves,black leather boots)  100,(brown leather gloves,brown leather boots)

# Patrolman, small, F1/R1 (14720)
7201
100,COINS BY VALUE,2,600,0
100,(WEAPON MARTIAL LIGHT,WEAPON HALFLING LIGHT)
100,quiver of arrows,0,0,3
60,LEATHER ARMOR  90,CHAIN SHIRT  100,ARMOR LIGHT SPECIAL
100,buckler
50,generic helm  100,hoodless circlet
100,patrol cloak
50,(black leather gloves,black leather boots)  100,(brown leather gloves,brown leather boots)

# Patrolman, Crossroads guide, F1/R1 (14720)
7202
100,scythe
100,(brown robes,black leather gloves,black leather boots)

# Patrol Sergeant, F4/R1 (14721)
721
100,COINS BY VALUE,2,600,0
75,WEAPON MARTIAL ONE-HANDED 0  100,WEAPON MARTIAL ONE-HANDED 1
100,WEAPON MARTIAL LIGHT
100,quiver of arrows,0,0,3
75,CHAIN SHIRT 0  100,CHAIN SHIRT 1
80,BUCKLER 0  100,buckler
10,RING OR AMULET 1
8,cloak of resistance +1 red  10,cloak of resistance +2 red
100,(patrol sergeant cloak,plumed helm,black leather gloves,black leather boots)

# Patrol Sergeant, small, F4/R1 (14721)
7211
100,COINS BY VALUE,2,600,0
75,WEAPON MARTIAL LIGHT 0  100,WEAPON MARTIAL LIGHT 1
100,WEAPON HALFLING LIGHT
100,quiver of arrows,0,0,3
75,CHAIN SHIRT 0  100,CHAIN SHIRT 1
80,BUCKLER 0  100,buckler
10,RING OR AMULET 1
8,cloak of resistance +1 red  10,cloak of resistance +2 red
100,(patrol sergeant cloak,plumed helm,black leather gloves,black leather boots)

# Watchman, F2 (14722)
722
100,COINS BY VALUE,2,600,0
50,LONGBOW  100,COMPOSITE LONGBOW STR14
100,WEAPON MARTIAL LIGHT
50,buckler
100,(ARMOR LIGHT,watch cloak,black leather gloves,black leather boots)

# Watchman, small, F2 (14722)
7221
100,COINS BY VALUE,2,600,0
50,shortbow  100,composite shortbow str 12
100,(quiver of arrows,quiver of arrows)
100,WEAPON MARTIAL LIGHT
50,buckler
100,(ARMOR LIGHT,watch cloak,black leather gloves,black leather boots)

# Watch Sergeant, F4 (14723)
723
100,COINS BY VALUE,2,600,0
75,COMPOSITE LONGBOW STR16 0  100,COMPOSITE LONGBOW STR16 1
100,WEAPON MARTIAL LIGHT
75,CHAIN SHIRT 0  100,CHAIN SHIRT 1
80,BUCKLER 0  100,buckler
10,RING OR AMULET 1
10,CLOAK OF RESISTANCE 1
100,(watch sergeant cloak,brown leather gloves,brown leather boots)

# Watch Sergeant, small, F4 (14723)
7231
100,COINS BY VALUE,2,600,0
75,masterwork composite shortbow str 14  100,composite shortbow str 14 +1
100,(quiver of arrows,quiver of arrows)
100,WEAPON MARTIAL LIGHT
75,CHAIN SHIRT 0  100,CHAIN SHIRT 1
80,BUCKLER 0  100,buckler
10,RING OR AMULET 1
10,CLOAK OF RESISTANCE 1
100,(watch sergeant cloak,brown leather gloves,brown leather boots)

# Ebon Sentry, F4 (14724)
724
100,COINS BY VALUE,2,600,0
100,masterwork ranseur
100,(black full plate,plate boots,plate gloves,great helm)
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 unseen

# Dockhand (14725)
725
100,COINS BY VALUE,2,600,0
100,pouch of bullets,0,0,3
5,straw cap  10,coolie hat  15,blue bandana  20,red bandana
50,FARMER GARB  100,SKIRTLESS GARB
50,black leather boots  100,brown leather boots

# Fisherman (14726)
726
100,COINS BY VALUE,2,600,0
100,pouch of bullets,0,0,3
100,FARMER GARB
50,black leather boots  100,brown leather boots

# Seaman (Rhondi), F2 (14727)
727
100,COINS BY VALUE,2,600,0
20,trident  40,rapier  60,longsword  73,TRIDENT 0  86,RAPIER 0  100,LONGSWORD 0
60,ARMOR LIGHT  100,ARMOR LIGHT 0
100,blue bandana
100,black leather boots

# Seaman (Trenn), B2 (14728)
728
100,COINS BY VALUE,2,600,0
20,trident  40,battleaxe  60,warhammer  73,TRIDENT 0  86,BATTLEAXE 0  100,WARHAMMER 0
60,ARMOR LIGHT  100,ARMOR LIGHT 0
100,simple circlet
100,black leather boots

# Seaman (Southlands), F3 (14729)
729
100,COINS BY VALUE,2,600,0
40,composite shortbow str 14  100,masterwork composite shortbow str 14
100,quiver of arrows,0,0,3
100,WEAPON LIGHT
40,studded leather armor  100,STUDDED LEATHER 0
100,(red padded gloves,black leather boots)

# Acolyte, C2 (14730)
730
100,COINS BY VALUE,2,600,0
60,FULL PLATE  100,FULL PLATE 0
50,hoodless circlet
100,(brown robes,white cloak,black leather boots)

# Traveller (14731)
731
100,COINS BY VALUE,2,600,0
33,FARMER GARB  66,VILLAGER GARB  100,ARMOR LIGHT
20,green robes  40,brown robes  50,ROBES  60,MONK ROBES
20,black cloak  40,green cloak  60,blue cloak
100,SIMPLE BOOTS

# Ebon Sentry, F8 (14732)
732
100,COINS BY VALUE,2,600,0
100,GREATSWORD 1
100,FULL PLATE 1
100,RING OF PROTECTION 1
100,AMULET 1
25,STRENGTH 1
100,cloak of resistance +1 unseen
100,(plate boots,plate gloves)
25,potion of stoneskin  50,potion of mirror image  75,potion of barkskin +4  100,potion of shield
100,potion of cure serious wounds

# Ebon Sentry, Gem Thief in Swamp, F8 (14784)
7321
100,COINS BY VALUE,2,600,0
100,kelvins gems
100,GREATSWORD 1
100,FULL PLATE 1
100,RING OF PROTECTION 1
100,AMULET 1
25,STRENGTH 1
100,cloak of resistance +1 unseen
100,(plate boots,plate gloves)
25,potion of stoneskin  50,potion of mirror image  75,potion of barkskin +4  100,potion of shield

# Farmhand (14733)
733
100,COINS BY VALUE,2,600,0
100,pouch of bullets,0,0,3
100,sickle
10,straw cap  20,coolie hat  30,blue bandana  40,red bandana  50,green bandana
50,FARMER GARB  100,SKIRTLESS GARB
50,black leather boots  100,brown leather boots

# Guard, F11 (14734)
734
100,COINS BY VALUE,2,600,0
50,LONGSWORD 0  100,LONGSPEAR 0
25,ARMOR LIGHT 2  50,CHAIN SHIRT 2  100,ARMOR MEDIUM 2
100,SHIELD NO TOWER
50,LIGHT HELM
50,(black leather gloves,black leather boots)  100,(brown leather gloves,brown leather boots)

# Musician (14736)
736
100,COINS BY VALUE,2,600,0
75,quarterstaff  100,dagger
100,MUSICAL INSTRUMENT
100,pouch of bullets,0,0,3
60,VILLAGER GARB  80,NOBLE GARB  100,FANCY GARB
50,HAT
100,BOOTS

# Dancer, elf (14737)
737
100,COINS BY VALUE,2,600,0
100,little red dress

# Dancer, human (14737)
7371
100,COINS BY VALUE,2,600,0
100,spiked chain
100,(chainmail bikini,black leather boots,black leather gloves)

# Bodyguard, F7/C1 (14738)
738
100,COINS BY VALUE,1,8000,0
100,FULL PLATE 0
50,GLAIVE 0  100,RANSEUR 0
25,CLOAK OF RESISTANCE 1
10,STRENGTH 1
100,(hoodless circlet,black cloak,plate gloves,plate boots)
100,potion of cure serious wounds
100,shield of faith_2

# Bodyguard, Flute Leader, F7/C1 (14738)
# inventory switched to 7381 in his script, Not set in his MOB
7381
100,COINS BY VALUE,1,8000,0
100,FULL PLATE 0
50,GLAIVE 0  100,RANSEUR 0
100,plate boots
100,shield of faith_2

# Bodyguard, F7/C1 (14739)
739
100,COINS BY VALUE,1,8000,0
100,FULL PLATE 0
100,LARGE SHIELD 0
50,LONGSWORD 0  100,WARHAMMER 0
25,CLOAK OF RESISTANCE 1
10,STRENGTH 1
100,(hoodless circlet,plate gloves,plate boots)
100,potion of cure serious wounds
100,shield of faith_2

# Bodyguard, F7/C1 (14740)
740
100,COINS BY VALUE,1,8000,0
100,CHAIN SHIRT 0
100,COMPOSITE LONGBOW STR14 0
25,CLOAK OF RESISTANCE 1
10,DEXTERITY 1
100,(hoodless circlet,BOOTS)
100,potion of cure serious wounds
100,shield of faith_2

# Bodyguard, Flute, F7/C1 (14740)
7401
100,COINS BY VALUE,1,8000,0
100,CHAIN SHIRT 1
100,LONGSPEAR 0
50,RING OF PROTECTION 1
50,AMULET 1
50,cloak of resistance +1 white
10,STRENGTH 1
100,(hoodless circlet,BOOTS)
100,potion of cure serious wounds
100,shield of faith_2

# Wizard 5, W5 (14741)
741
20,CGI,5,3,1  100,COINS BY VALUE,1,5000,0
100,dagger
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 orange
100,(brown robes,orange cloak,BOOTS)
100,scroll of magic missile,0,0,2
100,scroll of scorching ray,0,0,2
100,(scroll of lightning bolt,scroll of fireball)
100,mage armor
100,shield spell
25,resist energy cold  50,resist energy electricity  100,resist energy fire

# Laborer (14742)
742
100,COINS BY VALUE,2,600,0
100,pouch of bullets,0,0,3
25,light hammer  50,heavy pick
10,straw cap  20,coolie hat  30,blue bandana  40,red bandana  50,green bandana
50,FARMER GARB  100,SKIRTLESS GARB
50,black leather boots  100,brown leather boots

# Laborer, hammer for animated waypoint (14742)
7421
100,COINS BY VALUE,2,600,0
100,pouch of bullets,0,0,3
100,light hammer
10,straw cap  20,coolie hat  30,blue bandana  40,red bandana  50,green bandana
50,FARMER GARB  100,SKIRTLESS GARB
50,black leather boots  100,brown leather boots

# Dwarf (14743)
744
100,COINS BY VALUE,2,600,0
20,dwarven war axe  40,scimitar  60,shortsword  80,SHORTBOW  100,warhammer
20,WEAPON 0
50,ARMOR  100,GARB
100,BOOTS

# Inn Patron (14745)
745
100,COINS BY VALUE,2,2000,0
50,LEATHER ARMOR  100,CHAIN SHIRT
25,MONK ROBES
25,leather helm
100,BOOTS

# Seaman (Rivermist Inn) (14746)
746
100,COINS BY VALUE,1,20,0
100,dagger
100,cloth armor
100,(black leather boots)

# Seaman (Rivermist Inn) (14746)
7461
100,COINS BY VALUE,1,20,0
100,dagger
100,cloth armor
100,black leather boots

# Serving Girl, half-elf (14747)
747
100,COINS BY VALUE,2,600,0
100,(FARMER GARB,BOOTS)

# Serving Girl, human (14747)
7471
100,COINS BY VALUE,2,600,0
100,dagger
50,SKIRTLESS GARB  100,VILLAGER GARB
100,BOOTS

# Attendant, works in merchant's room (14748)
748
100,COINS BY VALUE,2,600,0
100,masterwork dagger
25,RING OF PROTECTION 1
25,AMULET 1
25,cloak of resistance +1 unseen
100,(scroll of deep slumber,scroll of slow)
100,(VILLAGER GARB,BOOTS)
100,mage armor
100,shield spell

# Attendant, works in far room (14748)
7481
100,COINS BY VALUE,2,600,0
100,HAT
100,(VILLAGER GARB,BOOTS)

# Dead Human Fighter
749
100,COINS BY VALUE,0,0,37
100,broken longsword
100,(black leather gloves,black leather boots)

# Dead Human Female Wizard
750
100,WAND ARCANE MINOR
100,longsword
100,(WIZARD CLOTHING,WIZARD HAT,black leather boots)

# Dead Dwarf Fighter
751
100,COINS BY VALUE,0,0,500
100,warhammer
100,(great helm,plate boots)

# Dead Guard
752
100,(generic helm,ragged clothes,brown leather boots)

# Dead Guard
7521
100,dagger
100,(ragged clothes,brown leather boots)
100,ornate leather belt

# Dead Guard, mithral impact map
7522
25,COINS BY VALUE,2,20,0
10,WONDROUS ITEM
50,WEAPON RUSTY
50,chainmail  100,ragged clothes
50,BOOTS

# Bones, Fighter
753
100,CGI,2,6,1
100,battleaxe
100,(breastplate,generic helm,black leather boots)

# Bones, Barbarian
7531
100,CGI,2,6,1
100,longsword
100,(hide armor,simple circlet,brown leather boots)

# Bones, Halfling
7532
100,CGI,2,3,1
100,throwing dagger,2,4,1
100,(LEATHER ARMOR,brown leather boots)
100,masterwork thieves tools

# Bones, Wizard
7533
100,CGI,2,3,1
100,quarterstaff
100,(black wizard robes,black wizard hat,black leather boots)
100,SCROLL ARCANE MINOR,1,3,1

# Bones, Druid, Garden room near pool
7534
100,quarterstaff
100,hide armor
100,(green robes,brown leather boots)
100,SCROLL DIVINE MINOR,1,3,1

# Bones, Druid, Garden room near back
7535
100,(scimitar,large wooden shield)
100,(brown robes,black leather boots)
100,(MAGIC MUSHROOMS,potion of restoration)

# Researcher, W3 (14754)
754
100,COINS BY VALUE,3,600,0
100,(ochre skirtless garb,BOOTS)
50,GLOVES
100,(scroll of burning hands,scroll of magic missile,scroll of magic missile,scroll of scorching ray)

# Captive (14755)
755
50,COINS BY VALUE,2,6,0
50,FARMER GARB  100,ragged clothes
25,SIMPLE BOOTS

# Captive, second group on pirate island, hezrou area (14755)
7551
50,COINS BY VALUE,2,6,0
50,FARMER GARB  100,ragged clothes
100,HAT

# Pilgrim (14757)
757
100,COINS BY VALUE,2,600,0
100,FANCY GARB

# Pilgrim (14757)
757
100,COINS BY VALUE,2,600,0
100,FANCY GARB

# Druid 1, D1 (14758)
758
100,CGI,1,3,1
100,LEATHER ARMOR
100,large wooden shield_2
100,(brown robes,BOOTS)
100,(scroll of calm animals,scroll of shillelagh)
100,barkskin_2

# Druid 3, D3 (14759)
759
20,CGI,3,3,1  100,COINS BY VALUE,1,3000,0
100,hide armor
100,large wooden shield_2
10,RING OF PROTECTION 1
5,cloak of resistance +1 green  100,green cloak
100,(brown robes,BOOTS)
100,(scroll of calm animals,scroll of chill metal)
100,barkskin_2

# Druid 5, D5 (14760)
760
20,CGI,5,1,1  100,COINS BY VALUE,1,5000,0
100,leather scale armor
100,large wooden shield_2
25,RING OF PROTECTION 1
25,cloak of resistance +1 white  100,white cloak
100,(brown robes,BOOTS)
100,(scroll of call lightning,scroll of summon natures ally iii)
100,barkskin_2

# Druid 7, D7 (14761)
761
20,CGI,7,1,1  100,COINS BY VALUE,1,7000,0
100,leather scale armor
100,wooden shield +1
50,RING OF PROTECTION 1
50,cloak of resistance +1 white  100,white cloak
100,(brown robes,BOOTS)
100,(scroll of cure serious wounds,scroll of dispel magic,scroll of flame strike)
100,potion of heal,0,0,3
100,barkskin_3

# Druid 7, Luxwood Poachers, D7 (14761)
7611
20,CGI,7,1,1  100,COINS BY VALUE,1,7000,0
100,leather scale armor
100,wooden shield +1
100,ring of fire resistance major
75,cloak of resistance +1 white  100,white cloak
100,(brown robes,BOOTS)
100,(scroll of cure serious wounds,scroll of dispel magic,scroll of flame strike)
100,barkskin_3

# Druid 9, D9 (14791)
791
20,CGI,9,1,1  100,COINS BY VALUE,1,9000,0
100,leather scale armor
100,wooden shield +2
100,RING OF PROTECTION 1
100,cloak of resistance +1 white
100,(brown robes,BOOTS)
100,(scroll of cure serious wounds,scroll of dispel magic,scroll of flame strike)
100,barkskin_3

# Pirate Bruiser, B1/F8/R2 (14770)
770
20,CGI,11,3,1  100,COINS BY VALUE,1,11000,0
50,GREATSWORD 1  100,spiked chain +1
50,BREASTPLATE 0  100,BREASTPLATE 1
100,buckler
50,RING OF PROTECTION 1
50,cloak of resistance +1 fur  100,fur cloak
25,STRENGTH 1
100,(hoodless circlet,brown leather gloves,brown leather boots)

# Pirate Swashbuckler, B2/F2/R7 (14771)
771
20,CGI,11,3,1  100,COINS BY VALUE,1,11000,0
20,kukri +1  60,RAPIER 1  100,SHORTSWORD 1
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
50,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
25,DEXTERITY 1
100,(black long coat,red bandana,black leather gloves,black leather boots)

# Pirate Skirmisher, B3/R3 (14772)
772
20,CGI,6,1,1  100,COINS BY VALUE,1,5000,0
100,WEAPON LIGHT 0
100,STUDDED LEATHER 0
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1
10,DEXTERITY 1
100,BOOTS

# Pirate Bowman, F6 (14773)
773
20,CGI,6,1,1  100,COINS BY VALUE,1,5000,0
10,LIGHT CROSSBOW 0  70,COMPOSITE LONGBOW STR14 0  100,COMPOSITE SHORTBOW STR14 0
100,cutlass
100,STUDDED LEATHER 0
20,RING OF PROTECTION 1
20,CLOAK OF RESISTANCE 1
10,DEXTERITY 1
100,(black leather gloves,black leather boots)

# Slave Girl, red hood portrait (14774)
774
100,COINS BY VALUE,3,6,6
100,red cloak

# Slave Girl, brown hair portrait (14774)
7741
100,COINS BY VALUE,3,6,6
100,brown farmer garb

# Concubine, Pirate Cave Bar, hair wisp in face portrait (14775)
775
100,COINS BY VALUE,3,600,0
50,GEMS BY VALUE,6,60,0
25,RING OF PROTECTION 1
100,(red gypsy dress,BOOTS)
100,(scroll of deep slumber,scroll of slow)
100,mage armor
100,shield spell

# Concubine, Pirate Cave middle room, hair pulled back portrait (14775)
7751
100,COINS BY VALUE,3,600,0
50,GEMS BY VALUE,6,60,0
25,RING OF PROTECTION 1
100,(green noble garb,BOOTS)
100,concubines key
100,scroll of dispel magic,0,0,2
100,mage armor
100,shield spell

# Concubine, Pirate Cave middle room, hair wisp in face portrait (14775)
7752
100,COINS BY VALUE,3,600,0
50,GEMS BY VALUE,6,60,0
25,RING OF PROTECTION 1
100,(red gypsy dress,BOOTS)
100,(scroll of deep slumber,scroll of slow)
100,mage armor
100,shield spell

# Concubine, with drow (14775)
7753
100,COINS BY VALUE,1,16000,0
100,dagger
100,(drow mask,violet robes,black leather boots,black leather gloves)
100,scroll of dispel magic,0,0,2
100,mage armor
100,shield spell
100,resist energy fire
50,resist energy cold  100,resist energy electricity

# Luxan Sharpshooter, F10/R2 (14776)
776
100,COINS BY VALUE,1,12000,0
100,COMPOSITE LONGBOW STR14 2
100,CHAIN SHIRT 0
100,BUCKLER 0
50,RING OF PROTECTION 1
50,cloak of resistance +1 green
25,DEXTERITY 1
100,bracers of archery lesser
100,BOOTS
100,potion of cure critical wounds

# Hill Giant, smuggler's ship, B1/F6 (14777)
777
100,CGI,13,3,1
100,hill giant spiked chain +1 shock
100,studded leather armor
100,potion of cure critical wounds

# Smuggler, F6 (14778)
778
20,CGI,6,1,1  100,COINS BY VALUE,1,5000,0
50,masterwork glaive  100,masterwork ranseur
100,CHAIN SHIRT 0
20,RING OF PROTECTION 1
20,cloak of resistance +1 blue  100,blue cloak
10,STRENGTH 1
100,(black monk robes,red bandana,black leather boots)

# Poacher, F11 (14780)
780
100,COINS BY VALUE,3,6000,0
100,masterwork greataxe
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,CLOAK OF RESISTANCE 1
25,gauntlets of ogre power
25,brown robes  50,brown monk robes  75,black cloak  100,green cloak
100,BOOTS

# Poacher, near unicorn, F11 (14780)
7801
100,COINS BY VALUE,3,6000,0
100,masterwork greataxe
100,unicorn horn
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,CLOAK OF RESISTANCE 1
25,gauntlets of ogre power
25,brown robes  50,brown monk robes  75,black cloak  100,green cloak
100,BOOTS

# Poacher, near pixies, F11 (14780)
7802
100,COINS BY VALUE,3,6000,0
100,pixie wings_2
100,masterwork greataxe
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,CLOAK OF RESISTANCE 1
25,gauntlets of ogre power
25,brown robes  50,brown monk robes  75,black cloak  100,green cloak
100,BOOTS

# Poacher, with giants, F11 (14780)
7803
100,COINS BY VALUE,3,6000,0
100,centaur hoof
100,masterwork greataxe
50,CHAIN SHIRT 0  100,CHAIN SHIRT 1
25,RING OF PROTECTION 1
25,CLOAK OF RESISTANCE 1
25,gauntlets of ogre power
25,brown robes  50,brown monk robes  75,black cloak  100,green cloak
100,BOOTS

# Poacher Bowman, F11 (14781)
781
100,COINS BY VALUE,3,6000,0
100,COMPOSITE LONGBOW STR16 0
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
100,chain shirt
50,BUCKLER X  100,BUCKLER 1
25,RING OF PROTECTION 1
25,CLOAK OF RESISTANCE 1
25,DEXTERITY 1
100,(brown monk robes,leather helm,brown leather boots)
100,potion of cure critical wounds,0,0,1

# Mercenary, Crossroads (14782)
782
100,COINS BY VALUE,2,600,0
100,WEAPON MARTIAL LIGHT
100,ARMOR LIGHT
25,black cloak  50,white cloak
100,(generic helm,black leather boots,black leather gloves)

# Vendetta Councilor, W5/F6 (14785)
785
100,COINS BY VALUE,2,600,0
100,cloak of resistance +1 red
100,(black robes,red cloak,BOOTS)
100,(scroll of deep slumber,scroll of lightning bolt,scroll of fireball)
100,scroll of magic missile,0,0,1
100,mage armor
100,shield spell

# Gnome Cabalist, C9 (14786)
# Appears in Guildhall after Vendetta takes power.
# Also appears as a random in Temple.
786
100,COINS BY VALUE,3,6000,0
100,masterwork light mace
100,FULL PLATE 1
100,SMALL SHIELD
50,amulet of wisdom +2
50,black robes  100,red robes
100,shield of faith_3
100,resist energy fire
100,resist energy acid

# Dwarven Warrior, F8 (14787)
787
100,COINS BY VALUE,2,6000,0
50,greathammer +1  100,WARHAMMER 1
100,TOWER SHIELD
100,FULL PLATE 1
100,AMULET 1
25,STRENGTH 1
100,(generic helm,plate boots,plate gloves)

# Elven Warrior, F8 (14787)
788
100,COINS BY VALUE,3,6000,0
100,composite longbow str 14 +1
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
100,masterwork longsword
100,elven chain +1
100,AMULET 1
100,brown leather boots
25,potion of stoneskin  50,potion of mirror image  75,potion of barkskin +3  100,potion of shield
100,potion of cure serious wounds

# Goon leader, F4 (14789)
789
100,COINS BY VALUE,2,60,0
100,masterwork battleaxe
100,(LEATHER ARMOR,buckler)
100,(red bandana,BOOTS)

# Goon, R3 (14790)
790
100,COINS BY VALUE,2,6,0
100,cutlass
100,LEATHER ARMOR
100,(BANDANA,BOOTS)

# Wizard 7, W7 (14792)
792
20,CGI,7,3,1  100,COINS BY VALUE,1,7000,0
100,masterwork dagger
50,RING OF PROTECTION 1
50,cloak of resistance +1 unseen
50,ioun stone pink
50,ioun stone scarlet and blue
100,(black robes,BOOTS)
100,mage armor
100,shield spell
100,barkskin_2
100,resist energy fire
50,resist energy cold  100,resist energy electricity
100,scroll of magic missile,0,0,2
100,scroll of scorching ray,0,0,2
100,(scroll of lightning bolt,scroll of fireball)
100,(scroll of bestow curse,scroll of confusion)

# Wizard 9, W9 (14793)
793
20,CGI,9,3,1  100,COINS BY VALUE,1,9000,0
100,masterwork dagger
100,RING OF PROTECTION 1
100,cloak of resistance +1 unseen
100,ioun stone pink
100,ioun stone scarlet and blue
100,(black robes,BOOTS)
100,mage armor
100,shield spell
100,barkskin_2
100,resist energy fire
50,resist energy cold  100,resist energy electricity
100,scroll of magic missile,0,0,2
100,scroll of scorching ray,0,0,2
100,(scroll of lightning bolt,scroll of fireball)
100,(scroll of bestow curse,scroll of confusion)

# Gypsy, melee, B1/F6/R2 (14795)
795
50,masterwork longsword  100,masterwork longspear
100,GYPSY GARB
16,ROBES
100,SIMPLE BOOTS
100,mage armor
100,barkskin_3

# Gypsy, archer, B1/F6/R2 (14795)
7951
100,masterwork composite longbow str 16
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
100,buckler
100,GYPSY GARB
16,ROBES
100,SIMPLE BOOTS
100,mage armor
100,barkskin_3

# Goblin Webwalker, 8d8 (14796)
796
100,poison dart,0,0,12
100,dagger
100,LEATHER ARMOR
100,brown leather boots

# Werebear, B8 (14797)
797
100,mage armor
100,resist energy fire

# Weretiger, B8 (14798)
798
100,mage armor
100,resist energy cold

# Goblin Female (14800)
800
100,CGI,1,9,1
25,WEAPON LIGHT  49,WEAPON ONE-HANDED  50,WEAPON ONE-HANDED 0
40,PADDED ARMOR  49,ARMOR LIGHT  50,ARMOR LIGHT 0  100,ragged clothes
25,SMALL SHIELD

# Hobgoblin Clubslinger, F1 (14801)
801
100,CGI,1,3,1
100,WEAPON SIMPLE LIGHT  
100,throwing club,2,4,1
80,ARMOR LIGHT  100,ARMOR LIGHT 0
50,large wooden shield  100,wooden tower shield

# Wererat (14820)
820
100,CGI,2,3,1
50,shortsword
50,buckler

# Wererat, with pirate key (14820)
8201
100,CGI,2,3,1
50,shortsword
50,buckler
100,wererats key

# Wererat (14820)
9820
50,shortsword
50,buckler

# Orc (14823)
823
100,CGI,1,6,1
75,falchion  100,WEAPON MARTIAL TWO-HANDED  
75,studded leather armor  100,ARMOR LIGHT
100,brown leather boots

# Orc, archer (14823)
8231
100,CGI,1,6,1
100,LONGBOW
75,LEATHER ARMOR  100,ARMOR LIGHT
100,brown leather boots

# Orc Sergeant, F3 (14824)
824
100,CGI,3,3,1
30,FALCHION 0  60,LONGSPEAR 0  70,falchion  80,longspear  90,orc double axe  100,maul  
75,studded leather armor  100,chain shirt
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,(fur cloak,hoodless circlet,black leather boots)

# Orc Thug, F3 (14825)
825
20,CGI,4,3,1  100,COINS BY VALUE,1,4000,0
75,SHORTSWORD 0  100,SHORTSWORD 1
75,LEATHER ARMOR 0  100,LEATHER ARMOR 1
10,RING OF PROTECTION 1
10,AMULET 1
10,cloak of resistance +1 unseen
100,black leather boots

# Orc Mystic, C3 (14826)
826
100,CGI,3,3,1
#60,BATTLEAXE 0  100,battleaxe
60,BATTLEAXE 0  100,battleaxe
60,BREASTPLATE 0  100,breastplate
60,BUCKLER 0  100,buckler
5,RING OF PROTECTION 1
5,AMULET 1
5,cloak of resistance +1 unseen
100,(plaid robes,black leather boots)
100,(scroll of death knell,scroll of hold person,scroll of sound burst)

# Swamp Barracuda, (14827)
827
100,barracuda

# Monkey, (14845)
845
10,GEMS,0,0,1

# Monkey, (14845)
9845
5,COINS BY VALUE,1,10,0

# Rust Monster, (14846)
846
100,GEMS,1,4,0

# Rust Monster, (14846)
8461
100,glyph,0,0,1

# Troll Skeleton, (14850)
850
25,GEMS,0,0,1

# Troll Skeleton, (14850)
8501
25,GEMS,0,0,1
100,masterwork longspear
100,studded leather armor
100,buckler

# Monkey, Rogue, Trinity Pet (14854)
854
100,dagger
100,LEATHER ARMOR
33,GEMS,0,0,1

# Troglodyte (14855)
855
100,CGI,1,6,1
50,(club,javelin,javelin)

# Troglodyte (14855)
9855
100,COINS BY VALUE,3,600,0
50,(club,javelin,javelin)

# Orc, archer, F2 (14858)
858
100,CGI,2,6,1
50,LONGBOW  100,SHORTBOW
100,dagger
75,LEATHER ARMOR  100,ARMOR LIGHT
100,brown leather boots

# Stone Berserker (14859,14974)
859
100,greathammer
100,SCALE MAIL
100,(generic helm,plate boots,plate gloves)

# Stone Berserker (14859,14974)
8591
100,masterwork greathammer
100,black banded mail
100,(generic helm,plate boots,plate gloves)

# Zombie Minotaur (14860)
860
5,potion of blur  10,potion of shield of faith +5  15,potion of displacement  20,potion of heroism  25,potion of good hope

# Goblin Skeleton (14862)
862
20,CGI,1,9,1
75,light mace   98,WEAPON LIGHT   100,WEAPON HALFLING MELEE 0
75,LEATHER ARMOR   98,ARMOR LIGHT   100,ARMOR LIGHT 0
75,SMALL SHIELD   98,SHIELD   100,SHIELD 0

# Hill Giant, random farm encounter (14864)
864
100,CGI,7,3,1
100,hill giant greataxe
100,hill giant rock,0,0,3

# Frost Giant, B1 (14865)
865
20,CGI,10,3,1  100,COINS BY VALUE,3,10000,0
100,frost giant greataxe +1
100,(frost giant chainmail,chainmail gloves, chainmail boots)
100,generic helm

# Frost Giant, Luxwood dungeon (14865)
8651
100,COINS BY VALUE,3,10000,0
100,frost giants key
100,frost giant greataxe
100,(frost giant chainmail,chainmail gloves, chainmail boots)
100,barbarian helm

# Mountain Gnoll (14868)
868
100,CGI,1,3,1
25,club  38,longspear  50,maul  100,SHORTBOW
75,LEATHER ARMOR  98,ARMOR LIGHT   100,ARMOR LIGHT 0
100,battered gnoll shield
50,gnoll helmet   100,gnoll helmet 2

# Gnome Fleshripper, B1/R3/F10 (14869)
869
100,COINS BY VALUE,1,13000,0
100,HALFLING SPIKED CHAIN 2
100,BREASTPLATE 1
25,RING OF PROTECTION 1
25,cloak of resistance +1 orange  100,orange cloak
25,STRENGTH 1
100,(hoodless circlet,brown leather gloves,brown leather boots)

# Dire Tiger (14871)
# also used in other animals, like Dire Bear in gypsy camp
871
100,mage armor

# Lizardfolk Pirate, B2/R5 (14873)
873
20,CGI,7,1,1  100,COINS BY VALUE,1,5000,0
25,GREATAXE 1  50,greatclub +1  75,trident +1  100,LONGSPEAR 1
100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 green
10,STRENGTH 1

# Lizardfolk Pirate, B2/R5 (14873)
8731
20,CGI,6,1,1  100,COINS BY VALUE,1,5000,0
100,LONGSPEAR 1
100,CHAIN SHIRT 0
25,RING OF PROTECTION 1
25,cloak of resistance +1 green
10,STRENGTH 1

# Monkey Witch (14874)
874
20,GEMS,1,3,1
100,pirate dagger,0,0,10
100,mage armor

# Monkey Witch (14874)
8741
20,GEMS,1,3,1
100,pirate dagger,0,0,10
100,mage armor

# Monkey Scrapper (14875)
875
20,GEMS,1,3,1
100,monkey club
100,bark armor

# Monkey Dung-Flinger (14876)
876
20,GEMS,1,3,1
100,monkey dung,0,0,12
100,bark armor

# Lizardfolk Chief, F8 (14877)
877
20,CGI,8,1,1  100,COINS BY VALUE,1,10000,0
100,LONGSPEAR 1
100,troll bone armor
50,RING OF PROTECTION 1
50,cloak of resistance +1 green
25,STRENGTH 1
33,potion of stoneskin  66,potion of mirror image  100,potion of shield

# Centaur (14879)
879
100,masterwork greatsword
100,chain shirt

# Unicorn, dead (14880)
880
100,rusty dagger

# Hobgoblin Elite, F9 (14881)
881
20,CGI,9,1,1  100,COINS BY VALUE,1,9000,0
100,masterwork heavy flail
100,STUDDED LEATHER 0
25,RING OF PROTECTION 1
100,BOOTS

# Fire Giant (14882)
882
20,CGI,11,3,1  100,COINS BY VALUE,3,11000,0
50,fire giant greatsword  100,fire giant greathammer
100,fire giant half-plate
100,(generic helm,plate boots,red padded gloves)

# Fire Giant, B1, Luxwood dungeon (14882)
8821
100,COINS BY VALUE,3,11000,0
100,fire giants key
100,fire giant greatsword
100,fire giant half-plate
100,(generic helm,plate boots,red padded gloves)

# Goblin Envoy, R11 (14883)
883
100,COINS BY VALUE,1,11000,0
100,GEMS OR JEWELRY,1,6,1
100,DAGGER 0
100,CHAIN SHIRT 0
50,RING OF PROTECTION 1
50,AMULET 1
50,CLOAK OF RESISTANCE 1
100,(BOOTS AND GLOVES)

# Orc Warrior, F6 (14889) 
889
20,CGI,6,1,1  100,COINS BY VALUE,1,6000,0
25,FALCHION 0  50,GLAIVE 0  75,GREATAXE 0  100,masterwork greatclub
50,breastplate  100,chainmail
100,(generic helm,black leather boots)
100,barkskin_2

# Orc General, B7/F5 (14890) 
890
100,COINS BY VALUE,1,12000,0
100,LONGSPEAR 2
100,BREASTPLATE 0
50,RING OF PROTECTION 1
50,cloak of resistance +1 fur   100,fur cloak 
25,STRENGTH 1
100,ring of freedom of movement
100,black leather boots
100,barkskin_3

# Orc Maniac, B3/R5 (14891) 
891
20,CGI,8,1,1  100,COINS BY VALUE,1,8000,0
100,masterwork sickle
100,STUDDED LEATHER 0
100,(red cloak,black leather boots)
100,barkskin_2

# Orc Deadeye, F6 (14892)
892
20,CGI,6,1,1  100,COINS BY VALUE,1,6000,0
100,COMPOSITE LONGBOW STR16 0
100,(quiver of arrows,quiver of arrows,quiver of arrows,quiver of arrows)
100,chain shirt
100,buckler
100,(leather helm,brown leather boots)
100,potion of cats grace
100,barkskin_2

# Orc Warlock, W9 (14893) 
893
20,CGI,9,1,1  100,COINS BY VALUE,2,6000,0
100,club
100,wooden elvish shield
50,RING OF PROTECTION 1
100,cloak of resistance +2 unseen
25,INTELLIGENCE 1
100,(eyeglasses,scholars kit)
100,(white robes,black leather boots)
100,mage armor
100,barkskin_3
100,resist energy fire
50,resist energy cold  100,resist energy electricity
100,(scroll of bulls strength mass, scroll of phantasmal killer)

# Fog Turtle, rhondi ship area (14897)
897
25,GEMS,0,0,1

# Fog Turtle, octagon area (14897)
8971
25,GEMS,0,0,1

# Hound Archon (14919)
# Summoned monster, InvenSource.mes holds the inventory
919
0,GREATSWORD

# Bearded Devil (14920)
# Summoned monster, InvenSource.mes holds the inventory
920
0,bearded devil glaive

# Bralani (14924)
# Summoned monster, InvenSource.mes holds the inventory
924
0,(bralani holy scimitar +1,bralani holy composite longbow +1,quiver of arrows,quiver of arrows)
0,bralani armor

# Janni (14925)
# Summoned monster, InvenSource.mes holds the inventory
925
0,(scimitar,LONGBOW,chainmail,generic helm)

# Chain Devil (14926)
# Summoned monster, InvenSource.mes holds the inventory
926
100,(spiked chain,chain devil helm,chain devil chains,chain devil boots,chain devil gloves)

# Nixie (14937)
# Summoned monster, InvenSource.mes holds the inventory
937
0,(LIGHT CROSSBOW,halfling rapier,nixie shawl,nixie feet,nixie hands)

# Pixie (14938)
# Summoned monster, InvenSource.mes holds the inventory
938
0,(SHORTBOW,halfling shortsword,pixie wings)

# Pixie (14938)
# Summoned monster, InvenSource.mes holds the inventory
9381
10,WONDROUS ITEM
25,MUNDANE PERSONAL,1,2,0
25,FOOD,1,2,0
50,HERBS AND SPICES,1,3,0
10,SHORTBOW  20,halfling shortsword
100,FARMER GARB

# Kobold (14939)
939
100,CGI,1,9,1
50,shortspear  75,WEAPON LIGHT  98,WEAPON ONE-HANDED  100,WEAPON ONE-HANDED 0 
75,(sling,pouch of bullets)
75,LEATHER ARMOR  98,ARMOR LIGHT  100,ARMOR LIGHT 0

# Kobold (14939)
9939
100,COINS BY VALUE,2,40,0
50,shortspear  75,WEAPON LIGHT  100,WEAPON ONE-HANDED
50,(sling,pouch of bullets)
75,LEATHER ARMOR  100,ARMOR LIGHT

# Kobold Sergeant, F3 (14940)
940
100,CGI,3,3,1
40,WEAPON MARTIAL ONE-HANDED  100,WEAPON MARTIAL ONE-HANDED 0 
40,ARMOR MEDIUM  100,ARMOR MEDIUM 0

# Kobold Captivator, W3 (14942)
942
100,CGI,3,2,1
100,halfling quarterstaff
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(scroll of scorching ray,scroll of sleep)
100,scroll of magic missile,0,0,2

# Kobold Whippersnapper, F1 (14943)
943
100,CGI,1,3,1
80,halfling spiked chain  100,WEAPON MARTIAL ONE-HANDED 0
80,ARMOR LIGHT  100,ARMOR LIGHT

# Werewolf Lord, F12 (14944)
944
50,GEMS,2,4,1
100,mage armor

# Wererat Lord, B1/R11 (14945)
945
50,GEMS,2,4,1
50,DAGGER 3  100,amulet of mighty fists +3
100,mage armor

# Wererat Lord, Temple solo, B1/R11 (14945)
9451
50,GEMS,2,4,1
50,DAGGER 3  100,amulet of mighty fists +3
100,buckler +2
100,mage armor

# Old White Dragon (14946)
946
100,MUNDANE PERSONAL,1,2,0

# Old Green Dragon (14947)
947
100,MUNDANE PERSONAL,1,2,0

# Old Blue Dragon (14948)
948
100,MUNDANE PERSONAL,1,2,0

# Old Red Dragon (14949)
949
100,MUNDANE PERSONAL,1,2,0

# Huge Spider (14954)  Not set in proto, manually added at encounter
954
50,GEMS BY VALUE,6,100,0
50,JEWELRY BY VALUE,6,100,0
50,SCROLL MINOR
100,empty bottle
100,(rusty dagger,generic helm,goblin leg bone)

# Nymph (14959)
959
100,masterwork dagger
100,barkskin_4
100,resist energy cold
100,resist energy electricity
100,resist energy fire

# Beholder (14960)
960
100,ioun stone deep red
100,ioun stone incandescent blue
100,ioun stone pale blue
100,ioun stone pink
100,ioun stone pink and green
100,ioun stone scarlet and blue
100,potion of dimension door
100,mage armor
100,shield spell

# White Dragon (14962)
962
100,MUNDANE PERSONAL,1,2,0

# Green Dragon (14963)
963
100,MUNDANE PERSONAL,1,2,0

# Blue Dragon (14964)
964
100,MUNDANE PERSONAL,1,2,0

# Red Dragon (14965)
965
100,MUNDANE PERSONAL,1,2,0

# Skeleton Giant (14966)
966
50,GEMS,1,4,1
100,WEAPON HILL GIANT
100,chain shirt

# Lich (14967)
968
100,masterwork quarterstaff
100,ring of protection +4
100,cloak of resistance +4 black
100,ring of fire resistance major
100,mage armor

# Derro, B1/F5/R3 (14969)
969
20,CGI,11,1,1  100,COINS BY VALUE,1,11000,0
50,masterwork shortsword
100,chain shirt
100,small wooden shield
100,black leather boots

# Derro Shooter, B1/F7 (14970)
970
20,CGI,10,1,1  100,COINS BY VALUE,1,10000,0
100,COMPOSITE SHORTBOW STR14 0
100,studded leather armor
100,buckler
100,black leather boots

# Skeleton Bugbear, B7 (14971)
971
50,WEAPON BUGBEAR TWO-HANDED  100,bugbear bardiche
100,chain shirt
25,bugbear bardiche  50,bugbear greatclub  75,bugbear greatsword  100,longspear

# Wererat Bandit, B1/R7 (14972)
972
100,COINS BY VALUE,3,600,0
100,masterwork dagger

# Bone Devil (14976)
976
100,LEATHER ARMOR

# Ice Devil (14976)
977
20,GEMS,1,4,1

# Test Wizard (14980)
980
100,LONGBOW 1
100,longspear
100,AMULET 1
100,RING OF PROTECTION 1
50,CLOAK OF RESISTANCE 1
100,(green gypsy dress,hoodless circlet)

# Iron Golem (14984)
984
100,(full plate,great helm,plate boots,plate gloves)

# Werewolf Bandit (14989)
989
100,COINS BY VALUE,3,600,0
100,GEMS,1,4,1
100,mage armor

# Barbarian starting equipment (25,20,30,1,0.5,1,1 = 78.5 gp)   start money = 100 gp
1000
100,POTION CURE
100,(studded leather armor,greataxe,shortbow,quiver of arrows,fur cloak,barbarian helm,BOOTS)

# Bard starting equipment (25,15,35,2,5,0.5,0.5,1 = 83 gp)   start money = 100 gp
1001
100,POTION CURE
100,(studded leather armor,longsword,LIGHT CROSSBOW,MUSICAL INSTRUMENT,CLOAK,jaunty hat,BOOTS)

# Cleric starting equipment (50,7,12,35,1,1,2 = 108 gp)   start money = 125 gp
1002
100,POTION CURE
100,(SCALE MAIL,LARGE WOODEN SHIELD,heavy mace,LIGHT CROSSBOW,ROBES,BOOTS AND GLOVES)

# Druid starting equipment (15,7,15,0,0,5,1 = 43 gp)   start money = 50 gp
1003
100,POTION CURE
100,(HIDE ARMOR,large wooden shield_2,scimitar,club,SLING,druidic helm,BOOTS)

# Fighter starting equipment (50,12,30,1,1,2 + (15,7)or(20)or(10,15)) = 116 gp)   start money = 150 gp
1004
100,POTION CURE
100,(SCALE MAIL,warhammer,shortbow,quiver of arrows,generic helm,BOOTS AND GLOVES)
50,(longsword,LARGE WOODEN SHIELD)  75,(greataxe)  100,(glaive,buckler)

# Monk starting equipment (5,0,1,1 = 7 gp)   start money = 12.5 gp
1005
100,POTION CURE
100,(quarterstaff,SLING,MONK ROBES,monk boots)

# Paladin starting equipment (50,7,15,35,1,1,2 = 111 gp)   start money = 150 gp
1006
100,POTION CURE
100,(SCALE MAIL,LARGE WOODEN SHIELD,longsword,SHORTBOW,white cloak,BOOTS AND GLOVES)

# Ranger starting equipment (25,15,10,75,1,1,1 = 128 gp)   start money = 150 gp
1007
100,POTION CURE
100,(studded leather armor,LONGSWORD,shortsword,LONGBOW,green cloak,black leather boots)

# Rogue starting equipment (10,10,2,35,1,1,1,10 = 70 gp)   start money = 125 gp
1008
100,POTION CURE
100,(LEATHER ARMOR,shortsword,dagger,LIGHT CROSSBOW,black leather boots,black leather gloves,thieves tools)

# Sorcerer starting equipment (5,2,35+1,1,1,1 = 46 gp)   start money = 75 gp
1009
100,POTION CURE
100,(longspear,dagger,LIGHT CROSSBOW,WIZARD CLOTHING,WIZARD HAT,BOOTS)

# Wizard starting equipment (5,2,35+1,1,1,1 = 46 gp)   start money = 75 gp
1010
100,POTION CURE
100,(quarterstaff,dagger,LIGHT CROSSBOW,WIZARD CLOTHING,WIZARD HAT,BOOTS)

# Garb and Boots only
1011
100,FARMER GARB

# Barrel, near flute + lyre
1049
70,fish,2,6,0  100,KEGS,1,3,0
50,pouch of bullets,2,4,1
3,WEAPON 0  5,MAGIC MINOR

# Barrel, near inn
1050
25,cheese,1,4,1
25,bread,1,4,1
25,mutton,1,4,1
25,grapes,1,4,1
25,apple,1,4,1
25,fish,1,4,1
25,bottle of wine,1,4,0
25,bottle of rum,1,4,0
25,bottle of goodberry wine,0,0,1
50,pouch of bullets,2,4,1
3,WEAPON 0  5,MAGIC MINOR

# Barrel, near inn
1051
100,KEGS,1,4,0
50,pouch of bullets,2,4,1
3,WEAPON 0  5,MAGIC MINOR

# 1053, 1054, 1055 reserved for poor boxes

# Food Table
1056
100,stew,1,4,1
100,seafood pie,1,4,1
100,cheese,1,4,1
100,bread,1,4,1
100,mutton,1,4,1
100,grapes,1,4,1
100,apple,1,4,1

# Junk Chest, Tosh
1060
100,pouch of bullets,0,0,10

# Junk Chest Ivory, Tosh
1062
100,pouch of bullets,0,0,10
10,WEAPON 0

# Stump, Crypt Cave Exterior
1063
100,GEMS OR JEWELRY
100,POTION
100,WEAPON 0

# B1, Rogahn Cabinet
1064
100,bottle of rum
100,POTION MINOR
100,VILLAGER GARB,1,4,1
100,NOBLE GARB,1,2,1
100,CLOAK,1,3,1
25,CLOAK OF RESISTANCE 1
100,(CHAIN SHIRT,BOOTS)

# Food and Drink Table
1066
100,mutton,0,0,1
100,chicken,0,0,1
100,stew,0,0,1
100,seafood pie,0,0,1
100,cheese,0,0,2
100,bread,0,0,2
100,grapes,0,0,1
100,apple,0,0,1
100,ale,0,0,2
100,stout,0,0,2
100,cider,0,0,1
100,mead,0,0,1
100,wine,0,0,1
100,rum,0,0,1

# B1, Smithy Anvil
1070
100,bronze statuette
100,HEAVY MACE 1,0,0,1
100,WEAPON,1,4,1

# B1, Advisor Desk Drawer
1071
10,potion of dimension door
10,boots of elvenkind
10,COINS BY VALUE,10,1000,0
5,carnelian  10,blue jasper
10,gold crown badge
10,scroll of identify
10,scroll of web
10,cursed scroll
10,ring of protection +1
10,potion of cure light wounds,0,0,2
10,dagger +1
100,rusty dagger

# B1, Mistress Dresser
1072
100,(silver comb,silver mirror,silver medallion on chain,blue ore)
100,FANCY GARB,2,3,1
100,LEATHER ARMOR
100,(white leather boots,fine leather boots)

# B1, Captain's Chest
1073
100,VILLAGER GARB,1,3,1
100,CLOAK,1,3,1
100,LONG COAT
100,BOOTS,0,0,2
100,rusty dagger
100,walnut plaque
100,onyx statue

# Church Attic End Table
1080
50,COINS BY VALUE,3,6,0
25,GLOVES
25,BOOTS
25,rusty dagger
25,cheese
25,bottle of wine  50,bottle of rum
50,SCROLL  100,POTION

# Rock at Random Farm encounter
1081
100,GEMS BY VALUE,6,100,100
100,WONDROUS ITEM MINOR
100,WEAPON MELEE 0

# Dead Gnome Wizard, Crossroads Swamp
1082
100,CGI,3,2,1
60,small wooden elvish shield
5,RING OF PROTECTION 1
5,AMULET 1
5,CLOAK OF RESISTANCE 1
100,(brown robes,BOOTS)
100,SCROLL ARCANE MINOR,1,4,1
25,SCROLL ARCANE MEDIUM

# B1, Fissure, Entry Cave
1084
50,ragged clothes,1,2,0
50,BOOTS
50,GLOVES
25,WIZARD HAT  50,HAT
25,ALL ROBES
25,CLOAK
100,WEAPON RUSTY,1,2,0
10,WEAPON  11,WEAPON 0
10,SHIELD  11,SHIELD 0
10,ARMOR LIGHT  11,ARMOR 0
5,broom  10,rake  15,shovel  20,rolling pin
50,rope 50 feet
50,chain 10 feet
50,iron spike,0,0,2
50,ten-foot pole
100,quiver of arrows,0,0,3
100,quarrel of bolts,0,0,3
1,holy water  2,jaers spheres of fire  3,eyeglasses  4,masterwork thieves tools  6,MW MUSICAL INSTRUMENT  7,smokestick  8,healers kit  9,scholars kit
1,helm of read magic  2,cloak of elvenkind  3,boots of elvenkind  4,extraplanar chest miniature

# B1, Loose Stone, in Throne Room
1085
100,SCROLL MINOR,1,4,1
100,WEAPON LIGHT 0
100,violet garnet,0,0,4

# B1, Sacrifice Pit
1088
100,goblin leg bone,0,0,1
100,white pearl_2,0,0,1
100,GEMS BY VALUE,4,10,0
100,GEMS BY VALUE,4,20,0

# B1, Crevice, near Erig's Statue
1093
100,wooden shield +1
100,shortsword +2
100,vial of green liquid

# B1, Teleport Room, South
1223
100,COINS BY VALUE,3,60,0

# B1, Teleport Room, North
1224
100,COINS BY VALUE,3,60,0
100,goblin leg bone,0,0,1

#Extended base classes added by temple+ below

# Favored Soul starting equipment (50,7,12,25,2,1,1 = 98 gp)   start money = 125 gp
1534
100,POTION CURE
100,(SCALE MAIL,LARGE WOODEN SHIELD,heavy mace,LIGHT CROSSBOW,BOOTS,GLOVES)

# Scout starting equipment (25,10,30,1,1,30 = 97 gp)   start money = 125 gp
1546
100,POTION CURE
100,(studded leather armor,shortsword,shortbow,quiver of arrows,green cloak,black leather boots,thieves tools)

# Warmage starting equipment (10,3,5,25,1 = 44 gp)   start money = 100 gp
1547
100,POTION CURE
100,(LEATHER ARMOR,SMALL SHIELD,light mace,LIGHT CROSSBOW,BOOTS)

# Beguiler starting equipment (10,10,25,2,1,1,30 = 79 gp)   start money = 150 gp
1548
100,POTION CURE
100,(LEATHER ARMOR,shortsword,LIGHT CROSSBOW,BOOTS,black leather gloves,thieves tools)

# Swashbuckler starting equipment (25,20,30,1,1,1 = 78gp)   start money = 150 gp
1549
100,POTION CURE
100,(studded leather armor,rapier,shortbow,quiver of arrows,red bandana,black leather boots)

#------------------------------------------------------------------------------
# general guidelines, 2/1/24
# most will be -----> "20, CGI, x, 3, 1   100, gold ,1, x, 0"
# some will be ------> "100, CGI"
# almost always 1/3 split, occasionally leader or solo creature will have full split

# These are 100%

# 20	100, CGI, 1
# 19	100, CGI, 2
# 23	100, CGI, 3
#  1	100, CGI, 4		sea hag
#  5	100, CGI, 5		minotaur, bug leader, bug deserter leader x2, angus shoreline,  

#  1	100, CGI, 6		king behemoth frog
#  4	100, CGI, 7		cave ogre chief, hill giant farm, lamia, efreeti
#  1	100, CGI, 8		troll chief Oohlgrist,
#  0	100, CGI, 9
#  1	100, CGI ,10	Remington chest

#  0	100, CGI, 11
#  1	100, CGI, 12	great ogre chief
#  4	100, CGI, 13	hill giant chief, scorpp, scorpp's mom, hill giant smuggler
#  1	100, CGI, 14	frost giant chief
#  1	100, CGI, 15	fire giant chief
#------------------------------------------------------------------------------
"""

import StringIO
def get_inv_source_lines():
	return StringIO.StringIO(inv_source_text)
















