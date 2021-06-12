from toee import *

def OnBeginSpellCast(spell):
    print "Faith Healing OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Faith Healing OnSpellEffect"
    
    spell.duration = 0
    spellTarget = spell.target_list[0].obj
    spellCasterDeity = spell.caster.get_deity()
    spellTargetDeity = spellTarget.get_deity()
    spellDamageDice = dice_new('1d8')
    spellDamageDice.bonus = min(spell.caster_level, 5) #capped at CL 5

    if spellCasterDeity == spellTargetDeity:
        #Check if target is a living creature:
        if spellTarget.is_category_type(mc_type_construct) or spellTarget.is_category_type(mc_type_undead):
            spellTarget.float_text_line("Not a living creature", tf_red)
            game.particles('Fizzle', spellTarget)
        else:
            spellTarget.spell_heal(spell.caster, spellDamageDice, D20A_CAST_SPELL, spell.id)
            spellTarget.healsubdual(spell.caster, spellDamageDice, D20A_CAST_SPELL, spell.id)
            game.particles('sp-Cure Light Wounds', spellTarget)
        
    else:
        spellTarget.float_text_line("Not same faith", tf_red)
        game.particles('Fizzle', spellTarget)

    spell.target_list.remove_target(spellTarget)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Faith Healing OnBeginRound"

def OnEndSpellCast(spell):
    print "Faith Healing OnEndSpellCast"