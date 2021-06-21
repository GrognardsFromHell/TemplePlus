from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import logbook
import roll_history

debug_enabled = False

def debug_print(*args):
    if debug_enabled:
        for arg in args:
            print arg,
    return

def handle_sanctuary(to_hit_eo, d20a):
    tgt = d20a.target
    if tgt == OBJ_HANDLE_NULL or not tgt.is_critter():
        return
    if d20a.query_can_be_affected_action_perform(tgt):
        return
    flags = to_hit_eo.attack_packet.get_flags()
    if flags & D20CAF_CRITICAL:
        flags &= ~D20CAF_CRITICAL
    if flags & D20CAF_HIT:
        flags &= ~D20CAF_HIT
        to_hit_eo.bonus_list.add_zeroed(262) # Action lost due to Sanctuary
    to_hit_eo.attack_packet.set_flags(flags)
    return


def add_percent_chance_history_stub():
    return


def mirror_image_attack_roll(d20a, spell_id):
    performer = d20a.performer
    target = d20a.target

    #Target AC 
    mi_ac_evt_obj = tpdp.EventObjAttack()
    mi_ac_evt_obj.attack_packet.attacker = performer
    mi_ac_evt_obj.attack_packet.target = target
    flags = d20a.flags
    flags |= D20CAF_TOUCH_ATTACK
    mi_ac_evt_obj.attack_packet.set_flags(flags)
    mi_ac_evt_obj.attack_packet.action_type = d20a.action_type
    mi_ac_evt_obj.dispatch(target, OBJ_HANDLE_NULL, ET_OnGetAC, EK_NONE)
    tgt_ac = mi_ac_evt_obj.bonus_list.get_sum()

    #Performer to Hit Bonus
    to_hit = tpdp.EventObjAttack()
    to_hit.dispatch(performer, OBJ_HANDLE_NULL, ET_OnToHitBonus2, EK_NONE)

    dc = 20
    to_hit_dice = dice_new("1d{}".format(dc))
    to_hit_roll = to_hit_dice.roll()
    to_hit_bonus = to_hit.bonus_list.get_sum()

    spell_enum = tpdp.SpellPacket(spell_id).spell_enum
    spell_name = game.get_spell_mesline(spell_enum)

    roll_id = tpdp.create_history_dc_roll(performer, tgt_ac, to_hit_dice, to_hit_roll, spell_name, to_hit.bonus_list)
    result = to_hit_roll - dc + to_hit_bonus
    d20a.roll_id_0 = roll_id
    return result

def hitMirrorImage(d20a, numberOfMirrorImages):
    #Check if real target was hit
    #A roll of 1 indicates hit on real target
    mirrorDice = dice_new("1d{}".format(numberOfMirrorImages+1) )
    mirrorRoll = mirrorDice.roll()
    if mirrorRoll == 1:
        return False
    
    performer = d20a.performer
    target = d20a.target
    
    #Get spell_id and spellName
    spell_id = target.d20_query_get_data(Q_Critter_Has_Mirror_Image,0)
    
    roll_result = mirror_image_attack_roll(d20a, spell_id)
    if roll_result >= 0:
        target.d20_send_signal(S_Spell_Mirror_Image_Struck, spell_id, 0)
        target.float_mesfile_line('mes\\combat.mes', 109)
        game.create_history_from_pattern(10, performer, target)
        return True
    else:
        #I am unsure how misses are actually handled in this version
        return False

def getDefenderConcealment(d20a):
    target = d20a.target
    defenderConcealment = tpdp.EventObjAttack()
    defenderConcealment.attack_packet.set_flags(d20a.flags)
    defenderConcealment.attack_packet.target = target
    defenderConcealment.attack_packet.attacker = d20a.performer
    return defenderConcealment.dispatch(target, OBJ_HANDLE_NULL, ET_OnGetDefenderConcealmentMissChance, EK_NONE)

def getAttackerConcealment(performer):
    performerConcealment = tpdp.EventObjAttack()
    performerConcealment.dispatch(performer, OBJ_HANDLE_NULL, ET_OnGetAttackerConcealmentMissChance, EK_NONE)
    return performerConcealment.bonus_list.get_highest()

def getSuppressConcealment(performer, target):
    #suppressingConditions can be easily expanded with new conditions if necessary
    suppressingConditions = [tpdp.get_condition_ref("sp-True Strike"), tpdp.get_condition_ref("Weapon Seeking")]
    if any(performer.d20_query_with_data(Q_Critter_Has_Condition, conRef, 0) for conRef in suppressingConditions):
        return True
    elif performer.can_blindsee(target):
        return True
    elif performer.d20_query("Ignore Concealment"): #Example for Arcane Archer; not implemented in AA
        return True
    return False

