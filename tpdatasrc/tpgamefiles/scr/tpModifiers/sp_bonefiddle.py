from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Bonefiddle"

def bonefiddleSpellBeginRound(attachee, args, evt_obj):
    duration = args.get_arg(1)
    if duration >= 0:
        spellId = args.get_arg(0)
        spellDc = args.get_arg(2)
        spellPacket = tpdp.SpellPacket(spellId)
        game.create_history_freeform("{} saves versus ~Bonefiddle~[TAG_SPELLS_BONEFIDDLE]\n\n".format(attachee.description))
        saveType = D20_Save_Fortitude
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_SONIC
        if attachee.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId): #save for no damage and to end spell immediately
            game.particles("sp-Bonefiddle-end", attachee)
            args.remove_spell()
            args.remove_spell_mod()
        else:
            attachee.float_text_line("Bonefiddle", tf_red)
            game.particles("sp-Bonefiddle-round", attachee)
            damageType = D20DT_SONIC
            spellDamageDice = dice_new("1d6")
            spellDamageDice.number = 3
            attachee.spell_damage(spellPacket.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, args.get_arg(0))
    return 0

bonefiddleSpell = SpellPythonModifier("sp-Bonefiddle", 4) # spell_id, duration, spellDc, empty
bonefiddleSpell.AddHook(ET_OnBeginRound, EK_NONE, bonefiddleSpellBeginRound, ())
bonefiddleSpell.AddSkillBonus(-20, bonus_type_untyped, skill_move_silently)
bonefiddleSpell.AddSpellConcentration()
bonefiddleSpell.AddSpellNoDuplicate()
