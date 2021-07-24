from toee import *

def OnBeginSpellCast(spell):
    print "Blessing of Bahamut OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Blessing of Bahamut OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 round/CL
    spellTarget = spell.target_list[0]

    if spellTarget.obj.condition_add_with_args('sp-Blessing of Bahamut', spell.id, spell.duration):
        spellTarget.partsys_id = game.particles('sp-Shield of Faith', spellTarget.obj)
    else:
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
        game.particles('Fizzle', spellTarget.obj)
        spell.target_list.remove_target(spellTarget.obj)
        spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Blessing of Bahamut OnBeginRound"

def OnEndSpellCast(spell):
    print "Blessing of Bahamut OnEndSpellCast"