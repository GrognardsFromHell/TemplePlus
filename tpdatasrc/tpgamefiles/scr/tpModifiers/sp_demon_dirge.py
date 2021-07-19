from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
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

demonDirgeSpell = PythonModifier("sp-Demon Dirge", 3, False) # spell_id, duration, empty
demonDirgeSpell.AddHook(ET_OnBeginRound, EK_NONE, demonDirgeSpellOnBeginRound, ())
demonDirgeSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #deals damage, replaceCondition is needed
demonDirgeSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
demonDirgeSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
demonDirgeSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
demonDirgeSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
demonDirgeSpell.AddSpellDispelCheckStandard()
demonDirgeSpell.AddSpellTeleportPrepareStandard()
demonDirgeSpell.AddSpellTeleportReconnectStandard()
demonDirgeSpell.AddSpellCountdownStandardHook()
