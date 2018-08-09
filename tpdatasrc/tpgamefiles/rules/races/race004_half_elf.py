from toee import *
import race_defs
###################################################

raceEnum = race_half_elf


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_HALF_ELVES"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Halfelf"
raceSpec.height_male     = [55, 71]
raceSpec.height_female   = [53, 69]
raceSpec.weight_male     = [114, 164]
raceSpec.weight_female   = [104, 144]
raceSpec.stat_modifiers  = [0, 0, 0, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13008
raceSpec.material_offset = 10         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)