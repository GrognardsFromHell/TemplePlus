from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Draconic Armor: Dragon Magic, p. 16

print "Registering Draconic Armor"

def getSpellLevel(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    if not spellPacket.is_divine_spell():
        spellLevel = spellPacket.spell_known_slot_level #not tested with metamagic hightend spells
        args.set_arg(1, spellLevel)
    return 0

def addDamageReduction(attachee, args, evt_obj):
    if args.get_arg(1):
        drAmount = args.get_arg(1)
        drBreakType = D20DAP_MAGIC
        damageMesId = 126 #ID126 in damage.mes is DR
        evt_obj.damage_packet.add_physical_damage_res(drAmount, drBreakType, damageMesId)
    return 0

def resetDamageReduction(attachee, args, evt_obj):
    if args.get_arg(1):
        args.set_arg(1, 0)
    return 0

draconicArmorFeat = PythonModifier("Draconic Armor Feat", 3) #FeatEnum, spellLevel, empty
draconicArmorFeat.MapToFeat("Draconic Armor", feat_cond_arg2 = 0)
draconicArmorFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, getSpellLevel, ())
draconicArmorFeat.AddHook(ET_OnTakingDamage2, EK_NONE, addDamageReduction,())
draconicArmorFeat.AddHook(ET_OnBeginRound, EK_NONE, resetDamageReduction, ())
