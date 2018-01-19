from toee import *

def GetActionName():
	return "Precise Strike"

def GetActionDefinitionFlags():
	return D20ADF_TargetSingleExcSelf | D20ADF_TriggersCombat | D20ADF_UseCursorForPicking
	
def GetTargetingClassification():
	return D20TC_SingleExcSelf


# Standard attack type of checks
def ActionCheckTargetStdAtk( obj, tgt ):
	if ( obj.d20_query(Q_Prone) or obj.d20_query(Q_Unconscious) ):
		return AEC_CANT_WHILE_PRONE
	if (tgt == OBJ_HANDLE_NULL):
		return AEC_TARGET_INVALID
	return AEC_OK