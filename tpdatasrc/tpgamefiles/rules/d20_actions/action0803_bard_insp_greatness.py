from toee import *
import tpactions

def GetActionName():
	return "Inspire Greatness"

def GetActionDefinitionFlags():
	return D20ADF_UseCursorForPicking | D20ADF_MagicEffectTargeting | D20ADF_Breaks_Concentration
	
def GetTargetingClassification():
	return D20TC_CastSpell

def GetActionCostType():
	return D20ACT_NULL

def AddToSequence(d20action, action_seq, tb_status):
	action_seq.add_action(d20action)
	return AEC_OK