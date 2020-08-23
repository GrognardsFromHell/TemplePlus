from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

#Improved Skirmish:   Complete Scoundrel, p. 78

print "Registering Improved Skirmish"

# Global variables for keeping track of the location of the scout at the beginning of the round
# They do not need to be persistent except during a scout's turn
improved_skimish_start_position_x = 0.0
improved_skimish_start_position_y = 0.0
improved_skimish_start_location = long()

#Determine if skrimish is enabled based on if the scout has moved 10 feet
def ImprovedSkirmishMovedDistance(attachee, args, evt_obj):
	#Keep track of how far the scout as moved from their initial position (not total distance moved)
	global improved_skimish_start_location
	global improved_skimish_start_position_x
	global improved_skimish_start_position_y
	
	#The distance needs to location at the beginning of the round needs to be adjusted by the radius (which is in inches)
	moveDistance = int(attachee.distance_to(improved_skimish_start_location, improved_skimish_start_position_x, improved_skimish_start_position_y) + (attachee.radius / 12.0))
	
	args.set_arg(0, moveDistance)
	
	return 0

def ImprovedSkirmishReset(attachee, args, evt_obj):
	global improved_skimish_start_location
	global improved_skimish_start_position_x
	global improved_skimish_start_position_y

	#Save the initial position for the scout and the distance moved for the round
	improved_skimish_start_position_x = attachee.off_x
	improved_skimish_start_position_y = attachee.off_y
	improved_skimish_start_location = attachee.location

	#Zero out the total distance moved from the start position
	args.set_arg(0, 0)
	return 0
	
def ImprovedSkirmishACBonus(attachee, args, evt_obj):
	distance = args.get_arg(0)
	
	#20 foot move required for the bonus
	if distance >= 20:
		evt_obj.return_val += 2
	return 0
	
def ImprovedSkirmishBonusDice(attachee, args, evt_obj):
	distance = args.get_arg(0)
	
	#20 foot move required for the bonus
	if distance >= 20:
		evt_obj.return_val += 2
	return 0

daringOutlaw = PythonModifier("Improved Skirmish", 4) # distance, spare, spare, spare
daringOutlaw.MapToFeat("Improved Skirmish")
daringOutlaw.AddHook(ET_OnD20Signal, EK_S_Combat_Critter_Moved, ImprovedSkirmishMovedDistance, ())
daringOutlaw.AddHook(ET_OnBeginRound, EK_NONE, ImprovedSkirmishReset, ())
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Skirmish Additional AC", ImprovedSkirmishACBonus, ())
daringOutlaw.AddHook(ET_OnD20PythonQuery, "Skirmish Additional Dice", ImprovedSkirmishBonusDice, ())

