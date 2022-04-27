from toee import *

def OnBeginSpellCast(spell):
    print "Retributive Invisibility OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Retributive Invisibility OnSpellEffect"

    spell.duration = spell.caster_level
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args("sp-Retributive Invisibility", spell.id, spell.duration, 0):
        #spellTarget.partsys_id = game.particles("sp-Retributive Invisibility", spellTarget.obj)
        spellTarget.partsys_id = game.particles("sp-Improved Invisibility", spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Retributive Invisibility OnBeginRound"

def OnEndSpellCast(spell):
    print "Retributive Invisibility OnEndSpellCast"

