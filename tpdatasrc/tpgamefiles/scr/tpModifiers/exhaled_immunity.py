from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from heritage_feat_utils import getDraconicHeritageElement

###################################################

print "Registering Draconic Breath"

# Exhaled Immunity: Races of the Dragon p. 102

draconicBreathEnum = 2311 #Using an enum of the DD number space

###################################################


def exhaledImmunityRadial(attachee, args, evt_obj):
    radialExhaledImmunity = tpdp.RadialMenuEntryParent("Draconic Breath")
    radialDraconicBreathId = radialDraconicBreath.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def exhaledImmunityPerform(attachee, args, evt_obj):
    if attachee.stat_level_get(stat_level_dragon_shaman):
        attachee.d20_send_signal("PS_Trigger_Breath_Weapon_Cooldown")
    elif attachee.stat_level_get(stat_level_dragon_disciple) > 2:
        attachee.d20_send_signal("PS_Deduct_Breath_Weapon_Use")
    heritage = attachee.d20_query("PQ_Selected_Draconic_Heritage")
    elementType = getDraconicHeritageElement(heritage)
    durationDice = dice_new("1d4")
    duration = durationDice.roll()
    target.condition_add_with_args("Exhaled Immunity Effect", duration, elementType, 0)
    return 0
    
exhaledImmunity = PythonModifier("Exhaled Immunity Feat", 3) #FeatEnum, empty, empty
exhaledImmunity.MapToFeat("Exhaled Immunity", feat_cond_arg2 = 0)
exhaledImmunity.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, exhaledImmunityRadial, ())
exhaledImmunity.AddHook(ET_OnD20PythonActionPerform, draconicBreathEnum, exhaledImmunityPerform, ())

### Exhaled Immunity Effect ###
def exhaledImmunityEffectTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def exhaledImmunityEffectImmunity(attachee, args, evt_obj):
    elementType = args.get_arg(1)
    evt_obj.damage_packet.add_mod_factor(0.0, elementType, 132) #ID 132 in damage.mes is Immunity
    return 0

exhaledImmunityEffect = PythonModifier("Exhaled Immunity Effect", 3) #duration, elementType, empty
exhaledImmunityEffect.AddHook(ET_OnBeginRound , EK_NONE, exhaledImmunityEffectTickdown, ())
exhaledImmunityEffect.AddHook(ET_OnTakingDamage2, EK_NONE, exhaledImmunityEffectImmunity, ())
