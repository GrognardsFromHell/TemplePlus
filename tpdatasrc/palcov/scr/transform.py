from toee import *
from marc import *
from utilities import *

#------------------------------------------------------------------------------------
# Transform a pc model with the random possibilities defined by the specified style.
# The model should always start as a human male in the original proto, and then it
# will changed based on random rolls of the dice. Portraits are not changed here, but
# can be changed in san_first_heartbeat().
#------------------------------------------------------------------------------------

def transform (pc, style="villager"):

	model, scale, gender, hair = -99, -99, -99, -99

	if style == "acolyte":
		scale = -1
		hair = -99
		if game.random_range(1,3) == 1:
			gender = 0
			model = 'human'
		roll = game.random_range(1,100)
		if roll in range (80,90):
			model = 'dwarf'
		elif roll in range (81,85):
			model = "elf"
		elif roll in range (86,101):
			model = 'halforc'

	elif style == "cavalier":
		scale = -1
		hair = -4
		if game.random_range(1,3) == 1:
			gender = 0
			model = 'human'

	elif style == "dockhand":
		scale = -1
		hair = -3
		if game.random_range(1,8) == 1:
			gender = 0
			model = 'human'
			hair = -99  # not enough portraits
		roll = game.random_range(1,100)
		if roll in range (70,80):
			model = 'dwarf'
		elif roll in range (80,85):
			model = "elf"
		elif roll in range (85,101):
			model = 'halforc'

	elif style == "dwarf":
		scale = -1
		hair = -3
		gender = -99
		model = -99

	elif style == "gypsy":
		scale = -1
		hair = -4  # conservative
		if game.random_range(1,2) == 1:
			gender = 0
			model = 'human'
		roll = game.random_range(1,100)
		if roll in range (80,85):
			model = 'dwarf'
		elif roll in range (85,90):
			model = "elf"
		elif roll in range (90,95):
			model = "halfelf"
		elif roll in range (95,101):
			model = 'halforc'

	elif style == "patrol":
		scale = -99
		hair = -99
		if game.random_range(1,6) == 1:
			gender = 0
			model = 'human'
		roll = game.random_range(1,100)
		if roll in range (70,80):
			model = 'dwarf'
		elif roll in range (80,90):
			model = "elf"
			scale = 100
		elif roll in range (90,95):
			model = 'halforc'
		elif roll in range (95,101):
			model = 'halfling'
			scale = 75

	elif style == "pirate":
		scale = -1
		hair = -4
		if game.random_range(1,4) == 1:
			gender = 0
			model = 'human'
		roll = game.random_range(1,100)
		if roll in range (80,83):
			model = 'dwarf'
		elif roll in range (83,84):
			model = "elf"
		elif roll in range (84,85):
			model = "gnome"
		elif roll in range (85,87):
			model = "halfelf"
		elif roll in range (87,99):
			model = 'halforc'
		elif roll in range (99,101):
			model = 'halfling'

	elif style == "researcher":
		scale = -1
		hair = -99
		model = 'human'
		if game.random_range(0,1) == 0:
			gender = 0
			hair_set (pc, 3, 2)  # brown, ponytail
			pc.obj_set_int(obj_f_critter_portrait,17540)
		else:
			gender = 1
			hair_set (pc, 6, 15)  # red, medium
			pc.obj_set_int(obj_f_critter_portrait,27540)

	elif style == "villager":
		scale = -1
		hair = -4
		if game.random_range(1,2) == 1:
			gender = 0
			model = 'human'
		roll = game.random_range(1,100)
		if roll in range (70,80):
			model = 'dwarf'
		elif roll in range (80,85):
			model = "elf"
		elif roll in range (85,90):
			model = "gnome"
		elif roll in range (90,94):
			model = "halfelf"
		elif roll in range (94,98):
			model = 'halforc'
		elif roll in range (98,101):
			model = 'halfling'

	reincarnate (pc, model, scale, gender, hair)

	float_stat(game.leader,pc.obj_get_int(obj_f_critter_race)+2000,4)
	float_stat(game.leader,pc.obj_get_int(obj_f_critter_gender)+4000,4)

