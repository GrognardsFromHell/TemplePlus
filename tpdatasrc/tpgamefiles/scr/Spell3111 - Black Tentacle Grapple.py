from toee import *

#Black Tentacle Grapple Spell is a helper spell to perform a spelcial Grapple Action
#It basically saves the Grapple Performer (Tentacle) and its target

def OnBeginSpellCast(spell):
    print "Black Tentacle Grapple OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Black Tentacle Grapple OnSpellEffect"

    spell.duration = 1
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args("sp-Tentacle Grapple", spell.id, spell.duration, 0)

def OnBeginRound(spell):
    print "Black Tentacle Grapple OnBeginRound"

def OnEndSpellCast(spell):
    print "Black Tentacle Grapple OnEndSpellCast"

