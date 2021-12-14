from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from heritage_feat_utils import countHeritageFeats

# Draconic Toughness: Races of the Dragon, p. 105

print "Registering Draconic Toughness"

def increaseMaxHp(attachee, args, evt_obj):
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    hpValue = 2 * countHeritageFeats(attachee, heritage)
    bonusType = 0 #ID 0 = untyped (stacking)
    evt_obj.bonus_list.add(hpValue, bonusType, "~Draconic Toughness~[TAG_DRACONIC_TOUGHNESS]")

draconicToughnessFeat = PythonModifier("Draconic Toughness Feat", 2) #FeatEnum, empty
draconicToughnessFeat.MapToFeat("Draconic Toughness", feat_cond_arg2 = 0)
draconicToughnessFeat.AddHook(ET_OnGetMaxHP, EK_NONE, increaseMaxHp, ())
