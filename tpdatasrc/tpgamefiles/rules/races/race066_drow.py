from toee import *
import race_defs

###################################################
print "Registering race: Drow"

raceEnum = race_elf + (2 << 5)

raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 2
raceSpec.help_topic      = "TAG_DROW"
raceSpec.flags           = 0
raceSpec.modifier_name   = "Drow"
raceSpec.height_male     = [53, 65]
raceSpec.height_female   = [53, 65]
raceSpec.weight_male     = [87, 121]
raceSpec.weight_female   = [82, 116]
raceSpec.stat_modifiers  = [0, 2, -2, 2, 0, 2]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13014
raceSpec.material_offset = 14         # offset into rules/material_ext.mes file
raceSpec.feats           = [feat_exotic_weapon_proficiency_hand_crossbow, feat_martial_weapon_proficiency_rapier, feat_martial_weapon_proficiency_short_sword]

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	if obj == OBJ_HANDLE_NULL:
		return stat_level_cleric
	return stat_level_wizard

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 2