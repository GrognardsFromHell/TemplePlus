from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
print "Registering sp-Demonhide"

def demonhideSpellGrantDr(attachee, args, evt_obj): 
    evt_obj.damage_packet.add_physical_damage_res(5, D20DAP_HOLY, 126) #Demonhide grants DR 5/good or cold iron; cold iron is not in the game; ID126 in damage.mes is DR
    return 0

def demonhideSpellTooltip(attachee, args, evt_obj):
    if args.get_arg(1) == 1:
        evt_obj.append("Demonhide ({} round)".format(args.get_arg(1)))
    else:
        evt_obj.append("Demonhide ({} rounds)".format(args.get_arg(1)))
    return 0

def demonhideSpellEffectTooltip(attachee, args, evt_obj):
    if args.get_arg(1) == 1:
        evt_obj.append(tpdp.hash("DEMONHIDE"), -2, " ({} round)".format(args.get_arg(1)))
    else:
        evt_obj.append(tpdp.hash("DEMONHIDE"), -2, " ({} rounds)".format(args.get_arg(1)))
    return 0

def demonhideSpellHasSpellActive(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    if evt_obj.data1 == spellPacket.spell_enum:
        evt_obj.return_val = 1
    return 0

def demonhideSpellKilled(attachee, args, evt_obj):
    args.remove_spell()
    args.remove_spell_mod()
    return 0

def demonhideSpellSpellEnd(attachee, args, evt_obj):
    print "Demonhide SpellEnd"
    return 0

demonhideSpell = PythonModifier("sp-Demonhide", 2) # spell_id, duration
demonhideSpell.AddHook(ET_OnTakingDamage , EK_NONE, demonhideSpellGrantDr,())
demonhideSpell.AddHook(ET_OnGetTooltip, EK_NONE, demonhideSpellTooltip, ())
demonhideSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, demonhideSpellEffectTooltip, ())
demonhideSpell.AddHook(ET_OnD20Signal, EK_S_Spell_End, demonhideSpellSpellEnd, ())
demonhideSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, demonhideSpellHasSpellActive, ())
demonhideSpell.AddHook(ET_OnD20Signal, EK_S_Killed, demonhideSpellKilled, ())
demonhideSpell.AddSpellDispelCheckStandard()
demonhideSpell.AddSpellTeleportPrepareStandard()
demonhideSpell.AddSpellTeleportReconnectStandard()
demonhideSpell.AddSpellCountdownStandardHook()
