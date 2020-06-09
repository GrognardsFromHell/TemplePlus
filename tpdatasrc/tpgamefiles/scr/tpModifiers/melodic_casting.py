from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Melodic Casting:  Complete Mage, p. 44

print "Registering Melodic Casting"

#Both queries are used by temple+

#Note this will effect all concentration checks not just those for spells (but that may be all concentration checks in TOEE)
def MelodicCastingSkillSwap(attachee, args, evt_obj):
	if evt_obj.data1 == skill_concentration:
		evt_obj.return_val = skill_perform + 1 #Add 1 since 0 means don't change and skill numbers start counting at 0
	return 0
	
def MelodicCastingAllowCastingDuringSong(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

#This is to make use of combat casting with a defensive casting perform check
def MelodicCastingCombatCasting(attachee, args, evt_obj):
	combatCastingBonus = attachee.d20_query("Combat Casting Bonus")
	if combatCastingBonus > 0:
		evt_obj.bonus_list.add(combatCastingBonus, 0, "Combat Casting")
		attachee.d20_send_signal("Combat Casting Used")
	return 0

MelodicCasting = PythonModifier("Melodic Casting", 2) # args are just-in-case placeholders
MelodicCasting.MapToFeat("Melodic Casting")
MelodicCasting.AddHook(ET_OnD20PythonQuery, "Skill Swap", MelodicCastingSkillSwap, ())
MelodicCasting.AddHook(ET_OnD20PythonQuery, "Allow Casting During Song", MelodicCastingAllowCastingDuringSong, ())
MelodicCasting.AddHook(ET_OnGetSkillLevel, EK_SKILL_PERFORM, MelodicCastingCombatCasting, ())


