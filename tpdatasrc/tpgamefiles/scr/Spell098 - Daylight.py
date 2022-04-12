from toee import *

def OnBeginSpellCast(spell):
    print "Daylight OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Daylight OnSpellEffect"

    targetsToRemove = []
    spell.duration = 100 * spell.caster_level # 10 min/cl
    spellTarget = spell.target_list[0]

    auraRadius = 60.0 + (spell.caster.radius / 12.0)
    auraEventId = spellTarget.obj.object_event_append(OLC_CRITTERS, auraRadius)

    spell.target_list.remove_target(spellTarget.obj)

    spellTarget.obj.condition_add_with_args("sp-Daylight", spell.id, spell.duration, 0, auraEventId, spell.dc, 0)

def OnBeginRound(spell):
    print "Daylight OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Daylight OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Daylight OnEndSpellCast"

