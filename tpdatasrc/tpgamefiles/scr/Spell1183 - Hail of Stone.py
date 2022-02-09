from toee import *

def OnBeginSpellCast(spell):
    print "Hail of Stone OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Hail of Stone OnSpellEffect"

    targetsToRemove = []
    spell.duration = 0
    spellDamageDice = dice_new('1d4')
    spellDamageDice.number = min(spell.caster_level, 5)
    damageType = D20DT_BLUDGEONING

    game.particles('sp-Hail of Stone', spell.target_loc)

    for spellTarget in spell.target_list:
        #Hail of Stone has no save
        spellTarget.obj.spell_damage(spell.caster, damageType, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
        targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Hail of Stone OnBeginRound"

def OnEndSpellCast(spell):
    print "Hail of Stone OnEndSpellCast"

