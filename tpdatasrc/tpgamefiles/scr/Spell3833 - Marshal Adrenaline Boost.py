from toee import *
from aura_utils import verifyTarget

def OnBeginSpellCast(spell):
    print "Marshal Adrenaline Boost OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Marshal Adrenaline Boost OnSpellEffect"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

    ### Workaround caster level ###
    if spell.caster_level < 1:
        spell.caster_level = spell.caster.stat_level_get(stat_level_marshal)

    targetsToRemove = []
    spell.duration = 10 * spell.caster_level

    for spellTarget in spell.target_list:
        if spellTarget.obj.is_friendly(spell.caster) and verifyTarget(spellTarget.obj):
            game.particles('sp-Cure Minor Wounds', spellTarget.obj)
            spellTarget.obj.float_text_line("Adrenaline Boost")
            spellTarget.obj.condition_add_with_args("sp-Marshal Adrenaline Boost", spell.id, spell.duration, 0, 0)
        else:
            targetsToRemove.append(spellTarget.obj)
    spell.target_list.remove_list(targetsToRemove)

def OnBeginRound(spell):
    print "Marshal Adrenaline Boost OnBeginRound"

def OnEndSpellCast(spell):
    print "Marshal Adrenaline Boost OnEndSpellCast"