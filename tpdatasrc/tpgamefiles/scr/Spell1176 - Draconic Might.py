from toee import *
from spell_utils import checkCategoryType

def OnBeginSpellCast(spell):
    print "Draconic Might OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Draconic Might OnSpellEffect"

    spell.duration = 10 * spell.caster_level #1 min/level
    spellTarget = spell.target_list[0]
    targetIsUndead = checkCategoryType(spellTarget.obj, mc_type_construct, mc_type_undead)

    if targetIsUndead:
        spellTarget.float_text_line("Not a living creature", tf_red)
        game.particles('Fizzle', spellTarget)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        if spellTarget.obj.condition_add_with_args('sp-Draconic Might', spell.id, spell.duration, 0):
            spellTarget.partsys_id = game.particles('sp-Bullstrength', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Draconic Might OnBeginRound"

def OnEndSpellCast(spell):
    print "Draconic Might OnEndSpellCast"

