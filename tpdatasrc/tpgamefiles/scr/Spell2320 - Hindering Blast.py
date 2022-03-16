from toee import *

def OnBeginSpellCast(spell):
    print "Hindering Blast OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Hindering Blast OnSpellEffect"

    spell.duration = 0 * spell.caster_level
    spellTarget = spell.target_list[0]
    spellEnum = spell_hindering_blast

    spellTarget.obj.condition_add_with_args("Hindering Blast", spellEnum, 0)

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Hindering Blast OnBeginRound"

def OnEndSpellCast(spell):
    print "Hindering Blast OnEndSpellCast"

