from toee import *

def OnBeginSpellCast(spell):
    print "Enervating Shadow OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Enervating Shadow OnSpellEffect"

    targetsToRemove = []
    spell.duration = 5

    auraRadius = 5.0 + (spell.caster.radius / 12.0)
    auraEventId = spell.caster.object_event_append(OLC_CRITTERS, auraRadius)

    for spellTarget in spell.target_list:
         targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

    spell.caster.condition_add_with_args("sp-Enervating Shadow", spell.id, spell.duration, 0, auraEventId, spell.dc, 0)

def OnBeginRound(spell):
    print "Enervating Shadow OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Enervating Shadow OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Enervating Shadow OnEndSpellCast"

