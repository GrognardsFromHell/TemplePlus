from toee import *

###################################################

def GetConditionName(): # used by API
	return "Human"

def GetCategory():
	return "Core 3.5 Ed Races"

def GetRaceDefFlags():
	return 1

def GetRaceHelpTopic():
	return "TAG_HUMAN"
	
raceEnum = race_human

###################################################



def GetProtoId():
	return 13000

def GetMaterialOffset():
	return 0

def GetStatModifiers():
	return [0, 0, 0, 0, 0, 0]

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return -1
	
def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 0
	
def GetMinMaxHeightWeight():
	minHeightMale = 58
	maxHeightMale = 78
	
	minHeightFemale = 53
	maxHeightFemale = 73
	
	minWeightMale = 124
	maxWeightMale = 200
	
	minWeightFemale = 89
	maxWeightFemale = 165
	
	return [minHeightMale, maxHeightMale, minHeightFemale, maxHeightFemale, minWeightMale, maxWeightMale, minWeightFemale, maxWeightFemale]