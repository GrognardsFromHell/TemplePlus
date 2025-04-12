from toee import *
from utilities import *
from scripts import GetCritterHandle

familiar_table = {
12045: 14990,
12046: 14991,
12047: 14992,
12048: 14993,
12049: 14994,
12050: 14995,
12051: 14996,
12052: 14997,
12053: 14998,
12054: 14999
}

def OnBeginSpellCast( spell ):
	print "Summon Familiar OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	#game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Summon Familiar OnSpellEffect"

	spell.duration = 2147483647
	master = spell.caster

	# get familiar inventory object handle
	inv_proto = FindFamiliarProto( spell.caster, 0 )
	familiar = spell.caster.item_find_by_proto( inv_proto )
	if ( get_ID( familiar ) != 0 ):
		return SKIP_DEFAULT
	
	# get the proto_id for this familiar 
	familiar_proto_id = FindFamiliarProto( spell.caster, 1 )
	if (familiar_proto_id == 0):	#  not a recognized familiar type
		return SKIP_DEFAULT
	
	# creates random ID number
	ID_number = game.random_range( 1,2147483647 )
	ID_number = ID_number^game.random_range( 1,2147483647 )#xor with next "random" number in line, should be more random
		
	# create familiar
	spell.summon_monsters( 1, familiar_proto_id )
	
	# get familiar's handle
	familiar_obj = GetCritterHandle( spell, familiar_proto_id )
	
	if ( familiar_obj == OBJ_HANDLE_NULL ): # no new familiar present
		return SKIP_DEFAULT
		
	# summoning effect
	#game.particles( 'Orb-Summon-Air-Elemental', familiar_obj )
		
	# assigns familiar ownership	
	set_ID( familiar_obj, ID_number )
	set_ID( familiar, ID_number )
	
	#game.particles( "sp-summon monster II", game.party[0] )
	
	# sets familiar's stat's and bonuses depending on it's masters level
	master_level = GetLevel( spell.caster )
	f_level = ( ( master_level + 1 ) / 2 )
	
	f_hp = ( ( spell.caster.stat_level_get( stat_hp_max ) ) / 2 ) ## familiar's hp = i/2 masters hp
	base_hp = familiar_obj.stat_level_get( stat_hp_max ) ## familiar's base hp from proto
	prev_max_hp = familiar.obj_get_int( obj_f_item_pad_i_1 ) ## familiar's max hp from last time summoned ( 0 if never summoned before)
	prev_curr_hp = familiar.obj_get_int( obj_f_item_pad_i_2 ) ## familiar's current xp from last time stowed ( 0 if never summoed before)
	if ( base_hp <= f_hp ): ## if 1/2 master's hp is greater than base hp from proto, will use 1/2 masters hp
		new_hp = familiar_obj.stat_base_set( stat_hp_max, f_hp )
	curr_max_hp = familiar_obj.stat_level_get( stat_hp_max ) ## familiar's max hp from current summons
	hp_diff = ( curr_max_hp - prev_max_hp ) ## difference between max hp from last time summoned and max hp now ( 0 if master has not gained a level since)
	if ( prev_max_hp != 0): ## has been summoned before
		if ( hp_diff >=1 ):  ## adds gained hp if master has gained hp since last time summoned
			hp_now = prev_currr_hp + hp_diff
		else:
			hp_now = prev_curr_hp
		dam = dice_new("1d1")
		dam.num = curr_max_hp - hp_now
		if (dam.num >= 1):
			familiar_obj.damage(OBJ_HANDLE_NULL, D20DT_FORCE, dam, D20DAP_NORMAL) 
		
	## This next bit gives the familiar it's masters BAB.  The familiar should have a BAB (without the masters BAB) of zero, but since
	## the game engine doesn't allow for Weapon Finesse with natural attacks( which would let the familiar use their dexterity modifier
	## instead of the strength modifier), I fiddled with the  "to hit" in the protos to counteract the negative attack modifier do to 
	## low strength and add the dexterity modifier. - Ceruleran the Blue ##
	f_to_hit = spell.caster.stat_base_get(stat_attack_bonus)

	## Remove the strength penalty and add the dex bonus to simulate weapon finesse, marc
	## Assumes all familars are using dex to hit and not strength.
