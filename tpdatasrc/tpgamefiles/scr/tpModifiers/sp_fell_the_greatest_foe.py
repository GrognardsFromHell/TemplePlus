from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Fell the Greatest Foe"

def fellTheGreatestFoeSpellBonusToDamage(attachee, args, evt_obj):
    target =  evt_obj.attack_packet.target
    attackerSize = attachee.get_size
    targetSize = target.get_size
    bonusDiceNumber = targetSize-attackerSize

    if bonusDiceNumber > 0:
        #Fell the Greatest Foe only works with melee attacks
        if not evt_obj.attack_packet.get_flags() & D20CAF_RANGED:
            bonusDice = dice_new('1d6')
            bonusDice.number = bonusDiceNumber
            evt_obj.damage_packet.add_dice(bonusDice, D20DT_UNSPECIFIED, 3003) #ID3003 added in damage.mes
    return 0

fellTheGreatestFoeSpell = SpellPythonModifier("sp-Fell the Greatest Foe") # spell_id, duration, empty
fellTheGreatestFoeSpell.AddHook(ET_OnDealingDamage, EK_NONE, fellTheGreatestFoeSpellBonusToDamage,())
fellTheGreatestFoeSpell.AddSpellNoDuplicate()
