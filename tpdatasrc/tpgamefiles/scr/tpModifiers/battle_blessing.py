from templeplus.pymod import PythonModifier
from toee import *

# Battle Blessing:  Complete Champion, p. 55

print "Registering Battle Blessing"



def BattleBlessingMetamagicUpdate(attachee, args, evt_obj):
	
	spellCastingClass = evt_obj.get_spell_casting_class()
	if spellCastingClass != stat_level_paladin:
		return 0

	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic

	#Don't quicken more than once
	if metaMagicData.get_quicken() < 1:
		metaMagicData.set_quicken(1)
	
	return 0
	
#Setup the feat
BattleBlessingFeat = PythonModifier("Battle Blessing Feat", 2) #Spare, Spare
BattleBlessingFeat.MapToFeat("Battle Blessing")
BattleBlessingFeat.AddHook(ET_OnMetaMagicMod, EK_NONE, BattleBlessingMetamagicUpdate, ())