#	str = familiar_obj.stat_base_get(stat_strength)
#	dex = familiar_obj.stat_base_get(stat_dexterity)
#	adj = game.get_stat_mod(dex) - game.get_stat_mod(str)
#	f_to_hit += adj

	new_to_hit = familiar_obj.condition_add_with_args( 'To Hit Bonus', f_to_hit, 0 )
	
	new_int = familiar_obj.stat_base_set( stat_intelligence, ( 5 + f_level ) ) ## familiar INT bonus
	
	armor = familiar_obj.obj_set_int( obj_f_npc_ac_bonus, (f_level) ) ## Natrual Armor bonus
	
	if ( master_level >= 11 ):
		spell_resistance = familiar_obj.condition_add_with_args( 'Monster Spell Resistance', ( 5 + master_level ), 0 ) ## spell resistance
		
	## familiar uses masters saving throw bonuses if they are higher than it's own.
	fortitude_bonus = Fortitude( spell.caster )
	if ( fortitude_bonus >= 3 ):
		fortitude_save = familiar_obj.obj_set_int( obj_f_npc_save_fortitude_bonus, fortitude_bonus )
	reflex_bonus = Reflex( spell.caster )
	if ( reflex_bonus >= 3 ):
		reflex_save = familiar_obj.obj_set_int( obj_f_npc_save_reflexes_bonus, reflex_bonus )
	will_bonus = Will( spell.caster )
	if ( will_bonus >= 1 ):
		wlll_save = familiar_obj.obj_set_int( obj_f_npc_save_willpower_bonus, will_bonus )
	
	# add familiar to follower list for spell_caster
	if not ( spell.caster.follower_atmax() ):
		spell.caster.follower_add( familiar_obj )
	else:
		spell.caster.ai_follower_add( familiar_obj )
	
	# add familiar_obj to d20initiative, and set initiative to spell_caster's
	caster_init_value = spell.caster.get_initiative()
	familiar_obj.add_to_initiative()
	familiar_obj.set_initiative( caster_init_value )
	game.update_combat_ui()

	# familiar should disappear when duration is over, apply "TIMED_DISAPPEAR" condition
	#familiar_obj.condition_add_with_args( 'sp-Summoned', spell.id, spell.duration, 0 )
	
	# add familiar to target list
	spell.num_of_targets = 1
	spell.target_list[0].obj = familiar_obj

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	familiar_obj = spell.target_list[0].obj
	if familiar_obj.stat_level_get(stat_hp_current) <= -10:
		# Remove familiar if dead after one day.
		game.timevent_add( RemoveDead, ( spell.caster, familiar_obj ), 86400000) # 1000 = 1 second
		return
	if familiar_obj not in game.party:
		familiar_obj.destroy()
		for f,p in familiar_table.items():
			itemA = spell.caster.item_find_by_proto( f )
			if itemA != OBJ_HANDLE_NULL:
				clear_ID( itemA )
	print "Summon Familiar OnBeginRound"

def OnEndSpellCast( spell ):
	print "Summon Familiar OnEndSpellCast"
	for f,p in familiar_table.items():
		itemA = spell.caster.item_find_by_proto( f )
		if itemA != OBJ_HANDLE_NULL:
			clear_ID( itemA )
			
####################################################################################################
#  Functions Called in the Spell
####################################################################################################
	
def FindFamiliarProto( master, x ):
# Returns either the familiar creature's proto ID ( x = 1 ) or the familiar inventory object ( x = 0 )
	for f,p in familiar_table.items():
		itemC = master.item_find_by_proto( f )
		if ( itemC != OBJ_HANDLE_NULL ):
			if x :
				return p
			else:
				return f
	return 0
	
def GetFamiliarHandle( spell, familiar_proto_id ):
# Returns a handle that can be used to manipulate the familiar creature object
	for npc in game.obj_list_vicinity( spell.target_loc, OLC_CRITTERS ):
		if (npc.name == familiar_proto_id):
			if  get_ID( npc ) == 0:
				return npc
	return OBJ_HANDLE_NULL
	