#------------------------------------------------------------------------------------
# Change a critter's physical appearance, for reincarnation or other purposes
#
# Arguments:
#    pc - the object handle of the critter.
#    model - the model the critter will be transformed into.
#        <str>: 'dwarf', 'elf', bugbear', etc.
#        'pc': roll a random model from the 7 possible PC models.
#        'humanoid': roll a random model from the non-PC models.
#        'random': roll a random model from all of the models.
#        -99: do not change the existing model
#    scale - how to set the scale.
#        <n>: set the scale to this exact value. Average size is 100.
#        -1: randomize the scale.
#       -99: do not change the existing scale.
#    gender - how to set the gender.
#         0: female.
#         1: male.
#        -1: randomize the gender.
#       -99: do not change the existing gender.
#    hair - how to set the hair.
#        <n>: set the hair to this exact value. See comments in hair_style_set().
#        -1: randomize the hair.
#        -2: randomize with no chance of pink, blue, or mohawks.
#        -3: 
#        -4: 
#       -99: do not change the existing hair.
#
# The last three arguments do not make visual changes, they are just internal values
# that need to match up with the model. They will be set automatically based on the
# model given ('dwarf', 'elf', etc.). They should generally be left alone unless
# there is some odd purpose in mind, such as setting race to dwarf to get dark skin. 
#
#    race - 
#        <n>: 0=human, 1=dwarf, 2=elf, ... 6=halfling
#         -1: adjust the race based on the given model. Recommended.
#        -99: do not adjust the race.
#    size - 
#        <n>: 2=fine, 3=tiny, 4=small, 5=medium, 6=large, etc.
#         -1: adjust the size based on the given model. Recommended.
#        -99: do not adjust the size.
#    abils - 
#         -1: adjust the 6 ability scores based on the given model. This assumes the
#             original is human. Recommended. 
#        -99: do not change the stats
#
# Right now it can change the following attributes of a PC or NPC:
#   1. Object model mesh ID,  physical change
#   2. Scale,  physical change
#   3. Gender,  numeric value 0-1
#   4. Hair Style,  physical change
#   5. Race,  numeric value 0-6,  generated based on model
#   6. Object Size,  numeric value 1-6,  generated based on model
#   7. Ability Scores,  numeric values,  generated based on model
#
# This function allows for full reincarnation transformation, but right now I am only
# going to use it to do a one-time change of a model that always starts out as a
# male human. Lines commented out with "###" are used for reincarnation and work
# fine, but are not needed right now.
#------------------------------------------------------------------------------------
def reincarnate (pc, model='random', scale=-1, gender=-1, hair=-1, race=-1, size=-1, abils=-1):

	models = {
		'human' : ((101,100,100), (730,710), 25, 5, 30, 0, mc_type_humanoid, mc_subtype_human, 5, (58,120), (0,0,0,0,0,0)),
		'dwarf' : ((103,102,100), (2070,2050), 25, 5, 20, 1, mc_type_humanoid, mc_subtype_dwarf, 5, (45,130), (0,0,2,0,0,-2)),
		'elf' : ((101,100,95), (1030,1080), 25, 5, 30, 2, mc_type_humanoid, mc_subtype_elf, 5, (53,85), (0,2,-2,0,0,0)),
		'gnome' : ((101,100,80), (70,20), 20, 5, 20, 3, mc_type_humanoid, mc_subtype_gnome, 4, (36,40), (-2,0,2,0,0,0)),
		'halfelf' : ((101,100,100), (1500,1510), 25, 5, 30, 4, mc_type_humanoid, mc_subtype_elf, 5, (55,100), (0,0,0,0,0,0)),
		'halforc' : ((105,104,100), (3050,3000), 25, 5, 30, 5, mc_type_humanoid, mc_subtype_orc, 5, (58,150), (2,0,0,-2,0,-2)),
		'halfling' : ((101,100,75), (2560,2530), 15, 5, 20, 6, mc_type_humanoid, mc_subtype_halfling, 4, (32,30), (-2,2,0,0,0,0)),
		'bugbear' : ((1092,1077,100), (4270,4130), 25, 5, 30, -1, mc_type_humanoid, mc_subtype_goblinoid, 5, (72,250), (4,2,2,0,0,-2)),
		'gnoll'  : ((1019,1014,100), (3830,3840), 25, 5, 30, -1, mc_type_humanoid, mc_subtype_gnoll, 5, (72,200), (4,0,2,-2,0,-2)),
		'goblin' : ((1056,1057,100), (3810,3820), 15, 5, 30, -1, mc_type_humanoid, mc_subtype_goblinoid, 4, (36,50), (-2,2,0,0,0,-2)),
		'kobold' : ((1187,1188,50), (5060,5070), 15, 5, 30, -1, mc_type_humanoid, mc_subtype_reptilian, 4, (36,50), (-4,2,-2,0,0,0)),
		'lizardman' : ((1017,1015,100), (3520,3530), 25, 5, 30, -1, mc_type_humanoid, mc_subtype_reptilian, 5, (58,180), (2,0,2,-2,0,0)),
		'orc' : ((105,104,110), (4980,4920), 25, 5, 30, -1, mc_type_humanoid, mc_subtype_orc, 5, (58,180), (4,0,0,-2,-2,-2))
		}

	tokens = { 'dwarf': 12700, 'elf': 12701, 'gnome': 12702, 'halfelf': 12703, 'halforc': 12704, 'halfling': 12705,
		'bugbear': 12707, 'gnoll': 12708, 'goblin': 12709, 'kobold': 12710, 'lizardman': 12711, 'orc': 12712 }

	# Sanity check the arguments
	if ( not (model in models or model in ('random','pc','humanoid',-99)) or
	not (scale in range(1,400) or scale in (-1,-99)) or
	not (gender in (0,1,-1,-99)) or
	not (hair in range(0,1023) or hair in (-1,-2,-3,-4,-99)) or
	not (race in range(-1,6) or race == -99) or
	not (size in range(-1,8) or size == -99) or
	not (abils in range(-1,6) or race == -99) ):
		return 0

	# Gender, this is done first as it is needed to choose the model and hair
	if gender != -99:
		if gender == -1:
			gender = game.random_range(0,1)
		pc.obj_set_int(obj_f_critter_gender, gender)
	else:
		gender = pc.obj_get_int(obj_f_critter_gender)

	# Model
	m, m_key = 0, model
	if model != -99:
		if model in models.keys():
			m = models[model]
		elif model in ['random','pc','humanoid']:
			model_keys = []
			if model == 'random':
				model_keys = models.keys()
			elif model == 'pc':
				for k in models:
					if models[k][5] >= 0:
						model_keys.append(k)
			elif model == 'humanoid':
				for k in models:
					if models[k][5] == -1:
						model_keys.append(k)
			roll = game.random_range(0,len(model_keys) - 1)
			m_key = "bugbear"  # model_keys[roll]
			m = models[m_key]
		if m:
			pc.obj_set_int (obj_f_base_anim, m[0][gender]) 
			pc.obj_set_int (obj_f_base_mesh, m[0][gender])

	# Scale
	if scale != -99:
		if scale == -1:
			if m == 0:
				smallest = pc.obj_get_int(8) * 0.88
				die = int(pc.obj_get_int(8) *0.12)
			else:
				smallest = m[0][2] * 0.88
				die = int(m[0][2] *0.12)
			scale = smallest + game.random_range(0,die) + game.random_range(0,die)
		pc.obj_set_int(8,int(scale))

	# Race, set for PC models only, must be set before changing hair
	if race != -99 and m != 0:
		if m[5] in range(0,7):
			pc.obj_set_int(obj_f_critter_race, m[5])
		else:
			pc.obj_set_int(obj_f_critter_race, 0)

	# Hair, set for PC models only.
	if hair != -99:
		if hair < 0:
			hair_random (pc, hair)
		else:
			pc.obj_set_int(obj_f_critter_hair_style, hair)

	# Hair value must be updated for the right race, or it may not look right
	hair = pc.obj_get_int(obj_f_critter_hair_style)
	hair = hair & int('1111111000',2)  # clear old race
	hair += pc.stat_base_get(stat_race)  # add in new race
	pc.obj_set_int(obj_f_critter_hair_style, hair)

	# Size, 1,2,3,4,5,6 for fine, dimin, tiny, small, med, large, huge
	if size != -99 and m != 0:
		pc.obj_set_int (obj_f_size, m[8])

	# Ability Scores
	if abils != -99 and m != 0:
		pc.stat_base_set (stat_strength, pc.stat_base_get (stat_strength) + m[10][0] )
		pc.stat_base_set (stat_dexterity, pc.stat_base_get (stat_dexterity) + m[10][1] )
		pc.stat_base_set (stat_constitution, pc.stat_base_get (stat_constitution) + m[10][2] )
		pc.stat_base_set (stat_intelligence, pc.stat_base_get (stat_intelligence) + m[10][3] )
		pc.stat_base_set (stat_wisdom, pc.stat_base_get (stat_wisdom) + m[10][4]) 
		pc.stat_base_set (stat_charisma, pc.stat_base_get (stat_charisma) + m[10][5] )

	# Speed, boost speed of small models which have old run speed of 1065353216
	if m_key in ('dwarf','halfling','gnome'):
		pc.obj_set_int (obj_f_speed_walk, 1068353216)
		pc.obj_set_int (obj_f_speed_run, 1070353216)

	# Set the model changes
	pc.obj_set_int (obj_f_animation_handle, 0)

	# Give the token
	if model == "random":
		for item in get_inv(pc):  # remove old token
			if item.name in range(12700,12713):
				item.destroy()
		add_to_inv (pc, tokens[m_key])
 
