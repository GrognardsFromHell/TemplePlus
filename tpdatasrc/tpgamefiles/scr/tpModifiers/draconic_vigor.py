from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Draconic Vigor: Dragon Magic, p. 17

print "Registering Draconic Vigor"

def getSpellLevel(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    if not spellPacket.is_divine_spell():
        spellLevel = spellPacket.spell_known_slot_level #not tested with metamagic hightend spells
        args.set_arg(1, spellLevel)
    return 0

def healEffect(attachee, args, evt_obj):
    if args.get_arg(1):
        spellId = evt_obj.data1
        spellPacket = tpdp.SpellPacket(spellId)
        if spellPacket.caster == attachee:
            ### workaround for heal ###
            #heal requires a dice
            spellLevel = args.get_arg(1)
            healDice = dice_new('1d1')
            healDice.bonus = spellLevel -1
            ### workaround end ###
            game.particles ('sp-Cure Minor Wounds', attachee)
            attachee.heal(attachee, healDice, D20A_HEAL, 0)
            attachee.healsubdual(attachee, healDice, D20A_HEAL, 0)
            game.create_history_freeform("{} is healed for {} by ~Draconic Vigor~[TAG_DRACONIC_VIGOR]\n\n".format(attachee.description, spellLevel))
            args.set_arg(1, 0)
    return 0

draconicVigorFeat = PythonModifier("Draconic Vigor Feat", 3) #FeatEnum, spellLevel, empty
draconicVigorFeat.MapToFeat("Draconic Vigor", feat_cond_arg2 = 0)
draconicVigorFeat.AddHook(ET_OnGetCasterLevelMod, EK_NONE, getSpellLevel, ())
draconicVigorFeat.AddHook(ET_OnD20Signal, EK_S_Spell_Cast, healEffect, ())