def get_ID(obj):
# Returns embedded ID number
	return obj.obj_get_int(obj_f_secretdoor_dc)

def set_ID( obj, val ):
# Embeds ID number into mobile object.  Returns ID number.
	obj.obj_set_int( obj_f_secretdoor_dc, val )
	return obj.obj_get_int( obj_f_secretdoor_dc )
	
def clear_ID( obj ):
# Clears embedded ID number from mobile object
	obj.obj_set_int( obj_f_secretdoor_dc, 0 )

def FindMaster( npc ):
# Not actually used in the spell, but could be handy in the future.  Returns the character that is the master for a given summoned familiar ( npc )
	for p_master in game.obj_list_vicinity( npc.location, OLC_CRITTERS ):
		for x,y in familiar_table.items():
			item = p_master.item_find_by_proto( x )
			if (item != OBJ_HANDLE_NULL):
				if ( get_ID(item) == get_ID( npc ) ):
					return p_master
	return OBJ_HANDLE_NULL

def GetLevel( npc ):
# Returns characters combined sorcerer and wizard levels
	level = npc.stat_level_get(stat_level_sorcerer) + npc.stat_level_get(stat_level_wizard)
	return level
	
def Fortitude( npc ):
# Returns Fortitude Save Bonus for all the casters class levels
	bonus = 0
	level = npc.stat_level_get(stat_level_barbarian) + npc.stat_level_get(stat_level_cleric) + npc.stat_level_get(stat_level_druid) + npc.stat_level_get(stat_level_fighter) + npc.stat_level_get(stat_level_paladin) + npc.stat_level_get(stat_level_ranger) + npc.stat_level_get(stat_level_monk)
	if ( level != 0 ):
		bonus = ( ( level / 2 ) + 2 )
	level = npc.stat_level_get(stat_level_bard) + npc.stat_level_get(stat_level_rogue) + npc.stat_level_get(stat_level_sorcerer) + npc.stat_level_get(stat_level_wizard)
	if ( level != 0 ):
		bonus = bonus + ( level / 3 )
	return bonus

def Reflex( npc ):
# Returns Reflex Save Bonus for all the casters class levels
	bonus = 0
	level = npc.stat_level_get(stat_level_barbarian) + npc.stat_level_get(stat_level_cleric) + npc.stat_level_get(stat_level_druid) + npc.stat_level_get(stat_level_fighter) + npc.stat_level_get(stat_level_paladin) + npc.stat_level_get(stat_level_sorcerer) + npc.stat_level_get(stat_level_wizard)
	if ( level != 0 ):
		bonus = ( level / 3 )
	level = npc.stat_level_get(stat_level_ranger) + npc.stat_level_get(stat_level_rogue) + npc.stat_level_get(stat_level_monk) + npc.stat_level_get(stat_level_bard)
	if ( level != 0 ):
		bonus = bonus + ( ( level / 2 ) + 2 )
	return bonus

def Will( npc ):
# Returns Will Save Bonus for all the casters class levels
	bonus = 0
	level = npc.stat_level_get(stat_level_bard) + npc.stat_level_get(stat_level_cleric) + npc.stat_level_get(stat_level_druid) + npc.stat_level_get(stat_level_monk) + npc.stat_level_get(stat_level_sorcerer) + npc.stat_level_get(stat_level_wizard)
	if ( level != 0 ):
		bonus = ( ( level / 2 ) + 2 )
	level = npc.stat_level_get(stat_level_barbarian) + npc.stat_level_get(stat_level_fighter) + npc.stat_level_get(stat_level_paladin) + npc.stat_level_get(stat_level_ranger) + npc.stat_level_get(stat_level_rogue)
	if ( level != 0 ):
		bonus = bonus + ( level / 3 )
	return bonus

def RemoveDead(npc, critter):
	if critter.stat_level_get(stat_hp_current) <= -10:
		npc.follower_remove(critter)
	return
