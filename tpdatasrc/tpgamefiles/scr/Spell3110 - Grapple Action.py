from toee import *

#Grapple Spell is a helper spell to perform Grapple Actions
#It basically saves the Grapple Performer (caster) and its target

def OnBeginSpellCast(spell):
    print "Grapple Action OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Grapple Action OnSpellEffect"

    spell.duration = 1
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args("sp-Grappled", spell.id, spell.duration, 0)
    spell.caster.condition_add_with_args("sp-Grappled", spell.id, spell.duration, 0)

def OnBeginRound(spell):
    print "Grapple Action OnBeginRound"
    if spell.num_of_targets == 0:
        spell.spell_end(spell.id)

def OnEndSpellCast(spell):
    print "Grapple Action OnEndSpellCast"

