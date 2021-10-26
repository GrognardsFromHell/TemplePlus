from toee import *

from __main__ import game

import _include ## Neccessary for timedEventAdd to work across saves and loads - CtB



# function: party_transfer_to( target, oname )
# arguments: target - PyObjHandle (the recepient of the item)
#            oname - the internal name of the object to be trasnfered
#                    from the party
# returns: the PyObjHandle of the object found, OBJ_HANDLE_NULL if not found
def party_transfer_to( target, oname ):
	for pc in game.party:
		item = pc.item_find( oname )
		if item != OBJ_HANDLE_NULL:
			pc.item_transfer_to( target, oname )
			return item
	return OBJ_HANDLE_NULL

def find_npc_near( obj, name ):
	for npc in game.obj_list_vicinity( obj.location, OLC_NPC ):
		if (npc.name == name):
			return npc
	return OBJ_HANDLE_NULL

def find_container_near( obj, name ):
	for container in game.obj_list_vicinity( obj.location, OLC_CONTAINER ):
		if (container.name == name):
			return container
	return OBJ_HANDLE_NULL

def group_average_level( pc ):
	count = 0
	level = 0
	for obj in pc.group_list():
		count = count + 1
		level = level + obj.stat_level_get( stat_level )
	if (count == 0):
		return 1
####Old method###############
	# level = level + ( count / 2 )
	# avg = level / count
#########################
	avg = (level/count) + (count - 4)/2
	return avg

def group_wilderness_lore( pc ):
	high = 0
	level = 0
	for obj in pc.group_list():
		level = obj.skill_level_get( skill_wilderness_lore )
		if (level > high):
			high = level
	if (high == 0):
		return 1
	return high

def obj_percent_hp( obj ):
	curr = obj.stat_level_get( stat_hp_current )
	max = obj.stat_level_get( stat_hp_max )
	if (max == 0):
		return 100
	if (curr > max):
		curr = max
	if (curr < 0):
		curr = 0
	percent = (curr * 100) / max
	return percent

