from toee import *

def OnBeginSpellCast(spell):
    print "Ironguts OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Ironguts OnSpellEffect"

    spell.duration = 100 * spell.caster_level # 10 minutes/CL
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args('sp-Ironguts', spell.id, spell.duration)
    spellTarget.partsys_id = game.particles('sp-Neutralize Poison', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Ironguts OnBeginRound"

def OnEndSpellCast(spell):
    print "Ironguts OnEndSpellCast"