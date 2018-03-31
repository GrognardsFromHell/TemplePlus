from toee import *
import race_defs
###################################################

def GetCategory():
	return "Core 3.5 Ed Classes"

raceEnum = race_hill_giant

raceSpec = race_defs.RaceSpec()
raceSpec.modifier_name   = "Hill Giant"              # Python modifier to be applied
raceSpec.flags           = 2
raceSpec.hit_dice        = dice_new("12sd8")
raceSpec.level_modifier  = 4                         # basic level modifier
raceSpec.stat_modifiers  = [14, -2, 8, -4, 0, -4]   # str, dex, con, int, wis, cha
raceSpec.natural_armor   = 9
raceSpec.proto_id        = 13018
raceSpec.help_topic      = "TAG_HILL_GIANT"
raceSpec.height_male     = [120, 140]
raceSpec.height_female   = [110, 130]
raceSpec.weight_male     = [1700, 2210]
raceSpec.weight_female   = [1600, 2200]
raceSpec.material_offset = 0                        # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	print "Registering race: Hill Giant"
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_fighter

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 4