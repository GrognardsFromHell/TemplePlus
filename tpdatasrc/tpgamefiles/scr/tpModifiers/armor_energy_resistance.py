from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#From the SRD

print "Registering Armor Energy Resistance"

def OnGetDamageResistance(attachee, args, evt_obj):
	damType = args.get_param(0)
	value = args.get_arg(0)
	
	#Add Resistance only if there is damage of the correct type
	damage = evt_obj.damage_packet.get_overall_damage_by_type(damType)
	if damage > 0:
		evt_obj.damage_packet.add_damage_resistance(value, damType, 124)
	return 0

armorFireRes = PythonModifier("Armor Fire Resistance", 3) # DR, spare, inv_idx
armorFireRes.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DT_FIRE,))

armorAcidRes = PythonModifier("Armor Acid Resistance", 3) # DR, spare, inv_idx
armorAcidRes.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DT_ACID,))

armorColdRes = PythonModifier("Armor Cold Resistance", 3) # DR, spare, inv_idx
armorColdRes.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DT_COLD,))

armorElectricityRes = PythonModifier("Armor Elec Resistance", 3) # DR, spare, inv_idx
armorElectricityRes.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DT_ELECTRICITY,))

armorSonicRes = PythonModifier("Armor Sonic Resistance", 3) # DR, spare, inv_idx
armorSonicRes.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DT_SONIC,))

