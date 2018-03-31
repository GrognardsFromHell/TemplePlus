from toee import *
import race_defs
###################################################

raceEnum = race_half_orc


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_HALF_ORCS"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Halforc"
raceSpec.height_male     = [58, 82]
raceSpec.height_female   = [53, 77]
raceSpec.weight_male     = [154, 294]
raceSpec.weight_female   = [114, 254]
raceSpec.stat_modifiers  = [2, 0, 0, -2, 0, -2]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 130010
raceSpec.material_offset = 4         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)