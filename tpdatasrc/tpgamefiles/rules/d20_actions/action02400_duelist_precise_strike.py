from toee import *

def GetActionName():
	return "Precise Strike"

def GetActionDefinitionFlags():
	return D20ADF_TargetSingleExcSelf | D20ADF_TriggersCombat | D20ADF_UseCursorForPicking
	
def GetTargetingClassification():
	return D20TC_SingleExcSelf