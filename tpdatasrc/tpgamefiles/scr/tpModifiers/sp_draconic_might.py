from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Draconic Might"

def sleepParalyzeImmunity(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Sleep"):
        evt_obj.return_val = 0
        combatMesLine = 5059 #ID 5059: "Sleep Immunity"
        historyMesLine = 31 #ID 31: {[ACTOR] is immune to ~sleep~[TAG_SPELLS_SLEEP].}
        attachee.float_mesfile_line('mes\\combat.mes', combatMesLine, tf_red)
        game.create_history_from_pattern(historyMesLine, attachee, OBJ_HANDLE_NULL)
    elif evt_obj.is_modifier("Paralyzed"):
        evt_obj.return_val = 0
        attachee.float_text_line("Paralyze Immunity", tf_red)
        game.create_history_freeform("{} is immune to ~paralyze~[TAG_PARALYZED] effects\n\n".format(attachee.description))
    return 0

draconicMightSpell = SpellPythonModifier("sp-Draconic Might") # spellId, duration, empty
draconicMightSpell.AddHook(ET_OnConditionAddPre, EK_NONE, sleepParalyzeImmunity, ())
draconicMightSpell.AddAbilityBonus(4, bonus_type_enhancement, stat_charisma, stat_constitution, stat_strength)
draconicMightSpell.AddAcBonus(4, bonus_type_natural_armor)
