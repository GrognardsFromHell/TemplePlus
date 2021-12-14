from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import heritage_feat_utils

# Draconic Resistance: Complete Arcane, p. 78

print "Registering Draconic Resistance"

def addElementalResistance(attachee, args, evt_obj):
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    resistanceValue = 3 * heritage_feat_utils.countHeritageFeats(attachee, heritage)
    elementType = heritage_feat_utils.getDraconicHeritageElement(heritage)
    damageMesId = 124 #ID 124 in damage.mes is Resistance to Energy
    evt_obj.damage_packet.add_damage_resistance(resistanceValue, elementType, damageMesId)

draconicResistanceFeat = PythonModifier("Draconic Resistance Feat", 2) #FeatEnum, empty
draconicResistanceFeat.MapToFeat("Draconic Resistance", feat_cond_arg2 = 0)
draconicResistanceFeat.AddHook(ET_OnTakingDamage2, EK_NONE, addElementalResistance, ())
