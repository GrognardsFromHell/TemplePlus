from toee import *

def OnBeginSpellCast(spell):
    print "Corona of Cold OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Corona of Cold OnSpellEffect"

    targetsToRemove = []
    spell.duration = 1 * spell.caster_level
    bonusValue = 0

    auraRadius = 10.0 + (spell.caster.radius / 12.0)
    auraEventId = spell.caster.object_event_append(OLC_CRITTERS, auraRadius)

    for spellTarget in spell.target_list:
         targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

    spell.caster.condition_add_with_args("sp-Corona of Cold", spell.id, spell.duration, bonusValue, auraEventId, spell.dc, 0)

def OnBeginRound(spell):
    print "Corona of Cold OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Corona of Cold OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Corona of Cold OnEndSpellCast"

