#Reckless Charge:  Miniatures Handbook, p. 27

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Reckless Charge"

def RecklessChargeRadial(attachee, args, evt_obj):
	checkboxRecklessCharge = tpdp.RadialMenuEntryToggle("Reckless Charge", "TAG_INTERFACE_HELP")
	checkboxRecklessCharge.link_to_args(args, 0)
	checkboxRecklessCharge.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
	return 0

def RecklessChargeHitBonus(attachee, args, evt_obj):

	#Check if the feat is enabled
	if not args.get_arg(0):
		return 0
		
	charging = attachee.d20_query("Charging")
	
	#If charging apply the attack bonus
	if charging:
		evt_obj.bonus_list.add(2, 0, "Reckless Charge") #+2 Bonus makes up for the -2 Rapid shot penalty
	return 0
	
def RecklessChargeACPenalty(attachee, args, evt_obj):

	#Check if the feat is enabled
	if not args.get_arg(0):
		return 0
		
	charging = attachee.d20_query("Charging")
	
	#If charging apply the ac penatly
	if charging:
		evt_obj.bonus_list.add(-2, 0, "Reckless Charge")  # Dodge bonus,  ~Class~[TAG_LEVEL_BONUSES]
	return 0

recklessCharge = PythonModifier("Reckless Charge", 2) # Enabled, Place Holder
recklessCharge.MapToFeat("Reckless Charge")
recklessCharge.AddHook(ET_OnToHitBonus2, EK_NONE, RecklessChargeHitBonus, ())
recklessCharge.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, RecklessChargeRadial, ())
recklessCharge.AddHook(ET_OnGetAC, EK_NONE, RecklessChargeACPenalty, ())