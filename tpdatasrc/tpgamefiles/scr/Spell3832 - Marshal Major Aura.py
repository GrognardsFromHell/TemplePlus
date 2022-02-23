from toee import *
import aura_utils

def OnBeginSpellCast(spell):
    print "Marshal Major Aura OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Marshal Major Aura OnSpellEffect"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    print "spell.id= ", spell.id

    ### Workaround caster level ###
    if spell.caster_level < 1:
        spell.caster_level = spell.caster.stat_level_get(stat_level_marshal)

    targetsToRemove = []
    spell.duration = 0

    auraRadius = 60.0 + (spell.caster.radius / 12.0)
    auraEventId = spell.caster.object_event_append(OLC_CRITTERS, auraRadius)
    auraEnum = spell.caster.d20_query_with_data("PQ_Activated_Aura", spell.id, 0)
    spell.caster.condition_add_with_args("sp-Marshal Major Aura", spell.id, auraEnum, auraEventId, aura_type_major, 0)
    auraName = aura_utils.getAuraName(auraEnum)
    for spellTarget in spell.target_list:
        if spellTarget.obj.is_friendly(spell.caster) and aura_utils.verifyTarget(spellTarget.obj):
            spellTarget.obj.condition_add_with_args("Marshal Major Aura {}".format(auraName), spell.id, auraEnum, auraEventId, 0, 0)
        else:
            targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

def OnBeginRound(spell):
    print "Marshal Major Aura OnBeginRound"

def OnEndSpellCast(spell):
    print "Marshal Major Aura OnEndSpellCast"