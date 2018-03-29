from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Many Shot, Point Blank Shot and Rapid Shot are required either standard feats or from the ranger class
	if attachee.has_feat(feat_manyshot) or attachee.has_feat(feat_ranger_manyshot):
		if attachee.has_feat(feat_rapid_shot) or attachee.has_feat(feat_ranger_rapid_shot):
			if attachee.has_feat(feat_point_blank_shot): 
				return 1
	
	return 0
