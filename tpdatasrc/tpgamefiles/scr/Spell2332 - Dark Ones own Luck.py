from toee import *

def OnBeginSpellCast(spell):
    print "Dark One's Own Luck OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Dark One's Own Luck OnSpellEffect"

    spell.duration = 14400 #1 day
    spellTarget = spell.target_list[0]
    bonusValue = (spellTarget.obj.stat_level_get(stat_charisma) - 10) / 2
    saveType = spell.spell_get_menu_arg(RADIAL_MENU_PARAM_MIN_SETTING)

    if saveType == 1 or saveType not in range(1, 4): #not in range is fallback
        saveTypeLabel = "Fortitude"
    elif saveType == 2:
        saveTypeLabel = "Reflex"
    elif saveType == 3:
        saveTypeLabel = "Will"

    if spellTarget.obj.condition_add_with_args("sp-Dark One's Own Luck {}".format(saveTypeLabel), spell.id, spell.duration, bonusValue, 0):
        spellTarget.partsys_id = game.particles("Dark One's Own Luck", spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 30000)
        game.particles("Fizzle", spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Dark One's Own Luck OnBeginRound"

def OnEndSpellCast(spell):
    print "Dark One's Own Luck OnEndSpellCast"

