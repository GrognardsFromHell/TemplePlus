dungeon_encs_text="""
#------------------------------------------------------------------------------
# These are all the possible creature encounters for all dungeon levels.
# The data is read in by read_encs() in dungeon.py.
#
# Format of data:
#
# Encounter Name (line 1)
#   - The name of the encounter.
#   - This will be used as the name of the dictionary entry for the encounter
#     lists that get generated when each level spawns.
#
# Dungeon Levels (line 2)
#   - A list of dungeon levels where this encounter can be used.
#   - 1 = B1 Level 1, 2 = B1 Level 2, 11 = Temple Level 1.
#   - Comma separated.
#
# Encounter Levels (line 3)
#   - The EL of the encounter for each of the five area sizes.
#   - This line MUST ALWAYS have FIVE values, and be comma separated.
#   - The EL is an approximation, and not adjusted for the actual number of
#     creatures rolled for that encounter.
#   - Each area that spawns creatures will have a size, which defines how
#     the dice will be rolled to get the number of creatures for that encounter,
#     so the EL should be set based on how many could possibly appear.
#   - The five area sizes are, in order:
#       0:	1 creature.
#       1:	1d4+1 creatures.	max 5	average 3.5
#       2:	2d4+2 creatures.	max 10	average 7
#       3:	2d6+4 creatures.  	max 16	average 11
#       4:	2d10+4 creatures.	max 24	average 15
#   - A value of 0 means this encounter will Never be used for that area size.
#     Use this if a certain encounter will not fit into one of the area sizes,
#     or if the encounter doesn't make sense for an area of that size.
#     For example, if there is an encounter with 8 specific creatures that you
#     always want spawned, it won't fit in a solo or small area size. So the
#     the first two EL numbers should be set to 0. 
#   - The EL is also used to determine treasure.
#
# Creature groups (lines 4+)
#   - Each line defines a group of creatures that will be created in the area.
#   - Each line MUST ALWAYS HAVE five values, and be comma separated.
#   - The creatures will be created in the order listed.
#   - PUT THEM IN ORDER BASED ON QUANTITY. Put those with an integer first,
#     then those with a percentage next, the those with a '0' last.
#
#   1. Proto number
#
#   2. Quantity
#      - Defines how many of this creature will be created.
#      - Dice are rolled based on the area size. See chart above.
#      - There are three possible formats.
#          integer:
#            - Create this exact number of creatures. Range is 1-24.
#            - That many WILL be created, regardless of dice roll. Exception.
#            - That many WILL NOT be allowed to exceed the max number of spots
#              for the area size. The code will reduce the number in that case. 
#            - So if the number is 7, but the area size is 1 (only 5 spots),
#              then only 5 creatures will be created.
#          float:
#            - Create this % of the remaining spots rolled. Range is 0.01-0.99.
#          0:
#            - Create this creature for all remaining spots rolled.
#            - This will use up all remaining spots, rolled for that encounter,
#              so put that creature last on the list, and only use it once.
#            - This is mostly used for encounters with only one creature proto.
#
#   3. Organization.
#      - Defines how the creatures will be arranged in the area.
#      - There are three possible values.
#          f: arrange the creature strategically on the formation spots.
#          u: arrange the creature on any unused spot, keep rotation.
#          s: scatter the creature on any unused spot, randomize rotation.
#
#   4. Duty
#      - Defines what the creatures role in the encounter is, which will
#        determine where he will be placed on the formation spots.  
#      - There are three possible values.
#          b: boss, at front of formation.
#          m: melee, place near the front of the formation.
#          r: ranged, place behind the melee guys, or in protected areas.
#          w: boss, far in the back, often a wizard.
#
#   5. KOS
#      - There are three possible values.
#          d: use the default KOS set in the creatures proto.
#          k: set ONF_KOS
#          o: set ONF_KOS_OVERRIDE 
#
#   VERY IMPORTANT:
#     As of right now, comments can only go in a block section at the start.
#------------------------------------------------------------------------------

# Done:
# x		18	Nightcrawler (water snake)
# x 	16	Nightwalker (bodak model)
# x		18	Red Dragon, Mature Adult
# x		18  Blue Dragon, Old
# x		18  Green Dragon, Old
# x		15  White Dragon, Old
# x		16	Greater Stone Golem
# x		14	Werewolf Lord (all 3 forms)
# x 	14	Wererat Lord (all 3 forms)
# x		14	Umber Hulk, Truly Horrid
# x		13	Ice Devil (white lizard man model)
# x		13	Beholder
# x		13	Iron Golem
# x		12	Basilisk, Abyssal greater
# x 	12	Frost Wurm
# x		12	Elder Black Pudding
# x		11	Dread Wraith
# x		11	Stone Golem

# TBD, SAVE FOR VERY HIGH LEVEL
# 20	Pit Fiend (balor model, devil)
# 20	Titan
# 20 	Tarrasque
# 17	Marileth (demon, snake model?)

# NOT USING, FOR NOW:
#   All 4 demons,
#   All 4 elementals,
#   Chaggrin, Illdriss, Kapoacinth, Vodyanoi, Drelb, Sea Hag,
#   Vortex, Windwalker, Eye of the Deep, Galeb Duhr, 


ascomid				# cr 4
1,2
4,6,0,0,0
14277,0,s,m,k

basidirond			# cr 4
1,2
4,6,0,0,0
14281,0,s,m,k

basilisk			# cr 5  ;1;2;
1,2
5,5,0,0,0
14295,1,s,m,k

basilisk greater [L]	# cr 12  ;11;
11
0,12,14,0,0
14987,1,s,m,k
14987,0,s,m,k

beholder [L]		# cr 13  ;11;21;22;
11,21,22
0,13,15,15,15
14960,1,u,m,k
14960,-2,u,m,k

black pudding		# cr 7
2,11
7,9,0,0,0
14143,0,s,m,k

black pudding elder [L]		# cr 12  ;11;21;22; 
11,21,22
0,12,12,0,0
14979,1,s,m,k

bodak				# cr 8  ;2;11;21;22;
2,11,21,22
8,10,0,0,0
14328,0,s,m,k

brigand				# cr 1  ;1;2;
1,2
1,3,5,6,7
14312,0,f,m,k

brigand warband		# cr 2,4,6,8,10  cr 2 average  ;1;2;
1,2
0,4,6,7,8
14069,1,f,b,k		# Brig Leader R3
14718,1,f,w,k		# Wizard 3
14070,1,f,m,k		# Brig Sneak F1/R1
14313,1,f,r,k		# Brig Archer F1
14314,1,f,r,k		# Brig Crossbowman F1
14070,0.5,f,m,k		# Brig Sneak F1/R1
14313,0.25,f,r,k	# Brig Archer F1
14314,0.25,f,r,k	# Brig Crossbowman F1

bugbear				# cr 2  ;1;2;11;
1,2,11
2,4,6,7,8
14093,0,u,m,k

bugbear warband		# cr 3,5,3,2,2  cr 3 average  ;1;2;
1,2
0,5,7,8,9
14170,1,f,b,k		# leader/tripper F2
14711,1,f,w,k		# witch W3
14171,1,f,m,k		# berzerker B1
14173,1,f,m,k		# flanker
14172,1,f,r,k		# archer
14171,0.25,f,m,k	# berzerker B1
14173,0.50,f,m,k	# flanker
14172,0.25,f,r,k	# archer

bugbear gang		# cr 8 average  ;11;
11
0,9,10,11,11
14174,1,f,b,k		# leader F9
14715,1,f,w,k		# bugbear witch W9
14175,1,f,m,k		# rager B7
14177,1,f,m,k		# flanker F4/R3
14176,1,f,r,k		# archer F7
14175,0.35,f,m,k	# rager B7
14177,0.35,f,m,k	# flanker F4/R3
14176,0.30,f,r,k	# archer F7

carrion crawler			# cr 4  ;2;
2
4,6,0,0,0
14190,0,s,m,k

centipedes medium and small	# cr 1/2,1/4
1
0,1,2,3,4
14805,0.5,s,m,k
14804,0.5,s,m,k

centipedes large	# cr 1  ;1;2;
1,2
1,3,5,6,7
14806,0,s,m,k

centipedes huge		# cr 2  ;2;
2
2,4,6,0,0
14807,0,s,m,k

crystal ooze		# cr 5  ;2;11;
2,11
5,7,0,0,0
14141,0,s,m,k

dire bat			# cr 2  ;2;
2
2,4,6,7,0
14390,0,s,m,k

dire rat			# cr 1/3
1,2
1,1,2,3,4
14056,0,s,m,k

dragon white young [1]	# cr 4  ;2;11;
2,11
4,6,8,0,0
14109,0,u,m,k

dragon green young [1]	# cr 5  ;2;11;
2,11
5,7,9,0,0
14817,0,u,m,k

dragon blue young [1]	# cr 6  ;2;11;
2,11
6,8,10,0,0
14818,0,u,m,k

dragon red young [1]	# cr 7  ;2;11;
11
7,9,11,0,0
14819,0,u,m,k

dragon white [L1]		# cr 10  ;11;
11
0,10,10,0,0
14962,1,f,r,k

dragon green [L1]		# cr 11  ;11;
11
0,11,11,0,0
14963,1,f,r,k

dragon blue [L1]		# cr 11  ;11;
11
0,11,11,0,0
14964,1,f,r,k

dragon red [L1]			# cr 13  ;11;
11
0,13,13,0,0
14965,1,f,r,k

dragon white old [L1]	# cr 15  ;11;
11
0,0,15,15,0
14946,1,f,b,k

dragon green old [L1]	# cr 18  ;11;
11
0,0,18,18,0
14947,1,f,b,k

dragon blue old [L1]	# cr 18  ;11;
11
0,0,18,18,0
14948,1,f,b,k

dragon gathering young [1]	# cr 7,6,5,4  ;2;11;
2,11
0,0,8,8,8
14819,1,u,m,k	# red
14818,1,u,m,k	# blue
14817,1,u,m,k	# green
14109,1,u,m,k	# white

dragon gathering [L1]		# cr 13,11,11,10  ;11;
11
0,0,0,14,14
14965,1,u,m,k	# red
14964,1,u,m,k	# blue
14963,1,u,m,k	# green
14962,1,u,m,k	# white

dread wraith		# cr 16  ;11;21;22;
11,21,22
11,0,0,0,0
14986,1,u,m,k

efreeti				# cr 7  ;2;11;
2,11
7,9,0,0,0
14340,0,u,m,k

Ettin [L]			# cr 6  ;1;2;11;
1,2,11
0,6,8,0,0
14238,0,u,m,k

fire bat			# cr 3  ;2;
2
3,5,7,0,0
14297,0,s,m,k

fire snake			# cr 3  ;2;
2
3,3,0,0,0
14299,1,s,m,k

fire toad [L]		# cr 8  ;2;11;
2
0,8,0,0,0
14300,1,s,m,k

flamebrother		# cr 3  ;2;
2
3,5,7,0,0
14384,0,u,m,k

frost worm [L]		# cr 12
11
0,12,12,0,0
14981,1,s,m,k

giant tick			# cr 2  ;1;2;
1,2
2,4,6,0,0
14089,0,s,m,k

gargoyle			# cr 4  ;1;2;11;21;22;
1,2,21,22
4,6,8,0,0
14239,0,u,m,k

gelatinous cube		# cr 3  ;1;2;
1,2
3,3,0,0,0
14139,1,s,m,k

ghast				# cr 3  ;1;2;
1,2
3,5,7,8,9
14135,0,s,m,k

ghoul				# cr 1  ;1;2;
1,2
1,3,5,6,7
14095,0,s,m,k

ghoul and ghast		# cr 2 average  ;1;2;11;
1,2,11
2,4,6,7,8
14095,0.5,s,m,k		# ghoul 1		
14135,0.5,s,m,k		# ghast 3

giant gathering [L]	# cr 8 average  ;11;
11
0,0,10,10,0
14882,1,f,b,k		# fire giant		10
14865,1,f,m,k		# frost giant		9
14221,1,f,m,k		# hill giant        7
14356,1,f,w,k		# ogre shaman C5	7
14309,1,f,r,k		# gorgon			6

gibbering mouther	# cr 5
2
5,5,0,0,0
14821,1,s,m,k

gnoll				# cr 1  ;1;2;
1,2
1,3,5,6,7
14067,0,u,m,k

gnoll warband		# cr 3,4,2,1  cr 2 average  ;1;2;
1,2
0,4,6,7,8
14066,1,f,b,k		# leader F2
14712,1,f,w,k		# shaman D3
14078,1,f,m,k		# berzerker B1
14067,2,f,r,k		# normal
14078,0.3,f,m,k		# berzerker B1
14067,0.7,f,r,k		# normal

goblin				# cr 1/3  ;1;2;
1,2
1,1,2,3,4
14186,0,u,m,k

goblin warband		# cr 3,3,1/3,1   cr 1 average  ;1;2;
1,2
0,3,5,6,7
14183,1,f,b,k		# leader F1/R2
14710,1,f,w,k		# wiz W3
14186,2,f,m,k		# norm
14185,1,f,r,k		# archer F1
14186,0.7,f,m,k		# norm
14185,0.3,f,r,k		# archer F1

gorgon				# cr 6  ;2;11;
2,11
6,6,0,0,0
14309,1,s,m,k

gray ooze			# cr 4  ;1;2;11;
1,2,11
4,6,0,0,0
14140,0,s,m,k

green slime			# cr 1  ;1;2;
1,2
1,3,5,0,0
14091,0,s,m,k

groaning spirit		# cr 6  ;1;2;
1,2
6,6,0,0,0
14280,1,s,m,k

harpy				# cr 3  ;2;
2
3,5,7,0,0
14243,0,u,m,k

hell hound			# cr 3  ;2;
2
3,5,0,0,0
14916,0,s,m,k

hill giant [L]		# cr 7  ;2;11;
2,11
0,7,9,10,0
14217,0,u,m,k

hobgoblin			# cr 1/2  ;1;2;
1,2
1,2,3,4,5
14188,0,u,m,k

hobgoblin warband	# cr 3,3,1/2,1   cr 2 average  ;1;2;
1,2
0,4,6,7,8
14189,1,f,b,k		# sergeant F3
14710,1,f,w,k		# goblin wizard
14188,2,f,m,k		# norm
14801,1,f,r,k		# clubslinger F1
14188,0.7,f,m,k		# norm
14801,0.3,f,r,k		# clubslinger F1

hydra [L]			# cr 4  ;2;11;
2,11
0,4,4,0,0
14343,1,u,m,k

hydra twelve-headed cryohydra [L]	# cr 13  ;11;
11
0,13,13,13,0
14973,1,u,w,k
14140,1,u,b,k		# grey ooze			5
14140,-4,u,m,k		# grey ooze			5

ice devil [L]		# cr 13
11
0,13,15,0,0
14977,1,f,m,k
14977,-3,f,m,k

iron golem [L]		# cr 13
11
0,13,15,0,0
14984,0,f,m,k

kobold				# cr 1/4  ;1;2;
1,2
1,1,2,3,4
14939,0,u,m,k

lamia				# cr 6  ;2;11;
2,11
6,6,0,0,0
14342,1,u,m,k

leucrotta			# cr 4  ;99;
99
4,6,0,0,0
14351,0,u,m,k

lizardfolk			# cr 1  ;1;2;
1,2
1,3,5,6,7
14084,0,u,m,k

lizardfolk warband	# cr 4,4,1,1  cr 2 average  ;1;2;
1,2
0,4,6,7,8
14086,1,f,b,k		# patriarch B3
14087,1,f,w,k		# shaman D3
14084,2,f,m,k		# male 
14085,1,f,r,k		# female
14084,0.7,f,m,k		# male 
14085,0.3,f,r,k		# female

minotaur			# cr 5  ;1;2;
1,2
5,5,5,0,0
14241,1,f,b,k

nightwalker [L]		# cr 16  ;11;
11
0,0,16,16,16
14985,1,f,w,k		# nightwalker		16
14986,1,f,b,k		# dread wraith		11
14941,1,u,m,k		# greater shadow	8
14941,-4,u,m,k		# greater shadow	8

nightcrawler [L]	# cr 18  ;11;
11
0,0,18,18,18
14988,1,f,w,k		# nightcrawler		18
14986,1,u,m,k		# dread wraith		11
14986,-3,u,m,k		# dread wraith		11

ochre jelly			# cr 5  ;2;11;21;22;
2,11,21,22
5,7,9,0,0
14142,0,s,m,k

ogre [L]			# cr 3  ;1;2;11;
1,2,11
0,3,5,7,0
14249,0,u,m,k

ogre warband [L]	# cr 7,6,3   cr 5 average  ;2;
2
0,0,7,9,10
14248,1,f,b,k		# ogre chief B4
14353,1,f,w,k		# ogre shaman C3
14249,0,f,m,k		# ogre norm

ogre gang [L]		# cr 9 average, mostly basher  ;11;
11
0,0,11,13,14
14354,1,f,b,k		# ogre chieftain B12
14356,1,f,w,k		# ogre shaman C5
14826,1,f,r,k		# orc mystic
14892,1,f,r,k		# orc deadeye
14065,1,f,m,k		# ogre basher B1/F6
14065,0,f,m,k		# ogre basher B1/F6

orc					# cr 1/2  ;1;2;
1,2
1,2,3,4,5
14823,0,f,m,k

orc warband			# cr 3,3,0.5,2   cr 2 average  ;1;2;
1,2
0,4,6,7,8
14824,1,f,b,k		# orc sergeant
14826,1,f,w,k		# orc mystic
14825,1,f,r,k		# orc thug
14823,1,f,m,k		# orc
14858,1,f,r,k		# orc archer
14823,0.7,f,m,k		# orc
14858,0.3,f,r,k		# orc archer

orc gang			# cr 7 average  ;11;
11
0,9,11,11,11
14890,1,f,b,k		# orc general B7/F5
14893,1,f,w,k		# orc warlock W9
14892,1,f,r,k		# orc deadeye F6
14891,1,f,m,k		# orc maniac B3/R5
14889,1,f,m,k		# orc warrior B1/F5
14892,0.33,f,r,k	# orc deadeye F6
14891,0.33,f,m,k	# orc maniac B3/R5
14889,0.33,f,m,k	# orc warrior B1/F5

owlbear				# cr 4  ;2;
2
4,4,0,0,0
14046,1,u,m,k

party of archers	# cr 1, average  ;1;2;
1,2
0,3,5,6,7
14172,1,f,w,k		# bugbear archer F1
14858,1,f,r,k		# orc archer F2
14801,1,f,r,k		# hobgob clubslinger F1
14313,1,f,r,k		# brigand archer F1
14185,1,f,r,k		# goblin archer F1
14172,0.2,f,r,k		# bugbear archer F1
14858,0.2,f,r,k		# orc archer F2
14801,0.2,f,r,k		# hobgob clubslinger F1
14313,0.2,f,r,k		# brigand archer F1
14185,0.2,f,r,k		# goblin archer F1

party of devils [L]	# cr 13,9,6,2  ;11;
11
0,0,15,17,17
14977,1,u,m,k		# ice devil
14976,1,u,m,k		# bone devil
14926,3,u,m,k		# chain devil
14977,1,u,m,k		# ice devil
14976,1,u,m,k		# bone devil
14926,3,u,m,k		# chain devil

party of cold [L]	# cr 9 average  ;11;
11
0,0,13,13,13
14962,1,f,w,k		# white dragon		10
14865,1,u,m,k		# frost giant		9
14865,1,u,m,k		# frost giant		9
14966,1,u,m,k		# skeleton giant	6
14966,1,u,m,k		# skeleton giant	6
14866,1,u,m,k		# winter wolf		5
14866,1,u,m,k		# winter wolf		5
14291,1,u,m,k		# will-o-the-wisp	6
14140,1,u,m,k		# grey ooze			5
14141,1,u,m,k		# crystal ooze		5

party of fire [L]	# cr 9 average, creatures after ooze are filler for large encounter areas  ;11;
11
0,0,14,14,14
14965,1,f,w,k		# red dragon		13
14882,1,u,m,k		# fire giant		10
14882,1,u,m,k		# fire giant		10
14300,1,u,m,k		# fire toad			8
14950,1,u,m,k		# noble salamander	10
14898,1,u,m,k		# dire hell hound	6
14898,1,u,m,k		# dire hell hound	6
14291,1,u,m,k		# will-o-the-wisp	6
14140,1,u,m,k		# grey ooze			5
14141,1,u,m,k		# crystal ooze		5
14297,1,u,m,k		# fire bat			3
14299,1,u,m,k		# fire snake		3

party of fungus		# cr 4, average  ;2;11;
2,11
0,6,8,9,10
14284,0.15,f,r,k	# phycomid			4
14283,0.1,f,r,k		# ustilagor			4
14276,0.15,s,m,k	# yellow mold		6
14277,0.2,s,m,k		# ascomid			4
14281,0.2,s,m,k		# basidirond		4
14273,0.1,s,m,k		# violet fungi		3
14282,0.1,s,m,k		# shrieker			0

party of gnomes     # cr 12    ;11;
11
0,12,14,14,14
14869,1,f,b,k			# gnome fleshripper		14
14786,1,f,w,k			# gnome cabalist		9
14200,3,f,m,k			# gnome daggermaster	11
14869,1,f,b,k			# gnome fleshripper		14
14786,1,f,r,k			# gnome cabalist		9
14200,1,f,m,k			# gnome daggermaster	11
14945,1,f,m,k			# wererat lord			12
14898,0.5,u,m,k 		# dire hell hound		6
14390,0.5,u,m,k			# dire bat				2

party of goblinoids		# cr 2, average  ;1;2;
1,2
0,4,6,7,8
14249,1,f,b,k			# ogre				3
14213,1,f,m,k			# female bugbear	2
14093,1,f,m,k			# bugbear			2
14188,1,f,m,k			# hobgoblin			1/2
14186,1,f,m,k			# goblin			1/3
14213,0.25,f,m,k		# female bugbear	2
14093,0.25,f,m,k		# bugbear			2
14188,0.25,f,m,k		# hobgoblin			1/2
14186,0.25,f,m,k		# goblin			1/3

party of lycanthropes	# cr 2  ;2;11;
2,11
0,4,6,7,8
14344,1,u,m,k		# werewolf			3
14391,1,u,m,k		# dire wolf			3
14820,2,u,m,k		# wererat			2
14056,1,u,m,k		# dire rat			1/3
14344,0.25,u,m,k	# werewolf			3
14391,0.25,u,m,k	# dire wolf			3
14820,0.25,u,m,k	# wererat			2
14056,0.25,u,m,k	# dire rat			1/3

party of lycanthropes high level	# cr 12  ;11;
11
0,12,13,13,13
14944,1,f,b,k		# werewolf lord		12
14945,1,f,m,k		# wererat lord		12
14714,1,f,w,k		# shadow wizard		9
14797,1,f,m,k		# werebear			11
14798,1,f,m,k		# weretiger			11
14936,1,f,m,k		# dire boar         4
14989,0.5,f,m,k		# werewolf bandit	9
14972,0.5,f,m,k		# wererat bandit	9

party of magical beasts [L]			# cr 12  ;11;
11
0,0,12,12,12
14351,1,f,w,k		# leucrotta, w5		10
14981,1,f,b,k		# frost worm		12
14987,1,f,m,k		# greater basilisk	12
14343,1,f,m,k		# hydra, 7-headed	6 
14309,1,f,r,k		# gorgon			6
14866,1,f,m,k		# winter wolf		5
14046,1,f,m,k		# owlbear			4
14342,1,f,r,k		# lamia				6
14295,1,f,r,k		# basilisk			5
14182,1,f,r,k		# stirge			0

party of ooze low	# cr 4 average  ;2;
2
0,6,6,6,0
14142,1,s,m,k		# ochre jelly		5
14821,1,s,m,k		# gibbering			5
14141,1,s,m,k		# crystal ooze		5
14140,1,s,m,k		# gray ooze			4
14091,1,s,m,k		# green slime		1

party of ooze		# cr 5 average  ;2;11;
2,11
0,7,7,7,7
14143,1,s,m,k		# black pudding		7
14142,1,s,m,k		# ochre jelly		5
14821,1,s,m,k		# gibbering			5
14141,1,s,m,k		# crystal ooze		5
14140,1,s,m,k		# gray ooze			4
14091,1,s,m,k		# green slime		1

party of ooze 2		# cr 5 average  ;11;21;22;
11,21,22
0,7,9,10,11
14143,1,s,m,k		# black pudding		7
14142,1,s,m,k		# ochre jelly		5
14821,1,s,m,k		# gibbering			5
14141,1,s,m,k		# crystal ooze		5
14140,1,s,m,k		# gray ooze			4
14091,1,s,m,k		# green slime		1
14143,0.2,s,m,k		# black pudding		7
14142,0.2,s,m,k		# ochre jelly		5
14821,0.1,s,m,k		# gibbering			5
14141,0.2,s,m,k		# crystal ooze		5
14140,0.2,s,m,k		# gray ooze			4
14091,0.1,s,m,k		# green slime		1

party of undead low		# cr 1/2 average  ;1;2;
1,2
0,2,3,4,5
14095,1,s,m,k		# ghoul				1
14081,1,s,m,k		# skeleton gnoll	1
14092,1,s,m,k		# zombie			1/2
14107,1,s,m,k		# skeleton			1/3
14862,1,s,m,k		# skeleton goblin	1/3
14095,0.2,s,m,k		# ghoul				1
14081,0.2,s,m,k		# skeleton gnoll	1
14092,0.2,s,m,k		# zombie			1/2
14107,0.2,s,m,k		# skeleton			1/3
14862,0.2,s,m,k		# skeleton goblin	1/3

party of undead mid		# cr 3 average  ;2;11;
2,11
0,5,7,8,9
14849,1,s,m,k		# wraith			5
14289,1,s,m,k		# shadow			3
14135,1,s,m,k		# ghast				3
14095,1,s,m,k		# ghoul				1
14081,1,s,m,k		# skeleton gnoll	1
14849,0.2,s,m,k		# wraith			5
14289,0.2,s,m,k		# shadow			3
14135,0.2,s,m,k		# ghast				3
14095,0.2,s,m,k		# ghoul				1
14081,0.2,s,m,k		# skeleton gnoll	1

party of undead high	# cr 8 average  ;11;21;22;
11,21,22
0,10,12,13,14
14986,1,u,m,k		# dread wraith		11
14328,1,u,m,k		# bodak				8
14941,1,u,m,k		# greater shadow	8
14861,1,u,m,k		# spectre			7
14849,1,u,m,k		# wraith			5
14986,0.2,u,m,k		# dread wraith		11
14328,0.2,u,m,k		# bodak				8
14941,0.2,u,m,k		# greater shadow	8
14861,0.2,u,m,k		# spectre			7
14849,0.2,u,m,k		# wraith			5

party of undead all		# cr 7 average, low-level undead add nothing to the cr  ;11;
11
0,9,11,11,11
14986,1,u,m,k		# dread wraith		11
14328,1,u,m,k		# bodak				8
14941,1,u,m,k		# greater shadow	8
14861,1,u,m,k		# spectre			7
14849,1,u,m,k		# wraith			5
14289,1,u,m,k		# shadow			3
14135,1,u,m,k		# ghast				3
14095,0.2,u,m,k		# ghoul				1
14081,0.2,u,m,k		# skeleton gnoll	1
14092,0.2,u,m,k		# zombie			1/2
14107,0.2,u,m,k		# skeleton			1/3
14862,0.2,u,m,k		# skeleton goblin	1/3

party of werewolves 	# cr 12  ;11;21;22;
11,21,22
0,12,13,13,13
14944,1,f,b,k		# werewolf lord		12
14989,2,f,m,k		# werewolf bandit	9
14893,1,f,w,k		# orc warlock		9
14391,1,u,r,k		# dire wolf			3
14989,0.40,f,m,k	# werewolf bandit	9
14344,0.30,u,m,k	# werewolf			3
14391,0.30,u,r,k	# dire wolf			3

party of wererats 	# cr 12  ;11;21;22;
11,21,22
0,12,13,13,13
14945,1,f,b,k		# wererat lord		12
14972,2,f,m,k		# wererat bandit	9
14714,1,f,w,k		# shadow wizard		9
14056,1,u,r,k		# dire rat			1
14972,0.40,f,m,k	# wererat bandit	9
14820,0.30,f,m,k	# wererat			3
14056,0.30,u,m,k	# dire rat			1

party of vermin low		# cr 1/2  ;1;2;
1,2
0,2,3,4,5
14804,1,s,m,k		# centipedes small	1/4
14805,1,s,m,k		# centipedes medium	1/2
14952,1,s,m,k		# spiders small		1/2
14953,1,s,m,k		# spiders medium	1
14089,1,s,m,k		# giant tick		2
14805,0.25,s,m,k	# centipedes medium	1/2
14952,0.25,s,m,k	# spiders small		1/2
14953,0.25,s,m,k	# spiders medium	1
14089,0.25,s,m,k	# giant tick		2

party of vermin mid		# cr 1  ;1;2;21;22;
1,2,21,22
0,3,5,6,7
14047,1,s,m,k		# spiders large		2
14953,1,s,m,k		# spiders medium	1
14417,1,s,m,k		# spiders black wid 2
14806,1,s,m,k		# centipedes large	1
14089,1,s,m,k		# giant tick		2
14047,0.2,s,m,k		# spiders large		2
14953,0.2,s,m,k		# spiders medium	1
14417,0.2,s,m,k		# spiders black wid 2
14806,0.2,s,m,k		# centipedes large	1
14089,0.2,s,m,k		# giant tick		2

party of vermin high [L]	# cr 3 average  ;2;11;
2,11
0,0,5,7,8
14954,1,s,m,k		# spiders huge		5
14047,1,s,m,k		# spiders large		2
14417,1,s,m,k		# spiders black wid 2
14807,1,s,m,k		# centipedes huge	2
14089,1,s,m,k		# giant tick		2
14954,0.2,s,m,k		# spiders huge		5
14047,0.2,s,m,k		# spiders large		2
14417,0.2,s,m,k		# spiders black wid 2
14807,0.2,s,m,k		# centipedes huge	2
14089,0.2,s,m,k		# giant tick		2

phycomid			# cr 4
1,2
4,6,0,0,0
14284,0,s,m,k

quasit				# cr 2  ;2;11;
2,11
2,4,6,0,0
14110,0,s,m,k		# quasit		2

quasit polymorphed	# cr 2 average  ;2;11;
2,11
0,0,6,7,8
14110,4,u,m,k		# quasit		2
14390,0.25,u,m,k	# dire bat		2
14391,0.25,u,m,k	# dire wolf		3
14057,0.25,u,m,k	# giant frog	2
14812,0.25,u,m,k	# f.m.g. cent.	1

flame brother		# cr 3  ;1;2;
1,2
3,5,7,0,0
14384,0,u,m,k

salamander			# cr 6  ;2;11;
2,11
6,8,10,0,0
14111,0,u,m,k

salamander noble [L]	# cr 10  ;11;
11
0,10,12,0,0
14950,0,u,m,k

salamander party [L]	# cr complicated  ;11;22;
11,22
0,0,11,12,13
14950,1,u,m,k		# noble			10
14111,1,u,m,k		# salamander	 6
14384,1,u,m,k		# flame brother	 3
14950,0.33,u,m,k	# noble			10
14111,0.33,u,m,k	# salamander	 6
14384,0.33,u,m,k	# flame brother	 3

shadow				# cr 3  ;1;2;
1,2
3,5,7,8,9
14289,0,s,m,k

shadow greater		# cr 5  ;2;11;21;22;
2,11,21,22
5,7,9,10,11
14941,0,f,m,k

shrieker			# cr none  ;2;
2
1,1,1,1,1
14282,1,s,m,k

skeleton			# cr 1/3  ;1;2;
1,2
1,1,2,3,4
14107,0,s,m,k

skeleton gnoll		# cr 1  ;1;2;
1,2
1,3,5,6,7
14081,0,s,m,k

skeleton and zombie		# cr 1/2, 1/3  ;1;2;
1,2
1,1,2,3,4
14092,0.5,s,m,k		# zombie
14107,0.5,s,m,k		# skeleton

skeleton gathering	# cr 5 average  ;11;21;22;
11,21,22
5,7,9,10,11
14966,1,s,m,k		# skeleton giant		6
14850,1,s,m,k		# skeleton troll		3
14967,1,s,m,k		# skeleton dire wolf	3
14966,1,s,m,k		# skeleton giant		6
14850,1,s,m,k		# skeleton troll		3
14966,0.33,s,m,k	# skeleton giant		6
14850,0.33,s,m,k	# skeleton troll		3
14967,0.33,s,m,k	# skeleton dire wolf	3

spiders black widow		# cr 2  ;1;2;
1,2
2,4,6,7,0
14417,0,s,m,k

spiders medium small and tiny	# cr 1,1/2,1/4   cr 1/2 average  ;1;
1
0,2,3,4,5
14953,0.33,s,m,k
14952,0.33,s,m,k
14951,0.33,s,m,k

spiders large medium and small	# cr 2,1,1/2   cr 1 average  ;1;2;
1,2
0,3,5,6,7
14047,0.33,s,m,k
14953,0.33,s,m,k
14952,0.33,s,m,k

spiders large		# cr 2  ;1;2;
1,2
2,4,6,7,8
14047,0,s,m,k

spiders huge [L]	# cr 5  ;2;11;
2,11
0,5,7,9,10
14954,0,s,m,k

stirge				# cr 1/2  ;2;21;22;
2,21,22
1,2,3,4,0
14182,0,s,m,k

stone golem [L]		# cr 11  ;11;
11
0,11,13,15,0
14982,0,f,m,k

stone golem elder [L]	# cr 16  ;16;
11
0,16,16,16,0
14983,1,f,m,k
14983,1,f,m,k

troglodyte			# cr 1  ;1;2;21;22;
1,2,21,22
1,3,5,6,7
14855,0,u,m,k

troll				# cr 5  ;1;2;11;21;22;
1,2,11,21,22
5,7,9,10,0
14262,0,u,m,k

umber hulk [L]		# cr 7  ;2;11;
2,11
0,7,9,0,0
14260,0,s,m,k

umber hulk horrid [L]		# cr 13  ;11;
11
0,13,15,0,0
14978,0,s,m,k

ustilagor			# cr 4  ;1;2;
1,2
4,6,0,0,0
14283,0,s,m,k

violet fungi		# cr 3  ;1;2;
1,2
3,5,0,0,0
14273,0,s,m,k

wererat				# cr 2  ;1;2;
1,2
2,4,6,0,0
14820,0,u,m,k

werewolf			# cr 3  ;1;2;
1,2
3,5,0,0,0
14344,0,u,m,k

werewolf lord		# cr 14
11
14,0,0,0,0
14944,1,f,b,k

wererat lord		# cr 14
11
14,0,0,0,0
14945,1,f,m,k

will o wisp			# cr 6  ;2;11;21;22;
2,11,21,22
6,8,10,0,0
14291,0,s,m,k

yellow mold			# cr 6  ;1;2;
1,2
6,8,0,0,0
14276,0,s,m,k

zombie				# cr 1/2  ;1;2;
1,2
1,2,3,4,5
14092,0,s,m,k

solo basilisk		# cr 5
99
5,0,0,0,0
14295,1,s,m,k

solo dire rat		# cr 0
99
0,0,0,0,0
14056,0,f,m,o

solo dragon young red	# cr 7
99
7,0,0,0,0
14819,0,u,m,k

solo fire giant		# cr 10
99
10,0,0,0,0
14882,1,f,m,k

solo frost giant	# cr 9
99
9,0,0,0,0
14865,1,f,m,k

solo hill giant		# cr 7
99
7,0,0,0,0
14221,1,f,m,k

solo ogre			# cr 3
99
3,3,0,0,0
14249,1,u,m,k

solo minotaur		# cr 5
99
5,5,0,0,0
14241,1,f,m,k

solo troll			# cr 5
99
3,3,0,0,0
14262,1,u,m,k

solo yellow mold		# cr 6
99
6,6,0,0,0
14276,1,s,m,k

crypt undead		# cr 5  ;11;
99
5,7,9,10,11
14968,1,f,w,k		# lich
14966,1,f,m,k		# skeleton giant	6
14966,1,f,m,k		# skeleton giant	6
14941,1,f,m,k		# greater shadow	8
14941,1,f,m,k		# greater shadow	8
14941,1,f,m,k		# greater shadow	8
14941,1,f,m,k		# greater shadow	8
14941,1,f,m,k		# greater shadow	8
14941,1,f,m,k		# greater shadow	8

orc slave male		# cr 1
99
1,2,3,4,5
14160,0,f,m,o

orc slave female	# cr 1/2
99
0,1,2,3,4
14161,0,f,m,o

orc slave			# cr 1
99
0,1,2,3,4
14160,0.7,f,m,o
14161,0.3,f,m,o

hill giant guard entry
99
0,8,8,8,8
14217,1,f,m,k		# hill giant           7
14217,1,f,m,k		# hill giant           7

hill giant guard alcoves
99
0,10,10,10,10
14219,1,f,b,k		# hill giant F4
14217,1,f,m,k		# hill giant           7
14217,1,f,m,k		# hill giant           7
14217,1,f,m,k		# hill giant           7
14217,1,f,m,k		# hill giant           7
14176,1,f,r,k		# bugbear gang archer  7
14176,1,f,r,k		# bugbear gang archer  7
14176,1,f,r,k		# bugbear gang archer  7
14176,1,f,r,k		# bugbear gang archer  7
14711,1,f,w,k		# bugbear witch W3

hill giant fighter
99
9,9,0,0,0
14219,0,u,m,k		# fighter F4

merrow B1
99
0,3,5,5,5
14108,1,f,b,k		# merrow
14108,1,f,w,k		# merrow
14108,0,f,m,k		# merrow

gnome in library
99
0,12,12,12,12
14786,1,f,w,k		# gnome cabalist
14786,1,f,r,k		# gnome cabalist
14162,1,f,r,k		# hill giantess
14164,1,f,r,k		# frost giantess
14166,1,f,r,k		# fire giantess

gnome in garden
99
0,12,12,12,12
14168,1,f,w,k		# gnome druid
14168,1,f,r,k		# gnome druid
14871,1,f,r,k		# dire tiger
14866,1,f,r,k		# winter wolf
14935,1,f,r,k		# dire bear
14200,1,f,m,o		# gnome daggermaster
14200,1,f,m,o		# gnome daggermaster
14162,1,f,b,o		# hill giantess

drow witch workroom, Not Used, Using MOBs instead
99
0,12,12,12,12
14499,1,f,w,k		# drow witch, W11
14200,2,f,m,k		# gnome daggermaster

fire giant smiths
99
0,12,12,12,12
14882,1,f,m,k		# fire giant		10
14882,1,f,m,k		# fire giant		10
14882,1,f,b,k		# fire giant		10
14898,1,f,r,k		# dire hell hound	6
14898,1,f,w,k		# dire hell hound	6

ogre basher			# cr 9 average 
99
0,11,13,14,0
14065,0,f,m,k		# ogre basher B1/F6

ogre gang B1		# cr 9 average, mostly basher  ;11;
99
0,0,11,13,14
14354,1,f,b,k		# ogre chieftain B12
14356,1,f,w,k		# ogre shaman C5
14176,1,f,r,k		# bugbear gang archer
14065,1,f,m,k		# ogre basher B1/F6
14065,1,f,m,k		# ogre basher B1/F6
14065,0,f,m,k		# ogre basher B1/F6

hill giantess 1
99
0,9,11,12,13
14162,1,f,r,k		# hill giantess
14793,1,f,w,k		# wizard 9
14871,1,f,b,k		# dire tiger
14871,1,f,m,k		# dire tiger
14161,2,f,r,k		# orc slave female

hill giantess 2
99
0,9,11,12,13
14935,1,f,b,k		# dire bear
14162,1,f,w,k		# hill giantess
14162,0.25,f,r,k	# hill giantess
14161,0.25,f,m,k	# orc slave female

hill giant hall
99
15,15,15,15,15
14163,1,f,r,k		# hill giant chief
14162,1,f,r,o		# hill giantess
14786,1,f,r,k		# gnome cabalist
14935,1,f,r,k		# dire bear
14356,1,f,w,k		# ogre shaman C5
14219,4,f,m,o		# hill giant F4
14890,1,f,b,o		# orc general

wererat guards 		# cr 12
99
0,12,0,0,0
14945,1,f,b,k		# wererat lord		12
14972,2,f,m,k		# wererat bandit	9
14972,2,f,r,k		# wererat bandit	9

lich B1				# cr 
99
15,15,15,0,0
14968,1,f,w,k		# lich				15
14714,4,f,m,k		# shadow wizard		9

frost giant hall
99
16,16,16,16,16
14165,1,f,b,k		# frost giant chief
14865,3,f,m,k		# frost giant
14866,1,f,m,k		# winter wolf
14786,1,f,w,k		# gnome cabalist
14176,1,f,r,k		# bugbear gang archer  7
14865,1,f,r,k		# frost giant
14884,2,f,r,k		# dire polar bear

frost giantess 1
99
0,12,12,12,0
14164,1,f,r,k		# frost giantess
14793,1,f,w,k		# wizard 9
14884,1,f,b,k		# dire polar bear
14884,1,f,m,k		# dire polar bear

frost giantess 2
99
0,11,11,11,11
14164,1,f,b,k		# frost giantess
14164,1,f,m,k		# frost giantess
14164,1,f,r,k		# frost giantess

frost giant band
99
0,11,13,14,15
14865,2,f,m,k		# frost giant 9
14884,1,f,m,k		# dire polar bear
14865,1,f,m,k		# frost giant 9
14164,1,f,b,k		# frost giantess
14865,2,f,r,k		# frost giant 9
14866,1,f,r,k		# winter wolf
14065,1,f,r,k		# ogre basher B1/F6
14356,1,f,w,k		# ogre shaman C5	7

ettin herdsmen
99
0,7,7,0,0
14238,1,f,b,k		# ettin		6
14238,1,f,m,k		# ettin		6
14921,3,f,m,k		# fiendish dire boar	5

fire giant guard entry
99
0,12,12,12,12
14882,1,f,m,k		# fire giant		10
14111,1,f,m,k		# salamander
14882,1,f,m,k		# fire giant		10
14111,1,f,m,k		# salamander
14898,1,f,b,k		# dire hell hound	6
14882,1,f,r,k		# fire giant		10
14111,1,f,r,k		# salamander
14898,1,f,w,k		# dire hell hound	6

fire giantess
99
0,12,12,12,0
14166,1,f,b,k		# fire giantess		11
14898,1,f,m,k		# dire hell hound	6
14166,1,f,m,k		# fire giantess		11
14166,1,f,r,k		# fire giantess		11
14712,1,f,w,k		# gnoll shaman, C3

fire toad
99
0,8,0,0,0
14300,1,f,m,k
14300,1,f,m,k
14300,-3,f,m,k

party of lycanthropes B1
99
0,12,13,13,13
14944,1,f,b,k		# werewolf lord		12
14945,1,f,m,k		# wererat lord		12
14714,1,f,w,k		# shadow wizard		9
14797,1,f,m,k		# werebear			11
14798,1,f,m,k		# weretiger			11
14936,1,f,m,k		# dire boar         4
14989,1,f,r,k		# werewolf bandit	9
14989,1,f,r,k		# werewolf bandit	9
14972,1,f,r,k		# wererat bandit	9
14972,1,f,r,k		# wererat bandit	9

gnoll guard
99
0,6,8,9,10
14066,1,f,b,k		# gnoll leader F2
14712,1,f,w,k		# gnoll shaman D3
14712,1,f,r,k		# gnoll shaman D3
14866,6,f,r,k		# winter wolf
14176,4,f,r,k		# bugbear gang archer  7
14079,7,f,m,k		# gnoll warrior B1/F4
14200,4,f,m,k		# gnome daggermaster	11

arena with cryohydra
99
0,0,15,0,0
14973,1,f,r,o		# Twelve-Headed CryoHydra
14160,4,f,r,o		# orc slaves
14176,1,f,r,o		# bugbear gang archer
14176,1,f,r,o		# bugbear gang archer
14176,1,f,w,o		# bugbear gang archer
14065,3,f,m,o		# ogre basher B1/F6
14079,4,f,m,o		# gnoll warrior B1/F4
14219,1,f,b,o		# hill giant F4

frost worm and snakes
99
12,12,0,0,0
14981,1,f,w,k
14385,1,f,b,k
14385,1,f,r,k
14385,1,f,m,k
14385,1,f,m,k

old red dragon
99
18,0,0,0,0
14949,1,f,m,k

drow patrol
99
0,12,12,12,12
14498,1,f,b,o
14498,1,f,m,k
14498,1,f,m,k
14498,1,f,r,o
14178,1,f,w,o

fire giant hall
99
17,17,17,17,17
14167,1,f,b,o		# fire giant chief	15
14166,1,f,m,o		# fire giantess		11
14882,3,f,m,k		# fire giant		11
14898,1,f,m,k		# dire hell hound	6
14945,1,f,m,k		# wererat lord		14
14224,2,f,r,k		# forest troll		
14882,3,f,r,k		# fire giant		11
14786,1,f,w,k		# gnome cabalist

brigand warband TEST 5
99
0,4,6,7,8
14069,1,f,b,k		# Brig Leader R3
14070,2,f,m,k		# Brig Sneak F1/R1
14314,1,f,r,k		# Brig Crossbowman F1
14718,1,f,w,k		# Wizard 3

brigand warband TEST 10
99
0,4,6,7,8
14069,1,f,b,k		# Brig Leader R3
14070,2,f,m,k		# Brig Sneak F1/R1
14866,2,f,m,k		# winter wolf
14313,1,f,r,k		# Brig Archer F1
14314,1,f,r,k		# Brig Crossbowman F1
14313,1,f,r,k		# Brig Archer F1
14314,1,f,r,k		# Brig Crossbowman F1
14718,1,f,w,k		# Wizard 3

brigand warband TEST 16
99
0,4,6,7,8
14069,1,f,b,k		# Brig Leader R3
14070,2,f,m,k		# Brig Sneak F1/R1
14070,2,f,m,k		# Brig Sneak F1/R1
14866,3,f,m,k		# winter wolf
14313,1,f,r,k		# Brig Archer F1
14314,2,f,r,k		# Brig Crossbowman F1
14313,2,f,r,k		# Brig Archer F1
14314,2,f,r,k		# Brig Crossbowman F1
14718,1,f,w,k		# Wizard 3

















"""

import StringIO
def get_dungeon_encs_lines():
	return StringIO.StringIO(dungeon_encs_text)