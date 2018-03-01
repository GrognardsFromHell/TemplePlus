from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Req 1, turn undead feat
	if not (attachee.has_feat(feat_turn_undead) or attachee.has_feat(feat_rebuke_undead)):
		return 0
		
	#Req2, Divine Caster Level 5 (calculate caster rather than calling function to avoid item/feat bonuses)
	paladinCasterLevel = attachee.stat_level_get(stat_caster_level_paladin)
	if paladinCasterLevel >= 5:
		return 1
		
	clericCasterLevel = attachee.stat_level_get(stat_caster_level_cleric)
	if clericCasterLevel >= 5:
		return 1
	
	blackGuardCasterLevel = attachee.stat_level_get(stat_caster_level, stat_level_blackguard)
	if blackGuardCasterLevel >= 5:
		return 1
		
	return 0
