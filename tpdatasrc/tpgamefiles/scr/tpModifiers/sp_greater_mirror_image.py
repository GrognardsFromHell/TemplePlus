from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import *
print "Registering sp_greater_mirror_image"

# args:
#		0: spell id
#		1: duration
#		2: cur images
#		3: inc images
#		4: cap images
#		5: spare
#		6: spare

def MirrorTooltip(attachee, args, evt_obj):
	name = spellName(args.get_arg(0))
	count = args.get_arg(2)

	evt_obj.append("{}[{}]".format(name, count))

	return 0

def HasImage(attachee, args, evt_obj):
	# number of images
	evt_obj.return_val = args.get_arg(2)
	# spell id
	evt_obj.data1 = args.get_arg(0)
	return 0

def AddImage(attachee, args, evt_obj):
	cur_images = args.get_arg(2)
	inc_images = args.get_arg(3)
	cap_images = args.get_arg(4)

	# increment with a cap; inc=0 is normal mirror image behavior
	upd_images = min(cap_images, cur_images + inc_images)

	args.set_arg(2, upd_images)
	return 0

def ImageStruck(attachee, args, evt_obj):
	spell_id = args.get_arg(0)

	# check that this spell was struck
	if evt_obj.data1 != spell_id:
		return 0

	game.particles('sp-Mirror Image Loss', attachee)
	game.sound(32228,1)

	images = args.get_arg(2)
	if images <= 1:
		args.remove_spell()
		args.remove_spell_mod()
	else:
		args.set_arg(2, images-1)
	return 0

cond = PythonModifier('sp-Greater Mirror Image', 7)
cond.AddHook(ET_OnGetTooltip,EK_NONE,MirrorTooltip,())
cond.AddHook(ET_OnGetEffectTooltip,EK_NONE,spellEffectTooltip,())
cond.AddHook(ET_OnBeginRound,EK_NONE,AddImage,())
cond.AddHook(ET_OnD20Query,EK_Q_Critter_Has_Mirror_Image,HasImage,())
cond.AddHook(ET_OnD20Query,EK_Q_Critter_Has_Spell_Active,queryActiveSpell,())
cond.AddHook(ET_OnD20Signal,EK_S_Killed,spellKilled,())
cond.AddHook(ET_OnD20Signal,EK_S_Spell_Mirror_Image_Struck,ImageStruck,())
cond.AddHook(ET_OnD20Signal,EK_S_Dismiss_Spells,checkRemoveSpell,())
cond.AddSpellDispelCheckStandard()
cond.AddSpellTeleportPrepareStandard()
cond.AddSpellTeleportReconnectStandard()
cond.AddSpellCountdownStandardHook()
