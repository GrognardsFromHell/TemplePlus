from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Clutch of Orcus"

def onBeginRoundEffects(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellDc = args.get_arg(2)
    spellPacket = tpdp.SpellPacket(spellId)
    if not attachee.d20_query(Q_Unconscious): #no save while unconscious
        game.create_history_freeform("{} saves versus ~Clutch of Orcus~[TAG_SPELLS_CLUTCH_OF_ORCUS]\n\n".format(attachee.description))
        if attachee.saving_throw_spell(spellDc, D20_Save_Fortitude, D20STD_F_NONE, spellPacket.caster, spellId): #save for no damage and to end spell immediately
            attachee.float_text_line("Clutch of Orcus end")
            attachee.d20_send_signal(S_Spell_End, spellId) # ???
            args.set_arg(1, -1) #switch to proper remove
            return 0
        spellDamageDice = dice_new("1d12")
        attachee.float_text_line("Clutch of Orcus damage", tf_red)
        attachee.spell_damage(spellPacket.caster, D20DT_MAGIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

clutchOfOrcusSpell = SpellPythonModifier("sp-Clutch of Orcus", 4) # spellId, duration, spellDc, empty
clutchOfOrcusSpell.AddHook(ET_OnBeginRound, EK_NONE, onBeginRoundEffects, ())
clutchOfOrcusSpell.AddSpellConcentration()
clutchOfOrcusSpell.AddSpellNoDuplicate()
