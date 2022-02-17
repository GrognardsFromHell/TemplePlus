from toee import *

def OnBeginSpellCast(spell):
    print "Summon Undead IV OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Summon Undead IV OnSpellEffect"
    spell.duration = 1 * spell.caster_level
    undeadToSummon = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)
    #ID 14135 = Ghast; ID 14821 = Greater Temple Bugbear Zombie; ID 14128 = Ghoul; ID 14602 = Skeleton Guard; ID 14081 = Gnoll Skeleton; ID 14092 = Zombie 2HD; ID 14107 = Skeleton 1 HD
    possibleUndeadSummons =[14135, 14821, 14128, 14602, 14081, 14092, 14107]
    isAiFollower = 1

    if undeadToSummon not in possibleUndeadSummons: #fallback
        undeadToSummon = 14821 # Greater Temple Bugbear Zombie

    if undeadToSummon == 14135 or undeadToSummon == 14821: #Level 4 summons
        numberToSummon = 1
    elif undeadToSummon == 14128 or undeadToSummon == 14602: #Level 3 summons
        numberToSummon = 2
    else:
        numberToSummon = 4 #Level 2 or less summons

    for summon in xrange(numberToSummon):
        spell.summon_monsters(isAiFollower, undeadToSummon)
        game.particles('sp-Summon Monster IV', spell.target_list[summon].obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Summon Undead IV OnBeginRound"

def OnEndSpellCast(spell):
    print "Summon Undead IV OnEndSpellCast"

