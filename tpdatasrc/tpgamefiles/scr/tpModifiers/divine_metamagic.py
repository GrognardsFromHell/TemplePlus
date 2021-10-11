from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#  Divine Metamagic:  Complete Divine, p. 80

print "Registering Divine Metamagic"

#Supported Metamgic Types
DMMEmpower = 1
DMMMaximize = 2
DMMHeighten = 3
DMMExtend = 4
DMMWiden = 5
DMMEnlarge = 6
DMMStill = 7
DMMSilent = 8
DMMQuicken = 9

#Text description
dmmFeatText = ["Invalid", "Empower", "Maximize", "Heighten", "Extend", "Widen", "Enlarge", "Still", "Silent", "Quicken"]

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
	
	#Get the cost from other DMM effects in use for this spell (so the cost ends up correct)
	DMMCost = attachee.d20_query("Divine Metamagic Cost")

	TurnCharges = TurnCharges - DMMCost

	if TurnCharges >= Needed:
		CanUse = 1
		
	return CanUse

def DMMRadial(attachee, args, evt_obj):
	#Add a checkbox to turn on and off divine metamagic
	
	mmType = args.get_param(0)
	
	text = "Divine Metamagic "
	text = text + dmmFeatText[mmType]
	
	checkboxMenu = tpdp.RadialMenuEntryToggle(text, "TAG_INTERFACE_HELP")
	checkboxMenu.link_to_args(args, 0)
	checkboxMenu.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

	#Heighten only (slider to check number of levels to add)
	if mmType == DMMHeighten:
		sliderMenu = tpdp.RadialMenuEntrySlider("Divine Metamagic Heighten Levels", "Number of levels to Heighten", 1, 9, "TAG_INTERFACE_HELP")
		sliderMenu.link_to_args(args, 2)
		sliderMenu.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)

	return 0


def DMMMetamagicUpdate(attachee, args, evt_obj):
	if not evt_obj.is_divine_spell():
		args.set_arg(1, 0)
		return 0
	
	#Check if the feat is turned on
	if not args.get_arg(0):
		args.set_arg(1, 0)
		return 0
	
	prevCost = args.get_arg(1)
	mmType = args.get_param(0)
	#Does the character have enough turn attempts
	if prevCost == 0:
		EnoughTurnAttempts = HasEnoughTurnAttempts(attachee, args, mmType)
		
		if not EnoughTurnAttempts:
			return 0

	#Apply the appropriate meta magic once only
	if mmType == DMMEmpower:
		if evt_obj.meta_magic.get_empower_count() < 1:  #Only once
			evt_obj.meta_magic.set_empower_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMMaximize:
		if not evt_obj.meta_magic.get_maximize():    #Only once
			evt_obj.meta_magic.set_maximize(true)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMHeighten:
		heightenCount =  args.get_arg(2)
		currentHeighten = evt_obj.meta_magic.get_heighten_count()
		newHeighten = heightenCount + currentHeighten
		
		#Max spell level is always 9.  Allowing heightining above the character's highest slot.
		if newHeighten + evt_obj.spell_level > 9:
			return #Don't apply if the spell level would be set above 9
		evt_obj.meta_magic.set_heighten_count(newHeighten)
	elif mmType == DMMExtend:
		if evt_obj.meta_magic.get_extend_count() < 1:  #Only once
			evt_obj.meta_magic.set_extend_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMWiden:
		if evt_obj.meta_magic.get_widen_count() < 1:  #Only once
			evt_obj.meta_magic.set_widen_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMEnlarge:
		if evt_obj.meta_magic.get_enlarge_count() < 1:  #Only once
			evt_obj.meta_magic.set_enlarge_count(1)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMStill:
		if not evt_obj.meta_magic.get_still():  #Only once
			evt_obj.meta_magic.set_still(true)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMSilent:
		if not evt_obj.meta_magic.get_silent():  #Only once
			evt_obj.meta_magic.set_silent(true)
		else:
			return #No effect, don't set the cost
	elif mmType == DMMQuicken:
		if evt_obj.meta_magic.get_quicken() < 1:  #Only once
			evt_obj.meta_magic.set_quicken(1)
		else:
			return #No effect, don't set the cost

	if prevCost == 0:
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
	
def DMMClearCost(attachee, args, evt_obj):
	args.set_arg(1, 0)
	return 0
	
#Used to create each DMM feat
def DMMAddFeat(FeatName, Type):
	modifier = PythonModifier(FeatName, 4) #Enable Flag, Cost, Heighten Value (Heighten MM ONly), Spare
	modifier.MapToFeat(FeatName)
	modifier.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DMMRadial, (Type,))
	modifier.AddHook(ET_OnMetaMagicMod, EK_NONE, DMMMetamagicUpdate, (Type,))
	modifier.AddHook(ET_OnD20PythonSignal, "Sudden Metamagic Deduct Charge", DMMDeduct, (Type,))
	modifier.AddHook(ET_OnD20PythonQuery, "Divine Metamagic Cost", DMMCostQuery, (Type,))
	modifier.AddHook(ET_OnConditionAdd, EK_NONE, DMMAdd, (Type,))
	modifier.AddHook(ET_OnBeginRound, EK_NONE, DMMClearCost, ())
	modifier.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, DMMClearCost, ())
	return modifier
	
dmmEmpowerModifier = DMMAddFeat("Divine Metamagic - Empower", DMMEmpower)
dmmMaximizeModifier = DMMAddFeat("Divine Metamagic - Maximize", DMMMaximize)
dmmHeightenModifier = DMMAddFeat("Divine Metamagic - Heighten", DMMHeighten)
dmmEnlargeModifier = DMMAddFeat("Divine Metamagic - Enlarge", DMMEnlarge)
dmmExtendModifier = DMMAddFeat("Divine Metamagic - Extend", DMMExtend)
dmmQuickenModifier = DMMAddFeat("Divine Metamagic - Quicken", DMMQuicken)
dmmWidenModifier = DMMAddFeat("Divine Metamagic - Widen", DMMWiden)
dmmStillModifier = DMMAddFeat("Divine Metamagic - Still", DMMStill)
dmmSilentModifier = DMMAddFeat("Divine Metamagic - Silent", DMMSilent)
