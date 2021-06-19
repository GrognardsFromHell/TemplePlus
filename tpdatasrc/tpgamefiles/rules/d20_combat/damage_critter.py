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
    
    missing_stub("evtObjDam.damage.CalcFinalDamage();")
    return