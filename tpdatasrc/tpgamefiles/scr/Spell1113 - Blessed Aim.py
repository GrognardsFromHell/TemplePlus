from toee import *

def OnBeginSpellCast(spell):
    print "Blessed Aim OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Blessed Aim OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 10 * spell.caster_level # 1 min/cl

    for spellTarget in spell.target_list:
        if spellTarget.obj.is_friendly(spell.caster):
            if spellTarget.obj.condition_add_with_args('sp-Blessed Aim', spell.id, spell.duration):
                spellTarget.partsys_id = game.particles('sp-Faerie Fire', spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                game.particles('Fizzle', spellTarget.obj)
                targetsToRemove.append(spellTarget.obj)
        else:
            targetsToRemove.append(spellTarget.obj)

    spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Blessed Aim OnBeginRound"

def OnEndSpellCast(spell):
    print "Blessed Aim OnEndSpellCast"