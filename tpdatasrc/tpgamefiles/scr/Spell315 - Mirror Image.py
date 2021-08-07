import tpdp
from toee import *

def OnBeginSpellCast( spell ):
	print "Mirror Image OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-illusion-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Mirror Image OnSpellEffect"

	spell.duration = 10 * spell.caster_level
	target_item = spell.target_list[0]
	
	bonus = 1 + (spell.caster_level / 3)
	num_of_images = spell.roll_dice_with_metamagic(1, 4, bonus)

	spellPkt = tpdp.SpellPacket(spell.id)
	mmData = spellPkt.get_metamagic_data()
	
	#Maximum of 8 images from the spell description (empower to 12)
	if mmData.get_empower_count() > 0:
		ImageCap = 12
	else:
		ImageCap = 8

	num_of_images = min(ImageCap, num_of_images)

	print "num of images=", num_of_images, "bonus=", bonus

	game.particles( 'sp-Mirror Image', target_item.obj )
	target_item.obj.condition_add_with_args( 'sp-Mirror Image', spell.id, spell.duration, num_of_images )
	#target_item.partsys_id = game.particles( 'sp-Mirror Image', target_item.obj )

def OnBeginRound( spell ):
	print "Mirror Image OnBeginRound"

def OnEndSpellCast( spell ):
	print "Mirror Image OnEndSpellCast"