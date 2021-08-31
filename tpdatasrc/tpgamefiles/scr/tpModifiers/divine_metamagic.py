from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#  Divine Metamagic:  Complete Divine, p. 80

print "Registering Divine Metamagic"

#Supported Metamgic Types
DMMEmpower = 1
DMMMaximize = 2
DMMQuicken = 3
DMMExtend = 4
DMMWiden = 5
DMMEnlarge = 6
DMMStill = 7
DMMSlient = 8
DMMHeighten = 9

def GetTurnCost(args, Type):
	if Type == DMMHeighten:
		Needed = args.get_arg(2) + 1
	if Type == DMMQuicken:
		Needed = 5
	elif Type == DMMMaximize:
		Needed = 4
	elif Type == DMMEmpower:
		Needed = 3
	else:
		Needed = 2

	return Needed

def HasEnoughTurnAttempts(attachee, args, Type):
	#Check for a charges
	TurnCharges = attachee.d20_query("Turn Undead Charges")
	
	CanUse = 0
	Needed = 0
	
	Needed = GetTurnCost(args, Type)

	#Get the cost from other DMM effects in use for this spell
	DMMCost = attachee.d20_query("Divine Metamagic Cost")

	TurnCharges = TurnCharges - DMMCost

	if TurnCharges >= Needed:
		CanUse = 1
		
	return CanUse

def DMMRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off divine metamagic
	
	mmType = args.get_param(0)
	
	text = "Divine Metamagic "
	
	if mmType == DMMEmpower:
		text = text + "Empower"
	elif mmType == DMMMaximize:
		text =  text + "Maximize"
	
	checkboxMenu = tpdp.RadialMenuEntryToggle(text, "TAG_INTERFACE_HELP")
	checkboxMenu.link_to_args(args, 0)
	checkboxMenu.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

	#Heighten only (slider to check number of levels)
	if mmType == DMMHeighten:
		sliderMenu = tpdp.RadialMenuEntrySlider("Divine Metamagic Heighten Levels", "Number of levels to Heighten", 1, 9, "TAG_INTERFACE_HELP")
		sliderMenu.link_to_args(args, 2)
		sliderMenu.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

	return 0


def DMMMetamagicUpdate(attachee, args, evt_obj):
	if not evt_obj.is_divine_spell():
		return
	
	#Check if the feat is turned on
	if not args.get_arg(0):
		return 0
	
	#Does the character have enough turn attempts
	mmType = args.get_param(0)
	EnoughTurnAttempts = HasEnoughTurnAttempts(attachee, args, mmType)
	
	if not EnoughTurnAttempts:
		return 0
	
	#Get the metamagic info
	metaMagicData = evt_obj.meta_magic

	##...Set count to the correct amount only if adding...##
	
	#Apply the appropriate meta magic once only
	if mmType == DMMEmpower:
		if metaMagicData.get_empower_count() < 1:
			metaMagicData.set_empower_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMMaximize:
		if metaMagicData.get_maximize_count() < 1:
			metaMagicData.set_maximize_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMHeighten:
		HeightenCount =  args.get_arg(2)
		#Heigten Count Needs to respect level also
		currentHeighten = metaMagicData.get_heighten_count()
		newHeighten = HeightenCount + currentHeighten
		if newHeighten + metaMagicData.spell_level > 9:
			return #Don't apply if the spell level would be set above 9
		metaMagicData.set_heighten_count(newHeighten)

	cost = GetTurnCost(args, mmType)
	args.set_arg(1, cost)
			
	return 0
	
def DMMDeduct(attachee, args, evt_obj):
	cost = args.get_arg(1)

	#Deduct turn undead charges
	for x in range(cost):
		attachee.d20_send_signal("Deduct Turn Undead Charge")

	args.set_arg(1, 0)

	return 0

def DMMCostQuery(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0

def DMMAdd(attachee, args, evt_obj):
	args.set_arg(0, 0)
	args.set_arg(1, 0)
	args.set_arg(2, 1) #Min Heighten levels is 1
	return 0

def DMMAddFeat(FeatName, Type):
	modifier = PythonModifier(FeatName, 4) #Enable Flag, Cost, Heighten Value (Heighten MM ONly), Spare
	modifier.MapToFeat(FeatName)
	modifier.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DMMRadial, (DMMEmpower,))
	modifier.AddHook(ET_OnMetaMagicMod, EK_NONE, DMMMetamagicUpdate, (DMMEmpower,))
	modifier.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", DMMDeduct, (DMMEmpower,))
	modifier.AddHook(ET_OnD20PythonQuery, "Divine Metamagic Cost", DMMCostQuery, (DMMEmpower,))
	modifier.AddHook(ET_OnConditionAdd, EK_NONE, DMMAdd, ())
	return modifier
	
dmmEmpower = DMMAddFeat("Divine Metamagic - Empower", DMMEmpower)
dmmMaximize = DMMAddFeat("Divine Metamagic - Maximize", DMMMaximize)
dmmHeighten = DMMAddFeat("Divine Metamagic - Heighten", DMMHeighten)


