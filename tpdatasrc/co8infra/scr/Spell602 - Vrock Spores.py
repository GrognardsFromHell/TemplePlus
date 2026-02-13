from toee import *

debug = False
def Debug(loc, *items):
	if not debug: return

	print "Vrock Spores ", loc
	for item in items:
		print item

def OnBeginSpellCast(spell):
	tl = "spell.target_list={}".format(spell.target_list)
	ca = "spell.caster={} caster.level={}".format(spell.caster, spell.caster_level)
	Debug("OnBeginSpellCast", tl, ca)

def	OnSpellEffect(spell):
	Debug("OnSpellEffect")

	vrock = spell.caster
	remove_list = []

	duration = 10
	damage_dice = dice_new( '1d8' )
	game.particles( 'Mon-Vrock-Spores', spell.caster )

	for target_item in spell.target_list:
		target = target_item.obj
		remove_list.append(target)

		if not target.is_category_subtype(mc_subtype_demon):
			# damage 1-8 (no save)
			target.damage(vrock, D20DT_POISON, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL)

			# add SPORE condition
			partsys_id = game.particles('Mon-Vrock-Spores-Hit', target)
			target.condition_add_with_args('sp-Vrock Spores', -1, duration, partsys_id)

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

def OnBeginRound(spell):
	Debug("OnBeginRound")

def OnEndSpellCast(spell):
	Debug("OnEndSpellCast")
