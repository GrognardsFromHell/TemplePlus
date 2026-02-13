from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
import math

# Ki Blast: Player Handbook II, p. 80

kiBlastEnum = 1224

def getFeatName():
    return "Ki Blast"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def addExtraCharges(attachee, args, evt_obj):
    evt_obj.return_val += 1
    return 0

def createRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, kiBlastEnum, radialData1, featTag)
    spell_data = tpdp.D20SpellData(spell_ki_blast)
    characterLevel = attachee.stat_level_get(stat_level)
    spell_data.set_spell_class(stat_level_monk)
    spell_data.set_spell_level(characterLevel)
    radialId.set_spell_data(spell_data)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def actionCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 2: #Ki Blast costs 2 charges
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def actionPerform(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    target = evt_obj.d20a.target
    if target == attachee or target == OBJ_HANDLE_NULL:
        target = currentSequence.spell_packet.get_target(0)
        evt_obj.d20a.target = target

    newSpellId = tpactions.get_new_spell_id()
    tpactions.register_spell_cast(currentSequence.spell_packet, newSpellId)
    evt_obj.d20a.spell_id = newSpellId
    currentSequence.spell_action.spell_id = newSpellId

    if attachee.anim_goal_push_attack(target, game.random_range(0,2), 0 ,0):
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

def actionFrame(attachee, args, evt_obj):
    #Deduct Stunning Fist Charges
    chargesToDeduct = 2 #Note: Ki Blast really costs 2 charges :(
    attachee.d20_send_signal("PS_Deduct_Stunning_Fist_Charge", chargesToDeduct)

    currentSequence = tpactions.get_cur_seq()
    target = evt_obj.d20a.target
    if target != OBJ_HANDLE_NULL:
        game.create_history_freeform("{} attacks {} with ~Ki Blast~[TAG_KI_BLAST]\n\n".format(attachee.description, target.description))
        projectileProto = 3000 #ID 3000: bp_projectile_standard
        projectileHandle = evt_obj.d20a.create_projectile_and_throw(projectileProto, target)
        projectileHandle.obj_set_float(obj_f_offset_z, 60.0)
        projectileHandle.obj_set_int(obj_f_projectile_part_sys_id, game.particles('ft-Ki Blast-proj', projectileHandle))
        game.sound(6900, 1) #Burning hands begin sound
        currentSequence.spell_packet.spell_id = evt_obj.d20a.spell_id
        currentSequence.spell_packet.set_projectile(0, projectileHandle)
        ammoItem = OBJ_HANDLE_NULL
        if evt_obj.d20a.projectile_append(projectileHandle, ammoItem):
            attachee.apply_projectile_particles(projectileHandle, evt_obj.d20a.flags)
            evt_obj.d20a.flags |= D20CAF_NEED_PROJECTILE_HIT

    # Perform attack and damage now
    return_val = attachee.perform_touch_attack( target )
    if return_val & D20CAF_HIT:
        game.particles( "ft-Ki Blast-Hit", target )
    damage_dice = dice_new("1d6")
    damage_dice.number = 3
    wisScore = attachee.stat_level_get(stat_wisdom)
    damage_dice.bonus = (wisScore - 10)/2
    target.spell_damage_weaponlike( attachee, D20DT_FORCE, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, currentSequence.spell_packet.spell_id , return_val, 0) #index of target??
        
    return 0

kiBlastFeat = PythonModifier("{} Feat".format(getFeatName()), 2) #featEnum, empty
kiBlastFeat.MapToFeat("{}".format(getFeatName()), feat_cond_arg2 = 0)
kiBlastFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, createRadial, ())
kiBlastFeat.AddHook(ET_OnD20PythonActionCheck, kiBlastEnum, actionCheck, ())
kiBlastFeat.AddHook(ET_OnD20PythonActionPerform, kiBlastEnum, actionPerform, ())
kiBlastFeat.AddHook(ET_OnD20PythonActionFrame, kiBlastEnum, actionFrame, ())
kiBlastFeat.AddHook(ET_OnD20PythonQuery, "PQ_Get_Extra_Stunning_Fist_Charges", addExtraCharges, ())
