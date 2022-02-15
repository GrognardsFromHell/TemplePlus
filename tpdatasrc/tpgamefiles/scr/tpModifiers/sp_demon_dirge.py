from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Demon Dirge"

def demonDirgeSpellOnBeginRound(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellDamageDice = dice_new('1d6')
    spellDamageDice.number = 2
    damageType = D20DT_MAGIC

    game.create_history_freeform("{} is affected by ~Demon Dirge~[TAG_SPELLS_DEMON_DIRGE]\n\n".format(attachee.description))
    attachee.float_text_line("Demon Dirge", tf_red)
    game.particles('hit-HOLY-medium', attachee)
    attachee.spell_damage(spellPacket.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, args.get_arg(0))
    return 0

demonDirgeSpell = SpellPythonModifier("sp-Demon Dirge") # spell_id, duration, empty
demonDirgeSpell.AddHook(ET_OnBeginRound, EK_NONE, demonDirgeSpellOnBeginRound, ())
demonDirgeSpell.AddSpellNoDuplicate #deals damage, replaceCondition is needed
