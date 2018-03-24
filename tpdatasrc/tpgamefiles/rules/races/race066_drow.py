from toee import *

###################################################

def GetConditionName(): # used by API
	return "Drow"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetRaceDefFlags():
	return 1

def GetRaceHelpTopic():
	return "TAG_DROW"
	
raceEnum = race_elf + (2 << 5)

###################################################

def GetMinMaxHeightWeight():
	minHeightMale = 53
	maxHeightMale = 65
	
	minHeightFemale = 53
	maxHeightFemale = 65
	
	minWeightMale = 87
	maxWeightMale = 121
	
	minWeightFemale = 82
	maxWeightFemale = 116
	
	return [minHeightMale, maxHeightMale, minHeightFemale, maxHeightFemale, minWeightMale, maxWeightMale, minWeightFemale, maxWeightFemale]

def GetStatModifiers():
	return [0, 2, -2, 2, 0, 2]

def GetProtoId():
	return 13014

def GetMaterialOffset(): # offset into rules/material_ext.mes file
	return 14

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	if obj == OBJ_HANDLE_NULL:
		return stat_level_cleric
	return stat_level_wizard
	
def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 2