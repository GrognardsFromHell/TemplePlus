from toee import *

def OnBeginSpellCast(spell):
    print "Ignore the Pyre OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Ignore the Pyre OnSpellEffect"

    spell.duration = 14400 #1 day
    spellTarget = spell.target_list[0]
    bonusValue = spell.caster_level
    radialChoice = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) #1 = Acid; 2 = Cold, 3 = Electricity, 4 = Fire, 5 = Sonic
    if not radialChoice in range(1,6): #Fallback
        radialChoice = game.random_range(1,5)

    if radialChoice == 1:
        elementType = D20DT_ACID
        spellParticles = "sp-Resist Elements-acid"
    elif radialChoice == 2:
        elementType = D20DT_COLD
        spellParticles = "sp-Resist Elements-cold"
    elif radialChoice == 3:
        elementType = D20DT_ELECTRICITY
        spellParticles = "sp-Resist Elements-water"
    elif radialChoice == 4:
        elementType = D20DT_FIRE
        spellParticles = "sp-Resist Elements-fire"
    elif radialChoice == 5:
        elementType = D20DT_SONIC
        spellParticles = "sp-Resist Elements-sonic"

    if spellTarget.obj.condition_add_with_args("sp-Ignore the Pyre", spell.id, spell.duration, bonusValue, elementType, 0):
        spellTarget.partsys_id = game.particles(spellParticles, spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Ignore the Pyre OnBeginRound"

def OnEndSpellCast(spell):
    print "Ignore the Pyre OnEndSpellCast"

