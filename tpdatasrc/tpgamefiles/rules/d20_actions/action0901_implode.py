from toee import *
import tpactions
import tpdp


def GetActionName():
	return "Implode"

def GetActionDefinitionFlags():
	return D20ADF_TargetSingleExcSelf | D20ADF_TriggersCombat

def GetTargetingClassification():
	return D20TC_SingleExcSelf

def GetActionCostType():
	return D20ACT_Standard_Action

def AddToSequence(d20action, action_seq, tb_status):
	packet = tpdp.SpellPacket(d20action.data1)

	for i in range(0, packet.target_count):
		if packet.get_target(i) == d20action.target:
			d20action.target.float_text_line('Already targeted',tf_red)
			return AEC_TARGET_INVALID

	packet.add_target(d20action.target, 0)
	action_seq.add_action(d20action)
	return AEC_OK
