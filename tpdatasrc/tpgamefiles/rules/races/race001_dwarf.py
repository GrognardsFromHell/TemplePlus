from toee import *
import race_defs
###################################################

raceEnum = race_dwarf


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_DWARVES"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Dwarf"
raceSpec.height_male     = [45, 53]
raceSpec.height_female   = [43, 51]
raceSpec.weight_male     = [148, 178]
raceSpec.weight_female   = [104, 134]
raceSpec.stat_modifiers  = [0, 0, 2, 0, 0, -2]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13002
raceSpec.material_offset = 6         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)