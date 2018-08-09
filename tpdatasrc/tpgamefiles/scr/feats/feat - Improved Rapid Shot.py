from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Many Shot, Point Blank Shot and Rapid Shot are required either standard feats or from the ranger class
	if char_editor.has_feat(feat_manyshot) or char_editor.has_feat(feat_ranger_manyshot):
		if char_editor.has_feat(feat_rapid_shot) or char_editor.has_feat(feat_ranger_rapid_shot):
			if char_editor.has_feat(feat_point_blank_shot): 
				return 1
	
	return 0
