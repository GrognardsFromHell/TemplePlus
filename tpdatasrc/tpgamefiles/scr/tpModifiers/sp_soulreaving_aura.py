from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Soulreaving Aura"

def addToSpellRegistry(attachee, args, evt_obj):
    particlesId = game.particles("sp-Soulreaving Aura-HP", attachee)
    spellPacket.add_target(attachee, particlesId)
    spellPacket.update_registry()
    return 0

soulreavingAuraSpell = SpellPythonModifier("sp-Soulreaving Aura", 4) # spellId, duration, tempHpAmount, empty
soulreavingAuraSpell.AddHook(ET_OnConditionAdd, EK_NONE, addToSpellRegistry, ())
soulreavingAuraSpell.AddTempHp(passed_by_spell)
