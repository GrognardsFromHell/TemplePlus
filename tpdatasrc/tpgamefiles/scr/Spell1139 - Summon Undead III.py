from toee import *

def OnBeginSpellCast(spell):
    print "Summon Undead III OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Summon Undead III OnSpellEffect"
    spell.duration = 1 * spell.caster_level
    undeadToSummon = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)
    possibleUndeadSummons =[14128, 14602, 14081, 14092, 14107] #ID 14128 = Ghoul; ID 14602 = Skeleton Guard; ID 14081 = Gnoll Skeleton; ID 14092 = Zombie 2HD; ID 14107 = Skeleton 1 HD
    isAiFollower = 1

    if undeadToSummon not in possibleUndeadSummons: #fallback
        undeadToSummon = 14128 # Ghoul

    if undeadToSummon == 14107:
        numberToSummon = 4
    elif undeadToSummon == 14081 or undeadToSummon == 14092:
        numberToSummon = 2
    else:
        numberToSummon = 1
    
    for summon in xrange(numberToSummon):
        spell.summon_monsters(isAiFollower, undeadToSummon)
        game.particles('sp-Summon Monster III', spell.target_list[summon].obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Summon Undead III OnBeginRound"

def OnEndSpellCast(spell):
    print "Summon Undead III OnEndSpellCast"

