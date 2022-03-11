from toee import *

def OnBeginSpellCast(spell):
    print "Curse of Despair OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Curse of Despair OnSpellEffect"

    spell.duration = 14400 #1 day
    spellTarget = spell.target_list[0]
    curseType = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)
    if curseType not in range(1, 9): #Fallback
        curseType = 8
    curseType -= 1 #Curse type is passed in range 1-8 but condition uses range 0-7
    if curseType in range(0, 6):
        conditionType = "Ability"
    elif curseType == 6:
        conditionType = "Rolls"
    else:
        conditionType = "Actions"

    if spellTarget.obj.saving_throw_spell(spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id): #success
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30001)
        #Even on a successful save a minor debuff will be applied for 1 min
        spell.duration = 10 #1 min
        if spellTarget.obj.condition_add_with_args("sp-Curse of Despair", spell.id, spell.duration, 0):
            spellTarget.partsys_id = game.particles("sp-Bestow Curse", spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
            game.particles("Fizzle", spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
            spell.spell_end(spell.id)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30002)
        if spellTarget.obj.condition_add_with_args("sp-Bestow Curse {}".format(conditionType), spell.id, spell.duration, curseType):
            spellTarget.partsys_id = game.particles("sp-Bestow Curse", spellTarget.obj)
        else:
            spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
            game.particles("Fizzle", spellTarget.obj)
            spell.target_list.remove_target(spellTarget.obj)
            spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Curse of Despair OnBeginRound"

def OnEndSpellCast(spell):
    print "Curse of Despair OnEndSpellCast"

