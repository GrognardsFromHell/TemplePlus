from toee import *

def OnBeginSpellCast(spell):
    print "Vigor OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Vigor OnSpellEffect"

    spell.duration = min(10 + 1 * spell.caster_level, 25)
    spellTarget = spell.target_list[0]
    healAmount = 2

    if spellTarget.obj.is_category_type(mc_type_undead) or spellTarget.obj.is_category_type(mc_type_construct):
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 32000) #ID 32000 = Target is immune!
        spell.target_list.remove_target(spellTarget.obj)
    else:
        if spellTarget.obj.condition_add_with_args('sp-Vigor Fast Healing', spell.id, spell.duration, healAmount, 0):
            spellTarget.partsys_id = game.particles('sp-Vigor', spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000) # ID 30000 = Spell has fizzled!
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Vigor OnBeginRound"

def OnEndSpellCast(spell):
    print "Vigor OnEndSpellCast"

