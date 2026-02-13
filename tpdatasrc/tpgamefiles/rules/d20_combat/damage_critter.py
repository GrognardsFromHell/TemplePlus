import tpdp
import roll_history
import logbook
from common import *

debug_enabled = True

def debug_print(*args):
    if debug_enabled:
        for arg in args:
            print arg
    return

def missing_stub(msg):
    print msg
    return 0

def party_affiliation_cmp(attacker, tgt):
    return (tgt in game.party) == (attacker in game.party)

#attacker.allegiance_shared(tgt) is not working
def floatFriendlyFire(attacker, tgt):
    if attacker == OBJ_HANDLE_NULL or tgt == OBJ_HANDLE_NULL:
        return
    if not party_affiliation_cmp(attacker, tgt):
        return
    if tgt.type != obj_t_npc or attacker.allegiance_shared(tgt):
        debug_print("Debug: Friendly Fire triggered")
        tgt.float_mesfile_line('mes\\combat.mes', 107) # Friendly Fire
    # else:
    #     debug_print("Debug: Friendly Fire NOT triggered", "Debug attacker.allegiance_shared(tgt): {}".format(attacker.allegiance_shared(tgt)), "Debug tgt in game.party: {}".format(tgt in game.party), "Debug attacker in game.party: {}".format(attacker in game.party))
    return

def playSoundEffect(weaponUsed, attacker, tgt, sound_event):
    sound_id = attacker.soundmap_item(weaponUsed, tgt, sound_event)
    game.sound_local_obj(sound_id, attacker)
    return

