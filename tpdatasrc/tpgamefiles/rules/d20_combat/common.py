from toee import *

# Gets the weapon in the left hand.
# This can be one of the following:
#
#  1. A weapon in the 'secondary' slot
#  2. A shield that allows bashing
#  3. A double wepaon in the 'primary' slot
def getLeftWeapon(attacker):
    left = attacker.item_worn_at(item_wear_weapon_secondary)

    if left == OBJ_HANDLE_NULL and attacker.d20_query(128):
        left = attacker.item_worn_at(item_wear_shield)

    if left == OBJ_HANDLE_NULL:
        right = attacker.item_worn_at(item_wear_weapon_primary)
        if right.is_double_weapon():
            left = right

    return left

# Gets the actual primary weapon, respecting the ability to select
# either of two weapons held to be designated as primary
def getPrimaryWeapon(attacker):
    weapl = getLeftWeapon(attacker)

    # if we're preferring left weapon, and we have something equipped
    # there, use it.
    if attacker.d20_query(127):
        if not weapl == OBJ_HANDLE_NULL:
            return weapl

    # otherwise default to the right hand
    return attacker.item_worn_at(item_wear_weapon_primary)

# Gets the secondary weapon, respecting the ability to select either
# of two weapons held to be designated as primary
def getSecondaryWeapon(attacker):
    weapr = attacker.item_worn_at(item_wear_weapon_primary)
    weapl = getLeftWeapon(attacker)

    # if preferring left weapon, and there actually is a left weapon,
    # then right hand is secondary.
    if attacker.d20_query(127):
        if not weapl == OBJ_HANDLE_NULL:
            return weapr

    # otherwise left hand is secondary
    return weapl

def getUsedWeapon(flags, attacker):
    print "getUsedWeapon"
    unarmed = OBJ_HANDLE_NULL
    #if flags & D20CAF_TOUCH_ATTACK:
    #    return unarmed # this fucks up thrown grenades
    if flags & D20CAF_SECONDARY_WEAPON:
        return getSecondaryWeapon(attacker)
    else:
        return getPrimaryWeapon(attacker)
    return unarmed

