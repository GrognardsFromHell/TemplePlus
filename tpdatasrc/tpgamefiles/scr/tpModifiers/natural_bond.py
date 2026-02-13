from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Natural Bond:   Complete Adventurer, p. 111

print "Registering Natural Bond"

def QueryNaturalBond(attachee, args, evt_obj):
	animalCompanionLevel = 0
	
	#Add Druid Level to Ranger effective druid level
	rangerLevel = attachee.stat_level_get(stat_level_ranger) 
	if rangerLevel >= 4:
		animalCompanionLevel = rangerLevel / 2
	animalCompanionLevel += attachee.stat_level_get(stat_level_druid)

	characterLevel = attachee.stat_level_get(stat_level)

	nonAnimalCompanionLevel = characterLevel - animalCompanionLevel

	#Level bonus is up to 3 levels but not more than the character level
	levelBonus = min(nonAnimalCompanionLevel, 3)

	#Return the bonus 
	evt_obj.return_val += levelBonus
	return 0

NaturalBondFeat = PythonModifier("Natural Bond Feat", 2) # spare, spare
NaturalBondFeat.MapToFeat("Natural Bond")
NaturalBondFeat.AddHook(ET_OnD20PythonQuery, "Animal Companion Level Bonus", QueryNaturalBond, ())