def deal_attack_damage(attacker, tgt, d20_data, flags, action_type): 
    # return value is used by Coup De Grace

    #Provoke Hostility
    attacker.attack(tgt, 1, 0)
    
    #Check if target is alive
    if tgt == OBJ_HANDLE_NULL or tgt.is_dead_or_destroyed():
        return -1
    
    #Create evt_obj_dam
    evt_obj_dam = tpdp.EventObjDamage()
    evt_obj_dam.attack_packet.action_type = tpdp.D20ActionType(action_type)
    evt_obj_dam.attack_packet.attacker = attacker
    evt_obj_dam.attack_packet.target = tgt
    evt_obj_dam.attack_packet.event_key = d20_data
    evt_obj_dam.attack_packet.set_flags(flags)

    #Set weapon used
    usedWeapon = getUsedWeapon(evt_obj_dam.attack_packet.get_flags(), attacker)
    evt_obj_dam.attack_packet.set_weapon_used(usedWeapon)

    #Check ammo
    evt_obj_dam.attack_packet.ammo_item = attacker.get_ammo_used()

    #Check Concealment Miss
    # is this actually set anywhere? check!
    if flags & D20CAF_CONCEALMENT_MISS:
        roll_history.add_from_pattern(11, attacker, tgt)
        attacker.float_mesfile_line('mes\\combat.mes', 45) # Miss (Concealment)
        playSoundEffect(evt_obj_dam.attack_packet.get_weapon_used(), attacker, tgt, 6)
        evt_obj_dam.send_signal(attacker, S_Attack_Made)
        return -1

    #Check normal Miss
    if (flags & D20CAF_HIT) == 0:
        attacker.float_mesfile_line('mes\\combat.mes', 29) # Miss
        evt_obj_dam.send_signal(attacker, S_Attack_Made)
        playSoundEffect(evt_obj_dam.attack_packet.get_weapon_used(), attacker, tgt, 6)
        #Check if arrow was deflected
        if flags & D20CAF_DEFLECT_ARROWS:
            tgt.float_mesfile_line('mes\\combat.mes', 5052) #{5052}{Deflect Arrows}
            roll_history.add_from_pattern(12, attacker, tgt)
        #dodge animation
        if tgt.is_unconscious() or tgt.d20_query(Q_Prone):
            return -1
        else:
            tgt.anim_goal_push_dodge(attacker)
        return -1

    #Check if Friendly Fire and trigger float if true
    floatFriendlyFire(attacker, tgt)

    #Check if is already unconscious
    wasAlreadyUnconscious = tgt.is_unconscious()

    #dispatch damage
    evt_obj_dam.dispatch(attacker, ET_OnDealingDamage, EK_NONE)

    #handle critical hit
    if evt_obj_dam.attack_packet.get_flags() & D20CAF_CRITICAL:
        #create evt_obj_crit_dice
        evt_obj_crit_dice = tpdp.EventObjAttack()
        evt_obj_crit_dice.attack_packet.action_type = tpdp.D20ActionType(action_type)
        evt_obj_crit_dice.attack_packet.attacker = attacker
        evt_obj_crit_dice.attack_packet.target = tgt
        evt_obj_crit_dice.attack_packet.event_key = d20_data
        evt_obj_crit_dice.attack_packet.set_flags(evt_obj_dam.attack_packet.get_flags())
        evt_obj_crit_dice.attack_packet.set_weapon_used(evt_obj_dam.attack_packet.get_weapon_used())
        #Check ammo
        evt_obj_crit_dice.attack_packet.ammo_item = attacker.get_ammo_used()
        #apply extra damage
        extraDamageDice = evt_obj_crit_dice.dispatch(attacker, OBJ_HANDLE_NULL, ET_OnGetCriticalHitExtraDice, EK_NONE)
        debug_print("Debug extraDamageDice: {}".format(extraDamageDice))
        
        evt_obj_dam.damage_packet.critical_multiplier_apply(extraDamageDice + 1)
        attacker.float_mesfile_line('mes\\combat.mes', 12) #{12}{Critical Hit!}
        #play crit hit sound
        soundIdTarget = tgt.soundmap_critter(0)
        game.sound_local_obj(soundIdTarget, tgt)
        playSoundEffect(evt_obj_crit_dice.attack_packet.get_weapon_used(), attacker, tgt, 7)
        #Logbook increase crit count
        logbook.inc_criticals(attacker)
    else:
        #if no crit only play sound
        playSoundEffect(evt_obj_dam.attack_packet.get_weapon_used(), attacker, tgt, 5)

    #Logbook
    logbook.is_weapon_damage = 1 # used for recording attack damage
    
    #Call damage_critter
    damage_critter(attacker, tgt, evt_obj_dam)

    #Play damage particle effects
    evt_obj_dam.damage_packet.play_pfx(tgt)
    
    #Send signals
    evt_obj_dam.send_signal(attacker, S_Attack_Made)
    if not wasAlreadyUnconscious and tgt.is_unconscious():
        evt_obj_dam.send_signal(attacker, S_Dropped_Enemy)
        
    debug_print("Debug: print overall damage: {}".format(evt_obj_dam.damage_packet.get_overall_damage_by_type(D20DT_UNSPECIFIED)))
    overall_dam = evt_obj_dam.damage_packet.get_overall_damage_by_type(D20DT_UNSPECIFIED)
    return overall_dam