def group_percent_hp( pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		percent = percent + obj_percent_hp(obj)
		cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent

def group_pc_percent_hp( attachee,pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		if (obj != attachee):
			percent = percent + obj_percent_hp(obj)
			cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent

def group_kobort_percent_hp( attachee,pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		if (obj != attachee and obj.name != 8005):
			percent = percent + obj_percent_hp(obj)
			cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent


def create_item_in_inventory( item_proto_num, npc ):
	item = game.obj_create(item_proto_num, npc.location)
	if (item != OBJ_HANDLE_NULL):
		npc.item_get(item)
	return item

def is_daytime():
	return game.is_daytime()


def is_safe_to_talk(speaker,listener, distt = 15, pc_only = 1):
	if (speaker.can_see(listener) and (listener.type == obj_t_pc or pc_only == 0) ):
		if (speaker.distance_to(listener) <= distt):
			return 1
	return 0

def start_game_with_quest(quest_number):
	game.quests[quest_number].state = qs_accepted
	# hommlet movies start at 1009 and go to 1017
	# but the quests start at 22, so offset by 987
	game.fade_and_teleport(0,0,0,5001,711,521)
	return

def start_game_with_botched_quest(quest_number):
	game.quests[quest_number].state = qs_mentioned
	game.quests[quest_number].state = qs_botched
	# hommlet no-voice movies start at 1018 and go to 1026
	# but the quests start at 22, so offset by 996
	game.fade_and_teleport(0,0,0,5001,711,521)
	return


def critter_is_unconscious(npc):
	curr = npc.stat_level_get( stat_hp_current )
	if (curr < 0):
		return 1
	if (npc.stat_level_get(stat_subdual_damage) > curr):
		return 1
	return 0

# HTN - returns true if obj is an "item" (obj.h)
def obj_is_item( obj ):

	return (obj.type == obj_t_projectile) or (obj.type == obj_t_weapon) or (obj.type == obj_t_ammo) or (obj.type == obj_t_armor) or (obj.type == obj_t_scroll) or (obj.type == obj_t_bag)

# HTN - compares two OBJ_HANDLE's by "hit_dice", used for sorting OBJ_HANDLE lists
def obj_comparison_by_hit_dice( obj_1, obj_2 ):

	obj_1_hitdice = obj_1.hit_dice
	obj_2_hitdice = obj_2.hit_dice

	return cmp( obj_1_hitdice, obj_2_hitdice )

def pyspell_targetarray_copy_to_obj_list( spell ):
	copy_list = []
	for item in spell.target_list:
		copy_list.append( item.obj )
	return copy_list

# Function: location_to_axis( PyLong loc )
# Author  : Steve Moret
# Returns : A tuple of two PyInts
# Purpose : returns an x, y for a specified location
#           can be used like:  
#  x, y = location_to_axis( npc.location )
#
def location_to_axis( loc ):
	if type(loc) == type(OBJ_HANDLE_NULL):
		loc = loc.location
		# in case the object was given as an argument instead of its location
	y = loc >> 32
	x = loc & 4294967295
	return ( x, y )

# Function: location_from_axis( PyInt x, PyInt y )
# Author  : Steve Moret
# Returns : A PyLong representing the location
# Purpose : returns a location represented by the x, and y components x,y
#
def location_from_axis( x, y ):
	# initialize loc to be a LONG integer
	loc = 0L + y
	loc = ( loc << 32 ) + x
	return loc


#################################################################################################################################
#   ENDSLIDES FOR STANDARD GAME
#################################################################################################################################
# Function: set_end_slides( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up all the end slides for the end game
#
def set_end_slides( attachee, triggerer ):
#################################################################################################################################
#   ZUGGTMOY
#################################################################################################################################
	if game.global_flags[189] == 1:
	# zuggtmoy dead
		if game.global_flags[183] == 1:
		# took pillar
			game.moviequeue_add(204)
			# Killed after her pillar of platinum was accepted and used to break open the enchanted seal, Zuggtmoy's spirit is banished to the Abyss for 40 years. But she will return to the Prime Material for vengeance...
		elif game.global_flags[326] == 0:
		# haven't destroyed orb
			game.moviequeue_add(200)
			# Zuggtmoy was killed while still trapped on the Prime Material plane. Her spirit destroyed, a major force for evil is now gone from the world of men.
		else:
			game.moviequeue_add(201)
			# Weakened by the destruction of the Orb of Golden Death, Zuggtmoy was killed while still trapped on the Prime Material plane. Her spirit destroyed, a major force for evil is now gone from the world of men.
	elif game.global_flags[188] == 1:
	# zuggtmoy banished
		game.moviequeue_add(202)
		# Her throne defaced, Zuggtmoy is banished to Abyss for 66 years. She plans to reform the Temple on her return.
	elif game.global_flags[326] == 1:
	# destroyed orb
		game.moviequeue_add(203)
		# Four days after the destruction of the Orb of Golden Death, a weakened Zuggtmoy is banished to Abyss for 40 years. But she will return to the Prime Material for vengeance...
	elif game.global_flags[186] == 1:
	# zuggtmoy surrendered
		game.moviequeue_add(205)
		# As your unwilling follower, Zuggtmoy and her temple raze the surrounding countryside, and all good people whisper your name with fear.
	elif game.global_flags[187] == 1:
	# zuggtmoy gave treasure
		game.moviequeue_add(206)
		# For a while, Zuggtmoy remains in her lair, but nature is demonic and she cannot be contained for long. She gathers her strength, and soon her evil machinations are loose in the world again.
	elif game.global_flags[184] == 1:
	# surrendered to zugtmoy
		game.moviequeue_add(207)
		# With your help, the temple rapidly gains power, and Furyondy and Veluna are threatened by its aggression. The evil one Iuz attacks from the north in a Clamp of Evil that changes the face of Greyhawk.
	elif ( game.global_flags[190] == 1 ) or ( game.global_flags[191] == 1 ):
	# gave orb to zuggtmoy for pillar or zuggtmoy charmed a PC
		game.moviequeue_add(208)
		# You are a thrall of Zuggtmoy. You are used on the frontline in the temple's raids and die an anonymous death on some nameless battlefield.
#################################################################################################################################
#   IUZ & CUTHBERT
#################################################################################################################################
	if game.global_flags[327] == 1:
	# iuz dead
		game.moviequeue_add(209)
		# In an almost unbelievable turn of events, you have managed to destroy Iuz. However, he is not killed but has immediately returned to his soul object on Zuggtmoy's Abyssal plane. It will take him years to recover his powers, during which time his country is overrun and his followers are scattered. Iuz fears you.
	elif game.global_flags[326] == 1:
	# destroyed orb
		game.moviequeue_add(211)
		# Iuz is weakened by the Orb's destruction. He now ranks you among his personal enemies and plans a revenge suitable for this annoyance.
#################################################################################################################################
	if game.global_flags[328] == 1:
	# cuthbert showed up
		game.moviequeue_add(210)
		# Iuz and St. Cuthbert meet. Their �discussion� is not for mortals to witness, but the enmity between their followers escalates.
#################################################################################################################################
#   PRINCE THROMMEL
#################################################################################################################################
	if game.global_flags[150] == 1:
	# thrommel dead
		game.moviequeue_add(215)
		# With the death of Prince Thrommel, divined by the priests of Furyondy, that kingdom suffers greatly in the wars to come.
	elif (game.global_flags[151] == 1) or (game.global_flags[152] == 1) or (anyone( triggerer.group_list(), "has_follower", 8031)):
	# thrommel freed and out OR thrommel reward scheduled OR thrommel in party
		game.moviequeue_add(214)
		# Prince Thrommel is rescued and marries Princess Jolene of Veluna. The kingdoms of Furyondy and Veluna unite, and you are made knights of the kingdom.
	else:
		game.moviequeue_add(216)
		# Left inside the temple dungeons, Prince Thrommel eventually becomes a vampire and strikes terror into the local populace. His sword Fragarach is lost in the mists of time...
#################################################################################################################################
#   TEMPLE & ASSOCIATES
#################################################################################################################################
	if ( game.global_flags[146] == 1 and game.global_flags[104] == 1 and game.global_flags[105] == 1 and game.global_flags[106] == 1 and game.global_flags[107] == 1 ):
	# hedrack dead, romag dead, belsornig dead, kelno dead, alrrem dead
		game.moviequeue_add(224)
		# With no high priests left alive after your raids, the temple of elemental evil loses its remaining worshippers, who disband and scatter to the four winds.
#################################################################################################################################
	if game.global_flags[339] == 1:
	# you are hedrack's bitch
		game.moviequeue_add(251)
		# With your help, the Temple rises in power and begins anew its attacks on neighboring countries.
#################################################################################################################################
	if (game.global_flags[146] == 0 and game.global_flags[189] == 0):
	# hedrack alive
		game.moviequeue_add(236)
		# Supreme Commander Hedrack escaped from the Temple of Elemental Evil, but he had earned the displeasure of Iuz for many years to come.
#################################################################################################################################
	if game.global_flags[147] == 0:
	# senshock alive
		game.moviequeue_add(225)
		# Senshock, the Lord Wizard of the Temple, escapes to Verbobonc where he secretly plans a return to power.
#################################################################################################################################
	if game.global_flags[104] == 0:
	# romag alive
		game.moviequeue_add(238)
		# Romag, high priest of the powerful Earth Temple, escaped and was too ashamed to ever return to the Temple of Elemental Evil.
#################################################################################################################################
	if game.global_flags[105] == 0:
	# belsornig alive
		game.moviequeue_add(237)
		# Belsornig, the plotting high priest of the Water temple, managed to escape from the Temple of Elemental Evil unharmed. Surely, he is somewhere on Oerth planning new diabolical schemes even now...
#################################################################################################################################
	if game.global_flags[107] == 0:
	# alrrem alive
		game.moviequeue_add(240)
		# Alrrem, high priest of the Fire Temple, escaped from the Temple of Elemental Evil, but not with his sanity intact. He still roams the countryside as a raving madman, his mind shattered.
#################################################################################################################################
	if game.global_flags[106] == 0:
	# kelno alive
		game.moviequeue_add(239)
		# Kelno, high priest of the Air Temple, escaped from the Temple of Elemental Evil, but without funds or friends, he lives out his days as a beggar on the streets of Mitrik, fearful and paranoid of being discovered by good or evil alike.
#################################################################################################################################
	if game.global_flags[335] == 0:
	# falrinth alive
		game.moviequeue_add(241)
		# Falrinth the wizard managed to escape the Temple of Elemental Evil. He dares not contact any former ally, having made too many enemies while in the service of evil there. He is currently slinking through the underdark, trying to regain the favor of the demoness Lolth, his patron.
#################################################################################################################################
	if game.global_flags[338] == 0:
	# smigmal alive
		game.moviequeue_add(249)
		# Smigmal Redhand, the temple assassin and femme fatale, escaped and returned to the orc tribe where she was spawned to become its leader, the first time a female ever ruled an orc tribe. Her burning hatred for humanity never dimmed. She was eventually eaten by an ogre, who suffered great indigestion afterwards.
#################################################################################################################################
	if game.global_flags[177] == 0:
	# feldrin alive
		game.moviequeue_add(226)
		# Feldrin flees over the Lortmil Mountains and starts his own rogue's guild in the Principality of Ulek. He is eventually killed during a skirmish with a rival guild.
#################################################################################################################################
	if game.global_flags[37] == 0 and ( not(anyone( triggerer.group_list(), "has_follower", 8002 )) ):
	# lareth alive AND not in party
		game.moviequeue_add(212)
		# Lareth the Beautiful escapes and starts his own temple of evil, dedicated to Llolth, the Spider Goddess. She is not placated and eventually turns Lareth into a drider.
	elif game.global_flags[37] == 1:
	# lareth dead
		game.moviequeue_add(213)
		# Lareth is killed, and you gain the enmity of his patron goddess Llolth. Over the years, she dispatches many drow assassins to kill you. All fail.
#################################################################################################################################
#   NULB & ASSOCIATES
#################################################################################################################################
	if ( game.global_flags[186] == 1 or game.global_flags[184] == 1 or game.global_flags[190] == 1 or game.global_flags[191] == 1 ):
	# zuggtmoy surrendered or you surrendered to zuggtmoy or you gave orb to zuggtmoy for pillar or zuggtmoy charmed you
		game.moviequeue_add(246)
		# Following the growth and prosperity of the Temple, Nulb expands into a great city, where the evil and wicked join forces and plot their evil schemes.
	elif game.global_flags[187] == 1:
	# zuggtmoy gave you treasure
		game.moviequeue_add(247)
		# As Zuggtmoy was spared, so was Nulb. Its wicked inhabitants continued their evil ways unchecked.
	elif ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ):
	# zuggtmoy dead or banished
		game.moviequeue_add(248)
		# With the destruction of the Temple of Elemental Evil, the fall of the village of Nulb came soon thereafter.
#################################################################################################################################
	if ( game.global_flags[97] == 1 and game.global_flags[329] == 1 ):
	# tolub dead and grud dead
		game.moviequeue_add(223)
		# With the deaths of Tolub and Grud Squinteye, the river pirates lose direction and eventually disband.
#################################################################################################################################
	if ( game.global_flags[90] == 1 and game.global_flags[324] == 1 and game.global_flags[365] == 0 ):
	# gay bertram free AND gay bertram met AND gay betram alive
		game.moviequeue_add(218)
		# You and Bertram are married in a small ceremony, and he opens a dentistry office in Verbobonc. You live happily ever after.
#################################################################################################################################
	if ( game.global_flags[83] == 1 and game.global_flags[330] == 0 ):
	# exclusive with jenelda and jenelda alive
		game.moviequeue_add(231)
		# After your beautiful wedding, you and Jenelda travel throughout Oerth. You were last seen in the east in the Grandwood Forest.
#################################################################################################################################
#   HOMMLET & ASSOCIATES
#################################################################################################################################
	if ( game.global_flags[299] == 0 and game.global_flags[337] == 0 and game.global_flags[336] == 0 and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# terjon alive and jaroo alive and burne and rufus alive and zuggtmoy dead or banished
		game.moviequeue_add(243)
		# In the quiet years that followed the fall of the temple of elemental evil, Hommlet grows into a bustling and prosperous town under the guidance of its elder citizens.
	elif ( ( game.global_flags[299] == 1 or game.global_flags[337] == 1 or game.global_flags[336] == 1 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# terjon dead or jaroo dead or burne or rufus dead and zuggtmoy dead or banished
		game.moviequeue_add(244)
		# As is often the case in sleepy, little villages, the excitement of the destruction of the temple soon died down, and life in Hommlet continued unaffected by such portentous events.
	elif ( game.global_flags[186] == 1 or game.global_flags[187] == 1 or game.global_flags[184] == 1 or game.global_flags[190] == 1 or game.global_flags[191] == 1 ):
	# zuggtmoy surrendered or gave you treasure or you surrendered to zuggtmoy or you gave orb to zuggtmoy for pillar or zuggtmoy charmed you
		game.moviequeue_add(245)
		# Hommlet does not survive Zuggtmoy's rampages and is burned to the ground. Those who did not flee early were subjected to torture and death.
#################################################################################################################################
	if ( game.quests[15].state != qs_completed and game.global_flags[336] == 0 ):
	# didn't complete agent revealed quest and burne or rufus dead
		game.moviequeue_add(242)
		# The tower of Burne and Rufus remained under construction for many years, plagued by mysterious delays and inauspicious accidents.
#################################################################################################################################
	if game.quests[20].state == qs_completed:
	# rescued paida
		game.moviequeue_add(250)
		# In returning Paida to Valden, you have overcome their history of misery and pain and allowed true love to prosper.
#################################################################################################################################
	if game.global_flags[61] == 1:
	# furnok retired
		game.moviequeue_add(222)
		# Furnok of Ferd uses his wealth to become a well-known and well-respected citizen of Verbobonc.
#################################################################################################################################
	if ( game.quests[12].state == qs_completed and game.global_flags[364] == 0 ):
	# completed one bride for one player quest AND fruella alive
		game.moviequeue_add(219)
		# As you promised, you and Fruella are married in the church of St. Cuthbert. She tames your wild, adventuring nature, and you settle down as a farmer in Hommlet and have many, many, many children.
#################################################################################################################################
	if ( game.global_flags[68] == 1 and game.global_flags[331] == 0 ):
	# married laszlo and laszlo alive
		game.moviequeue_add(230)
		# You marry Laszlo. You settle down with him, live the life of a herdsman's wife and have many children.
#################################################################################################################################
	if ( game.global_flags[46] == 1 and game.global_flags[196] == 0 and game.global_flags[318] == 0 ):
	# married meleny and meleny not dead or mistreated
		if game.global_flags[332] == 1:
		# killed an ochre jelly
			game.moviequeue_add(232)
			# You marry Meleny and take her on all sorts of grand adventures.
		else:
			game.moviequeue_add(233)
			# You marry Meleny and take her on all sorts of grand adventures, until an ochre jelly eats her.
#################################################################################################################################
	if ( game.quests[6].state == qs_completed and game.global_flags[333] == 0 and game.global_flags[334] == 0 ):
	# completed cupid's arrow quest and filliken alive and mathilde alive
		if game.global_flags[332] == 1:
		# killed an ochre jelly
			game.moviequeue_add(234)
			# You convince Filliken and Mathilde to marry, and they enjoy many happy years together in Hommlet.
		else:
			game.moviequeue_add(235)
			# You convince Filliken and Mathilde to marry, and they enjoy many happy years together in Hommlet, until they are eaten by ochre jellies.
#################################################################################################################################
#   JOINABLE NPCS
#################################################################################################################################
	if game.global_vars[29] >= 10:
	# 10 or more NPCs killed while in party
		game.moviequeue_add(217)
		# You abuse your faithful followers, many of whom lose their lives in the dungeons beneath the temple.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8060 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# morgan in party and zuggtmoy dead or banished
		game.moviequeue_add(263)
		# Morgan survives the encounter with Zuggtmoy and thanks you most profusely for saving his life. He returns to his life as a pirate, and eventually becomes a feared captain of his own ship on the high seas.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8023 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# oohlgrist in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(265)
		# Like the coward he is, Oohlgrist sneaks off soon after your encounter with Zuggtmoy. He is never heard from again.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8034 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# scorrp in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(266)
		# To show his gratitude, Scorpp swears loyalty to you and is evermore your faithful servant.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8061 ) and anyone( triggerer.group_list(), "has_follower", 8029 ) and anyone( triggerer.group_list(), "has_follower", 8030 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# ted, ed, and ed in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(264)
		# The stars flicker, the gods pause, the winds stop. Throughout the world there is silence, as if the world has stopped to catch its breath. For you have survived the game with Ted, Ed and Ed. Congratulations.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8014 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# otis in party and zuggtmoy dead or banished
		game.moviequeue_add(259)
		# Otis is very pleased with the results of your adventures together. His reports to Furyondy and Veluna gain you even more favor in those countries.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8021 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# y'dey in party and zuggtmoy dead or banished
		game.moviequeue_add(260)
		# Y'dey is overjoyed to have adventured with you. Her reports to the Archcleric of Veluna feature you most prominently, and the Archcleric is impressed with your results.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8062 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# zaxis in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(267)
		# Shortly after your Temple adventure, Zaxis leaves your party to continue looking for his sister. He finally finishes his song about your grand adventure, although he never does find a word that rhymes with Zuggtmoy.
#################################################################################################################################
#   VIGNETTE RESOLUTION
#################################################################################################################################
	if ( game.quests[23].state == qs_completed and game.global_flags[306] == 1 ):
	# completed NG vignette quest and discovered y'dey
		game.moviequeue_add(252)
		# Discovering Cannoness Y'dey alive proved to be a high point in your group's adventures.
#################################################################################################################################
	if game.quests[24].state == qs_completed:
	# completed CG opening vignette quest
		game.moviequeue_add(221)
		# Countess Tillahi encourages Queen Yolande of Celene to join forces with Veluna to move against Iuz and his city of Molag.
#################################################################################################################################
	if ( game.quests[25].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# completed LN vignette quest and zuggtmoy dead or banished
		game.moviequeue_add(253)
		# In recognition of your completion of the arduous task of destroying the Temple of Elemental Evil, the Lord Mayor of Greyhawk grants you knighthood.
#################################################################################################################################
	if ( game.quests[26].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# completed TN opening vignette quest AND zuggtmoy dead or banished
		game.moviequeue_add(220)
		# As life returns to normal after the events of the last weeks, the druids of the Gnarley Woods move through Hommlet in an effort to rebalance the area.
#################################################################################################################################
	if (game.quests[27].state == qs_completed):
	# completed CN vignette quest
		game.moviequeue_add(254)
		# Your tasks near Hommlet are complete, and you move onward to new and bigger adventures.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8031 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# thrommel in party and zuggtmoy dead or banished
		if (game.party_alignment == LAWFUL_EVIL):
			game.moviequeue_add(262)
			# Prince Thrommel is most useful in helping overcome Zuggtmoy.  He is most surprised when you kill him afterwards for his sword.
			game.moviequeue_add(255)
			# With the recovery of the artifact sword Fragarach and for safely conveying it a representative of the temple of Hextor, you have advanced the cause of Hextor tremendously. 			# You are granted of favor the god Hextor.
		else:
			game.moviequeue_add(261)
			# Prince Thrommel is most useful in helping overcome Zuggtmoy.
	elif (game.party_alignment == LAWFUL_EVIL):
		if (game.quests[28].state == qs_completed):
		# completed LE vignette quest
			game.moviequeue_add(255)
			# With the recovery of the artifact sword Fragarach and for safely conveying it a representative of the temple of Hextor, you have advanced the cause of Hextor tremendously. 			# You are granted of favor the god Hextor.
		else:
			game.moviequeue_add(256)
			# For failing to deliver the sword Fragarach to the representative of Hextor, you have earned the enmity of the god. The shadow of his curse follows you for years to come.
#################################################################################################################################
	if ( game.quests[29].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
	# completed NE vignette quest and didn't give orb to zuggtmoy and zuggtmoy didn't charm a PC
		game.moviequeue_add(257)
		# You are a richer and more powerful than ever. Yes, this has been an excellent time for evil.
#################################################################################################################################
	if ( game.quests[30].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
	# completed CE vignette quest and didn't give orb to zuggtmoy and zuggtmoy didn't charm a PC
		game.moviequeue_add(258)
		# You killed a lot of good and a lot of evil. You did whatever you wanted and in the end, you bowed to no one.
#################################################################################################################################
	return


#################################################################################################################################
#   SLIDES FOR NC MID-GAME
#################################################################################################################################
# Function: set_end_slides_nc( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up all the end slides before going to verbobonc/co8
#
def set_end_slides_nc( attachee, triggerer ):
#################################################################################################################################
#   ZUGGTMOY
#################################################################################################################################
	if game.global_flags[189] == 1:
	# zuggtmoy dead
		if game.global_flags[183] == 1:
		# took pillar
			game.moviequeue_add(204)
			# Killed after her pillar of platinum was accepted and used to break open the enchanted seal, Zuggtmoy's spirit is banished to the Abyss for 40 years. But she will return to the Prime Material for vengeance...
		elif game.global_flags[326] == 0:
		# haven't destroyed orb
			game.moviequeue_add(200)
			# Zuggtmoy was killed while still trapped on the Prime Material plane. Her spirit destroyed, a major force for evil is now gone from the world of men.
		else:
			game.moviequeue_add(201)
			# Weakened by the destruction of the Orb of Golden Death, Zuggtmoy was killed while still trapped on the Prime Material plane. Her spirit destroyed, a major force for evil is now gone from the world of men.
	elif game.global_flags[188] == 1:
	# zuggtmoy banished
		game.moviequeue_add(202)
		# Her throne defaced, Zuggtmoy is banished to Abyss for 66 years. She plans to reform the Temple on her return.
	elif game.global_flags[326] == 1:
	# destroyed orb
		game.moviequeue_add(203)
		# Four days after the destruction of the Orb of Golden Death, a weakened Zuggtmoy is banished to Abyss for 40 years. But she will return to the Prime Material for vengeance...
	elif game.global_flags[187] == 1:
	# zuggtmoy gave treasure
		game.moviequeue_add(206)
		# For a while, Zuggtmoy remains in her lair, but nature is demonic and she cannot be contained for long. She gathers her strength, and soon her evil machinations are loose in the world again.
#################################################################################################################################
#   IUZ & CUTHBERT
#################################################################################################################################
	if game.global_flags[328] == 1:
	# cuthbert showed up
		game.moviequeue_add(210)
		# Iuz and St. Cuthbert meet. Their �discussion� is not for mortals to witness, but the enmity between their followers escalates.
#################################################################################################################################
#   TEMPLE & ASSOCIATES
#################################################################################################################################
	if ( game.global_flags[146] == 1 and game.global_flags[104] == 1 and game.global_flags[105] == 1 and game.global_flags[106] == 1 and game.global_flags[107] == 1 ):
	# hedrack dead, romag dead, belsornig dead, kelno dead, alrrem dead
		game.moviequeue_add(224)
		# With no high priests left alive after your raids, the temple of elemental evil loses its remaining worshippers, who disband and scatter to the four winds.
#################################################################################################################################
	if (game.global_flags[146] == 0 and game.global_flags[189] == 0):
	# hedrack alive and zuggtmoy alive
		game.moviequeue_add(236)
		# Supreme Commander Hedrack escaped from the Temple of Elemental Evil, but he had earned the displeasure of Iuz for many years to come.
#################################################################################################################################
	if game.global_flags[147] == 0:
	# senshock alive
		game.moviequeue_add(225)
		# Senshock, the Lord Wizard of the Temple, escapes to Verbobonc where he secretly plans a return to power.
#################################################################################################################################
	if game.global_flags[104] == 0:
	# romag alive
		game.moviequeue_add(238)
		# Romag, high priest of the powerful Earth Temple, escaped and was too ashamed to ever return to the Temple of Elemental Evil.
#################################################################################################################################
	if game.global_flags[105] == 0:
	# belsornig alive
		game.moviequeue_add(237)
		# Belsornig, the plotting high priest of the Water temple, managed to escape from the Temple of Elemental Evil unharmed. Surely, he is somewhere on Oerth planning new diabolical schemes even now...
#################################################################################################################################
	if game.global_flags[107] == 0:
	# alrrem alive
		game.moviequeue_add(240)
		# Alrrem, high priest of the Fire Temple, escaped from the Temple of Elemental Evil, but not with his sanity intact. He still roams the countryside as a raving madman, his mind shattered.
#################################################################################################################################
	if game.global_flags[106] == 0:
	# kelno alive
		game.moviequeue_add(239)
		# Kelno, high priest of the Air Temple, escaped from the Temple of Elemental Evil, but without funds or friends, he lives out his days as a beggar on the streets of Mitrik, fearful and paranoid of being discovered by good or evil alike.
#################################################################################################################################
	if game.global_flags[335] == 0:
	# falrinth alive
		game.moviequeue_add(241)
		# Falrinth the wizard managed to escape the Temple of Elemental Evil. He dares not contact any former ally, having made too many enemies while in the service of evil there. He is currently slinking through the underdark, trying to regain the favor of the demoness Lolth, his patron.
#################################################################################################################################
	if game.global_flags[338] == 0:
	# smigmal alive
		game.moviequeue_add(249)
		# Smigmal Redhand, the temple assassin and femme fatale, escaped and returned to the orc tribe where she was spawned to become its leader, the first time a female ever ruled an orc tribe. Her burning hatred for humanity never dimmed. She was eventually eaten by an ogre, who suffered great indigestion afterwards.
#################################################################################################################################
	if game.global_flags[177] == 0:
	# feldrin alive
		game.moviequeue_add(226)
		# Feldrin flees over the Lortmil Mountains and starts his own rogue's guild in the Principality of Ulek. He is eventually killed during a skirmish with a rival guild.
#################################################################################################################################
	if game.global_flags[37] == 0 and ( not(anyone( triggerer.group_list(), "has_follower", 8002 )) ):
	# lareth alive AND not in party
		game.moviequeue_add(212)
		# Lareth the Beautiful escapes and starts his own temple of evil, dedicated to Llolth, the Spider Goddess. She is not placated and eventually turns Lareth into a drider.
	elif game.global_flags[37] == 1:
	# lareth dead
		game.moviequeue_add(213)
		# Lareth is killed, and you gain the enmity of his patron goddess Llolth. Over the years, she dispatches many drow assassins to kill you. All fail.
#################################################################################################################################
#   VIGNETTE RESOLUTION
#################################################################################################################################
	if ( game.quests[23].state == qs_completed and game.global_flags[306] == 1 ):
	# completed NG vignette quest and discovered y'dey
		game.moviequeue_add(252)
		# Discovering Cannoness Y'dey alive proved to be a high point in your group's adventures.
#################################################################################################################################
	if game.quests[24].state == qs_completed:
	# completed CG opening vignette quest
		game.moviequeue_add(221)
		# Countess Tillahi encourages Queen Yolande of Celene to join forces with Veluna to move against Iuz and his city of Molag.
#################################################################################################################################
	if ( game.quests[25].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# completed LN vignette quest and zuggtmoy dead or banished
		game.moviequeue_add(253)
		# In recognition of your completion of the arduous task of destroying the Temple of Elemental Evil, the Lord Mayor of Greyhawk grants you knighthood.
#################################################################################################################################
	if ( game.quests[26].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# completed TN opening vignette quest AND zuggtmoy dead or banished
		game.moviequeue_add(220)
		# As life returns to normal after the events of the last weeks, the druids of the Gnarley Woods move through Hommlet in an effort to rebalance the area.
#################################################################################################################################
	if ( game.quests[29].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
	# completed NE vignette quest and didn't give orb to zuggtmoy and zuggtmoy didn't charm a PC
		game.moviequeue_add(257)
		# You are a richer and more powerful than ever. Yes, this has been an excellent time for evil.
#################################################################################################################################
	if ( game.quests[30].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
	# completed CE vignette quest and didn't give orb to zuggtmoy and zuggtmoy didn't charm a PC
		game.moviequeue_add(258)
		# You killed a lot of good and a lot of evil. You did whatever you wanted and in the end, you bowed to no one.
#################################################################################################################################
#   MOVING ON
#################################################################################################################################
	if game.global_flags[500] == 1:
	# playing NC
		game.moviequeue_add(254)
		# Your tasks near Hommlet are complete, and you move onward to new and bigger adventures.
#################################################################################################################################
	return


#################################################################################################################################
#   ENDSLIDES FOR NC END-GAME
#################################################################################################################################
# Function: set_co8_slides( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up all the co8 slides for the NC end game
#
def set_end_slides_co8( attachee, triggerer ):
#################################################################################################################################
#   INTRO
#################################################################################################################################
	game.moviequeue_add(700)
#################################################################################################################################
#   IUZ & CUTHBERT & ALZOLL
#################################################################################################################################
	if (game.global_flags[327] == 1 or (game.quests[102].state == qs_completed and game.global_vars[694] == 4)):
	# iuz dead or demons and demigods quest completed and iuz's 4 avatars dead
		game.moviequeue_add(533)
		# In an almost unbelievable turn of events, you have managed to destroy Iuz. However, he is not killed but has immediately returned to his soul object on Zuggtmoy's Abyssal plane. It will take him years to recover his powers, during which time his country is overrun and his followers are scattered. Iuz fears you.
		if (game.global_flags[884] == 0 and game.global_vars[898] == 1):
		# asherah alive and asherah saved from iuz
			game.moviequeue_add(536)
			# You saved Asherah from her dismal fate at the hands of Iuz. In the years to come, she becomes one of the greatest philanthropists the city has ever known.
		elif (game.global_flags[884] == 0 and game.global_vars[898] == 2):
		# asherah alive and asherah enslaved by iuz
			game.moviequeue_add(537)
			# You seemingly saved Asherah from her dismal fate at the hands of Iuz, only to find that Iuz's subterfuge had brought about her ruination. She now serves her new master in misery, while Iuz ignores you.
		elif (game.global_flags[884] == 1 and game.global_vars[898] == 3):
		# asherah dead and asherah killed by iuz
			game.moviequeue_add(538)
			# You seemingly saved Asherah from her dismal fate at the hands of Iuz, only to find that Iuz, in his subterfuge, had killed her anyway and exacted his revenge. He eyes you next.
	elif game.global_flags[326] == 1:
	# destroyed orb
		game.moviequeue_add(211)
		# Iuz is weakened by the Orb's destruction. He now ranks you among his personal enemies and plans a revenge suitable for this annoyance.
#################################################################################################################################
	if (game.global_flags[884] == 1 and game.global_vars[898] == 4):
	# asherah dead and you ran away from iuz
		game.moviequeue_add(539)
		# You ran away from Iuz. In your wake, Iuz visited Asherah and had his revenge. She is dead.
#################################################################################################################################
	if (game.quests[102].state == qs_completed and game.global_vars[695] == 4):
	# demons and demigods quest completed and cuthbert's 4 avatars dead
		game.moviequeue_add(535)
		# In an almost unbelievable turn of events, you have managed to destroy St. Cuthbert. However, he is not killed but has immediately returned to his Bastion of Law, the Basilica of Saint Cuthbert. It will take him years to recover his powers, during which time his church is weakened and his followers are lessened. St. Cuthbert fears you.
		if (game.global_flags[884] == 0 and game.global_vars[898] == 5):
		# asherah alive and asherah saved from cuthbert
			game.moviequeue_add(544)
			# You delivered Asherah from her grim fate at the hands of St. Cuthbert. In the years to come, she becomes one of the most cutthroat financial provocateurs the city has ever known.
		elif (game.global_flags[884] == 0 and game.global_vars[898] == 6):
		# asherah alive and asherah enslaved by cuthbert
			game.moviequeue_add(545)
			# You seemingly saved Asherah from her grim fate at the hands of St. Cuthbert, only to find that Cuthbert's subterfuge had brought about her ruination. She now serves her new master in misery, while Cuthbert ignores you.
		elif (game.global_flags[884] == 1 and game.global_vars[898] == 7):
		# asherah dead and asherah killed by cuthbert
			game.moviequeue_add(546)
			# You seemingly saved Asherah from her grim fate at the hands of St. Cuthbert, only to find that Cuthbert, in his subterfuge, had killed her anyway and exacted his revenge. He eyes you next.
#################################################################################################################################
	if (game.global_flags[884] == 1 and game.global_vars[898] == 8):
	# asherah dead and you ran away from cuthbert
		game.moviequeue_add(547)
		# You ran away from St. Cuthbert. In your wake, Cuthbert visited Asherah and had his revenge. She is dead.
#################################################################################################################################
	if (game.quests[102].state == qs_completed and game.global_vars[749] == 4):
	# demons and demigods quest completed and alzoll and company dead
		game.moviequeue_add(534)
		# In an incredible turn of events, you have managed to destroy the balor Alzoll and its cohorts Errtu, Ter-Soth, and Wendonai. The demon lords, klurichirs, and myrmyxicus of the Abyss now fear you as well.
		if (game.global_flags[884] == 0 and game.global_vars[898] == 9):
		# asherah alive and asherah saved from alzoll
			game.moviequeue_add(540)
			# You saved Asherah from her dire fate at the hands of Alzoll. In the years to come, she goes on to prosper even more greatly, to the bane of some and the boon of others.
		elif (game.global_flags[884] == 0 and game.global_vars[898] == 10):
		# asherah alive and asherah enslaved by alzoll
			game.moviequeue_add(541)
			# You seemingly saved Asherah from her dire fate at the hands of Alzoll, only to find that his subterfuge had brought about her ruination. She now serves her new master in misery, while Alzoll mocks you from The Abyss.
		elif (game.global_flags[884] == 1 and game.global_vars[898] == 11):
		# asherah dead and asherah killed by alzoll
			game.moviequeue_add(542)
			# You seemingly saved Asherah from her dire fate at the hands of Alzoll, only to find that in his subterfuge, he had killed her anyway and exacted his revenge. Alzoll eyes you next.
#################################################################################################################################
	if (game.global_flags[884] == 1 and game.global_vars[898] == 12):
	# asherah dead and you ran away from alzoll
		game.moviequeue_add(543)
		# You ran away from Alzoll. In your wake, Alzoll visited Asherah and had his revenge. She is dead.
#################################################################################################################################
#   VERBOBONC & ASSOCIATES
#################################################################################################################################
	if (game.global_flags[935] == 0 and game.global_flags[992] == 0 and game.global_flags[971] == 0 and game.quests[74].state == qs_completed and game.quests[78].state == qs_completed and game.quests[97].state == qs_completed):
	# wilfrick alive and elysia alive and frozen assets quest completed and slave traders quest completed and war of the golden skull defender quest completed
		game.moviequeue_add(503)
		# Lord Viscount Wilfrick is hailed as protector of Verbobonc for thwarting the plans of its worst enemies; a year later, a downturn in the economy prompts his scheming daughter Elysia to gain support and force him to abdicate the seat of power.
#################################################################################################################################
	if (game.global_flags[995] == 1 and game.quests[74].state == qs_completed):
	# white dragon dead and frozen assets quest completed
		game.moviequeue_add(504)
		# The death of the ice dragon is eventually discovered by other dragons, and in their fury over the slaughter, dragon attacks quickly become a common threat when traveling across the countryside.
	elif (game.global_flags[995] == 0 and game.quests[74].state != qs_completed):
	# white dragon alive and frozen assets quest not completed
		game.moviequeue_add(505)
		# Many Verbobonc merchants are put out of business due to mysterious cold weather circumstances, but after the area becomes a known danger, trading routes are reworked and the threat becomes no more.
#################################################################################################################################
	if (game.global_flags[949] == 1 and game.quests[78].state == qs_completed):
	# tarah dead and slave traders quest completed
		game.moviequeue_add(506)
		# The fissures of Emridy Meadows become a popular destination for thrill-seekers wishing to tap into whatever dark static energy remains from your legendary liberation of the slave children from Tarah Ka Turah. Many never return, though the fissures themselves never divulge what secrets they swallow, nor say who waits for visitors below.
	elif (game.quests[78].state == qs_botched):
	# slave traders quest botched
		game.moviequeue_add(507)
		# The fissures of Emridy Meadows become a popular destination for thrill-seekers wishing to tap into whatever dark static energy remains from your attack on the liberators of the slave children. Many never return, though the fissures themselves never divulge what secrets they swallow, nor say who waits for visitors below.
#################################################################################################################################
	if (game.quests[97].state == qs_completed and game.global_flags[501] == 1):
	# war of the golden skull defender quest completed and wakefield dead
		game.moviequeue_add(555)
		# You defended Hommlet from the vicious attack by Wakefield and his militant band of Hextorites. Hextor mourns not the loss of his instrument Wakefield, but resigns himself to the loss of the Golden Skull and the power that would have come with it.
	elif (game.quests[103].state == qs_completed):
	# war of the golden skull attacker quest completed
		game.moviequeue_add(556)
		# You helped Wakefield and the Hextorites pillage Hommlet and murder the townsfolk. For many years afterward, no one dares resettle the village for fear of awakening its sleeping ghosts.
#################################################################################################################################
	if ((game.quests[76].state == qs_completed or game.quests[87].state == qs_completed or game.quests[88].state == qs_completed) or game.global_flags[989] == 1):
	# catching the cousin quest or snitching on the sb quest or narcing on the sb quest completed OR darlia dead
		game.moviequeue_add(528)
		# You put a halt to the Scarlet Brotherhood's dubious plots in Verbobonc. And having failed at their attempts for revenge, they regress to a third rate fencing operation within the viscounty.
		game.moviequeue_add(529)
		# With the Scarlet Brotherhood run out of Verbobonc, crime statistics reach an all time low. But the labyrinth network of tunnels below the city causes new outbreaks of disease and plague.
	elif (game.quests[77].state == qs_completed or game.global_flags[935] == 1 or game.global_flags[992] == 1):
	# removing wilfrick quest completed or wilfrick dead
		game.moviequeue_add(527)
		# You murdered Lord Viscount Wilfrick, allowing the deceitful Lerrik and the Scarlet Brotherhood to assume control of the city. Verbobonc wallows in political corruption for years to come, and its citizens and those of the viscounty suffer profoundly.
#################################################################################################################################
	if (game.quests[84].state == qs_completed and game.global_flags[844] == 1):
	# contract on canon thaddeus quest completed and thaddeus dead
		game.moviequeue_add(530)
		# You murdered Canon Thaddeus on his way from Narwell to Verbobonc. The following season, his corpse is reanimated by wights. Now the Canon roams the lonely roads by night, looking for converts to his "new religion."
	elif (game.quests[86].state == qs_completed):
	# ratting out the hextorites quest completed
		game.moviequeue_add(531)
		# You saved Canon Thaddeus from being killed by the Hextorites. But the following season, his travel caravan is overrun by wights. Now the Canon roams the lonely roads by night, looking for converts to his "new religion."
#################################################################################################################################
	if (game.quests[90].state == qs_completed and game.global_flags[929] == 1):
	# gremlich quest completed and gremlich dead
		game.moviequeue_add(532)
		# With the dreaded Gremlich banished, for now, you're left to wonder what may be in store for you and your descendants 49 years into your retirement.
#################################################################################################################################
	if (game.quests[69].state == qs_completed and game.global_flags[885] == 0):
	# under attack from underground quest completed and didn't get proof with holly
		game.moviequeue_add(508)
		# You cleared the Drow from the Gnome Tunnels beneath the city. While rumors of your actions and the purported existence of Drow persist, most view such stories as old wives' tales.
	elif (game.quests[69].state == qs_completed and game.global_flags[885] == 1):
	# under attack from underground quest completed and got proof with holly
		game.moviequeue_add(509)
		# You cleared the Drow from the Gnome Tunnels beneath the city. Corporal Holly's testimony lends credence to this tale, and citizens are left to wonder what brought the Drow to Verbobonc in the first place.
#################################################################################################################################
	if (game.global_flags[996] == 1):
	# bought the CotL
		if (game.global_vars[765] >= 1):
		# killed the big 3
			if (game.quests[83].state == qs_completed):
			# completed fear of ghosts quest
				game.moviequeue_add(512)
				# In your lavish retirement, you outgrow the Castle of the Lords. Years later, the gnome quarter buys it out from you and gives it to the the city as a museum. It remains untroubled by spirits.
			elif (game.quests[83].state != qs_completed):
			# didn't complete fear of ghosts quest
				game.moviequeue_add(511)
				# Your dreams remain troubled in the Castle of the Lords. Years later, the gnome quarter buys it out from you and gives it to the the city as a museum. Visitors report strange sights and sounds.
		elif (game.global_vars[765] == 0):
		# didn't kill the big 3
			game.moviequeue_add(513)
			# In your lavish retirement, you outgrow the Castle of the Lords. Years later, the gnome quarter buys it out from you and gives it to the the city as a museum.
		if (game.global_vars[886] == 1):
		# met orrengaard
			game.moviequeue_add(510)
			# The mechanism controlling the panel that leads to the Rabbit Hole in the Castle of the Lords is mysteriously broken, preventing any further visits to that intangible locale. Custodians of the castle insist no such panel ever existed, and the gutting of the stairwell reveals no hidden chamber. But the faint sounds of gnomish merrymaking can sometimes be heard by visitors to the castle basement in the wee hours of the night.
	elif (game.global_flags[996] == 0):
	# didn't buy the CotL
		game.moviequeue_add(514)
		# After standing vacant for years, the gnome quarter buys out the Castle of the Lords from the Silver Consortium and gives it to the the city as a museum. It's repossessed after the gnomes fail to make timely payments.
#################################################################################################################################
	if (game.quests[79].state == qs_completed and game.global_flags[956] == 1):
	# wanted gladstone quest completed and gladstone dead
		game.moviequeue_add(518)
		# With Gunter Gladstone dead by your hand, the Viscounty is spared numerous future robberies, rapes, and torturings.
	elif (game.quests[79].state == qs_completed and game.global_flags[956] == 0):
	# wanted gladstone quest completed and gladstone alive
		game.moviequeue_add(519)
		# Gunter Gladstone is incarcerated. He remains so for 30 days before mysteriously dying in his sleep.
	elif ((game.quests[79].state == qs_mentioned or game.quests[79].state == qs_accepted) and game.global_flags[956] == 0):
	# wanted gladstone quest mentioned or accepted and gladstone alive
		game.moviequeue_add(520)
		# Gunter Gladstone flees to Dyvers, where he soon joins with an organization devoted to supplying the burgeoning demands of the child slavery underground.
#################################################################################################################################
	if (game.quests[80].state == qs_completed and game.global_flags[957] == 1):
	# wanted commonworth quest completed and commonworth dead
		game.moviequeue_add(521)
		# With Kendrew Commonworth dead by your hand, the wild places of the Viscounty are that much safer for good folk, and children's tales of terror by night are reduced to legend.
	elif (game.quests[80].state == qs_completed and game.global_flags[957] == 0):
	# wanted commonworth quest completed and commonworth alive
		game.moviequeue_add(522)
		# Kendrew Commonworth is incarcerated. He is eventually killed by an unlikely team of jailers and prisoners when he breaks out of his cell and sets out to kill them all rather than flee.
	elif ((game.quests[80].state == qs_mentioned or game.quests[80].state == qs_accepted) and game.global_flags[957] == 0):
	# wanted commonworth quest mentioned or accepted and commonworth alive
		game.moviequeue_add(523)
		# Kendrew Commonworth remains hidden in the wilds. He occasionally preys upon a remote farmstead or woodsman's cottage to sustain his grotesque needs.
#################################################################################################################################
	if (game.quests[81].state == qs_completed and game.global_flags[958] == 1):
	# wanted corpus quest completed and corpus dead
		game.moviequeue_add(524)
		# With Quintus Corpus dead by your hand, a new 'visionary' of extreme redemption arises and begins to administer his brand of purification, morbidly inspired by Corpus' memory and deeds.
	elif (game.quests[81].state == qs_completed and game.global_flags[958] == 0):
	# wanted corpus quest completed and corpus alive
		game.moviequeue_add(525)
		# Quintus Corpus is incarcerated. He becomes something of a messiah to many of his fellow inmates, preaching incessantly about the need to purge the evils from good men and women.
	elif ((game.quests[81].state == qs_mentioned or game.quests[81].state == qs_accepted) and game.global_flags[958] == 0):
	# wanted corpus quest mentioned or accepted and corpus alive
		game.moviequeue_add(526)
		# Quintus Corpus returns to Celene in a vain attempt to formally reclaim his role of 'paladin.' He is captured and put to death in the town square the following morning.
#################################################################################################################################
	if (game.quests[109].state == qs_completed and game.global_vars[542] == 1 and game.global_flags[540] == 0):
	# what lies beneath quest completed and boroquin guilty and boroquin alive
		game.moviequeue_add(515)
		# Boroquin finishes his days incarcerated in the Verbobonc lockup. He dies late one night, raving about two children at his bedside though jailer reports insist he was alone.
	elif (game.quests[109].state == qs_completed and game.global_vars[542] == 2 and game.global_flags[809] == 0):
	# what lies beneath quest completed and panathaes guilty and panathaes alive
		game.moviequeue_add(517)
		# Panathaes is readmitted to the Verbobonc asylum, where he regales his fellow inmates daily with tales of his psychic bond with the spirit of Abel Mol. He doesn't share the fact that every night he hears giggling voices from the drainage grate in his cell beckoning him to come back down.
	elif (game.quests[109].state == qs_completed and game.global_vars[542] == 3 and game.global_flags[539] == 0):
	# what lies beneath quest completed and rakham guilty and rakham alive
		game.moviequeue_add(516)
		# Rakham is transferred to a labor camp, where he breaks rocks for seven years before being granted parole. He retires to seclusion and takes work as a night watchman.
#################################################################################################################################
	if (game.global_flags[562] == 1 and game.global_flags[564] == 1):
	# angra mainyu dead and phylactery destroyed
		game.moviequeue_add(553)
		# You defeated the lich Angra Mainyu at Hickory Branch. With its phylactery destroyed, a terrible force of evil has been removed from the region.
	elif (game.global_flags[562] == 1 and game.global_flags[564] == 0):
	# angra mainyu dead and phylactery not destroyed
		game.moviequeue_add(554)
		# You defeated the lich Angra Mainyu at Hickory Branch. But with its phylactery intact, you're left to wonder when the monster will return.
#################################################################################################################################
	if (game.quests[62].state == qs_completed and game.global_flags[560] == 1 and game.global_flags[561] == 1 and game.global_flags[562] == 1):
	# hickory branch quest completed and hungous dead and noostoron dead and angra mainyu dead
		game.moviequeue_add(551)
		# Having routed the orc invasion force, the hickory branch area becomes a safe rest stop area for travelers once again. It's discovered to be a location rich in natural ores, presenting opportunity for a new quarry.
		if (game.global_flags[572] == 1):
		# you delayed finishing hb
			game.moviequeue_add(552)
			# However, the start-stop rhythm of your assault on Hickory has allowed its former inhabitants ample time for enacting their contingency plan. Cursed are you now; for the fruit of your loins shall all be Orcs, burning with militaristic ambition.
#################################################################################################################################
	if (game.quests[62].state == qs_completed and game.global_flags[826] == 0):
	# hickory branch quest completed and bethany alive
		game.moviequeue_add(549)
		# You assisted Captain Bethany in validating her findings and discrediting Captain Asaph, leading to her promotion to head of the Regional Patrol. In the years that follow, the countryside becomes safer and better policed, but the institution itself is more greatly influenced by political maneuvering. Bethany, meanwhile, profits greatly.
		if (game.global_flags[962] == 0 and game.global_flags[419] == 0):
		# holly alive and absalom alive
			game.moviequeue_add(550)
			# Corporal Holly is soon promoted to Captain of the Watch in Verbobonc, leapfrogging a number of lieutenants and sergeants. Due to her new found power and influence, no one dares suggest that Captain Bethany had anything to do with her friend's ascension, though former Captain Absalom, demoted to sergeant, has his suspicions.
#################################################################################################################################
	if (game.quests[96].state == qs_completed and game.global_flags[506] == 1):
	# of castles and quarries quest completed and battlehammer dead
		game.moviequeue_add(557)
		# Following your sacking of the dwarves at the Hommlet quarry, dwarven emissaries from the Lortmil Mountains appear in Verbobonc to account for King Battlehammer and his men, now revealed to be a rogue contingent of miners expelled from the Lortmils for disavowing their true king.
#################################################################################################################################
	if (game.quests[95].state == qs_completed and game.global_flags[249] == 1):
	# season of the witch quest completed and MR witch dead
		game.moviequeue_add(548)
		# The moathouse, having served and deposed three different masters in the course of its life, is slowly reclaimed by its final proprietor, mother nature.
#################################################################################################################################
#   PRINCE THROMMEL
#################################################################################################################################
	if game.global_flags[150] == 1:
	# thrommel dead
		game.moviequeue_add(215)
		# With the death of Prince Thrommel, divined by the priests of Furyondy, that kingdom suffers greatly in the wars to come.
	elif (game.global_flags[151] == 1) or (game.global_flags[152] == 1) or (anyone( triggerer.group_list(), "has_follower", 8031)):
	# thrommel freed and out OR thrommel reward scheduled OR thrommel in party
		game.moviequeue_add(214)
		# Prince Thrommel is rescued and marries Princess Jolene of Veluna. The kingdoms of Furyondy and Veluna unite, and you are made knights of the kingdom.
	else:
		game.moviequeue_add(216)
		# Left inside the temple dungeons, Prince Thrommel eventually becomes a vampire and strikes terror into the local populace. His sword Fragarach is lost in the mists of time...
#################################################################################################################################
#   NULB & ASSOCIATES
#################################################################################################################################
	if game.global_flags[187] == 1:
	# zuggtmoy gave you treasure
		game.moviequeue_add(247)
		# As Zuggtmoy was spared, so was Nulb. Its wicked inhabitants continued their evil ways unchecked.
	elif ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ):
	# zuggtmoy dead or banished
		game.moviequeue_add(248)
		# With the destruction of the Temple of Elemental Evil, the fall of the village of Nulb came soon thereafter.
#################################################################################################################################
	if ( game.global_flags[97] == 1 and game.global_flags[329] == 1 ):
	# tolub dead and grud dead
		game.moviequeue_add(223)
		# With the deaths of Tolub and Grud Squinteye, the river pirates lose direction and eventually disband.
#################################################################################################################################
	if ( game.global_flags[90] == 1 and game.global_flags[324] == 1 and game.global_flags[365] == 0 ):
	# gay bertram free AND gay bertram met AND gay betram alive
		game.moviequeue_add(218)
		# You and Bertram are married in a small ceremony, and he opens a dentistry office in Verbobonc. You live happily ever after.
#################################################################################################################################
	if ( game.global_flags[83] == 1 and game.global_flags[330] == 0 ):
	# exclusive with jenelda and jenelda alive
		game.moviequeue_add(231)
		# After your beautiful wedding, you and Jenelda travel throughout Oerth. You were last seen in the east in the Grandwood Forest.
#################################################################################################################################
#   HOMMLET & ASSOCIATES
#################################################################################################################################
	if ( game.global_flags[299] == 0 and game.global_flags[337] == 0 and game.global_flags[336] == 0 and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) and game.quests[103].state != qs_completed ):
	# terjon alive and jaroo alive and burne and rufus alive and zuggtmoy dead or banished and didn't complete wotgs attacker
		game.moviequeue_add(243)
		# In the quiet years that followed the fall of the temple of elemental evil, Hommlet grows into a bustling and prosperous town under the guidance of its elder citizens.
	elif ( ( game.global_flags[299] == 1 or game.global_flags[337] == 1 or game.global_flags[336] == 1 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) and game.quests[103].state != qs_completed ):
	# terjon dead or jaroo dead or burne or rufus dead and zuggtmoy dead or banished and didn't complete wotgs attacker
		game.moviequeue_add(244)
		# As is often the case in sleepy, little villages, the excitement of the destruction of the temple soon died down, and life in Hommlet continued unaffected by such portentous events.
#################################################################################################################################
	if game.quests[65].state == qs_completed:
	# completed hero's prize quest
		game.moviequeue_add(502)
		# You have risen to the challenge of the Arena of Heroes, and emerged victorious, a champion of the arena. Many years would pass before a new group of adventurers, seeking to stamp out an emerging cult of Tharizdun, comes to entertain the ethereal crowd of the arena once more.
#################################################################################################################################
	if ( game.quests[73].state == qs_completed and game.global_flags[976] == 1 ):
	# completed welkwood bog quest and mathel dead
		game.moviequeue_add(501)
		# Mathel, the would-be lich of Welkwood Bog, is no more. The people of Hommlet pray for his redemption; Beory takes heed, and reincarnates his soul as a Swamp Lotus.
#################################################################################################################################
	if ( game.quests[15].state != qs_completed and game.global_flags[336] == 0 and game.quests[103].state != qs_completed ):
	# didn't complete agent revealed quest and burne or rufus dead and didn't complete wotgs attacker
		game.moviequeue_add(242)
		# The tower of Burne and Rufus remained under construction for many years, plagued by mysterious delays and inauspicious accidents.
#################################################################################################################################
	if (game.quests[20].state == qs_completed and game.quests[103].state != qs_completed):
	# rescued paida and didn't complete wotgs attacker
		game.moviequeue_add(250)
		# In returning Paida to Valden, you have overcome their history of misery and pain and allowed true love to prosper.
#################################################################################################################################
	if (game.global_flags[61] == 1 and game.quests[103].state != qs_completed):
	# furnok retired and didn't complete wotgs attacker
		game.moviequeue_add(222)
		# Furnok of Ferd uses his wealth to become a well-known and well-respected citizen of Verbobonc.
#################################################################################################################################
	if ( game.quests[12].state == qs_completed and game.global_flags[364] == 0 and game.quests[103].state != qs_completed ):
	# completed one bride for one player quest AND fruella alive and didn't complete wotgs attacker
		game.moviequeue_add(219)
		# As you promised, you and Fruella are married in the church of St. Cuthbert. She tames your wild, adventuring nature, and you settle down as a farmer in Hommlet and have many, many, many children.
#################################################################################################################################
	if ( game.global_flags[68] == 1 and game.global_flags[331] == 0 and game.quests[103].state != qs_completed ):
	# married laszlo and laszlo alive and didn't complete wotgs attacker
		game.moviequeue_add(230)
		# You marry Laszlo. You settle down with him, live the life of a herdsman's wife and have many children.
#################################################################################################################################
	if ( game.global_flags[46] == 1 and game.global_flags[196] == 0 and game.global_flags[318] == 0 and game.quests[103].state != qs_completed ):
	# married meleny and meleny not dead or mistreated and didn't complete wotgs attacker
		if game.global_flags[332] == 1:
		# killed an ochre jelly
			game.moviequeue_add(232)
			# You marry Meleny and take her on all sorts of grand adventures.
		else:
			game.moviequeue_add(233)
			# You marry Meleny and take her on all sorts of grand adventures, until an ochre jelly eats her.
#################################################################################################################################
	if ( game.quests[6].state == qs_completed and game.global_flags[333] == 0 and game.global_flags[334] == 0 and game.quests[103].state != qs_completed ):
	# completed cupid's arrow quest and filliken alive and mathilde alive and didn't complete wotgs attacker
		if game.global_flags[332] == 1:
		# killed an ochre jelly
			game.moviequeue_add(234)
			# You convince Filliken and Mathilde to marry, and they enjoy many happy years together in Hommlet.
		else:
			game.moviequeue_add(235)
			# You convince Filliken and Mathilde to marry, and they enjoy many happy years together in Hommlet, until they are eaten by ochre jellies.
#################################################################################################################################
#   JOINABLE NPCS
#################################################################################################################################
	if game.global_vars[29] >= 10:
	# 10 or more NPCs killed while in party
		game.moviequeue_add(217)
		# You abuse your faithful followers, many of whom lose their lives in the dungeons beneath the temple.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8060 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# morgan in party and zuggtmoy dead or banished
		game.moviequeue_add(263)
		# Morgan survives the encounter with Zuggtmoy and thanks you most profusely for saving his life. He returns to his life as a pirate, and eventually becomes a feared captain of his own ship on the high seas.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8023 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# oohlgrist in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(265)
		# Like the coward he is, Oohlgrist sneaks off soon after your encounter with Zuggtmoy. He is never heard from again.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8034 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# scorrp in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(266)
		# To show his gratitude, Scorpp swears loyalty to you and is evermore your faithful servant.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8061 ) and anyone( triggerer.group_list(), "has_follower", 8029 ) and anyone( triggerer.group_list(), "has_follower", 8030 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# ted, ed, and ed in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(264)
		# The stars flicker, the gods pause, the winds stop. Throughout the world there is silence, as if the world has stopped to catch its breath. For you have survived the game with Ted, Ed and Ed. Congratulations.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8014 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# otis in party and zuggtmoy dead or banished
		game.moviequeue_add(259)
		# Otis is very pleased with the results of your adventures together. His reports to Furyondy and Veluna gain you even more favor in those countries.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8021 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# y'dey in party and zuggtmoy dead or banished
		game.moviequeue_add(260)
		# Y'dey is overjoyed to have adventured with you. Her reports to the Archcleric of Veluna feature you most prominently, and the Archcleric is impressed with your results.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8062 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 or game.global_flags[185] == 1) ):
	# zaxis in party and zuggtmoy dead or banished or surrendered
		game.moviequeue_add(267)
		# Shortly after your Temple adventure, Zaxis leaves your party to continue looking for his sister. He finally finishes his song about your grand adventure, although he never does find a word that rhymes with Zuggtmoy.
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8730 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# ronald in party and zuggtmoy dead or banished
		game.moviequeue_add(500)
		# At the conclusion of your adventures, Ronald returns to his position at the Church of St. Cuthbert in Hommlet. He serves as a much higher ranking official than when he left, eventually moving to and joining a larger church in Mitrik.
#################################################################################################################################
#   VIGNETTE RESOLUTION
#################################################################################################################################
	if ( anyone( triggerer.group_list(), "has_follower", 8031 ) and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
	# thrommel in party and zuggtmoy dead or banished
		if (game.party_alignment == LAWFUL_EVIL):
			game.moviequeue_add(262)
			# Prince Thrommel is most useful in helping overcome Zuggtmoy.  He is most surprised when you kill him afterwards for his sword.
			game.moviequeue_add(255)
			# With the recovery of the artifact sword Fragarach and for safely conveying it a representative of the temple of Hextor, you have advanced the cause of Hextor tremendously. You are granted of favor the god Hextor.
		else:
			game.moviequeue_add(261)
			# Prince Thrommel is most useful in helping overcome Zuggtmoy.
	elif (game.party_alignment == LAWFUL_EVIL):
		if (game.quests[28].state == qs_completed):
		# completed LE vignette quest
			game.moviequeue_add(255)
			# With the recovery of the artifact sword Fragarach and for safely conveying it a representative of the temple of Hextor, you have advanced the cause of Hextor tremendously. You are granted of favor the god Hextor.
		else:
			game.moviequeue_add(256)
			# For failing to deliver the sword Fragarach to the representative of Hextor, you have earned the enmity of the god. The shadow of his curse follows you for years to come.
#################################################################################################################################
	return


# Function: set_join_slides( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up the end slides if you join the temple
#
def	set_join_slides( attachee, triggerer ):
	if game.global_flags[327] == 1:
		game.moviequeue_add(209)
	elif game.global_flags[326] == 1:
		game.moviequeue_add(211)
	if game.global_flags[328] == 1:
		game.moviequeue_add(210)
	if game.global_flags[37] == 0 and ( not(anyone( triggerer.group_list(), "has_follower", 8002 )) ):
		game.moviequeue_add(212)
	elif game.global_flags[37] == 1:
		game.moviequeue_add(213)
	if game.global_flags[150] == 1:
		game.moviequeue_add(215)
	if game.global_vars[29] >= 10:
		game.moviequeue_add(217)
	if ( game.global_flags[90] == 1 and game.global_flags[324] == 1 and game.global_flags[365] == 0 ):
		game.moviequeue_add(218)
	if ( game.quests[12].state == qs_completed and game.global_flags[364] == 0 ):
		game.moviequeue_add(219)
	if ( game.quests[26].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
		game.moviequeue_add(220)
	if game.quests[24].state == qs_completed:
		game.moviequeue_add(221)
	if game.global_flags[61] == 1:
		game.moviequeue_add(222)
	if ( game.global_flags[97] == 1 and game.global_flags[329] == 1 ):
		game.moviequeue_add(223)
	if ( game.global_flags[68] == 1 and game.global_flags[331] == 0 ):
		game.moviequeue_add(230)
	if ( game.global_flags[83] == 1 and game.global_flags[330] == 0 ):
		game.moviequeue_add(231)
	if ( game.global_flags[46] == 1 and game.global_flags[196] == 0 and game.global_flags[318] == 0 ):
		if game.global_flags[332] == 1:
			game.moviequeue_add(232)
		else:
			game.moviequeue_add(233)
	if ( game.quests[6].state == qs_completed and game.global_flags[333] == 0 and game.global_flags[334] == 0 ):
		if game.global_flags[332] == 1:
			game.moviequeue_add(234)
		else:
			game.moviequeue_add(235)
	if ( game.quests[15].state != qs_completed and game.global_flags[336] == 0 ):
		game.moviequeue_add(242)
	game.moviequeue_add(245)
	game.moviequeue_add(246)
	if game.quests[20].state == qs_completed:
		game.moviequeue_add(250)
	if game.global_flags[339] == 1:
		game.moviequeue_add(251)
	if ( game.quests[23].state == qs_completed and game.global_flags[306] == 1 ):
		game.moviequeue_add(252)
	if ( game.quests[25].state == qs_completed and ( game.global_flags[189] == 1 or game.global_flags[188] == 1 ) ):
		game.moviequeue_add(253)
	if game.quests[27].state == qs_completed:
		game.moviequeue_add(254)
	if (game.party_alignment == LAWFUL_EVIL):
		if (game.quests[28].state == qs_completed):
			game.moviequeue_add(255)
		else:
			game.moviequeue_add(256)
	if ( game.quests[29].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
		game.moviequeue_add(257)
	if ( game.quests[30].state == qs_completed and game.global_flags[190] == 0 and game.global_flags[191] == 0 ):
		game.moviequeue_add(258)
	game.moviequeue_add(206)	
	return


# Function: should_heal_hp_on( obj )
# Author  : Tom Decker
# Returns : 1 if character is not at full health, else 0
# Purpose : to heal only characters that need it
#
def should_heal_hp_on( obj ):
	cur = obj.stat_level_get( stat_hp_current )
	max = obj.stat_level_get( stat_hp_max )
	if (not cur == max) and (obj.stat_level_get(stat_hp_current) >= -9):
		return 1
	return 0

# Function: should_heal_disease_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is diseased, else 0
# Purpose : to heal only characters that need it
#
def should_heal_disease_on( obj ):
    # check if obj is diseased
	if (obj.stat_level_get(stat_hp_current) >= -9):
		return 1
	return 0

# Function: should_heal_poison_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is poisoned, else 0
# Purpose : to heal only characters that need it
#
def should_heal_poison_on( obj ):
	# check if obj has poison on them
	if (obj.stat_level_get(stat_hp_current) >= -9):
		return 1
	return 0

# Function: should_resurrect_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is dead, else 0
# Purpose : to heal only characters that need it
#
def should_resurrect_on( obj ):
	if (obj.stat_level_get(stat_hp_current) <= -10):
		return 1
	return 0

def zap( attachee, triggerer ):
	damage_dice = dice_new( '2d4' )
	game.particles( 'sp-Shocking Grasp', triggerer )
	if triggerer.reflex_save_and_damage( OBJ_HANDLE_NULL, 20, D20_Save_Reduction_Half, D20STD_F_NONE, damage_dice, D20DT_ELECTRICITY, D20DAP_UNSPECIFIED, 0 , D20DAP_NORMAL ):
#		saving throw successful
		triggerer.float_mesfile_line( 'mes\\spell.mes', 30001 )
	else:
#		saving throw unsuccessful
		triggerer.float_mesfile_line( 'mes\\spell.mes', 30002 )

	return RUN_DEFAULT

#### TESTING UTILITIES
def flash_signal(on_whom = 0, effect_type = -1):
	party_size = len(game.party)
	if effect_type == -1:
		effect_type = on_whom
	while on_whom > party_size:
		on_whom = on_whom - party_size

	game.leader.float_mesfile_line( 'mes\\test.mes', effect_type )
	if effect_type == 0:
		game.particles( "sp-summon monster I", game.party[on_whom])
	elif effect_type == 1:
		game.particles( 'Orb-Summon-Air-Elemental', game.party[on_whom])
	elif effect_type == 2:
		game.particles( 'Orb-Summon-Fire-Elemental', game.party[on_whom])
	elif effect_type == 3:
		game.particles( 'sp-Righteous Might', game.party[on_whom])
	elif effect_type == 4:
		game.particles( 'sp-Dimension Door', game.party[on_whom])

def uberize():
	import t
	t.uberize()

def homm():
	import teleport_shortcuts
	teleport_shortcuts.homm()
	
def inn():
	import teleport_shortcuts
	teleport_shortcuts.inn()

def shopmap():
	import teleport_shortcuts
	teleport_shortcuts.shopmap()

def emridy():
	import teleport_shortcuts
	teleport_shortcuts.emridy()
	
def tp(locName):
	try:
		import teleport_shortcuts
		exec('teleport_shortcuts.'+locName + '()')
	except:
		print "Bad location name"

def restup():
	for pc in game.party[0].group_list():
		pc.spells_pending_to_memorized() # Memorizes Spells
		pc.obj_set_int( obj_f_hp_damage, 0) # Removes all damage (doesn't work for companions?)
		if pc.stat_level_get(stat_level_bard) >= 1:
			pc.spells_cast_reset(stat_level_bard)
		if pc.stat_level_get(stat_level_sorcerer) >= 1:
			pc.spells_cast_reset(stat_level_sorcerer)

def rsetup(): # I commonly make this typo :P - SA
	restup()

def reward_pc( proto_id, pc, npc = OBJ_HANDLE_NULL):
	aaa = game.obj_create( proto_id, game.leader.location )
	if pc.item_get(aaa) == 1:
		return
	for obj in game.party:
		if obj.item_get(aaa) == 1:
			return
	return
	
def party_closest( attachee, conscious_only = 1, mode_select = 1, exclude_warded = 0, exclude_charmed = 1, exclude_spiritual_weapon = 1):
	# mode select: 0 - PCs only; 1 - Controllable PCs and NPCs (summons excluded); 2- group_list
	closest_one = OBJ_HANDLE_NULL
	for obj in game.leader.group_list():
		if ( (obj.is_unconscious() == 0 ) or (conscious_only == 0) ) and (obj.type == obj_t_pc or mode_select >= 1) and (obj.d20_query(Q_ExperienceExempt) == 0 or mode_select == 2 or obj.type == obj_t_pc):
			if closest_one == OBJ_HANDLE_NULL:
				if ( (obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) == 0 and obj.d20_query_has_spell_condition(sp_Meld_Into_Stone) == 0 ) or exclude_warded == 0) and (obj.d20_query(Q_Critter_Is_Charmed) == 0 or exclude_charmed == 0  ) and ( ( not obj.name in [14370, 14604, 14621, 14629] ) or exclude_spiritual_weapon == 0 ):
					closest_one = obj
			elif attachee.distance_to(obj) <= attachee.distance_to(closest_one):
				if ( (obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) == 0 and obj.d20_query_has_spell_condition(sp_Meld_Into_Stone) == 0 ) or exclude_warded == 0) and (obj.d20_query(Q_Critter_Is_Charmed) == 0 or exclude_charmed == 0  ) and ( ( not obj.name in [14370, 14604, 14621, 14629] ) or exclude_spiritual_weapon == 0 ):
					closest_one = obj
	return closest_one
	
def willing_and_capable( attachee ): # meant for NPCs, to see if they should be capable of manoeuvring
	if attachee.leader_get() == OBJ_HANDLE_NULL and attachee.is_unconscious() == 0 and attachee.d20_query(Q_Is_BreakFree_Possible) == 0 and attachee.d20_query(Q_Prone) == 0 and attachee.d20_query(Q_Helpless) == 0 and attachee.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) == 0:
		return 1
	else:
		return 0

# gets a PC that can talk
# optional inputs: maximum distance, and requirement of LOS
def GetDelegatePc( attachee, distt = 20, requireLos = 0 ):
	delegatePc = OBJ_HANDLE_NULL
	for pc in game.party:
		if (pc.type == obj_t_pc and pc.is_unconscious() == 0 and pc.d20_query(Q_Prone) == 0  and attachee.distance_to(pc) <= distt and (not requireLos or attachee.can_see(pc))):
			delegatePc = pc
	return delegatePc