from toee import *

def OnBeginSpellCast(spell):
    print "Resist Energy Mass OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Resist Energy Mass OnSpellEffect"

    targetsToRemove = []
    spell.duration = 100 * spell.caster_level
    radialChoice = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) #1 = Acid; 2 = Cold, 3 = Electricity, 4 = Fire, 5 = Sonic
    if not radialChoice in range(1,6): #Fallback
        radialChoice = game.random_range(1,5)

    if radialChoice == 1:
        elementType = ACID
        spellParticles = 'sp-Resist Elements-acid'
    elif radialChoice == 2:
        elementType = COLD
        spellParticles = 'sp-Resist Elements-cold'
    elif radialChoice == 3:
        elementType = ELECTRICITY
        spellParticles = 'sp-Resist Elements-water'
    elif radialChoice == 4:
        elementType = FIRE
        spellParticles = 'sp-Resist Elements-fire'
    elif radialChoice == 5:
        elementType = SONIC
        spellParticles = 'sp-Resist Elements-sonic'

    for spellTarget in spell.target_list:
        if spellTarget.obj.condition_add_with_args('sp-Resist Elements', spell.id, elementType, spell.duration):
            spellTarget.partsys_id = game.particles(spellParticles, spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Resist Energy Mass OnBeginRound"

def OnEndSpellCast(spell):
    print "Resist Energy Mass OnEndSpellCast"

