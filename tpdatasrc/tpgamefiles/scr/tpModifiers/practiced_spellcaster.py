from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

print "Registering Practiced Spellcaster"

### Moved to new event type so that it can modify caster level without a specific spell
### This will be used e.g. for crafting
# def PracticedSpellcasterLevelModArcane(attachee, args, evt_obj):
    # spell_packet = evt_obj.get_spell_packet()
    # if spell_packet.is_divine_spell():
        # return 0

    # bonVal = 4
    # cur_caster_lvl = spell_packet.caster_level

    # cur_hd = attachee.hit_dice_num
    # bonVal = min(4, cur_hd - cur_caster_lvl)

    # if bonVal > 0:
        # print "Practiced Spellcaster: Adding to caster level " + str(bonVal)
        # evt_obj.return_val += bonVal
    # return 0

# def PracticedSpellcasterLevelModDivine(attachee, args, evt_obj):
    # spell_packet = evt_obj.get_spell_packet()
    # if not spell_packet.is_divine_spell():
        # return 0

    # bonVal = 4
    # cur_caster_lvl = spell_packet.caster_level

    # cur_hd = attachee.hit_dice_num
    # bonVal = min(4, cur_hd - cur_caster_lvl)

    # if bonVal > 0:
        # print "Practiced Spellcaster: Adding to caster level " + str(bonVal)
        # evt_obj.return_val += bonVal
    # return 0

def OnAddSpellCastingArcane(attachee, args, evt_obj):
	# arg1 holds the Arcane class
	if args.get_arg(1) == 0:
		highestArcane = char_class_utils.GetHighestArcaneClass(attachee)
		args.set_arg(1, highestArcane)
	return 0


def OnAddSpellCastingDivine(attachee, args, evt_obj):
	# arg1 holds the Divine class
	if args.get_arg(1) == 0:
		highestDivine = char_class_utils.GetHighestDivineClass(attachee)
		args.set_arg(1, highestDivine)
	return 0
	
def OnGetBaseCasterLevel2(attachee, args, evt_obj):
	classEnum = args.get_arg(1)
	if evt_obj.arg0 != classEnum:
		return 0
	
	cur_caster_lvl = evt_obj.bonus_list.get_total()
	cur_hd = attachee.hit_dice_num
	bon_val = min(4, cur_hd - cur_caster_lvl)
	if bon_val > 0:
		evt_obj.bonus_list.add(bon_val, 0, "Practiced Spellcaster")
	return 0

# arg0 - featEnum (autoset by engine)
# arg1 - set to chosen classEnum
	
pracSC_Arcane = PythonModifier("Practiced Spellcaster Feat - Arcane", 2) # args are just-in-case placeholders
pracSC_Arcane.MapToFeat("Practiced Spellcaster - Arcane")
# pracSC_Arcane.AddHook(ET_OnGetCasterLevelMod, EK_NONE, PracticedSpellcasterLevelModArcane, ())
pracSC_Arcane.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCastingArcane, ())
pracSC_Arcane.AddHook(ET_OnSpellCasterGeneral, EK_SPELL_Base_Caster_Level_2, OnGetBaseCasterLevel2, ())

pracSC_Divine = PythonModifier("Practiced Spellcaster Feat - Divine", 2) # args are just-in-case placeholders
pracSC_Divine.MapToFeat("Practiced Spellcaster - Divine")
# pracSC_Divine.AddHook(ET_OnGetCasterLevelMod, EK_NONE, PracticedSpellcasterLevelModDivine, ())
pracSC_Divine.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCastingDivine, ())
pracSC_Divine.AddHook(ET_OnSpellCasterGeneral, EK_SPELL_Base_Caster_Level_2, OnGetBaseCasterLevel2, ())