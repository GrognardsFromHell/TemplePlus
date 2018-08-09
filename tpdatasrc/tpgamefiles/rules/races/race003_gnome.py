from toee import *
import race_defs
###################################################

raceEnum = race_gnome


raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 0
raceSpec.help_topic      = "TAG_GNOMES"
raceSpec.flags           = 1
raceSpec.modifier_name   = "Gnome"
raceSpec.height_male     = [36, 44]
raceSpec.height_female   = [34, 42]
raceSpec.weight_male     = [42, 44]
raceSpec.weight_female   = [37, 39]
raceSpec.stat_modifiers  = [-2, 0, 2, 0, 0, 0]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13006
raceSpec.material_offset = 8         # offset into rules/material_ext.mes file


###################################################
def RegisterRace():
	raceSpec.register(raceEnum)