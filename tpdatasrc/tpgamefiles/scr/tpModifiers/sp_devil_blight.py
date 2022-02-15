from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Devil Blight"

def devilBlightSpellOnBeginRound(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = 2
    damageType = D20DT_MAGIC

    game.create_history_freeform("{} is affected by ~Devil Blight~[TAG_SPELLS_DEVIL_BLIGHT]\n\n".format(attachee.description))
    attachee.float_text_line("Devil Blight", tf_red)
    game.particles('hit-HOLY-medium', attachee)
    attachee.spell_damage(spellPacket.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, args.get_arg(0))
    return 0

devilBlightSpell = SpellPythonModifier("sp-Devil Blight") # spell_id, duration, empty
devilBlightSpell.AddHook(ET_OnBeginRound, EK_NONE, devilBlightSpellOnBeginRound, ())
devilBlightSpell.AddSpellNoDuplicate()