def rollConcealment(concealmentMissChance):
    concealmentDice = dice_new("1d100")
    concealmentDiceRoll = concealmentDice.roll()
    if concealmentDiceRoll > concealmentMissChance:
        return True, concealmentDiceRoll
    return False, concealmentDiceRoll

def toHitResult(performerToHit, targetAc):
    toHitDice = dice_new("1d20")
    toHitRoll = toHitDice.roll()
    if toHitRoll == 1:
        return False, toHitRoll
    elif toHitRoll == 20:
        return True, toHitRoll
    elif toHitRoll + performerToHit >= targetAc:
        return True, toHitRoll
    return False, toHitRoll


def to_hit_processing(d20a):
    performer = d20a.performer #auto performer = d20a.d20APerformer;
    d20Data = d20a.data1 #auto d20Data = d20a.data1;
    target = d20a.target #auto tgt = d20a.d20ATarget;
    if not target:
        return

    #Mirror Image
    numberOfMirrorImages = target.d20_query(Q_Critter_Has_Mirror_Image)
    if numberOfMirrorImages:
        if hitMirrorImage(d20a, numberOfMirrorImages):
            return

    #Concealment
    debug_print("Concealment")
    targetConcealment = getDefenderConcealment(d20a)
    performerCanSuppressConcealment = getSuppressConcealment(performer, target)
    if performerCanSuppressConcealment:
        targetConcealment = 0
    concealmentMissChance = max(targetConcealment, getAttackerConcealment(performer))
    if concealmentMissChance > 0:
        is_success, miss_chance_roll = rollConcealment(concealmentMissChance)
        if is_success:
            roll_id = roll_history.add_percent_chance_roll(performer, target, concealmentMissChance, 60, miss_chance_roll, 194, 193)
            d20a.roll_id_1 = roll_id
        else: # concealment miss
            roll_id = roll_history.add_percent_chance_roll(performer, target, concealmentMissChance, 60, miss_chance_roll, 195, 193)
            d20a.roll_id_1 = roll_id

            # Blind fight - give second chance
            if not performer.has_feat(feat_blind_fight):
                return

            is_success, miss_chance_roll = rollConcealment(concealmentMissChance)
            if not is_success:
                roll_id = roll_history.add_percent_chance_roll(performer, target, concealmentMissChance, 61, miss_chance_roll, 195, 193)
                return
            
            roll_id = roll_history.add_percent_chance_roll(performer, target, concealmentMissChance, 61, miss_chance_roll, 194, 193)
            d20a.roll_id_2 = roll_id

    #ToHitBonus Actions
    debug_print("To Hit")
    to_hit_eo = tpdp.EventObjAttack()
    to_hit_eo.attack_packet.set_flags(d20a.flags)
    to_hit_eo.attack_packet.target = target
    to_hit_eo.attack_packet.action_type = d20a.action_type #dispIoToHitBon.attackPacket.d20ActnType = d20a.action_type
    to_hit_eo.attack_packet.attacker = performer
    to_hit_eo.attack_packet.event_key = d20Data #dispIoToHitBon.attackPacket.dispKey = d20Data
    unarmed = OBJ_HANDLE_NULL
    if to_hit_eo.attack_packet.get_flags() & D20CAF_TOUCH_ATTACK:
        to_hit_eo.attack_packet.set_weapon_used(unarmed)
    elif to_hit_eo.attack_packet.get_flags() & D20CAF_SECONDARY_WEAPON:
        offhandItem = performer.item_worn_at(item_wear_weapon_secondary)
        if offhandItem.type != obj_t_weapon:
            to_hit_eo.attack_packet.set_weapon_used(unarmed)
        else:
            to_hit_eo.attack_packet.set_weapon_used(offhandItem)
    else:
        mainhandItem = performer.item_worn_at(item_wear_weapon_primary)
        if mainhandItem.type != obj_t_weapon:
            to_hit_eo.attack_packet.set_weapon_used(unarmed)
        else:
            to_hit_eo.attack_packet.set_weapon_used(mainhandItem)
    to_hit_eo.attack_packet.ammo_item = performer.get_ammo_used()
    flags = to_hit_eo.attack_packet.get_flags()
    flags |= D20CAF_FINAL_ATTACK_ROLL
    to_hit_eo.attack_packet.set_flags(flags)
    to_hit_eo.dispatch(performer, OBJ_HANDLE_NULL, ET_OnGetBucklerAcPenalty , EK_NONE)
    to_hit_eo.dispatch(performer, OBJ_HANDLE_NULL, ET_OnToHitBonus2, EK_NONE) # // note: the "Global" condition has ToHitBonus2 hook that dispatches the ToHitBonusBase
    to_hit_bon_final = to_hit_eo.dispatch(performer, OBJ_HANDLE_NULL, ET_OnToHitBonusFromDefenderCondition, EK_NONE)

    #targetAc Actions
    debug_print("Target AC")
    target_ac_eo = to_hit_eo.__copy__()
    target_ac_eo.bonus_list.reset()
    
    target_ac_eo.dispatch(target, OBJ_HANDLE_NULL, ET_OnGetAC, EK_NONE)
    tgt_ac_final = target_ac_eo.dispatch(target, OBJ_HANDLE_NULL, ET_OnGetAcModifierFromAttacker, EK_NONE)

    #Check if attacks hits
    attackDidHit, toHitRoll = toHitResult(to_hit_bon_final, tgt_ac_final)

    critAlwaysCheat = cheats.critical #Note: changed behavior from vanilla (this used to toggle the property)

    #Check for special hit conditions
    if not attackDidHit:
        if to_hit_eo.attack_packet.get_flags() & D20CAF_ALWAYS_HIT:
            attackDidHit = True
        elif critAlwaysCheat:
            attackDidHit = True
        else:
            #Reroll Check
            if performer.d20_query(Q_RerollAttack):
                tpdp.create_history_attack_roll(performer, target, toHitRoll, to_hit_eo.bonus_list, target_ac_eo.bonus_list, to_hit_eo.attack_packet.get_flags()  )
                rerollDidHit, toHitRoll = toHitResult(to_hit_bon_final, tgt_ac_final)
                flags = to_hit_eo.attack_packet.get_flags()
                flags |= D20CAF_REROLL
                to_hit_eo.attack_packet.set_flags(flags)
                if not rerollDidHit:
                    logbook.inc_misses(performer)
                else:
                    attackDidHit = True

    if not attackDidHit:
        debug_print("Missed")
        roll_id = tpdp.create_history_attack_roll(performer, target, toHitRoll, to_hit_eo.bonus_list, target_ac_eo.bonus_list, to_hit_eo.attack_packet.get_flags()  )
        d20a.roll_id_0 = roll_id
        return

    #We have a hit sir!
    debug_print("Scored a hit")
    flags = to_hit_eo.attack_packet.get_flags()
    flags |= D20CAF_HIT
    to_hit_eo.attack_packet.set_flags(flags)
    logbook.inc_hits(performer)
    

    #Check if attack was a critical hit
    performerCritRange = to_hit_eo.__copy__()
    performerCritRange.bonus_list.reset()
    critRange = 21 - performerCritRange.dispatch(performer, OBJ_HANDLE_NULL, ET_OnGetCriticalHitRange, EK_NONE)

    if target.d20_query(Q_Critter_Is_Immune_Critical_Hits):
        isCritical = False
    elif toHitRoll == 20:
        isCritical = True
    elif toHitRoll >= critRange:
        isCritical = True
    elif critAlwaysCheat:
        isCritical = True
    else:
        isCritical = False

    #Check to Confirm Critical Hit
    crit_hit_roll = -1
    if isCritical:
        debug_print("Confirm critical:")
        to_hit_bon_final += to_hit_eo.dispatch(performer, OBJ_HANDLE_NULL, ET_OnConfirmCriticalBonus, EK_NONE)
        critConfirmed, crit_hit_roll = toHitResult(to_hit_bon_final, tgt_ac_final)

        #Check for special confirm conditions
        if not critConfirmed:
            if performer.d20_query("Always Confirm Criticals"):
                critConfirmed = True
            elif critAlwaysCheat:
                critConfirmed = True
            else:
                if performer.d20_query(Q_RerollCritical):
                    tpdp.create_history_attack_roll(performer, target, toHitRoll, to_hit_eo.bonus_list, target_ac_eo.bonus_list, to_hit_eo.attack_packet.get_flags(), crit_hit_roll  )
                    critConfirmed, crit_hit_roll = toHitResult(to_hit_bon_final, tgt_ac_final)
                    #no reroll flag seems to be added in original code

        if critConfirmed:
            debug_print("Crit confirm")
            flags = to_hit_eo.attack_packet.get_flags()
            flags |= D20CAF_CRITICAL
            to_hit_eo.attack_packet.set_flags(flags)
    
    #Deflect Arrows
    #Unsure why it is done after confirm crit,
    #If done before, history window for normal attack
    #could be done earlier
    #dispIoToHitBon.Dispatch(dispIoToHitBon.attackPacket.victim, objHndl::null, dispTypeDeflectArrows, DK_NONE)
    #unsure why it is not simply tgt, will copy it
    to_hit_eo.dispatch(to_hit_eo.attack_packet.target, OBJ_HANDLE_NULL, ET_OnDeflectArrows, EK_NONE)

    handle_sanctuary(to_hit_eo, d20a)
    
    #Set flags
    debug_print("Final")
    d20a.flags = to_hit_eo.attack_packet.get_flags()
    roll_id = tpdp.create_history_attack_roll(performer, target, toHitRoll, to_hit_eo.bonus_list, target_ac_eo.bonus_list, to_hit_eo.attack_packet.get_flags(), crit_hit_roll  )
    d20a.roll_id_0 = roll_id
    return