#------------------------------------------------------------------------------------
# Changes a model's hair 
#
# Arguments:
#   npc:   Object handle of the model.
#   color: 0=black, 1=blonde, 2=blue, 3=brown, 4=light brown, 5=pink, 6=red, 7=white
#   style: 0=Long f       4=Medium f     8=Pigtails f  12=Mohawk f
#          1=Long m       5=Short m      9=Mullet m    13=Mohawk m
#          2=Ponytail f   6=Topknot f   10=Braids f    14=Ponytail(very long) f
#          3=Ponytail m   7=Topknot m   11=Bald m      15=Medium m
#   race:  0=human, 1=dwarf, 2=elf, 3=gnome, 4=h-elf, 5=h-orc, 6-halfling, 7=unknown
#          -1 = use creature's existing race (highly recommended).
#
# Beards: A beard will only appear if race is set to dwarf(1) or unknown(7), and
# only for the male styles (1,3,5,7,9,11,13,15). It works, but the beard doesn't
# always look right on a model that is not a male dwarf, so use with discretion. 
#------------------------------------------------------------------------------------
def hair_set (npc, color, style, race=-1):
	if color in range(0,8) and style in range(0,16) and race in range(-1,7):
		if race == -1:
			race = npc.obj_get_int(obj_f_critter_race)
		hair = color*128 + style*8 + race
		npc.obj_set_int(obj_f_critter_hair_style, hair)
		npc.obj_set_int(obj_f_animation_handle, 0)

