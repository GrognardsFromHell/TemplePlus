from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from warlock import isInvocation

print "Registering Spell Focus (Invocation)"

def dcBonus(attachee, args, evt_obj):
    spellEntry = evt_obj.spell_entry
    if isInvocation(spellEntry.spell_enum):
        bonusValue = 1
        bonusType = bonus_type_untyped # Stacking!
        bonusLabel = "~Spell Foucs (Invocation)~[TAG_SPELL_FOCUS]"
        evt_obj.bonus_list.add(bonusValue, bonusType, bonusLabel)
    return 0

sfInvocationFeat = PythonModifier("Spell Focus (Invocation)", 2) #featEnum, empty
sfInvocationFeat.MapToFeat("Spell Focus (Invocation)")
sfInvocationFeat.AddHook(ET_OnGetSpellDcMod, EK_NONE, dcBonus, ())
