from toee import *

def OnBeginSpellCast(spell):
    print "Touch of Madness OnBeginSpellCast"
    print "spell.target_list=", spell.target_list
    print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
    game.particles('sp-Touch Of Madness', spell.caster)

def OnSpellEffect(spell):
    print "Touch of Madness OnSpellEffect"

    spell.duration = 1 * spell.caster_level
    spellTarget = spell.target_list[0]

    #Only living creatures are affected
    if spellTarget.obj.is_category_type(mc_type_construct) or spellTarget.obj.is_category_type(mc_type_undead):
        spellTarget.obj.float_mesfile_line('mes\\spell.mes', 32000)
        spell.target_list.remove_target(spellTarget.obj)
    else:
        #Touch of Madness is a touch attack
        attackResult = spell.caster.perform_touch_attack(spellTarget.obj)
        if attackResult & D20CAF_HIT:
            #spell.caster.float_mesfile_line('mes\\combat.mes', 68)
            #Saving Thronw to negate Dazed condition
            saveType = D20_Save_Will
            saveDescriptor = D20STD_F_NONE
            if spellTarget.obj.saving_throw_spell(spell.dc, saveType, saveDescriptor, spell.caster, spell.id): #success
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30001)
                game.particles('Fizzle', spellTarget.obj)
                spell.target_list.remove_target(spellTarget.obj)
            else:
                spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30002)
                if spellTarget.obj.condition_add_with_args('sp-Daze', spell.id, spell.duration, 0):
                    spellTarget.partsys_id = game.particles('sp-Daze Monster', spellTarget.obj)
                else:
                    spellTarget.obj.float_mesfile_line('mes\\spell.mes', 30000)
                    game.particles('Fizzle', spellTarget.obj)
                    spell.target_list.remove_target(spellTarget.obj)
        else:
            spell.caster.float_mesfile_line('mes\\combat.mes', 69)
            spell.target_list.remove_target(spellTarget.obj)

    spell.spell_end(spell.id)

def OnBeginRound(spell):
    print "Touch of Madness OnBeginRound"

def OnEndSpellCast(spell):
    print "Touch of Madness OnEndSpellCast"

