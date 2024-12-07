from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Spiritual Weapon OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Spiritual Weapon OnSpellEffect"

	spell.duration = 1 * spell.caster_level

	# get the caster's deity
	deity = spell.caster.get_deity()

	# find the deity's preferred weapon, default is dagger
	weapon_proto = 4142
	weapon_portrait = 0;
	if (deity == DEITY_BOCCOB):
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
	elif (deity == DEITY_CORELLON_LARETHIAN):
		# longsword
		weapon_proto = 4146
		weapon_portrait = 8010;
	elif (deity == DEITY_EHLONNA):
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
	elif (deity == DEITY_ERYTHNUL):
		# morningstar
		weapon_proto = 4147
		weapon_portrait = 8040;
	elif (deity == DEITY_FHARLANGHN):
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
	elif (deity == DEITY_GARL_GLITTERGOLD):
		# battleaxe blue
		weapon_proto = 4140
		weapon_portrait = 7950;
	elif (deity == DEITY_GRUUMSH):
		# spear
		weapon_proto = 4151
		weapon_portrait = 8050;
	elif (deity == DEITY_HEIRONEOUS):
		# longsword
		weapon_proto = 4146
		weapon_portrait = 8010;
	elif (deity == DEITY_HEXTOR):
		# morningstar
		weapon_proto = 4147
		weapon_portrait = 8040;
	elif (deity == DEITY_KORD):
		# greatsword
		weapon_proto = 4143
		weapon_portrait = 7980;
	elif (deity == DEITY_MORADIN):
		# warhammer
		weapon_proto = 4153
		weapon_portrait = 8070;
	elif (deity == DEITY_NERULL):
		# scythe
		weapon_proto = 4149
		weapon_portrait = 8030;
	elif (deity == DEITY_OBAD_HAI):
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
	elif (deity == DEITY_OLIDAMMARA):
		# rapier
		weapon_proto = 4148
		weapon_portrait = 8020;
	elif (deity == DEITY_PELOR):
		# heavymace
		weapon_proto = 4144
		weapon_portrait = 7970;
	elif (deity == DEITY_ST_CUTHBERT):
		# heavymace
		weapon_proto = 4144
		weapon_portrait = 7970;
	elif (deity == DEITY_VECNA):
		# dagger
		weapon_proto = 4142
		weapon_portrait = 7960;
	elif (deity == DEITY_WEE_JAS):
		# dagger
		weapon_proto = 4142
		weapon_portrait = 7960;
	elif (deity == DEITY_YONDALLA):
		# shortsword
		weapon_proto = 4150
		weapon_portrait = 8000;
	elif (deity == DEITY_OLD_FAITH):
		# heavymace
		weapon_proto = 4144
		weapon_portrait = 7970;
	elif (deity == DEITY_ZUGGTMOY):
		# warhammer
		weapon_proto = 4153
		weapon_portrait = 8070;
	elif (deity == DEITY_IUZ):
		# greatsword
		weapon_proto = 4143
		weapon_portrait = 7980;
	elif (deity == DEITY_LOLTH):
		# dagger
		weapon_proto = 4142
		weapon_portrait = 7960;
	elif (deity == DEITY_PROCAN):
		# spear
		weapon_proto = 4151
		weapon_portrait = 8050;
	elif (deity == DEITY_NOREBO):
		# dagger
		weapon_proto = 4142
		weapon_portrait = 7960;
	elif (deity == DEITY_PYREMIUS):
		# longsword
		weapon_proto = 4146
		weapon_portrait = 8010;
	elif (deity == DEITY_RALISHAZ):
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
	else:
		# staff
		weapon_proto = 4152
		weapon_portrait = 8060;
		print "SPIRITUAL WEAPON WARNING: deity=", deity," not found!"

	# figure out the proto_id from the deity
	monster_proto_id = 14370

	# Barnibus, Alterius, Parvo, Foote, Pia, Ileviam
	if spell.caster.name in [14514,14605,14606,14607,14608,14623]:
		monster_proto_id = 14464  # faction 9

	# create monster
	monster_obj = game.obj_create( monster_proto_id, spell.target_loc )
	monster_obj.obj_set_int( obj_f_critter_portrait, weapon_portrait )

	hit_points = 6 * spell.caster_level
	hit_points = 25 + hit_points
	monster_obj.stat_base_set(stat_hp_max, hit_points)


	# equip the tempman with the appropriate weapon
	weapon_obj = game.obj_create( weapon_proto, monster_obj.location )
	monster_obj.item_get( weapon_obj )
	monster_obj.item_wield_best_all()
	print "SPIRITUAL WEAPON: equipped obj=( ", monster_obj, " ) with weapon=( ", weapon_obj, " )!"

	# add monster to follower list for spell_caster
	spell.caster.ai_follower_add( monster_obj )
	print "added as follower"
	
	# add monster_obj to d20initiative, and set initiative to spell_caster's
	caster_init_value = spell.caster.get_initiative()
	print "got the caster's initiative"
	monster_obj.add_to_initiative()
	print "added to initiative"
	
	if not (spell.caster in game.party[0].group_list()):
		highest = -999
		initt = -999
		for dude in game.party:
			if dude.get_initiative() > highest and critter_is_unconscious(dude) == 0:
				highest = dude.get_initiative()
			if dude.get_initiative() > initt and dude.get_initiative() < caster_init_value and critter_is_unconscious(dude) == 0:
				initt = max(dude.get_initiative() - 1, 1)
		if initt == -999:
			initt = max( highest , 1)
	else:
		initt = caster_init_value
	monster_obj.set_initiative( initt ) # changed by S.A. - in case you have the same faction as the summoned weapon, it needs to see you fighting other members of its faction otherwise it won't act
	#monster_obj.set_initiative( caster_init_value ) # removed by S.A. - in case you have the same faction as the summoned weapon, it needs to see you fighting other members of its faction otherwise it won't act and lose a turn
	game.update_combat_ui()
	print "update cmbat ui"

	# monster should disappear when duration is over, apply "TIMED_DISAPPEAR" condition
	monster_obj.condition_add_with_args( 'sp-Summoned', spell.id, spell.duration, 0 )
	monster_obj.condition_add_with_args( 'sp-Spiritual Weapon', spell.id, spell.duration, weapon_proto )

	print "condition have been added to Spiritual Weapon"
	
	# add monster to target list
	spell.num_of_targets = 1
	spell.target_list[0].obj = monster_obj
	spell.target_list[0].partsys_id = game.particles( 'sp-spell resistance', spell.target_list[0].obj )
	print "particles"
	
	spell.spell_end( spell.id )

	print "spell ended, end of OnSpellEffect script"
		
def OnBeginRound( spell ):
	print "Spiritual Weapon OnBeginRound"

def OnEndSpellCast( spell ):
	print "Spiritual Weapon OnEndSpellCast"