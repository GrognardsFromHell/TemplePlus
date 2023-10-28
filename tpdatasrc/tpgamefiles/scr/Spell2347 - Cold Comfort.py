from toee import *

def OnBeginSpellCast(spell):
    print "Cold Comfort OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Cold Comfort OnSpellEffect"

    targetsToRemove = []
    spell.duration = 14400
    radialChoice = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING) #1 = Cold, 2 = Fire
    damageType = D20DT_COLD if radialChoice == 1 else D20DT_FIRE

    auraRadius = 30.0 + (spell.caster.radius / 12.0)
    auraEventId = spell.caster.object_event_append(OLC_CRITTERS, auraRadius)

    for spellTarget in spell.target_list:
         targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

    spell.caster.condition_add_with_args("sp-Cold Comfort", spell.id, spell.duration, damageType, auraEventId, spell.dc, 0)

def OnBeginRound(spell):
    print "Cold Comfort OnBeginRound"

def OnAreaOfEffectHit(spell):
    print "Cold Comfort OnAreaOfEffectHit"

def OnEndSpellCast(spell):
    print "Cold Comfort OnEndSpellCast"

