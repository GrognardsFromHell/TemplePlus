from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, skillCheck

print "Registering sp-Insidious Rhythm"

def insidiousRhythmSpellConcentrationCheck(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    spellLevel = spellPacket.spell_known_slot_level #not tested with metamagic hightend spells
    spellDC = args.get_arg(2)
    skillCheckDc = spellDC + spellLevel
    attachee.float_text_line("Concentration Check", tf_red)
    if not skillCheck(attachee, skill_concentration, skillCheckDc):
        attachee.float_text_line("failed", tf_red)
        evt_obj.return_val = 100
    else:
        attachee.float_text_line("success")
    return 0

insidiousRhythmSpell = SpellPythonModifier("sp-Insidious Rhythm", 4) # spell_id, duration, spellDc, empty
insidiousRhythmSpell.AddHook(ET_OnD20Query, EK_Q_SpellInterrupted, insidiousRhythmSpellConcentrationCheck,())
insidiousRhythmSpell.AddSkillBonus(-4, bonus_type_untyped, skill_concentration, skill_appraise, skill_craft, skill_decipher_script, skill_disable_device, skill_forgery,
skill_knowledge_arcana, skill_knowledge_religion, skill_knowledge_nature, skill_search, skill_spellcraft)
insidiousRhythmSpell.AddSpellNoDuplicate()
