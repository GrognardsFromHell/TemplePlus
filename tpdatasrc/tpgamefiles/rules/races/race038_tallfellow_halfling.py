from toee import *
import race_defs
###################################################
print "Registering race: Tallfellow (Halfling)"

raceEnum = race_halfling + (1 << 5)


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_TALLFELLOW_HALFLING"
raceSpec.flags           = 0
raceSpec.modifier_name   = "Tallfellow"
raceSpec.height_male     = [40, 48]
raceSpec.height_female   = [38, 46]
raceSpec.weight_male     = [33, 35]
raceSpec.weight_female   = [28, 30]
raceSpec.stat_modifiers  = [-2, 2, 0, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13039
raceSpec.material_offset = 12         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)
	
def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_rogue

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 0