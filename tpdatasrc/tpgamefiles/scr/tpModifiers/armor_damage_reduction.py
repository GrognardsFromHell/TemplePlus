from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 7

print "Adding Agility Axeblock, Hammerblock and Spearblock"

def OnGetDamageResistance(attachee, args, evt_obj):
	drType = args.get_param(0)
	drValue = 5
	evt_obj.damage_packet.add_physical_damage_res(drValue, drType, 126)
	return 0

armorAxeblock = PythonModifier("Armor Axeblock", 3) # spare, spare, inv_idx
armorAxeblock.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DAP_BLUDGEONING|D20DAP_PIERCING,)) #Param = DR type

armorHammerblock = PythonModifier("Armor Hammerblock", 3) # spare, spare, inv_idx
armorHammerblock.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DAP_SLASHING|D20DAP_PIERCING,)) #Param = DR type

armorSpearblock = PythonModifier("Armor Spearblock", 3) # spare, spare, inv_idx
armorSpearblock.AddHook(ET_OnTakingDamage2, EK_NONE, OnGetDamageResistance, (D20DAP_BLUDGEONING|D20DAP_SLASHING,)) #Param = DR type
