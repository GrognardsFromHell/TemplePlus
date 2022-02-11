from toee import *

def OnBeginSpellCast(spell):
    print "Evard's Black Tentacles OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Evard's Black Tentacles OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 1 * spell.caster_level # 1 round/cl

    cloudObject = game.obj_create(OBJECT_SPELL_BLACK_TENTACLES, spell.target_loc)

    casterInitiative = spell.caster.get_initiative()
    cloudObject.d20_status_init()
    cloudObject.set_initiative(casterInitiative)
    game.update_combat_ui()


    cloudRadius = 20.0 #+ (spell.target_list[0].obj.radius / 12.0)
    cloudEventId = cloudObject.object_event_append(OLC_CRITTERS, cloudRadius)

    for spellTarget in spell.target_list:
         targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

    cloudObject.condition_add_with_args("sp-Evard's Black Tentacles", spell.id, spell.duration, spell.dc, cloudEventId, 0, 0)

def OnBeginRound(spell):
    print "Evard's Black Tentacles OnBeginRound"

def OnEndSpellCast(spell):
    print "Evard's Black Tentacles OnEndSpellCast"


def OnAreaOfEffectHit(spell):
    print "Evard's Black Tentacles OnAreaOfEffectHit"

def OnBeginRoundD20Ping(spell):
    print "Evard's Black Tentacles OnBeginRoundD20Ping"