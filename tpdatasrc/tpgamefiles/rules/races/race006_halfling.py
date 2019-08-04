from toee import *
import race_defs
###################################################

raceEnum = race_halfling


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_HALFLINGS"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Halfling"
raceSpec.height_male     = [32, 40]
raceSpec.height_female   = [30, 38]
raceSpec.weight_male     = [32, 34]
raceSpec.weight_female   = [27, 29]
raceSpec.stat_modifiers  = [-2, 2, 0, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13012
raceSpec.material_offset = 12         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)