from toee import *
import race_defs
###################################################
print "Registering race: Strongheart (Halfling)"

raceEnum = race_halfling + (3 << 5)


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_STRONGHEART_HALFLING"
raceSpec.flags           = 4
raceSpec.modifier_name   = "Strongheart"
raceSpec.height_male     = [32, 40]
raceSpec.height_female   = [30, 38]
raceSpec.weight_male     = [32, 34]
raceSpec.weight_female   = [27, 29]
raceSpec.stat_modifiers  = [-2, 2, 0, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13038
raceSpec.material_offset = 12         # offset into rules/material_ext.mes file
raceSpec.bonus_first_level_feat = 1

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)
	
def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_rogue

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 0