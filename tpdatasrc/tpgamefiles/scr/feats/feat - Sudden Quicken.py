from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Must have the quicken feat
	if not char_editor.has_feat(feat_quicken_spell):
		return 0
		
	#Must have the sudden extend feat
	if not char_editor.has_feat("Sudden Extend"):
		return 0
		
	#Must have the sudden maximize feat
	if not char_editor.has_feat("Sudden Maximize"):
		return 0
		
	#Must have the sudden empower feat
	if not char_editor.has_feat("Sudden Empower"):
		return 0
		
	#Must have the sudden still feat
	if not char_editor.has_feat("Sudden Still"):
		return 0
		
	#Must have the sudden silent feat
	if not char_editor.has_feat("Sudden Silent"):
		return 0

	return 1
