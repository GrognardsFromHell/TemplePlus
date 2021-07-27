from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
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

devilBlightSpell = PythonModifier("sp-Devil Blight", 3, False) # spell_id, duration, empty
devilBlightSpell.AddHook(ET_OnBeginRound, EK_NONE, devilBlightSpellOnBeginRound, ())
devilBlightSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ()) #deals damage, replaceCondition is needed
devilBlightSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
devilBlightSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
devilBlightSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
devilBlightSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
devilBlightSpell.AddSpellDispelCheckStandard()
devilBlightSpell.AddSpellTeleportPrepareStandard()
devilBlightSpell.AddSpellTeleportReconnectStandard()
devilBlightSpell.AddSpellCountdownStandardHook()