import tpdp
import roll_history
import logbook

def deal_attack_damage(attacker, tgt, d20_data, flags, action_type): 
    # return value is used by Coup De Grace
    
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
    missing_stub("tgt.obj_set_int(obj_f_last_hit_by, attacker) #attacker needs to be an int")

    #Assign damage
    damTot = max(evt_obj_dam.damage_packet.get_overall_damage(), 0)
    hpDam = tgt.stat_level_get(stat_hp_max) - tgt.stat_level_get(stat_hp_current) #python_object has no get_hp_damage
    hpDam += damTot
    tgt.set_hp_damage(hpDam)

    #History Roll Window
    history_roll_id = roll_history.add_damage_roll(attacker, tgt, evt_obj_dam.damage_packet)
    game.create_history_from_id(history_roll_id)

    #Send Signal HP_Changed
    #d20Sys.d20SendSignal(tgt, D20DispatcherKey::DK_SIG_HP_Changed, -damTot, damTot < 0 ? -1 : 0);
    #How can damTot be below 0 at this point? I don't think it can be
    #def not in my code but in the c++ as well:
    #auto damTot = evtObjDam.damage.GetOverallDamage();
    #if (damTot < 0) damTot = 0;
    #will keep the orig line for now but would suggest to simply use 0
    signalCode = -1 if damTot < 0 else 0
    tgt.d20_send_signal(S_HP_Changed, -damTot, signalCode);

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
                missing_stub("tgt.sound_play_friendly_fire() # I don't know what arg to pass")

    #Subdual Damage TBD
    missing_stub("Subdual Damage")
    subdualDamTot = 0

    #Create Floatline
    #python_object has no isPC or isNPC
    leaderOfAttacker = attacker.leader_get()
    if leaderOfAttacker in game.party:
        floatColor = tf_yellow
    elif attacker in game.party:
        floatColor = tf_white
    else:
        floatColor = tf_red
    if damTot:
        combatMesLine = game.get_mesline("mes/combat.mes", 1)
        tgt.float_text_line("{} {}".format(damTot, combatMesLine), floatColor)
    elif subdualDamTot:
        combatMesLine = game.get_mesline("mes/combat.mes", 25)
        tgt.float_text_line("{} {}".format(subdualDamTot, combatMesLine), floatColor)

    #Push hit Animation
    if attacker:
        if not skipHitAnim:
            missing_stub("gameSystems->GetAnim().PushGoalHitByWeapon(attacker, tgt);")