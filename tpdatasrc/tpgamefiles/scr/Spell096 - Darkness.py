from toee import *

def OnBeginSpellCast(spell):
    print "Darkness OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Darkness OnSpellEffect"

    targetsToRemove = []
    spell.duration = 100 * spell.caster_level # 10 min/cl
    spellTarget = spell.target_list[0]

    auraRadius = 20.0 + (spell.caster.radius / 12.0)
    auraEventId = spellTarget.obj.object_event_append(OLC_CRITTERS, auraRadius)

    spell.target_list.remove_target(spellTarget.obj)

    spellTarget.obj.condition_add_with_args("sp-Darkness", spell.id, spell.duration, 0, auraEventId, spell.dc, 0)

def OnBeginRound(spell):
    print "Darkness OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Darkness OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Darkness OnEndSpellCast"

