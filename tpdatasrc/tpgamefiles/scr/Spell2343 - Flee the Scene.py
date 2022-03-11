from toee import *

def OnBeginSpellCast(spell):
    print "Flee the Scene OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles("sp-transmutation-conjure", spell.caster)

def OnSpellEffect(spell):
    print "Flee the Scene OnSpellEffect"

    spell.duration = 0
    spellTarget = spell.caster

    if spellTarget.d20_query_has_spell_condition(sp_Dimensional_Anchor): #Taken from DD, needs a revisit!
        spellTarget.float_mesfile_line("mes\\spell.mes", 30011)
        game.particles("Fizzle", spellTarget)
        spell.spell_end(spell.id)
    else:
        spellTarget.fade_to(0, 10, 40)
        game.particles("sp-Dimension Door", spellTarget)
        time = 750
        realtime = 1
        game.timeevent_add(fade_back_in, (spellTarget, spell.target_loc, spell), time, realtime)

def OnBeginRound(spell):
    print "Flee the Scene OnBeginRound"

def OnEndSpellCast(spell):
    print "Flee the Scene OnEndSpellCast"

def fade_back_in(spellTarget, loc, spell):
    spellTarget.move(loc, 0.0, 0.0)
    game.particles("sp-Dimension Door", spellTarget)
    spellTarget.fade_to(255, 10, 5)
    spell.spell_end(spell.id)
