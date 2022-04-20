from toee import *

def OnBeginSpellCast(spell):
    print "Radiance OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Radiance OnSpellEffect"

    targetsToRemove = []
    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]

    auraRadius = 60.0 + (spell.caster.radius / 12.0)
    auraEventId = spellTarget.obj.object_event_append(OLC_CRITTERS, auraRadius)

    spell.target_list.remove_target(spellTarget.obj)

    spellTarget.obj.condition_add_with_args("sp-Radiance", spell.id, spell.duration, 0, auraEventId, spell.dc, 0)

    # Better would be:
    #if not spellTarget.obj.condition_add_with_args("sp-Radiance", spell.id, spell.duration, 0, auraEventId, spell.dc, 0):
    #    spell.spell_end(spell.id)
    # But this leads to a non removed eventId as I can't remove the eventId in Python manually

def OnBeginRound(spell):
    print "Radiance OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Radiance OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Radiance OnEndSpellCast"

