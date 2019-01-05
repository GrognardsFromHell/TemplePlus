from toee import *
import race_defs

###################################################
print "Registering race: Svirfneblin"

raceEnum = race_gnome + (1 << 5)

raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 3
raceSpec.help_topic      = "TAG_SVIRFNEBLIN"
raceSpec.flags           = 0
raceSpec.modifier_name   = "Svirfneblin"
raceSpec.height_male     = [36, 44]
raceSpec.height_female   = [34, 42]
raceSpec.weight_male     = [42, 44]
raceSpec.weight_female   = [37, 39]
raceSpec.stat_modifiers  = [-2, 2, 0, 0, 2, -4]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13034
raceSpec.material_offset = 8         # offset into rules/material_ext.mes file
raceSpec.feats           = []

###################################################
def RegisterRace():
	raceSpec.spell_like_abilities = GetSpellLikeAbilities()
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_rogue

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 3

def GetSpellLikeAbilities(obj = OBJ_HANDLE_NULL):
	spBlindnessDeafness  = PySpellStore(spell_blindness_deafness, domain_special, 2)
	spBlur  = PySpellStore(spell_blur, domain_special, 2)
	return {spBlindnessDeafness: 1, spBlur:1}
	