def deal_spell_damage(tgt, attacker, dice, damageType, attackPower, reduction, damageDescId, action_type, spellId, flags, projectile_idx = 1, is_weaponlike = False):
    debug_print("Debug Hook deal_spell_damage")
    debug_print("Debug spell target: {}".format(tgt), "Debug spell caster: {}".format(attacker), "Debug Dice: {}".format(dice), "Debug damageType: {}".format(damageType), "Debug attackPower: {}".format(attackPower), "Debug reduction: {}".format(reduction), "Debug damageDescId: {}".format(damageDescId), "Debug action_type: {}".format(action_type), "Debug spellId: {}".format(spellId), "Debug flags: {}".format(flags))
    spellPacket = tpdp.SpellPacket(spellId)
    if not tgt:
        debug_print("Debug No Target!")
        return

    #Check if Friendly Fire and trigger float if true
    floatFriendlyFire(attacker, tgt)

    #Provoke Hostility
    #missing_stub("aiSys.ProvokeHostility(attacker, tgt, 1, 0);")
    attacker.attack(tgt, 1, 0)

    #Check if target is alive
    if tgt == OBJ_HANDLE_NULL or tgt.is_dead_or_destroyed():
        debug_print("Debug target destroyed!")
        return

    if is_weaponlike:    
        if tgt.is_flanked_by(attacker):
            flags |= D20CAF_FLANKED

    #create evt_obj_dam
    evt_obj_dam = tpdp.EventObjDamage()
    evt_obj_dam.attack_packet.action_type = tpdp.D20ActionType(action_type)
    evt_obj_dam.attack_packet.attacker = attacker
    evt_obj_dam.attack_packet.target = tgt
    evt_obj_dam.attack_packet.event_key = projectile_idx
    evt_obj_dam.attack_packet.set_flags(flags | D20CAF_HIT)

    #Set weapon used
    if attacker != OBJ_HANDLE_NULL and attacker.is_critter():
        usedWeapon = getUsedWeapon(evt_obj_dam.attack_packet.get_flags(), attacker)
        evt_obj_dam.attack_packet.set_weapon_used(usedWeapon)
        #Check ammo
        evt_obj_dam.attack_packet.ammo_item = attacker.get_ammo_used()
    else:
        evt_obj_dam.attack_packet.set_weapon_used(OBJ_HANDLE_NULL)
        evt_obj_dam.attack_packet.ammo_item = OBJ_HANDLE_NULL

    #Add damage mod factor
    if reduction != 100:
        evt_obj_dam.damage_packet.add_mod_factor((reduction * 0.01), damageType, damageDescId)

    if is_weaponlike:
        if flags & D20CAF_CONCEALMENT_MISS:
            roll_history.add_from_pattern(11, attacker, tgt)
            attacker.float_mesfile_line('mes\\combat.mes', 45) # Miss (Concealment)
            # note: do net send S_Attack_Made because casting a spell is not considered an attack action
            return
        if (flags & D20CAF_HIT) == 0:
            attacker.float_mesfile_line("mes\\combat.mes", 29) # miss
            # play dodge animation
            if tgt.is_unconscious() or tgt.d20_query(Q_Prone):
                return
            else:
                tgt.anim_goal_push_dodge(attacker)
            return

    #Add damage dice
    #If we could get a string support for add_dice
    #we could get rid of this ugly ID 103 in damage.mes
    #ID 103 = Unknown
    #Could be replaced by:
    #spellEnum = tpdp.SpellPacket(spellId).spell_enum
    #spellName = game.get_spell_mesline(spellEnum)
    #spellNameTag = spellName.upper().replace(" ", "_")
    #evt_obj_dam.damage_packet.add_dice_with_custom_string(dice, damageType, "~{}~[TAG_SPELLS_{}]".format(spellName, spellNameTag))
    evt_obj_dam.damage_packet.add_dice(dice, damageType, 103)

    #Add attackPower
    evt_obj_dam.damage_packet.attack_power |= attackPower

    #set MetaMagic
    metaMagicData = spellPacket.get_metamagic_data()
    if metaMagicData.get_empower_count():
        evt_obj_dam.damage_packet.flags |= 2; # empowered
    if  metaMagicData.get_maximize():
        evt_obj_dam.damage_packet.flags |= 1; # maximized

    if is_weaponlike:
            #handle critical hit
        if evt_obj_dam.attack_packet.get_flags() & D20CAF_CRITICAL:
            
            # should this section apply?

            # #create evt_obj_crit_dice
            # evt_obj_crit_dice = tpdp.EventObjAttack()
            # evt_obj_crit_dice.attack_packet.action_type = tpdp.D20ActionType(action_type)
            # evt_obj_crit_dice.attack_packet.attacker = attacker
            # evt_obj_crit_dice.attack_packet.target = tgt
            # evt_obj_crit_dice.attack_packet.event_key = d20_data
            # evt_obj_crit_dice.attack_packet.set_flags(evt_obj_dam.attack_packet.get_flags())
            # evt_obj_crit_dice.attack_packet.set_weapon_used(evt_obj_dam.attack_packet.get_weapon_used())
            # #Check ammo
            # evt_obj_crit_dice.attack_packet.ammo_item = attacker.get_ammo_used()
            # #apply extra damage
            # extraDamageDice = evt_obj_crit_dice.dispatch(attacker, OBJ_HANDLE_NULL, ET_OnGetCriticalHitExtraDice, EK_NONE)
            # debug_print("Debug extraDamageDice: {}".format(extraDamageDice))

            extraDamageDice = 1
            evt_obj_dam.damage_packet.critical_multiplier_apply(extraDamageDice + 1)
            attacker.float_mesfile_line('mes\\combat.mes', 12) #{12}{Critical Hit!}
            #play crit hit sound
            soundIdTarget = tgt.soundmap_critter(0)
            game.sound_local_obj(soundIdTarget, tgt)
            playSoundEffect(evt_obj_dam.attack_packet.get_weapon_used(), attacker, tgt, 7)
            #Logbook increase crit count
            logbook.inc_criticals(attacker)
        evt_obj_dam.dispatch( attacker, ET_OnDealingDamageWeaponlikeSpell, EK_NONE)
        

    #Dispatch damage
    overall_dam = evt_obj_dam.dispatch_spell_damage(attacker, tgt, spellPacket)
    debug_print("Debug spell overall_dam: {}".format(overall_dam))

    #Logbook
    logbook.is_weapon_damage = 0 # used for recording attack damage

    #Call Damage Critter
    damage_critter(attacker, tgt, evt_obj_dam)

    return

