from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, getSpellHelpTag

print "Registering sp-Tactical Precision"

def verifyTarget(attackPacket, spellId):
    flags = attackPacket.get_flags()

    #Only melee attacks qualify
    if flags & D20CAF_RANGED:
        return False
    elif flags & D20CAF_THROWN:
        return False
    elif flags & D20CAF_THROWN_GRENADE:
        return False

    target = attackPacket.target
    flankCount = 0
    spellPacket = tpdp.SpellPacket(spellId)
    targetCount = spellPacket.target_count
    for indexNumber in range(0, targetCount):
        if target.is_flanked_by(spellPacket.get_target(indexNumber)):
            flankCount += 1
            if flankCount == 2:
                break
    if flankCount < 2:
        return False
    return True

def tacticalPrecisionSpellBonusToHit(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    if verifyTarget(evt_obj.attack_packet, spellId):
        bonusValue = 2 #Tactical Precision grants +2 to hit
        bonusType = bonus_type_insight
        bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, spellHelpTag))
        game.particles('sp-Tactical Precision', attachee)
    return 0

def tacticalPrecisionSpellBonusDamage(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    if verifyTarget(evt_obj.attack_packet, spellId):
        if evt_obj.attack_packet.target.d20_query(Q_SneakAttack):
            bonusDice = dice_new('1d6') #Tactical Precision Bonus Damage
            damageType = D20DT_UNSPECIFIED
            damageMesId = 3009 #ID3009 NEW! added in damage.mes
            evt_obj.damage_packet.add_dice(bonusDice, damageType, damageMesId)
        else:
            evt_obj.damage_packet.bonus_list.add_zeroed(377) #ID 377 = NEW!
    return 0

tacticalPrecisionSpell = SpellPythonModifier("sp-Tactical Precision") # spellId, duration, empty
tacticalPrecisionSpell.AddHook(ET_OnToHitBonus2, EK_NONE, tacticalPrecisionSpellBonusToHit,())
tacticalPrecisionSpell.AddHook(ET_OnDealingDamage, EK_NONE, tacticalPrecisionSpellBonusDamage,())
