from toee import *
import race_defs

###################################################

raceEnum = race_dwarf + (5 << 5)

raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_GOLD_DWARF"
raceSpec.flags           = 4
raceSpec.modifier_name   = "Gold Dwarf"
raceSpec.height_male     = [45, 53]
raceSpec.height_female   = [43, 51]
raceSpec.weight_male     = [148, 178]
raceSpec.weight_female   = [104, 134]
raceSpec.stat_modifiers  = [0, -2, 2, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13036
raceSpec.material_offset = 6         # offset into rules/material_ext.mes file
raceSpec.use_base_race_for_deity = 1

###################################################
def RegisterRace():
	print "Registering race: Gold Dwarf"
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_fighter

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 0
