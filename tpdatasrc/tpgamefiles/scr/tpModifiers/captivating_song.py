from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# A special removal function beyond the spell_utils one because this
# condition is a bit more complicated.
def CountersongSpecial(char, args, evt_obj):
	if not evt_obj.is_modifier('Countersong'): return 0
	spell_id = args.get_arg(0)
	packet = tpdp.SpellPacket(spell_id)

	if packet.dc <= evt_obj.arg2:
		char.d20_send_signal('End Captivating Song')
		args.remove_spell()
		args.remove_spell_mod()

	return 0

csong = PythonModifier()
csong.ExtendExisting('Captivating Song')
csong.AddHook(ET_OnConditionAddPre, EK_NONE, CountersongSpecial, ())

# End Captivated condition when Captivating Song is ended in the above
# way.
def End(char, args, evt_obj):
	args.condition_remove()
	return 0

capt = PythonModifier()
capt.ExtendExisting('Captivated')
capt.AddHook(ET_OnD20PythonSignal, 'End Captivating Song', End, ())
