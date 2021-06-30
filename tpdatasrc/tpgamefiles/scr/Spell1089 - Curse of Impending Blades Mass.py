from toee import *

def OnBeginSpellCast(spell):
    print "Curse of Impending Blades Mass OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect(spell):
    print "Curse of Impending Blades Mass OnSpellEffect"
    
    targetsToRemove = []
    spell.duration = 10 * spell.caster_level # 1 min/cl

    for spellTarget in spell.target_list:
        targetIsFriendly = spellTarget.obj.is_friendly(spell.caster)
        if targetIsFriendly: # Curse only affects enemies
            targetsToRemove.append(spellTarget.obj)
        else: #Curse has no saving throw
            if spellTarget.obj.condition_add_with_args('sp-Curse of Impending Blades', spell.id, spell.duration):
                spellTarget.obj.float_text_line("Curse of Impending Blades", tf_red)
                game.create_history_freeform("{} is affected by ~Curse of Impending Blades~[TAG_SPELLS_CURSE_OF_IMPENDING_BLADES]\n\n".format(spellTarget.obj.description))
                spellTarget.partsys_id = game.particles('sp-Bestow Curse', spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                game.particles('Fizzle', spellTarget.obj)
                targetsToRemove.append(spellTarget.obj)

    if targetsToRemove:
        spell.target_list.remove_list(targetsToRemove)
    spell.spell_end(spell.id)

    
def OnBeginRound(spell):
    print "Curse of Impending Blades Mass OnBeginRound"

def OnEndSpellCast(spell):
    print "Curse of Impending Blades Mass OnEndSpellCast"