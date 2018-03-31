from toee import *
import race_defs
###################################################

raceEnum = race_human


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_HUMANS"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Human"
raceSpec.height_male     = [58, 78]
raceSpec.height_female   = [53, 73]
raceSpec.weight_male     = [124, 200]
raceSpec.weight_female   = [89, 165]
raceSpec.stat_modifiers  = [0, 0, 0, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13000
raceSpec.material_offset = 0         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	raceSpec.register(raceEnum)