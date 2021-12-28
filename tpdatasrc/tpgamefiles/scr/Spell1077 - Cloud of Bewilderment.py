from toee import *

def OnBeginSpellCast(spell):
    print "Cloud of Bewilderment OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Cloud of Bewilderment OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 1 * spell.caster_level # 1 round/cl

    cloudObject = game.obj_create(OBJECT_SPELL_GENERIC, spell.target_loc)

    casterInitiative = spell.caster.get_initiative()
    cloudObject.d20_status_init()
    cloudObject.set_initiative(casterInitiative)

    cloudRadius = 10.0 #+ (spell.target_list[0].obj.radius / 12.0)
    cloudEventId = cloudObject.object_event_append(OLC_CRITTERS, cloudRadius)

    for spellTarget in spell.target_list:
         targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

    cloudObject.condition_add_with_args("sp-Cloud of Bewilderment", spell.id, spell.duration, spell.dc, cloudEventId, 0, 0)

def OnBeginRound(spell):
    print "Cloud of Bewilderment OnBeginRound"
    #spellTarget = spell.begin_round_obj
    #if not spellTarget.d20_query_has_condition("Nauseated"):
    #    if not spellTarget.saving_throw_spell(spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id):
    #        duration = 1
    #        persistentFlag = 1
    #        spellTarget.condition_add_with_args("Nauseated", duration, persistentFlag)

def OnEndSpellCast(spell):
    print "Cloud of Bewilderment OnEndSpellCast"


def OnAreaOfEffectHit(spell):
    print "Cloud of Bewilderment OnAreaOfEffectHit"

def OnBeginRoundD20Ping(spell):
    print "Cloud of Bewilderment OnBeginRoundD20Ping"