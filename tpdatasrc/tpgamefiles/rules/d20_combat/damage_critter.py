import tpdp
import roll_history
import logbook

def deal_attack_damage(attacker, tgt, d20_data, flags, action_type): 
    # return value is used by Coup De Grace
    print('Quel dommage')
    #...
    # should call damage_critter() eventually
    #...
    return -1

def missing_stub(msg):
    print msg
    return 0

def checkAnimationSkip(tgt):
    if tgt.d20_query(Q_Dead):
        return True
    return False

def float_hp_damage(attacker, tgt, damTot, subdualDamTot):

    leaderOfAttacker = attacker.leader_get()
    if leaderOfAttacker in game.party:
        floatColor = tf_yellow
    elif attacker.type == obj_t_pc:
        floatColor = tf_white
    else:
        floatColor = tf_red
    if damTot:
        combatMesLine = game.get_mesline("mes/combat.mes", 1) # HP
        tgt.float_text_line("{} {}".format(damTot, combatMesLine), floatColor)
    elif subdualDamTot:
        combatMesLine = game.get_mesline("mes/combat.mes", 25) # Nonlethal
        tgt.float_text_line("{} {}".format(subdualDamTot, combatMesLine), floatColor)
    return

def damage_critter(attacker, tgt, evt_obj_dam):
    if not tgt:
        return

    #skipHitAnim
    skipHitAnim = checkAnimationSkip(tgt)

    #Check Invulnerability
    tgtFlags = tgt.object_flags_get()
    if tgtFlags & OF_INVULNERABLE:
        evt_obj_dam.damage_packet.add_mod_factor(0.0, D20DT_UNSPECIFIED, 104)

    #Dispatch Damage
    evt_obj_dam.damage_packet.calc_final_damage()
    evt_obj_dam.dispatch(tgt, ET_OnTakingDamage, EK_NONE)
    if evt_obj_dam.attack_packet.get_flags() & D20CAF_TRAP:
        attacker = OBJ_HANDLE_NULL
    elif attacker != OBJ_HANDLE_NULL:
        evt_obj_dam.dispatch(attacker, ET_OnDealingDamage2, EK_NONE)
    evt_obj_dam.dispatch(tgt, ET_OnTakingDamage2, EK_NONE)

    #Set last hit by
    if attacker != OBJ_HANDLE_NULL:
        tgt.obj_set_obj(obj_f_last_hit_by, attacker)

    #Assign damage
    damTot = max(evt_obj_dam.damage_packet.get_overall_damage(), 0)
    hpDam = tgt.obj_get_int(obj_f_hp_damage)
    hpDam += damTot
    tgt.set_hp_damage(hpDam)

    #History Roll Window
    history_roll_id = roll_history.add_damage_roll(attacker, tgt, evt_obj_dam.damage_packet)
    game.create_history_from_id(history_roll_id)

    #Send Signal HP_Changed
    tgt.d20_send_signal(S_HP_Changed, -damTot, -1 if damTot > 0 else 0)

    #Triggers for dealing damage
    if damTot:
        #Add Damaged condition
        tgt.condition_add("Damaged", damTot)
        if attacker and tgt:
            #Log Highest Damage Record
            isWeaponDamage = logbook.is_weapon_damage
            logbook.record_highest_damage(isWeaponDamage, damTot, attacker, tgt)
            #Check for friendly fire
            if attacker != tgt and attacker.is_friendly(tgt):
                tgt.sound_play_friendly_fire(attacker)

    #Subdual Damage
    subdualDamTot = evt_obj_dam.damage_packet.get_overall_damage_by_type(D20DT_SUBDUAL)
    if subdualDamTot > 0 and tgt != OBJ_HANDLE_NULL:
        tgt.condition_add("Damaged", subdualDamTot)
    subdual_dam = tgt.obj_get_int(obj_f_critter_subdual_damage)
    tgt.set_subdual_damage(subdual_dam + subdualDamTot)
    tgt.d20_send_signal(S_HP_Changed, subdualDamTot, -1 if subdualDamTot > 0 else 0)

    #Create Floatline
    float_hp_damage(attacker, tgt, damTot, subdualDamTot)

    #Push hit Animation
    if attacker:
        if not skipHitAnim:
            tgt.anim_goal_push_hit_by_weapon(attacker)
    return