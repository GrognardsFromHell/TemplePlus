from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Req 1, turn undead or rebuke undead feat
	if not (char_editor.has_feat(feat_turn_undead) or char_editor.has_feat(feat_rebuke_undead)):
		return 0
	
	#Req2, Cast First Level Divine Spells
	if attachee.divine_spell_level_can_cast() > 0:
		return 1
		
	return 0
