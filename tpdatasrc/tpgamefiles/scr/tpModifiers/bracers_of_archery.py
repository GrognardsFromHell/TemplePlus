from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fix for Atari Bug 90
# Extend bracers of archery (greater - 6267 and lesser - 6268) to provide the appropriate proficiencies
print "Bracers of Archery extender"
	
def BracersOfArcheryProficientWithWeapon(attachee, args, evt_obj):
	# Makes the character proficient with all bows (except crossbows)
	weaponType = evt_obj.data1
	if (weaponType == wt_longbow or weaponType == wt_shortbow or weaponType == wt_composite_shortbow or weaponType == wt_composite_longbow):
		evt_obj.return_val = 1
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Bracers of Archery")
modExtender.AddHook(ET_OnD20PythonQuery, "Proficient with Weapon", BracersOfArcheryProficientWithWeapon, ())
