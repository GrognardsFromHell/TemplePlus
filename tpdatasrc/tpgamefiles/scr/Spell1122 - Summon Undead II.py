from toee import *

def OnBeginSpellCast(spell):
    print "Summon Undead II OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Summon Undead II OnSpellEffect"
    spell.duration = 1 * spell.caster_level
    undeadToSummon = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)
    possibleUndeadSummons =[14081, 14092, 14107] #ID 14081 = Gnoll Skeleton; ID 14092 = Zombie 2HD; 14107 = Skeleton 1HD
    isAiFollower = 1

    if undeadToSummon not in possibleUndeadSummons: #fallback
        undeadToSummon = 14081

    if undeadToSummon == 14107:
        numberToSummon = 2
    else:
        numberToSummon = 1
    
    for summon in xrange(numberToSummon):
        spell.summon_monsters(isAiFollower, undeadToSummon)
        game.particles('sp-Summon Monster III', spell.target_list[summon].obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Summon Undead II OnBeginRound"

def OnEndSpellCast(spell):
    print "Summon Undead II OnEndSpellCast"
