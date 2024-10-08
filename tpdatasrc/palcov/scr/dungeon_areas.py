dungeon_areas_text="""
#------------------------------------------------------------------------------
# These are all areas for all dungeon levels where an encounter can occur.
# The data is read in by read_areas() in dungeon.py.
#
# Each value below will have its own line in the file:
#
# Dungeon Level
#  - The level of the dungeon that this area applies to.
#      1: B1, Level 1
#      2: B1, Level 2
#     11: Temple, Level 1
#  - Multiple values can be listed, separated by comma.
#
# Area Size
#  - The five area sizes are, in order:
#      0:	1 creature.
#      1:	1d4+1 creatures.	max 5	average 3.5
#      2:	2d4+2 creatures.	max 10	average 7
#      3:	2d6+4 creatures.  	max 16	average 11
#      4:	2d10+4 creatures.	max 24	average 15
#
# Chance of occurrence
#  - Percent chance that this area will have an encounter.
#
# Chance of predetermined
#  - Percent chance that the encounter is predetermined, else it is random.
#  - If rolled, the encounter will be used regardless of party level or EL.
#
# Predetermined list
#  - If the encounter is predetermined, one will be rolled from this list.
#  - Format is a comma separated list of encounter keys from dungeon_encs.txt.
#  - This line is only specified if chance of predetermined is more than 0,
#  - otherwise, it MUST be left blank
#
# Number of creature formations
#  - The number of different possible formations for this area.
#
# Creature formations:
#
#  - A list of x/y/rot locations where the creatures will be created.
#
#  - Multiple lists can be given, and one will be rolled to be used as
#    the actual formation. This is done to provide variation for the area
#    allowing replays of the adventure to be different.
#
#  - The spots in each list are specifically and intentionally ordered to allow
#    for logical placement of the enemies:
#
#      - The first half of the list is where front line creatures will be
#        positioned. Melee creatures will go here.
#        m: Creatures with a Duty value of 'm' in dungeon_encs.txt will go here.
#
#      - The last spot in the first half of the list is where the is where a
#        front-line melee boss will go, such as Hedrack.
#        b: A creature with a Duty value of 'b' in dungeon_encs.txt will go here.
#
#      - The second half of the list is where second line creatures will be
#        positioned. Ranged creatures will usually go here, and also maybe
#        tougher creatures that are waiting until the fodder gets used up.
#        r: Creatures with a Duty value of 'r' in dungeon_encs.txt will go here.
#
#      - The last spot is where a protected 'boss' will spawn. This guy is
#        usually a protected spellcaster, or a leader with a bow. 
#        He is positioned behind the rest of the creatures for protection.
#        Sometimes, this puts him behind a table or other blocked area where
#        he is tough to attack with melee weapons.
#        This can also make it impossible for him to attack with melee himself.   
#        w: A creature with a Duty value of 'w' in dungeon_encs.txt will go here.
#
#      - NOTE: In any given encounter, the quantity of creatures with each
#        duty (m,r,b or w) will not always match up with the number of spots
#        reserved for each of them, and in most cases actually won't.
#        This is handled by putting the excess guys from one duty into spots
#        reserved for other duties. For Example...
#
#        An encounter has 10 orcs, and the encounter area has 10 spots reserved
#        for them. All good. There are 4 spots for melee guys, 4 spots for ranged
#        guys, 1 spot for a front boss, and 1 spot for a rear boss.
#        But, lets say the orcs have 9 melee grunts ('m'), and 1 archer ('r').
#        How are the 9 melee guys ('m') going to fit into only 4 melee spots?
#
#        Answer: The first 4 melee guys will be placed at the 4 spots reserved
#        for melee guys, and the remaining 5 melee guys will placed in spots
#        reserved for creatures with other duties. So 4 of the remaining will
#        be placed in the ranged spots, and 1 will be placed in a boss spot.
#        The logic in get_spot() handles this.
#
#        So, when creating an encounter in dungeon_encs.txt, list the duties with
#        the fewest number of creatures first, so the overflow from too many
#        creatures with another duty don't steal the spots from creatures
#        with the fewest duties. So if you have an encounter with a front boss,
#        a rear boss, 2 archers, and 6 melee guys, then list them in that order.
#
#    - The function get_spot() is called for each creature, and attempts to
#      place it sensibly. See the comments for get_spot() for details.
#
# Number of treasure formations
#
# Treasure locations
#
#  - A list of x/y/rot locations where the container will be created.
#
#  - Multiple lines can be given, and one will be rolled to be used as
#    the actual formation. This is done to provide variation for the area
#    allowing replays of the adventure to be different.
#
#  - A fourth argument can be given. A list of container proto numbers
#    numbers. The container will be rolled from that list.
#    If this is omitted, then the container will be rolled from all possible
#    containers, except for special ones like hidden shelf and anvil.
#
#    Proto	Name			description				rotation facing forward
#    -----	-------------	------------------		----------------------- 
#    1001	Treasure Chest	medium, medium brown	0.0
#    1002	Gold Chest		small, gold and shiny	3.1
#    1003	Footlocker		small, dark brown		3.1
#    1004	Chest			medium, dark brown		3.1
#    1005	Large Chest		large, light brown		3.1
#    1006	Fine Chest		medium, gold and shiny	3.1
#    1028	Well			large					n/a
#    1029	Book			tiny					1.6 (spine facing)
#    1030	Anvil			large					n/a
#    1031	Rock			large					3.1
#    1045	Hidden Shelf	medium					3.1 (2 metal dots facing)
#    1046	Ivory Chest		small, ivory			3.1
#    1049	Barrel			large					3.1
#
#    0.0  0.8  1.6  2.4  3.1  3.9  4.7  5.5  0.0
#
#-----------------------------------------------------------------------------


###############################################################################
#  B1 LEVEL 1
###############################################################################

# NOTE: 10 of the 38 areas do NOT have treasure spots, in B1 LEVEL 1.
# Usually, it's because treasure is already in the room before random treasure. 

# 0. Entry
1
0
100
100
solo dire rat,
1
490,611,3.5
0
0

# 1. Alcoves
1
2
100
100
party of vermin low
1
496,546,1.8, 493,546,2.8, 496,538,0.8, 493,539,4.3, 494,542,2.4, 493,536,3.1, 496,534,0.0, 493,531,3.9, 496,531,0.8, 494,527,2.4
2
490,515,0.0,(1049,)
489,533,2.4,(1003,)
0

# 2. Kitchen
1
1
50
100
solo yellow mold
1
518,494,3.9, 509,496,5.5, 509,494,0.8, 513,494,5.5, 514,492,2.4
2
521,491,3.1,(1003,1049,1049,1049)
515,491,0.8,(1046,)
0

# 3. Dining Room
1
1
50
100
solo ogre,party of archers,hobgoblin warband
1
479,493,3.7, 472,500,2.4, 474,495,5.5, 470,497,3.1, 473,492,3.1
2
469,503,4.0,(1002,1003)
485,491,0.8,(1002,1003)
1
473,494,2.8

# 4. Lounge
1
1
25
0
1
473,521,6.0, 472,524,2.4, 471,521,5.2, 473,526,0.8, 470,526,3.9
1
468,527,3.9,(1003,1004,1046)
0

# 5,6,7. Zel Bedroom
1
3
50
0
2
480,452,1.0, 480,458,0.8, 472,468,1.6, 470,470,4.7, 484,463,1.6, 482,465,5.5, 485,467,5.5, 480,455,0.8, 483,452,1.1, 486,452,0.8, 483,455,0.8, 483,458,0.8, 486,458,0.8, 472,475,5.5, 473,473,5.5, 486,455,1.1
466,458,1.6, 469,461,0.8, 467,455,1.6, 455,457,3.9, 455,460,3.9, 469,457,1.6, 472,460,1.0, 463,457,2.0, 468,452,1.8, 470,455,3.1, 473,457,1.6, 472,453,3.1, 457,452,2.4, 475,453,1.6, 479,454,0.8, 459,452,2.6
2
451,459,3.9,(1002,1003,1004,1005,1006,1031,1046)
473,452,2.4,(1002,1003,1004,1005,1006,1031,1046)
0

# 8. Zel Workroom
1
1
100
100
shadow
1
505,468,4.7, 513,466,1.0, 511,468,6.0, 516,472,0.0, 509,474,5.5
2
511,472,3.9,(1003,1004)
518,466,2.4,(1003,1004)
0

# 9. Zel Lab
1
1
0
0
1
516,450,1.0, 506,449,3.9, 513,452,6.0, 517,455,0.0, 510,455,2.4
2
504,458,3.9,(1002,1003,1004,1006,1046)
517,449,0.8,(1002,1003,1004,1006,1046)
0

# 10. Storage Room
1
1
75
0
1
536,470,6.0, 535,460,2.4, 536,463,6.0, 539,474,5.8, 543,468,0.8
1
546,470,1.0,(1049,)
0

# 11. Supply Room
1
1
75
0
1
558,452,1.8, 558,456,5.5, 554,453,2.4, 551,453,2.9, 555,450,2.4
1
549,453,4.1,(1004,)
0

# 12. Library
1
2
50
0
1
523,426,3.9, 531,423,0.8, 528,420,2.4, 525,420,3.1, 526,424,5.5, 524,414,4.7, 531,417,1.8, 523,418,2.6, 518,420,0.8, 526,412,1.6
1
523,412,3.1,(1003,1004)
0

# 13. Portcullis Room Hall
1
0
90
0
1
537,403,0.8
0
0

# 14. Auxiliary Storeroom
1
0
50
0
2
564,379,2.4
558,378,3.9
1
555,379,3.9
0

# 15 Teleport Room, North
1
0
0
0
1
557,400,2.3
0
0

# 15, 16. Teleport Rooms Hall
1
2
75
0
1
562,418,0.8, 567,409,5.5, 560,415,0.8, 569,409,5.5, 566,418,3.4, 569,413,5.5, 563,415,1.0, 566,414,0.9, 569,416,2.4, 569,420,0.0
0
0

# 17, 18, 19. Smithy
1
2
75
0
2
557,506,2.4, 556,509,5.3, 558,497,2.4, 553,507,3.9, 560,506,3.9, 553,510,4.4, 550,509,3.9, 551,512,2.4, 551,515,5.5, 552,518,4.7
565,508,5.3, 562,511,3.9, 566,511,0.8, 559,511,0.0, 562,508,4.5, 564,513,5.5, 560,516,4.7, 562,515,1.4, 556,509,3.1, 565,517,0.0
3
548,509,3.9,(1003,1004)
558,517,3.9,(1006,1046)
556,498,3.9,(1049,)
0

# 20. Dead End Room Hallway
1
1
50
0
1
539,551,3.5, 539,553,4.0, 542,552,0.8, 535,551,3.5, 536,553,4.6
0
0

# 20. Dead End Room
1
1
100
0
1
542,569,3.1, 541,571,3.8, 545,572,0.6, 545,567,2.4, 543,562,2.6
2
540,566,3.9,(1003,1004,1046)
541,560,3.14,(1003,1004,)
0

# 21. Meeting Room
1
1
90
0
1
515,557,3.0, 515,554,2.7, 517,558,0.5, 517,554,2.2, 517,552,2.4 
1
515,581,3.9,(1003,)
0

# 22. Garden, left side
1
0
100
100
ascomid,basidirond,hydra [L],phycomid,shadow,shrieker,ustilagor,violet fungi
1
550,543,0.8
0
0

# 22. Garden, main body
1
2
50
0
1
519,515,2.5, 527,518,2.3, 517,517,3.1, 527,515,2.0, 519,520,0.0, 519,507,0.8, 522,511,2.4, 525,511,2.2, 517,513,3.9, 517,507,2.5
1
527,509,0.0,(1031,)
0

# 23. Storage Room
1
1
75
0
1
431,581,3.1, 431,585,4.0, 428,579,1.6, 425,585,3.6, 426,581,4.7 
1
432,572,2.4,(1005,1046)
0

# 24. Mistress Room
1
2
50
0
2
415,577,2.3, 414,573,5.5, 409,576,2.8, 413,575,2.7, 411,576,2.0, 417,576,3.9, 416,573,2.6, 418,573,2.2, 419,576,0.8, 420,573,2.2 
415,582,2.3, 411,574,1.6, 416,585,5.8, 409,576,4.7, 414,580,2.7, 418,583,3.3, 414,574,5.5, 414,577,0.5, 418,573,3.1, 418,576,1.2 
1
408,581,3.9,(1003,1004)
0

# 25. Rogahn's Bedroom
1
2
50
0
1
416,538,4.3, 415,534,3.9, 407,541,4.2, 415,529,3.3, 412,539,1.3, 409,537,2.4, 412,531,1.3, 409,531,3.2, 408,527,3.4, 411,534,5.8 
2
406,532,4.2,(1003,1004,1005,)
418,523,2.2,(1046,)
0

# 26. Trophy Room
1
1
100
100
solo basilisk,
2
470,540,3.4, 470,542,4.8, 468,543,1.0, 473,543,0.0, 472,540,1.50
461,543,0.0, 455,543,2.7, 457,541,3.0, 461,540,2.7, 452,541,3.0
1
467,539,2.4,(1003,1004)
0

# 27. Throne Room
1
2
100
0
1
426,557,3.0, 426,560,4.5, 422,551,1.0, 422,561,6.2, 423,557,3.4, 421,559,2.5, 414,563,4.7, 414,561,4.1, 419,552,4.0, 414,557,3.9
1
413,566,3.9,(1005,1006,1046)
0

# 28. Worship Room
1
1
25
0
1
451,557,2.1, 449,561,4.8, 448,558,3.1, 454,556,1.4, 453,560,6.2
1
447,556,3.0,(1003,1004)
0

# 29. Captain's Room
1
1
50
0
1
453,590,5.0, 453,587,3.4, 450,588,0.0, 450,586,1.3, 447,587,4.0
1
450,590,3.9,(1003,1004)
0
 
# 29. Captain's Room 2
1
1
50
0
1
451,577,5.0, 450,575,3.4, 448,578,5.5, 446,576,4.2, 447,574,3.3
1
456,574,2.4,(1003,1004)
0

# 30. Stairs
1
0
100
100
solo dire rat,
1
463,558,2.4
0
0

# 31. Pool Room, North
1
0
50
0
1
427,462,2.4
0
0

# 31. Pool Room, South
1
0
50
0
1
448,521,5.5
0
0

# 32. Advisor Room
1
0
25
0
1
412,505,4.0
0
0

# 33. Barracks
1
3
100
0
1
412,463,0.0, 409,463,5.6, 416,469,2.4, 409,468,0.0, 409,473,5.6, 415,472,4.6, 417,472,0.4, 408,466,2.5, 414,475,5.9, 407,470,5.0, 416,473,0.9, 404,470,3.9, 411,480,3.2, 410,477,2.4, 414,478,5.6, 408,479,4.6 
1
406,482,5.5,(1003,1004,1005,1049)
0

# 35. Guest Room 1
1
1
50
0
1
415,422,2.1, 414,428,4.8, 416,425,0.8, 413,425,3.7, 410,426,4.1 
2
408,429,3.9,(1003,1004,1005,1049)
412,423,3.9,(1003,1004,1005)
0

# 35. Guest Room 2
1
1
50
0
1
417,407,4.4, 416,400,3.3, 415,404,3.9, 409,406,4.3, 412,404,1.3
2
418,399,2.4,(1003,1004,1005,1049)
409,409,4.7,(1005,1046)
0

# 35. Guest Room 3
1
1
50
0
1
415,379,3.0, 413,381,3.9, 416,385,4.8, 412,384,0.3, 409,382,3.3
2
408,388,3.9,(1003,1004,1005,1049)
412,379,3.9,(1003,1004,1005)
0

# 36. Empty Room Near Pit Trap
1
2
75
0
1
442,433,3.2, 449,436,1.3, 446,432,0.0, 450,433,1.9, 446,435,0.3, 443,431,4.1, 450,428,3.4, 452,431,5.9, 453,428,2.0, 445,428,2.0
1
459,427,1.6
0

# 37. Rec Room
1
2
50
0
2
492,433,2.7, 494,429,1.5, 492,430,4.6, 490,435,3.6, 495,434,1.7, 496,426,2.4, 493,422,3.7, 492,426,3.1, 488,431,3.3, 496,422,0.7
479,433,3.2, 480,425,3.8, 483,424,1.8, 479,436,4.1, 478,430,4.1, 477,427,3.5, 477,424,4.8, 480,421,0.9, 476,434,4.0, 477,421,3.3 
2
473,427,3.9,(1005,)
478,419,2.4,(1003,1004)
0


###############################################################################
#  B1 LEVEL 2
###############################################################################

# 38. Access Room
2
3
100
0
1
470,553,3.2, 482,552,1.7, 469,556,4.8, 485,556,1.0, 474,560,4.9, 482,559,5.5, 469,561,4.9, 477,558,5.5, 464,553,3.6, 462,558,5.3, 467,551,1.6, 485,560,6.2, 460,550,4.1, 480,563,0.0, 477,563,4.9, 460,555,3.5
1
482,549,2.4,(1046,)
0

# 39. Museum
2
1
100
0
1
447,561,5.6, 436,567,0.4, 433,564,2.4, 447,558,2.3, 433,567,3.9 
1
431,565,3.9,(1003,1004,1005)
0

######## 40. Secret Room, NO ENCOUNTER, ROOM IS ON ANOTHER MAP #######

# 41. Cavern
2
1
100
0
1
483,608,5.0, 480,612,5.8, 481,601,4.2, 478,604,3.4, 477,608,3.8
1
475,605,3.9,(1003,1004,1005)
0

# 42. Webbed Cave, NO ENCOUNTER, SCRIPTED ENCOUNTER WITH WEBS IS THERE

# 43. Cavern
2
1
75
0
1
548,548,2.6, 546,550,3.7, 547,544,2.7, 544,545,3.5, 550,546,2.0
1
544,543,3.5,(1046,)
0

# 44. Cavern
2
0
100
0
1
497,557,3.3
1
494,559,4.7,(1003,1004)
0

# 45. Mystical Stone, NO ENCOUNTER, LEAVING EMPTY FOR MOOD

# 46, 47: Cavern
2
2
100
0
1
472,528,3.5, 471,534,4.5, 470,530,4.3, 475,535,5.0, 473,531,4.5, 468,532,2.4, 465,528,3.7, 468,527,1.0, 468,535,6.1, 464,532,4.1
1
463,529,3.9,(1003,1004)
0

# 48. Arena Cavern, NO ENCOUNTER, SCRIPTED ENCOUNTER IS THERE

# 49. Phosphorescent Cave, NO ENCOUNTER, ROOM IS NOT ON MY MAP
 
# 50. Water Pit, NO ENCOUNTER, SCRIPTED ENCOUNTER IS THERE

# 51. Side Cavern, Blue Walls
2
1
100
0
1
408,509,3.0, 399,516,5.5, 399,520,0.0, 399,512,3.0, 395,518,3.9
1
393,514,3.5,(1005,1006,1046)
0

# 52. Raised Cavern
2
2
100
0
1
412,435,3.1, 411,438,3.6, 410,449,3.6, 413,452,5.7, 410,442,4.0, 408,435,1.4, 405,437,4.7, 407,445,4.1, 414,458,4.6, 406,440,4.0
2
402,436,3.7,(1002,1003,1004,1005,1006,1046)
407,433,2.8,(1046,)
0

# 53. Grand Cavern of the Bats 1
2
2
100
0
1
489,397,5.0, 478,405,1.9, 486,393,4.0, 473,400,3.0, 483,401,6.1, 485,398,4.8, 478,401,2.4, 480,392,4.5, 475,395,2.4, 480,398,3.0
1
483,409,6.0,(1002,1003,1004,1005,1006,1046,1049)
0

# 53. Grand Cavern of the Bats 2
2
1
100
0
1
522,423,1.5, 515,410,0.1, 514,417,2.2, 521,413,3.3, 525,416,0.2
1
519,416,5.2,(1031,)
0

# 54. Treasure Cave, NO ENCOUNTER, STATUE ENCOUNTER IS THERE 

# 54. Treasure Cave, nearby hall Solo
2
0
75
0
3
573,446,1.6
562,453,1.9 
550,452,3.2 
0
0

# 55. Exit Cave
2
1
100
0
2
585,402,2.0, 574,400,4.9, 579,398,0.8, 575,394,3.0, 581,392,2.4
569,415,5.8, 568,410,2.7, 567,404,1.8, 564,406,3.7, 564,398,3.0
1
572,394,2.8,(1003,1004)
0

# 56. Cavern of the Statue
2
2
100
0
2
577,513,5.7, 584,506,6.1, 585,503,1.3, 563,513,5.2, 579,511,0.5, 583,514,6.0, 587,508,0.4, 580,515,6.0, 559,513,4.5, 585,512,1.3
564,489,1.0, 559,495,0.3, 568,490,0.4, 565,501,6.2, 562,498,3.0, 567,486,1.7, 566,505,6.0, 568,502,5.8, 570,500,0.7, 570,488,1.3
2
578,518,5.2,(1003,)
583,496,1.5,(1003,1004)
0

# 56 A. Cavern of the Statue, nearby nook Solo
2
0
100
0
1
555,517,1.2
0
0

# 56 B. Cavern of the Statue, large cavern south
2
3
100
0
1
570,535,1.6, 571,540,2.4, 574,542,1.3, 565,528,1.0, 567,533,2.1, 570,530,1.4, 556,560,5.2, 566,537,4.2, 578,534,1.8, 576,536,5.2, 577,526,0.4, 574,524,2.7, 569,527,2.0, 574,528,5.2, 574,533,0.9, 578,530,1.4
1
580,528,0.8,(1003,1004)
0



###############################################################################
#  TEMPLE LEVEL 1
###############################################################################

# earth temple
11
4
100
0
1
485,418,2.4, 481,418,2.8, 478,419,3.1, 488,419,1.7, 490,421,1.7, 476,421,3.5, 475,424,3.4, 491,424,1.5, 492,427,1.0, 475,427,3.8, 474,430,4.0, 484,422,2.6, 483,415,2.2, 479,416,3.0, 487,416,1.9, 491,418,1.6, 475,418,3.1, 473,422,3.5, 493,422,1.4, 494,425,1.2, 472,425,3.8, 471,418,2.3, 495,418,1.7, 483,411,2.2
5
482,409,2.3,(1031,)
488,404,4.0,(1031,)
467,412,3.1,(1031,)
497,437,0,(1031,)
470,429,4.0,(1031,)
1
487,417,2.2, 480,417,2.6, 491,419,1.7, 475,419,3.1, 492,424,1.3, 474,424,3.8, 492,430,0.8, 474,429,3.8, 475,414,2.8, 491,414,1.8, 495,416,1.7, 471,420,3.4, 496,421,1.5, 470,424,3.6, 496,427,0.9, 483,412,2.4   

# earth temple upper left
11
2
50
0
1
482,388,1.6, 492,388,3.2, 495,383,4.8, 478,384,1.6, 487,390,2.4, 489,385,2.4, 485,385,2.4, 483,381,1.6, 492,381,4.2, 487,381,2.4
2
490,381,2.4,(1031,)
480,381,2.4,(1031,)
1
480,389,1.6, 493,388,3.4, 487,390,2.3, 489,384,2.6, 484,383,2.3

# earth temple upper right
11
0
50
0
1
470,396,3.9
1
467,396,3.9,(1031,)
0

# bugbear office
11
0
50
50
wererat lord
1
520,417,5.0
1
525,418,0.8,(1002,1003,1004,1005,1006,1046)
0

# top left room level 1
11
2
75
0
3
534,398,1.5, 530,397,1.8, 531,400,1.5, 526,397,2.0, 527,400,2.0, 537,397,1.5, 536,394,1.5, 538,400,1.5, 540,398,1.5, 540,394,1.5
537,392,2.4, 542,395,2.4, 534,395,2.4, 537,395,2.4, 537,398,2.4, 538,389,2.4, 536,389,2.4, 540,391,2.4, 534,391,2.4, 537,386,2.4
536,397,1.5, 536,400,1.0, 539,398,1.5, 532,400,1.0, 532,397,1.5, 539,394,2.4, 534,394,2.4, 539,391,2.4, 534,391,2.4, 537,388,2.4
2
540,386,2.35
533,386,2.35
1
540,396,0.5, 540,392,1.5, 533,399,0.8, 534,389,2.7, 539,388,1.7  

# cell room
11
2
75
0
1
549,420,0.0, 549,426,1.6, 556,418,0.0, 557,435,1.5, 546,416,5.5, 553,426,1.6,  553,420,0.0, 551,423,0.8, 556,428,1.6,  556,423,0.8
4
556,407,1.6
556,414,0.8
556,426,0.8
556,432,0.8
1
547,418,5.5, 548,432,2.8, 547,424,3.8, 554,428,0.3, 555,423,0.8

# cell 1
11
0
33
0
1
537,416,3.8
1
537,413,2.3  
1
537,416,3.8

# cell 2
11
0
33
0
1
537,424,3.8
1
533,422,3.1  
1
537,424,3.8

# cell 3
11
0
33
0
1
537,431,3.8
1
535,429,2.3 
1
537,431,3.8

# cell 4
11
0
33
0
1
537,439,3.8
1
535,437,2.3 
1
537,439,3.8

# torture chamber
11
1
50
0
3
571,464,5.4, 567,464,5.4, 569,461,5.4, 572,467,5.4, 568,467,5.4 
564,457,0.8, 561,456,0.8, 557,457,0.8, 567,456,0.8, 570,457,0.8
567,462,5.4, 572,460,0.0, 565,456,0.8, 572,467,0.0, 567,468,5.4
3
570,455,2.35
560,466,3.1
570,473,3.9
2
567,457,0.8
569,466,5.3

# snake room
11
2
75
0
1
546,495,5.8, 547,483,3.7, 551,485,5.6, 543,492,1.2, 543,486,0.3, 550,489,5.1, 548,493,1.1, 543,489,1.7, 546,486,4.7, 541,483,3.3 
6
548,482,2.4
544,482,2.4
539,482,3.9
539,487,3.9
539,492,3.9
539,497,3.9
1
550,483,5.3, 543,496,0.8, 549,494,5.8, 552,488,1.2, 544,485,3.1

# picnic table room
11
4
100
0
1
562,489,103.9, 566,498,101.6, 561,491,103.9, 562,496,101.6, 569,501,100.8, 572,503,100.9, 561,482,105.5, 573,518,100.3, 561,507,103.9, 572,507,100.8, 568,518,105.5, 561,510,103.8, 568,491,101.6, 571,485,100.0, 570,493,101.6, 570,488,100.8, 568,483,100.0, 567,487,100.8, 573,492,101.6, 566,495,101.6, 570,497,100.8, 573,498,100.9, 562,486,103.9, 573,489,100.8
3
574,487,0.8
560,486,3.9
560,493,3.9
1
561,491,103.9, 562,496,101.6, 569,501,100.8, 561,482,105.5, 573,518,100.3, 561,507,103.9, 572,507,100.8, 561,511,103.8, 570,493,101.6, 568,483,100.0, 567,487,100.8, 566,495,101.6, 570,497,100.8, 567,514,105.6, 562,486,103.9, 573,489,100.8

# picnic table room south
11
0
50
0
1
566,526,0.8
4
569,524,0.8
569,527,0.8
560,521,3.9
560,525,3.9
1
566,526,0.8

# ghoul room
11
2
75
0
1
551,551,3.9, 546,533,5.2, 550,539,0.0, 553,547,3.1, 552,542,3.8, 546,546,3.5, 546,538,5.5, 549,544,3.9, 546,552,4.7, 546,542,3.9
6
544,536,3.9
544,544,3.9
544,549,3.9
548,554,5.5
553,554,5.5
556,541,2.4
1
551,551,3.9, 546,533,5.2, 552,542,3.8, 547,547,3.5, 546,542,3.9

# pirate cell
11
1
50
0
1
547,577,5.5, 550,577,5.5, 548,574,5.5, 550,580,5.5, 547,580,5.5
2
544,572,3.1
544,576,3.9
1
549,577,5.5

# Keep these next 4 rooms at 100%, so keys will always have a container available.

# south hall - room 1
11
1
75
0
1
562,601,0.8, 562,598,1.6, 559,600,0.8, 565,599,0.8, 568,599,0.8
3
566,595,2.4
562,595,2.4
564,602,5.5
1
564,596,2.4

# south hall - room 2
11
1
75
0
1
508,600,3.9,  508,603,4.7,  511,600,3.9,  505,603,4.4,  505,600,3.9
5
503,602,3.9
503,605,3.9
504,607,5.5
506,607,5.5
509,605,5.5
1
505,605,4.7

# south hall - room 3
11
1
75
0
1
463,600,0.8, 462,603,0.0, 460,600,0.8, 465,603,0.0, 466,601,0.8 
4
460,604,3.9
461,607,5.5
463,606,0.8
467,603,0.8
1
462,604,5.9

# south hall - room 4
11
1
75
0
1
406,597,3.2, 406,600,4.3, 409,599,3.9, 403,596,3.2, 403,599,3.9
3
401,599,3.9
404,601,5.5
408,601,5.5
1
406,594,2.7

# harpy room lower
11
1
75
0
2
413,565,5.3, 413,551,2.4, 413,562,5.3, 413,548,2.4, 413,568,5.3 
410,566,5.5, 402,566,5.5, 406,563,5.5, 406,566,5.5, 406,569,5.5
4
398,565,3.9
398,569,4.7
402,569,5.5
408,569,5.5
1
407,559,4.7

# harpy room upper
11
1
75
0
1
399,544,105.0, 409,535,100.9, 405,543,106.1, 401,534,103.0, 399,537,104.0
3
404,534,2.4
399,535,3.2
398,541,3.9
1
401,535,103.0

# harpy ante 1
11
1
100
50
skeleton gathering
1
416,518,102.4, 408,518,103.1, 419,523,100.8, 409,525,104.0, 413,524,102.0
4
419,519,1.6
412,518,2.4
406,519,3.2
405,522,3.9
1
409,525,3.1

# harpy ante 2
11
1
50
0
1
419,546,0.0, 421,541,5.5, 419,543,0.6, 424,543,0.6, 422,547,0.0  
3
419,540,2.4
425,545,0.8
425,541,0.8
1
421,544,0.0

# harpy ante 3
11
1
50
0
1
421,567,0.8, 418,572,5.9, 419,569,0.4, 423,570,0.6, 421,573,0.0
3
416,571,3.9
416,574,4.7
424,573,0.8
1
421,569,0.6

# throne room
11
2
100
0
1
412,489,202.2, 405,489,202.6, 428,515,100.0, 417,494,201.5, 409,488,202.4, 400,491,203.1,  404,484,202.7,  430,520,100.0,  414,484,202.1, 401,488,203.1
3
412,480,2.4
403,482,3.3
401,485,3.3
1
417,494,201.5, 428,515,100.0, 409,488,202.4, 414,484,202.1, 404,484,202.7

# secret room
11
2
50
0
1
413,460,0.0, 400,458,3.9, 402,454,4.5, 413,457,1.0, 411,454,0.0, 402,462,5.6, 410,464,6.0, 400,461,4.5, 412,462,0.5, 406,465,5.4
1
400,455,3.5
1
400,458,3.9, 411,456,0.0, 402,454,4.5, 410,464,6.0, 404,465,5.1

# top-right room
11
2
100
0
1
385,401,3.2, 389,401,2.4, 382,406,3.6, 402,414,100.8, 392,403,2.4,    389,397,2.4, 385,397,2.4, 385,393,2.4, 412,414,100.8, 388,392,2.4  
2
379,391,3.9,(1003,)
386,385,2.4,(1003,)
# 392,386,2.4
# 380,400,3.9
# 395,388,0.8
# 395,397,0.8
1
382,401,3.6, 402,414,100.8, 392,398,2.4, 385,396,2.6, 386,391,2.4

# top-right room 2
# TRIGGERS WHEN PARTY GETS WITHIN 10' OF GUARD
# 100% predeterminde encounted, Lich, Skel Giants and Greater Shadows
11
2
100
100
crypt undead
1
390,387,102.4, 383,386,102.6, 380,387,103.5, 380,391,103.5, 380,396,104.0, 380,400,104.0, 380,404,104.5, 380,408,104.5, 386,387,102.5, 386,387,102.5 
0
0

# spider room
11
1
50
0
1
438,388,2.4, 434,390,3.2, 441,390,2.4, 435,385,3.0, 439,384,2.4
2
439,381,2.4
431,386,3.9
2
434,387,2.8
432,391,4.5

# wonilon
11
1
50
0
1
417,375,2.4, 413,377,3.2, 417,381,2.4, 421,377,0.8, 417,372,2.4
1
419,372,1.6
2
417,375,2.4
421,377,0.8

# library
11
2
75
0
1
428,440,0.0, 418,440,3.2, 434,436,4.5, 427,443,2.4, 431,438,0.0, 427,436,3.2, 430,432,4.1, 422,437,2.4, 429,428,3.9, 424,433,3.2  
3
426,431,1.6
424,430,3.2
416,437,3.2
1
431,438,0.0, 418,440,3.2, 427,443,2.4, 430,432,4.1, 424,433,3.2 

# romag office
11
1
50
0
1
445,442,4.1, 442,448,3.2, 446,446,2.8, 441,444,4.3, 441,440,3.9 
2
440,438,2.4
440,446,3.9
2
449,448,2.4
446,440,3.9

# romag troops
11
3
100
0
1
455,462,3.2, 447,458,5.2, 457,469,3.9, 455,466,3.4, 440,451,104.0, 465,482,104.0, 441,449,105.0, 453,458,0.2, 451,465,5.6, 445,461,5.0, 447,465,5.4,  441,459,4.4, 464,483,104.6, 442,439,102.5, 457,456,102.0, 450,461,5.6 
3
453,455,2.4
439,459,3.9
443,469,3.9
1
455,463,3.2, 447,458,5.2, 457,468,3.9, 465,482,104.0, 453,458,0.2,    445,463,5.4,  441,459,4.4,  448,469,5.5,    441,449,105.0, 451,465,5.6

# romag troops lower
11
0
100
0
1
466,479,3.2
1
463,477,3.9
1
466,479,3.2

# earth troop room
11
2
75
0
1
442,505,2.0, 446,499,3.9, 438,506,2.4, 445,503,4.6, 455,506,104.6,  441,501,3.2, 438,502,2.4, 443,498,4.7, 438,491,2.5, 438,498,3.2
3
437,489,3.9
437,500,3.9
437,504,3.9
1
439,506,2.2,  447,499,3.9, 455,506,104.6, 443,503,3.2, 439,499,3.2

# big room right
11
3
100
0
1
460,539,1.6, 468,536,3.9, 447,535,6.2, 475,541,0.0, 444,531,0.3, 465,540,0.8, 471,533,3.5, 457,536,2.2, 453,534,2.6, 463,535,3.2, 466,532,3.5, 446,542,3.9, 444,539,5.5, 460,531,3.5, 450,532,0.8, 455,530,2.4 
4
452,529,2.4
462,529,2.4
443,535,3.9
443,541,3.9
1
460,539,1.6, 468,539,3.9, 447,535,6.2,   475,541,0.0,  454,536,2.2, 463,535,3.2,  450,532,0.8, 466,532,3.5,  446,540,3.9, 456,531,2.4 

# big room left
11
3
100
0
1
512,538,1.6, 509,535,0.8, 514,541,1.6, 508,551,3.8,  492,530,3.1,  486,526,102.4,  508,567,105.5, 510,542,1.9, 516,537,1.8, 513,534,1.6, 519,539,1.8, 516,532,1.6, 508,531,1.0, 502,564,105.4,  511,531,1.6, 519,535,1.6
4
518,529,2.4
514,529,2.4
521,537,0.8
521,533,0.8
1
508,540,1.9, 492,530,3.1, 508,551,3.8, 502,564,105.4, 507,534,0.4, 513,537,1.6, 519,539,1.8, 511,531,1.6, 486,526,102.4,  518,531,1.6 

# vapor rat room
11
1
50
0
1
455,571,3.2, 451,574,3.5, 456,575,3.2, 448,572,3.5, 452,568,3.2
2
450,569,3.2
448,577,4.8
0

# small room 1
11
1
50
0
1
519,573,0.9, 518,578,0.0, 515,575,0.7, 523,578,0.6, 524,574,0.8
4
525,572,2.4
522,572,2.4
512,577,3.9
526,576,0.8
0

# small room 2
11
0
50
0
5
497,572,3.7
498,576,4.1
499,578,4.8
502,578,5.3
500,574,4.8
3
499,571,2.4
496,574,3.9
496,577,3.9
0

# bugbear 3-door room
11
1
75
0
1
504,473,3.9, 504,481,2.4, 506,477,3.9, 500,479,3.9, 500,475,3.9
2
501,471,2.4
497,477,3.9
0

# south stairs alcove
11
1
50
0
1
486,580,2.4, 482,577,2.6, 481,583,3.2, 488,576,2.2, 484,574,2.4
3
480,573,3.9
480,575,3.9
489,574,0.8
0

# hall, stairs up, SW
11
1
50
0
1
546,588,1.4, 546,591,0.0, 543,589,4.0, 550,591,0.0, 551,588,1.4 
0
1
546,590,1.4

# hall, stairs up, SE
11
1
50
0
1
424,590,4.0, 420,588,2.0, 421,591,6.0, 424,588,3.9, 416,589,4.0
0
1
416,589,4.0

# hall, near ghoul room, SW
11
1
50
0
1
535,551,5.5, 536,557,2.2, 534,557,2.4, 536,554,6.0, 534,554,5.5 
0
1
535,551,5.5

# hall, near bugbear guard room, W
11
1
50
0
1
520,494,2.5, 518,488,5.5, 525,486,4.7, 522,488,1.7, 520,491,5.0 
0
1
522,488,1.7

# hall, near bugbear guard room, NW
11
1
50
0
1
523,437,2.3, 522,431,5.5, 525,432,6.0, 522,434,3.0, 525,435,2.0
0
1
525,435,2.0

# hall, long hall leading to stairs, NW
11
2
75
0
1
565,425,102.0, 568,425,102.5, 565,441,105.8, 568,441,105.4, 566,413,2.5, 565,410,2.4, 567,409,2.4, 567,406,2.3, 565,405,2.4, 566,402,2.5 
0
1
565,425,102.0, 565,441,105.8, 566,413,2.5, 567,409,2.4, 566,402,2.5

# hall, off earth temple, NW
11
1
50
0
1
507,390,4.0, 507,383,5.5, 509,388,2.4, 511,386,3.0, 514,388,1.0
0
1
509,388,2.4

# hall, stairs down to level two
11
2
50
0
1
488,461,4.0, 480,461,0.7, 479,464,6.2, 489,464,4.7, 484,466,1.9, 481,468,4.7, 487,469,4.9, 486,475,4.8, 482,475,5.9, 483,472,5.6
0
1
479,464,6.2, 489,464,4.7, 484,466,1.9, 484,461,4.0, 483,472,5.6

# hall, off earth temple, NE
11
1
50
0
1
455,399,5.5, 453,408,2.3, 455,406,5.9, 456,403,1.5, 453,404,4.0
0
1
455,406,5.9

# hall, south of library
11
1
50
0
1
440,479,103.5, 432,478,100.7, 433,476,100.8, 438,479,2.3, 435,479,101.9
0
1
435,477,0.74

# hall, skeleton hall
11
2
75
0
1
493,522,0.8, 474,521,3.8, 468,513,103.0, 496,515,103.0, 481,516,104.1, 488,514,102.0, 476,513,103.0, 460,513,103.0, 493,514,102.4, 485,514,102.0 
0
1
491,520,0.8, 477,520,3.8, 481,516,4.1, 476,513,3.0, 485,514,2.0 

# hall, south of two big rooms
11
1
75
50
bugbear gang
1
466,559,103.9, 474,558,100.6,471,563,1.6, 470,565,5.5, 471,570,5.4 
0
1
471,565,5.5


###############################################################################
#  B1 LEVEL 1, Chapter 4
###############################################################################

# 0. Entry
21
1
100
100
hill giant guard entry
1
491,606,3.0, 491,611,5.0, 496,601,2.5, 494,597,2.7, 497,595,2.5
1
502,606,104.6,(1002,)
0

# 1. Alcoves
21
2
100
100
hill giant guard alcoves
1
493,546,2.8, 489,563,105.6, 500,562,101.0, 494,542,2.4, 496,547,1.8, 493,536,3.1, 496,534,0.0, 493,531,3.9, 496,531,0.8, 494,527,2.4
2
490,515,0.0,(1049,)
489,533,2.4,(1003,)
0

# 2. Kitchen
21
1
100
100
orc slave female
1
518,494,3.9, 509,496,5.5, 509,494,0.8, 513,494,5.5, 514,492,2.4
2
521,491,3.1,(1003,1049,1049,1049)
515,491,0.8,(1046,)
0

# 3. Dining Room
21
1
100
100
orc slave male
1
479,493,3.7, 472,500,2.4, 474,495,5.5, 470,497,3.1, 473,492,3.1
2
469,503,4.0,(1002,1003)
485,491,0.8,(1002,1003)
1
473,494,2.8

# 4. Lounge
21
0
100
100
hill giant fighter
5
473,521,6.0
472,524,2.4
471,521,5.2
473,526,0.8
470,526,3.9
1
468,527,3.9,(1003,1004,1046)
0

# 5,6,7. Zel Bedroom
21
3
50
0
2
480,452,1.0, 480,458,0.8, 472,468,1.6, 470,470,4.7, 484,463,1.6, 482,465,5.5, 485,467,5.5, 480,455,0.8, 483,452,1.1, 486,452,0.8, 483,455,0.8, 483,458,0.8, 486,458,0.8, 472,475,5.5, 473,473,5.5, 486,455,1.1
466,458,1.6, 469,461,0.8, 467,455,1.6, 455,457,3.9, 455,460,3.9, 469,457,1.6, 472,460,1.0, 463,457,2.0, 468,452,1.8, 470,455,3.1, 473,457,1.6, 472,453,3.1, 457,452,2.4, 475,453,1.6, 479,454,0.8, 459,452,2.6
2
451,459,3.9,(1002,1003,1004,1005,1006,1031,1046)
473,452,2.4,(1002,1003,1004,1005,1006,1031,1046)
0

# 8. Zel Workroom
21
1
0
0
1
505,468,4.7, 513,466,1.0, 511,468,6.0, 516,472,0.0, 509,474,5.5
2
511,472,3.9,(1003,1004)
518,466,2.4,(1003,1004)
0

# 9. Zel Lab
21
1
0
0
1
516,450,1.0, 506,449,3.9, 513,452,6.0, 517,455,0.0, 510,455,2.4
2
504,458,3.9,(1002,1003,1004,1006,1046)
517,449,0.8,(1002,1003,1004,1006,1046)
0

# 10. Storage Room
21
1
50
0
1
536,470,6.0, 535,460,2.4, 536,463,6.0, 539,474,5.8, 543,468,0.8
1
546,470,1.0,(1049,)
0

# 11. Supply Room
21
1
50
0
1
558,452,1.8, 558,456,5.5, 554,453,2.4, 551,453,2.9, 555,450,2.4
1
549,453,4.1,(1004,)
0

# 12. Library
21
2
100
100
gnome in library
1
523,426,3.9, 531,423,0.8, 528,420,2.4, 525,420,3.1, 526,424,5.5, 524,414,4.7, 531,417,1.8, 523,418,2.6, 518,420,0.8, 526,412,1.6
1
523,412,103.1,(1003,1004)
0

# 13. Portcullis Room Hall
21
0
50
0
1
537,403,0.8
0
0

# 14. Auxiliary Storeroom
21
0
50
0
2
564,379,2.4
558,378,3.9
1
555,379,3.9
0

# 15 Teleport Room, North
21
0
50
0
1
557,400,2.3
0
0

# 15, 16. Teleport Rooms Hall
21
2
100
100
merrow B1
1
562,418,0.8, 567,409,5.5, 560,415,0.8, 569,409,5.5, 566,418,3.4, 569,413,5.5, 563,415,1.0, 566,414,0.9, 569,416,2.4, 569,420,0.0
0
0

# 17, 18, 19. Smithy
21
1
100
100
fire giant smiths
1
554,511,3.9, 558,498,2.1, 556,508,2.6, 551,516,5.5, 550,509,4.0
3
548,509,3.9,(1003,1004)
558,517,3.9,(1006,1046)
556,498,3.9,(1049,)
0

# 20. Dead End Room Hallway
21
1
100
100
ogre basher
1
540,551,0.9, 554,567,105.4, 543,554,3.9, 535,551,3.5, 534,554,4.0 
0
0

# 20. Dead End Room
21
1
100
100
ogre gang B1
1
542,568,3.1, 541,571,3.8, 545,572,0.6, 545,567,2.4, 543,562,2.6
2
540,566,3.9,(1003,1004,1046)
541,560,3.14,(1003,1004,)
0

# 21. Meeting Room
21
1
50
0
1
515,557,3.0, 515,554,2.7, 517,558,0.5, 517,554,2.2, 517,552,2.4 
1
515,581,3.9,(1003,)
0

# 22. Garden, left side
21
0
100
100
ascomid,basidirond,hydra [L],phycomid,shadow,shrieker,ustilagor,violet fungi
1
550,543,0.8
0
0

# 22. Garden, main body
21
2
100
100
gnome in garden
1
516,538,102.2, 529,537,103.0, 527,518,2.3, 517,517,3.1, 519,520,3.0, 519,507,0.8, 522,511,2.4, 525,511,2.2, 517,513,3.9, 517,507,2.5
1
527,509,100.0,(1031,)
0

# 23. Storage Room
21
1
0
0
1
431,581,3.1, 431,585,4.0, 428,579,1.6, 425,585,3.6, 426,581,4.7 
1
432,572,2.4,(1005,1046)
0

# 24. Mistress Room
21
2
100
100
hill giantess 1
2
415,577,2.3, 414,573,5.5, 409,576,2.8, 413,575,2.7, 411,576,2.0, 417,576,3.9, 416,573,2.6, 418,573,2.2, 419,576,0.8, 420,573,2.2 
415,582,2.3, 411,574,1.6, 416,585,5.8, 409,576,4.7, 414,580,2.7, 418,583,3.3, 414,574,5.5, 414,577,0.5, 418,573,3.1, 418,576,1.2 
1
408,581,3.9,(1003,1004)
0

# 25. Rogahn's Bedroom
21
2
100
100
hill giantess 2
1
416,538,4.3, 415,534,3.9, 407,541,4.2, 415,529,3.3, 412,539,1.3, 409,537,2.4, 412,531,1.3, 409,531,3.2, 408,527,3.4, 411,534,5.8 
2
406,532,4.2,(1003,1004,1005,)
418,523,2.2,(1046,)
0

# 26. Trophy Room
21
1
100
100
bugbear gang
1
470,544,5.0, 450,545,4.0, 466,541,3.8, 470,550,100.7, 473,541,1.0
1
467,539,2.4,(1003,1004)
0

# 27. Throne Room
21
2
100
100
hill giant hall
1
425,552,2.5, 425,567,4.8, 429,552,2.3, 430,567,5.4, 426,558,0.8, 421,557,4.0, 421,560,4.0, 419,553,3.3, 419,565,4.5, 415,553,3.5
1
413,566,103.9,(1005,1006,1046)
0

# 28. Worship Room
21
1
0
0
1
451,557,2.1, 449,561,4.8, 448,558,3.1, 454,556,1.4, 453,560,6.2
1
447,556,3.0,(1003,1004)
0

# 29. Captain's Room
21
1
100
100
hill giant [L]
1
465,575,2.0, 473,592,105.5, 453,587,3.2, 459,575,3.0, 450,590,4.0
1
450,590,3.9,(1003,1004)
0
 
# 29. Captain's Room 2
21
1
0
0
1
451,577,5.0, 450,575,3.4, 448,578,5.5, 446,576,4.2, 447,574,3.3
1
456,574,2.4,(1003,1004)
0

# 30. Stairs
21
1
100
100
wererat guards
1
474,567,101.0, 457,569,103.9, 462,557,3.0, 472,572,106.2, 469,556,0.7
0
0

# 31. Pool Room, North
21
0
50
0
1
427,462,2.4
0
0

# 31. Pool Room, South
21
0
50
0
1
448,521,5.5
0
0

# 32. Advisor Room
21
1
100
100
lich B1	
1
421,496,101.8, 419,508,104.6, 424,508,105.2, 427,507,106.0, 412,505,4.0
0
0

# 33. Barracks
21
3
100
100
bugbear gang
1
412,463,0.0, 409,463,5.6, 416,469,2.4, 409,468,0.0, 409,473,5.6, 415,472,4.6, 417,472,0.4, 408,466,2.5, 414,475,5.9, 407,470,5.0, 416,473,0.9, 404,470,3.9, 411,480,3.2, 410,477,2.4, 414,478,5.6, 408,479,4.6 
1
406,482,105.5,(1003,1004,1005,1049)
0

# 35. Guest Room 1
21
1
100
100
frost giantess 1
1
415,422,2.1, 414,428,4.8, 416,425,0.8, 413,425,3.7, 410,426,4.1 
2
408,429,103.9,(1003,1004,1005,1049)
412,423,103.9,(1003,1004,1005)
0

# 35. Guest Room 2
21
2
100
100
frost giant hall
1
412,408,4.7, 416,400,2.5, 420,407,5.4, 435,414,100.5, 412,403,3.8, 409,409,5.0, 425,429,105.0, 434,388,102.5, 435,392,100.0, 408,406,4.0
2
418,399,102.4,(1003,1004,1005,1049)
409,409,104.7,(1005,1046)
0

# 35. Guest Room 3
21
1
100
100
frost giantess 2
1
415,379,3.0, 413,381,3.9, 416,385,4.8, 412,384,0.3, 409,382,3.3
2
408,388,3.9,(1003,1004,1005,1049)
412,379,3.9,(1003,1004,1005)
0

# 36. Empty Room Near Pit Trap
21
2
50
0
1
442,433,3.2, 449,436,1.3, 446,432,0.0, 450,433,1.9, 446,435,0.3, 443,431,4.1, 450,428,3.4, 452,431,5.9, 453,428,2.0, 445,428,2.0
1
459,427,1.6
0

# 37. Rec Room [3rd melee spot and 3rd ranged spot are outside in the hall]
21
2
100
100
frost giant band
1
495,427,1.4, 489,428,3.8, 498,449,105.4, 492,432,5.5, 488,436,3.9, 491,422,3.2, 495,422,1.2, 487,447,103.8, 484,424,3.6, 486,421,3.1 
2
473,427,3.9,(1005,)
478,419,2.4,(1003,1004)
0

# 37 A. Maze Area Cave
21
1
0
0
1
453,398,3.7, 479,398,1.4, 466,392,2.3, 463,393,3.0, 472,394,1.9 
0
0


###############################################################################
#  B1 LEVEL 2, Chapter 4
###############################################################################

# 38. Access Room
22
3
100
100
fire giant guard entry
1
470,553,3.2, 482,552,1.7, 469,556,4.8, 485,556,1.0, 474,560,4.9, 482,559,5.5, 469,561,4.9, 477,558,5.5, 464,553,3.6, 462,558,5.3, 467,551,1.6, 485,560,6.2, 460,550,4.1, 480,563,0.0, 477,563,4.9, 460,555,3.5
1
482,549,2.4,(1046,)
0

# 39. Museum
22
1
100
100
fire giantess
1
447,561,5.6, 436,567,0.4, 433,564,2.4, 447,558,2.3, 433,567,3.9 
1
431,565,3.9,(1003,1004,1005)
0

######## 40. Secret Room, NO ENCOUNTER, ROOM IS ON ANOTHER MAP #######

# 41. Cavern
22
1
100
100
fire toad
1
480,611,5.1, 510,595,1.0, 481,602,2.0, 511,601,1.5, 477,605,3.2 
1
475,605,3.9,(1003,1004,1005)
0

# 42. Webbed Cave
22
1
100
100
spiders medium small and tiny
1
507,580,4.8, 509,586,5.0, 510,577,1.0, 503,573,3.4, 501,577,3.6
0
0

# 42 A. Webbed Cave, South
22
2
100
100
party of lycanthropes B1
1
546,579,101.0, 555,590,3.5, 542,572,102.5, 554,595,5.0, 547,584,1.0, 549,588,3.0, 551,592,6.0, 543,583,3.1, 546,572,101.5, 545,587,5.5 
0
0

# 43. Cavern
22
1
50
0
1
548,548,2.6, 546,550,3.7, 547,544,2.7, 544,545,3.5, 550,546,2.0
1
544,543,3.5,(1046,)
0

# 44. Cavern
22
0
50
0
1
497,557,3.3
1
494,559,4.7,(1003,1004)
0

# 45. Mystical Stone
22
3
100
100
fire giant hall
1
508,540,3.3, 507,549,3.8, 510,554,4.8, 521,551,0.0, 518,536,102.5, 530,531,103.5, 532,553,1.2, 506,544,3.8, 501,543,3.9, 503,538,3.0, 506,560,4.6, 529,557,5.0, 538,528,102.0, 511,536,3.0, 514,536,2.0, 496,543,3.5 
3
503,535,103.0,(1005,1046,)
496,553,102.4,(1005,1046,)
534,549,102.0,(1046,)
0

# 46, 47: Cavern
22
1
0
0
1
478,526,3.1, 478,532,5.5, 472,528,3.5, 466,527,2.6, 466,533,4.0
0
0

# 48. Arena Cavern (boss by entrance, 7 melee around rim, first 4 ranged are orcs in pit, then 3 archers around back, wizard is hydra)
22
3
100
100
arena with cryohydra
1
480,487,4.5, 503,480,0.8, 481,482,3.5, 501,485,0.4, 500,474,1.7, 484,485,4.0, 499,478,1.3, 484,493,5.0,   492,481,2.7, 497,483,0.9,  491,488,4.75, 496,480,1.5, 489,487,4.7, 493,472,2.4, 496,472,3.0, 486,475,3.5
0
0

# 49. Phosphorescent Cave, NO ENCOUNTER, ROOM IS NOT ON MY MAP
 
# 50. Water Pit, NO ENCOUNTER, SCRIPTED ENCOUNTER IS THERE
22
1
100
100
frost worm and snakes
1
424,482,1, 424,479,1.5, 418,494,206.0, 422,478,2.5, 413,482,101.4  
0
0

# 51. Side Cavern, Blue Walls
22
1
0
0
1
411,510,3.6, 412,521,4.4, 424,510,100.8, 427,497,103.0, 395,512,4.0
0
0

# 52. Raised Cavern
22
2
50
0
1
412,435,3.1, 411,438,3.6, 410,449,3.6, 413,452,5.7, 410,442,4.0, 408,435,1.4, 405,437,4.7, 407,445,4.1, 414,458,4.6, 406,440,4.0
2
402,436,3.7,(1002,1003,1004,1005,1006,1046)
407,433,2.8,(1046,)
0

# 53. Grand Cavern of the Bats 1
22
2
0
0
1
489,397,5.0, 478,405,1.9, 486,393,4.0, 473,400,3.0, 483,401,6.1, 485,398,4.8, 478,401,2.4, 480,392,4.5, 475,395,2.4, 480,398,3.0
1
483,409,6.0,(1002,1003,1004,1005,1006,1046,1049)
0

# 53. Grand Cavern of the Bats 2
22
1
50
0
1
522,423,1.5, 515,410,0.1, 514,417,2.2, 521,413,3.3, 525,416,0.2
1
519,416,5.2,(1031,)
0

# 53. Grand Cavern of the Bats 3
22
0
100
100
old red dragon
5
438,405,3.2
428,410,3.2 
422,421,3.7 
438,420,3.3 
451,407,2.2 
0
0

# 54. Treasure Cave, NO ENCOUNTER, STATUE ENCOUNTER IS THERE 

# 54. Treasure Cave, nearby hall Solo
22
0
0
0
3
573,446,1.6
562,453,1.9 
550,452,3.2 
0
0

# 55. Exit Cave
22
1
100
100
ettin herdsmen
2
585,402,2.0, 574,400,4.9, 579,398,0.8, 575,394,3.0, 581,392,2.4
569,415,5.8, 568,410,2.7, 567,404,1.8, 564,406,3.7, 564,398,3.0
1
572,394,2.8,(1003,1004)
0

# 56. Cavern of the Statue
22
2
100
0
2
577,513,5.7, 584,506,6.1, 585,503,1.3, 563,513,5.2, 579,511,0.5, 583,514,6.0, 587,508,0.4, 580,515,6.0, 559,513,4.5, 585,512,1.3
564,489,1.0, 559,495,0.3, 568,490,0.4, 565,501,6.2, 562,498,3.0, 567,486,1.7, 566,505,6.0, 568,502,5.8, 570,500,0.7, 570,488,1.3
2
578,518,5.2,(1003,)
583,496,1.5,(1003,1004)
0

# 56 A. Cavern of the Statue, nearby nook Solo
22
0
100
100
basilisk greater [L]
1
555,517,1.2
0
0

# 56 B. Cavern of the Statue, large cavern south
22
4
100
100
gnoll guard
1
570,535,1.6, 571,540,202.4, 574,542,201.3, 565,528,1.0, 567,533,2.1, 570,530,1.4, 556,560,105.2, 553,528,103.0, 551,538,100.3, 560,547,104.4, 556,527,102.0, 566,537,4.2, 578,534,1.8, 576,536,5.2, 577,526,0.4, 574,524,2.7, 569,527,2.0, 574,528,5.2, 574,533,0.9, 547,535,104.0, 557,556,104.0, 560,559,105.0, 564,560,105.0,  578,530,1.4
1
580,528,0.8,(1003,1004)
0


"""

import StringIO
def get_dungeon_areas_lines():
	return StringIO.StringIO(dungeon_areas_text)