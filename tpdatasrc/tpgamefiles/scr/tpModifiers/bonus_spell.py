from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Bonus Spell Slot Feats"

def setSpellLevelArg(attachee, args, evt_obj):
    spellLevel = args.get_param(0)
    args.set_arg(2, spellLevel)
    return 0

def applyExtraSpell(attachee, args, evt_obj):
    classEnum = args.get_arg(1)
    spellLevel = args.get_arg(2)
    #I am unsure why evt_obj.caster_class is not working, can't get a print of it either
    #print "evt_obj.caster_class: {}".format(evt_obj.caster_class)
    #Disabeld for now due to this
    #if not evt_obj.caster_class == classEnum:
    #    return 0
    if evt_obj.spell_level == spellLevel:
        bonusValue = 1
        bonusType = 0 #ID 0 untyped (stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "Bonus Spell Slot")
    return 0

class BonusSpellModifier(PythonModifier):
    # BonusSpellModifier have 4 arguments:
    # 0: featEnum, 1: classEnum, 2: spellLevel, 3: empty
    def __init__(self, name):
        PythonModifier.__init__(self, name, 4, False)
        self.AddHook(ET_OnGetSpellsPerDayMod, EK_NONE, applyExtraSpell, ())

    #There is no feat_cond_arg3 so I can't use a for loop
    #And have to set the arg manually
    def bonusSpellLevelSetArg(self, slotLevel):
        self.AddHook(ET_OnConditionAdd, EK_NONE, setSpellLevelArg, (slotLevel,))

bonusSpellSorc0 = BonusSpellModifier("Bonus Spell Sorc 0")
bonusSpellSorc0.MapToFeat("Bonus Spell (Sorcerer 0)", feat_cond_arg2 = stat_level_sorcerer)
bonusSpellSorc0.bonusSpellLevelSetArg(0)

bonusSpellSorc1 = BonusSpellModifier("Bonus Spell Sorc 1")
bonusSpellSorc1.MapToFeat("Bonus Spell (Sorcerer 1)", feat_cond_arg2 = stat_level_sorcerer)
bonusSpellSorc1.bonusSpellLevelSetArg(1)

bonusSpellSorc2 = BonusSpellModifier("Bonus Spell Sorc 2")
bonusSpellSorc2.MapToFeat("Bonus Spell (Sorcerer 2)", feat_cond_arg2 = stat_level_sorcerer)
bonusSpellSorc2.bonusSpellLevelSetArg(2)

bonusSpellSorc3 = BonusSpellModifier("Bonus Spell Sorc 3")
bonusSpellSorc3.MapToFeat("Bonus Spell (Sorcerer 3)", feat_cond_arg2 = stat_level_sorcerer)
bonusSpellSorc3.bonusSpellLevelSetArg(3)

bonusSpellWiz0 = BonusSpellModifier("Bonus Spell Wiz 0")
bonusSpellWiz0.MapToFeat("Bonus Spell (Wizard 0)", feat_cond_arg2 = stat_level_wizard)
bonusSpellWiz0.bonusSpellLevelSetArg(0)

bonusSpellWiz0 = BonusSpellModifier("Bonus Spell Wiz 1")
bonusSpellWiz0.MapToFeat("Bonus Spell (Wizard 1)", feat_cond_arg2 = stat_level_wizard)
bonusSpellWiz0.bonusSpellLevelSetArg(1)

bonusSpellWiz0 = BonusSpellModifier("Bonus Spell Wiz 2")
bonusSpellWiz0.MapToFeat("Bonus Spell (Wizard 0)", feat_cond_arg2 = stat_level_wizard)
bonusSpellWiz0.bonusSpellLevelSetArg(2)

bonusSpellWiz0 = BonusSpellModifier("Bonus Spell Wiz 2")
bonusSpellWiz0.MapToFeat("Bonus Spell (Wizard 2)", feat_cond_arg2 = stat_level_wizard)
bonusSpellWiz0.bonusSpellLevelSetArg(2)