def checkAnimationSkip(tgt):
    if tgt.is_unconscious() or tgt.d20_query(Q_Prone):
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

    if damTot != 0 and subdualDamTot != 0:
        combatMesLine = game.get_mesline("mes/combat.mes", 1) # HP
        tgt.float_text_line("{} {}".format(damTot, combatMesLine), floatColor)
        combatMesLine = game.get_mesline("mes/combat.mes", 25) # Nonlethal
        tgt.float_text_line("{} {}".format(subdualDamTot, combatMesLine), floatColor)
    elif subdualDamTot !=0: # damTot ==0
        combatMesLine = game.get_mesline("mes/combat.mes", 25) # Nonlethal
        tgt.float_text_line("{} {}".format(subdualDamTot, combatMesLine), floatColor)
    #if attack deals 0 damage to due damage reduction/resistance
    else: # subdualDamTot ==0; damTot can be 0 or not
        combatMesLine = game.get_mesline("mes/combat.mes", 1) # HP
        tgt.float_text_line("{} {}".format(damTot, combatMesLine), floatColor)
    return

# Check if the attack could deal damage _at all_. Some messages are only
# displayed if that is the case.
#
# The current check looks for a condition that should only occur if
# `damage_packet.reset()` (or an equivalent) has been called, e.g. for holy
# water.
def could_damage(evt_obj_dam):
    return evt_obj_dam.damage_packet.dice_count > 0

def damage_critter(attacker, tgt, evt_obj_dam):
    if not tgt:
        return

    #skipHitAnim
    skipHitAnim = checkAnimationSkip(tgt)

    #Check Invulnerability
    tgtFlags = tgt.object_flags_get()
    if tgtFlags & OF_INVULNERABLE:
        evt_obj_dam.damage_packet.add_mod_factor(0.0, D20DT_UNSPECIFIED, 104)

    disp_msg = could_damage(evt_obj_dam)

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
    if disp_msg:
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
    if disp_msg:
        float_hp_damage(attacker, tgt, damTot, subdualDamTot)

    #Push hit Animation
    if attacker:
        if not skipHitAnim:
            tgt.anim_goal_push_hit_by_weapon(attacker)
    return
