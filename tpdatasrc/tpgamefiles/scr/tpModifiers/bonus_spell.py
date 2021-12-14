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

for spellLevel in range(0, 10):
    bonusSpellSorc = BonusSpellModifier("Bonus Spell Sorc {}".format(spellLevel))
    bonusSpellSorc.MapToFeat("Bonus Spell (Sorcerer {})".format(spellLevel), feat_cond_arg2 = stat_level_sorcerer)
    bonusSpellSorc.bonusSpellLevelSetArg(spellLevel)

for spellLevel in range(0, 10):
    bonusSpellWiz = BonusSpellModifier("Bonus Spell Wiz {}".format(spellLevel))
    bonusSpellWiz.MapToFeat("Bonus Spell (Wizard {})".format(spellLevel), feat_cond_arg2 = stat_level_wizard)
    bonusSpellWiz.bonusSpellLevelSetArg(spellLevel)

for spellLevel in range(0, 10):
    bonusSpellBard = BonusSpellModifier("Bonus Spell Bard {}".format(spellLevel))
    bonusSpellBard.MapToFeat("Bonus Spell (Bard {})".format(spellLevel), feat_cond_arg2 = stat_level_bard)
    bonusSpellBard.bonusSpellLevelSetArg(spellLevel)


