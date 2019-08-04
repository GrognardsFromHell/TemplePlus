from toee import *
import race_defs
###################################################

def GetCategory():
	return "Core 3.5 Ed Classes"

print "Registering race: Troll"

raceEnum = race_troll

raceSpec = race_defs.RaceSpec()
raceSpec.modifier_name   = "Troll"                   # Python modifier to be applied
raceSpec.flags           = 2
raceSpec.hit_dice        = dice_new("6d8")
raceSpec.level_modifier  = 5                         # basic level modifier
raceSpec.stat_modifiers  = [12, 4, 12, -4, -2, -4]   # str, dex, con, int, wis, cha
raceSpec.natural_armor   = 5
raceSpec.proto_id        = 13016
raceSpec.help_topic      = "TAG_TROLL"
raceSpec.height_male     = [100, 120]
raceSpec.height_female   = [100, 120]
raceSpec.weight_male     = [870, 1210]
raceSpec.weight_female   = [800, 1200]
raceSpec.feats           = [feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all]
raceSpec.material_offset = 0                        # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_fighter

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 5