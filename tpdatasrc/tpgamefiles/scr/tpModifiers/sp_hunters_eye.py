from templeplus.pymod import PythonModifier
from toee import *
from spell_utils import SpellPythonModifier

print "Registering sp-Hunter's Eye"

def querySneakDice(attachee, args, evt_obj):
    bonusDice = args.get_arg(2)
    evt_obj.return_val += bonusDice
    return 0

def sneakDamage(attachee, args, evt_obj):
    #Currently I don't think there is a method
    #to add temporaly a feat (in this case feat_sneak_attack)
    #So I add sneak damage manually if the character has no
    #sneak attack feat.
    if not attachee.has_feat(feat_sneak_attack):
        target = evt_obj.attack_packet.target
        if attachee.can_sneak_attack(target):
            bonusDice = dice_new('1d6')
            bonusDice.number = args.get_arg(2)
            damageType = D20DT_UNSPECIFIED
            damageMesId = 106 #ID 106 = ~Sneak Attack~[TAG_CLASS_FEATURES_ROGUE_SNEAK_ATTACK]
            evt_obj.damage_packet.add_dice(bonusDice, damageType, damageMesId)
            #Add Sneak float and history
            attachee.float_mesfile_line('mes\\combat.mes', 90) #ID 90 = Sneak Attack!
            game.create_history_from_pattern(26, attachee, target) #ID 26 = [ACTOR] ~sneak attacks~[TAG_CLASS_FEATURES_ROGUE_SNEAK_ATTACK] [TARGET]!
    return 0

huntersEyeSpell = SpellPythonModifier("sp-Hunter's Eye", 4) # spellId, duration, bonusDice, empty
huntersEyeSpell.AddHook(ET_OnD20PythonQuery, "Sneak Attack Dice", querySneakDice, ())
huntersEyeSpell.AddHook(ET_OnDealingDamage, EK_NONE, sneakDamage, ())