#------------------------------------------------------------------------------------
# Changes a model's hair to a random color and style
#
# Arguments:
#   npc: Object handle of the model.
#   cat: Category of potential colors and styles.
#     -1: Randomize from all possible colors and styles.
#     -2: Exclude pink, blue, and mohawk.  
#     -3: Exclude pink, blue, and mohawk.
#         Give weight to common colors such as black and brown,
#         Give weight to roll short hair for men, and long hair for women.
#         Others still possible, but much less likely.
#     -4: Only very conservative colors and styles, enough for a small variation.
#         Colors: brown, black
#         Styles: male (short, medium, mullet), female (long, pony, medium) 
#------------------------------------------------------------------------------------
def hair_random (npc, cat=-1):

	categories = {
		-2: ( ((0,1,3,4,6,7),(0,2,4,6,8,10,14)),    # f, no pink/blue/mohawk
		((0,1,3,4,6,7),(1,3,5,6,7,11,15)) ),  # m, no pink/blue/mohawk
		-3: ( ((0,0,0,0,1,3,3,3,3,4,4,6,7,7),(0,0,2,2,4,4,6,8,10,14)), # female
		((0,0,0,0,1,3,3,3,3,4,6,7,7),(1,3,5,5,5,5,5,5,9,9,9,15,15,15)) ), # male
		-4: ( ((0,3),(0,2,4)),          # female
		((0,3),(5,5,9,15,15)) ),  # male
	}

	gender = npc.obj_get_int(obj_f_critter_gender)

	if cat in categories.keys():
		colors = categories[cat][gender][0]
		color = colors[game.random_range(0,len(colors)-1)]
		styles = categories[cat][gender][1]
		style = styles[game.random_range(0,len(styles)-1)]
	else:
		color = game.random_range(0,7)
		style = game.random_range(0,7) * 2 + gender

	hair_set(npc,color,style)
