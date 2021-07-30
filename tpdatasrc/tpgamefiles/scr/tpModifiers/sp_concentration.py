from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions

def UseAct(char, args, evt_obj):
	if tpdp.config_get_bool("StricterRulesEnforcement"):
		if evt_obj.tb_status.hourglass_state > 1:
			# use up a standard action
			evt_obj.tb_status.hourglass_state = 1
	return 0

def GiveAct(char, args, evt_obj):
	if tpdp.config_get_bool("StricterRulesEnforcement"):
		tpactions.get_cur_seq().tb_status

		if tb_status.hourglass_state == 0:
			tb_status.hourglass_state = 2
		elif tb_status.hourglass_state == 1:
			tb_status.hourglass_state = 4

	return 0

conc = PythonModifier()
conc.ExtendExisting('sp-Concentrating')
conc.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, UseAct, ())
conc.AddHook(ET_OnD20ActionPerform, EK_D20A_STOP_CONCENTRATION, GiveAct, ())
