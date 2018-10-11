#Powerful Charge:  Miniatures Handbook, p. 27

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Powerful Charge"

def PowerfulChargeBeginRound(attachee, args, evt_obj):
	# Reset the already used this round flag
	args.set_arg(0, 0)
	return 0

def PowerfulChargeDamageBonus(attachee, args, evt_obj):

	print "Powerful Charge Damage Bonus"

	# already used this round, do nothing
	if args.get_arg(0):
		return 0

	#Must be charging 
	charging = attachee.d20_query("Charging")
	
	if charging == 0:
		return 0
	
	size = attachee.stat_level_get(stat_size)
	
	#Size must be medium or larger
	if (size > STAT_SIZE_COLOSSAL) or (size < STAT_SIZE_MEDIUM):
		return 0
		
	#Damage Dice determined based on size
	if size == STAT_SIZE_MEDIUM:
		diceString = '1d8'
		numDice = 1
	elif size == STAT_SIZE_LARGE:
		diceString = '1d6'
		numDice = 2
	elif size == STAT_SIZE_HUGE:
		diceString = '1d6'
		numDice = 3
	elif size == STAT_SIZE_HUGE:
		diceString = '1d6'
		numDice = 4
	elif size == STAT_SIZE_GARGANTUAN:
		diceString = '1d6'
		numDice = 5
	else:  #STAT_SIZE_COLOSSAL
		diceString = '1d6'
		numDice = 6
		
	damage_dice = dice_new(diceString)
	damage_dice.number = numDice
	evt_obj.damage_packet.add_dice(damage_dice, -1, 127)
	
	# set the already used this round flag
	args.set_arg(0, 1)
	
	return 0

powerfulCharge = PythonModifier("Powerful Charge", 2) # userd this round, spare
powerfulCharge.MapToFeat("Powerful Charge")
powerfulCharge.AddHook(ET_OnDealingDamage, EK_NONE, PowerfulChargeDamageBonus, ())
powerfulCharge.AddHook(ET_OnBeginRound, EK_NONE, PowerfulChargeBeginRound, ())
