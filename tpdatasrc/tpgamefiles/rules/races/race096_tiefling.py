from toee import *
import race_defs

###################################################

raceEnum = race_human + (3 << 5)

raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 1
raceSpec.help_topic      = "TAG_TIEFLING"
raceSpec.flags           = 0
raceSpec.modifier_name   = "Tiefling"
raceSpec.height_male     = [58, 78]
raceSpec.height_female   = [53, 73]
raceSpec.weight_male     = [124, 200]
raceSpec.weight_female   = [89, 165]
raceSpec.stat_modifiers  = [0, 2, 0, 2, 0, -2]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13031
raceSpec.material_offset = 0         # offset into rules/material_ext.mes file
raceSpec.feats           = [feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all]

###################################################
def RegisterRace():
	print "Registering race: Tiefling"
	raceSpec.register(raceEnum)


def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 1
