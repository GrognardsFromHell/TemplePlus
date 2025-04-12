from toee import *
from spell_utils import checkCategoryType

def OnBeginSpellCast(spell):
    print "Bonefiddle OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Bonefiddle OnSpellEffect"
    
    spell.duration = 1 * spell.caster_level # 1 round/cl
    spellTarget = spell.target_list[0]

    #only targets creature with a skeleton or exoskeleton are valid targets
    mcTypeImmunity = checkCategoryType(spellTarget.obj, mc_type_construct, mc_type_elemental, mc_type_plant, mc_type_ooze)

    if mcTypeImmunity or spellTarget.obj.is_category_subtype(mc_subtype_incorporeal): #incorporal are also invalid targets
        spellTarget.obj.float_text_line("Unaffected due to Racial Immunity")
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        if spellTarget.obj.condition_add_with_args("sp-Bonefiddle", spell.id, spell.duration, spell.dc, 0):
            spellTarget.partsys_id = game.particles("sp-Bonefiddle", spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
            game.particles('Fizzle', spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Bonefiddle OnBeginRound"

def OnEndSpellCast(spell):
    print "Bonefiddle OnEndSpellCast"