from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Augment Healing, Complete Divine: p. 79

print "Registering Augment Healing"
	
def QueryHealingBonus(attachee, args, evt_obj):
#Note:  This implementation effects only damage healed by effected spells.  Damage done to undead is not increased.
#       Also empower will apply to this bonus.  This seems to be the best interpretation of the feat.
	healingBonus = 0

	#Find the spell being cast (argument is the spell enum)
	spPacket = tpdp.SpellPacket(evt_obj.data1)
	spEntry = tpdp.SpellEntry(spPacket.spell_enum)
	
	#Is it a conjuration(healing) spell
	if spEntry.spell_school_enum == Conjuration and spEntry.spell_subschool_enum == Healing:
		#Bonus is twice the spell level
		healingBonus = 2 * spPacket.spell_know_slot_level

	#Return the bonus 
	evt_obj.return_val += healingBonus
	return 0

LingeringSong = PythonModifier("Augment Healing", 2) #Extra, Extra
LingeringSong.MapToFeat("Augment Healing")
LingeringSong.AddHook(ET_OnD20PythonQuery, "Healing Bonus", QueryHealingBonus, ())
