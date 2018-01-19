from utilities import *

def SummonMonster_Rectify_Initiative(spell, proto_id):
	monster_obj = SummonMonster_GetHandle(spell, proto_id)
	
	if monster_obj != OBJ_HANDLE_NULL:
		SummonMonster_Set_ID(monster_obj, game.random_range(1, 2**30) )
		caster_init_value = spell.caster.get_initiative()
		monster_obj.set_initiative( caster_init_value - 1 )
		game.update_combat_ui()
	return

def SummonMonster_GetHandle( spell, proto_id ):
# Returns a handle that can be used to manipulate the familiar creature object
	for obj in game.obj_list_vicinity( spell.target_loc, OLC_CRITTERS ):
		stl = spell.target_loc
		stlx, stly = location_to_axis(stl)
		ox, oy = location_to_axis(obj.location)
		if (obj.name == proto_id) and ( (ox-stlx)**2 + (oy-stly)**2 ) <= 25:
			if not ( SummonMonster_Get_ID( obj ) ):
				return obj
	return OBJ_HANDLE_NULL

def SummonMonster_Get_ID(obj):
# Returns embedded ID number
	return obj.obj_get_int(obj_f_secretdoor_dc)

def SummonMonster_Set_ID( obj, val ):
# Embeds ID number into mobile object.  Returns ID number.
	obj.obj_set_int( obj_f_secretdoor_dc, val )
	return obj.obj_get_int( obj_f_secretdoor_dc )
	
def SummonMonster_Clear_ID( obj ):
# Clears embedded ID number from mobile object
	obj.obj_set_int( obj_f_secretdoor_dc, 0 )