from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Allegro"

def allegroSpellMovementBonus(attachee, args, evt_obj):
    #Allegro adds 30ft. to movement speed, but is capped at double original speed.
    bonusValue = min(attachee.stat_level_get(stat_movement_speed), 30)
    bonusType = 12 # ID12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType ,"~Allegro~[TAG_SPELLS_ALLEGRO] ~Enhancement~[TAG_ENHANCEMENT_BONUS] Bonus")
    return 0

allegroSpell = PythonModifier("sp-Allegro", 3, False) # spell_id, duration, empty
allegroSpell.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, allegroSpellMovementBonus,())
allegroSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
allegroSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
allegroSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
allegroSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
allegroSpell.AddSpellDispelCheckStandard()
allegroSpell.AddSpellTeleportPrepareStandard()
allegroSpell.AddSpellTeleportReconnectStandard()
allegroSpell.AddSpellCountdownStandardHook()
