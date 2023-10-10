from toee import *
import race_defs
###################################################

def GetCategory():
	return "Core 3.5 Ed Classes"

raceEnum = race_bugbear

raceSpec = race_defs.RaceSpec()
raceSpec.modifier_name   = "Bugbear"                   # Python modifier to be applied
raceSpec.flags           = 2
raceSpec.hit_dice        = dice_new("3d8")
raceSpec.level_modifier  = 1                         # basic level modifier
raceSpec.stat_modifiers  = [4, 2, 2, 0, 0, -2]   # str, dex, con, int, wis, cha
raceSpec.saving_throw_modifiers = [1, 3, 1] # fort, ref, will
raceSpec.natural_armor   = 3
raceSpec.proto_id        = 13044
raceSpec.help_topic      = "TAG_BUGBEAR"
raceSpec.height_male     = [80, 90]
raceSpec.height_female   = [80, 90]
raceSpec.weight_male     = [430, 520]
raceSpec.weight_female   = [390, 480]
raceSpec.feats           = [feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all, feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_shield_proficiency]
raceSpec.material_offset = 0                        # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	print "Registering race: Bugbear"
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_rogue

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 1