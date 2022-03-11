from toee import *

def OnBeginSpellCast(spell):
    print "Walk Unseen OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Walk Unseen OnSpellEffect"

    spell.duration = 14400 #1 day
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args("sp-Invisibility", spell.id, spell.duration, 0):
        spellTarget.partsys_id = game.particles("sp-Invisibility", spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Walk Unseen OnBeginRound"

def OnEndSpellCast(spell):
    print "Walk Unseen OnEndSpellCast"

