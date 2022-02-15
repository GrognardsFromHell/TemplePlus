from toee import *

def OnBeginSpellCast(spell):
    print "Wounding Whispers OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    #game.particles( "sp-divination-conjure", spell.caster )

def OnSpellEffect(spell):
    print "Wounding Whispers OnSpellEffect"

    spell.duration = 1 * spell.caster_level # 1 rnd/cl
    spellTarget = spell.target_list[0]

    spellTarget.obj.condition_add_with_args('sp-Wounding Whispers', spell.id, spell.duration)
    spellTarget.partsys_id = game.particles('sp-True Strike', spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Wounding Whispers OnBeginRound"

def OnEndSpellCast(spell):
    print "Wounding Whispers OnEndSpellCast"