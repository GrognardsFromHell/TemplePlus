from toee import *
from spell_utils import checkCategoryType

def OnBeginSpellCast(spell):
    print "Revivify OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Revivify OnSpellEffect"

    #https://github.com/GrognardsFromHell/TemplePlus/blob/35e50618c87d8f9a22bbcfc2a6d18e8c0a81801b/TemplePlus/critter.h#L32

    spell.duration = 0
    spellTarget = spell.target_list[0]

    if not spellTarget.obj.d20_query(Q_Dead):
        spell.caster.float_mesfile_line("mes\\spell.mes", 30016) #ID 30016 = Target is not dead!
    #Constructs, elementals, outsiders, and undead can't be raised
    elif checkCategoryType(spellTarget.obj, mc_type_construct, mc_type_elemental, mc_type_outsider, mc_type_undead):
        spellTarget.obj.float_text_line("Cannot be raised!")
    elif not spellTarget.obj.d20_query("PQ_Recently_Deceased"):
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 20036) #ID 20036 = Resurrection failed!
    else:
        spellTarget.obj.resurrect(3)
        game.particles("sp-Raise Dead", spellTarget.obj)
        spellTarget.obj.float_mesfile_line("mes\\spell.mes", 20037) #ID 20037 = Resurrection successful!
        spellTarget.obj.condition_add_with_args("Unconscious")
        spellTarget.obj.fall_down()
        ### Workaround to set life to -1 ###
        #python_obj.resurrect sets HP to HitDice number
        maxHP = spellTarget.obj.stat_level_get(stat_hp_max)
        spellTarget.obj.set_hp_damage(maxHP + 1)
        ### Workaround End ###

    spell.target_list.remove_target(spellTarget.obj)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Revivify OnBeginRound"

def OnEndSpellCast(spell):
    print "Revivify OnEndSpellCast"

