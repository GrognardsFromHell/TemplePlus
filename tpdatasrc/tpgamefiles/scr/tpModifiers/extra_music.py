#Extra Music: SRD

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

print "Registering Extra Music"

def EMNewDay(attachee, args, evt_obj):

	ExtraMusicCount = attachee.has_feat("Extra Music")

	#Extra Music grants 4 additional uses of Bardic Music each time the feat is taken
	MusicCount = args.get_arg(0)
	MusicCount += ExtraMusicCount * 4
	MusicCount += attachee.d20_query("Bardic Music Bonus Levels")
	args.set_arg(0, MusicCount)

	return 0
	
def QueryMaxBardicMusic(attachee, args, evt_obj):

	#Total uses = bard level + extra music count * 4 + Bard Bonus Levels
	MaxMusicCount = attachee.has_feat("Extra Music") * 4
	MaxMusicCount += attachee.stat_level_get(stat_level_bard)
	MaxMusicCount += attachee.d20_query("Bardic Music Bonus Levels")
	evt_obj.return_val = MaxMusicCount
	return 0
	
def QueryCurrentBardicMusic(attachee, args, evt_obj):
	MusicCount = args.get_arg(0)
	evt_obj.return_val = MusicCount
	return 0
	
def DeductBardicMusic(attachee, args, evt_obj):
	MusicCount = args.get_arg(0)
	MusicCount = max(MusicCount -1, 0)
	args.set_arg(0, MusicCount)
	return 0


eSF = PythonModifier()
eSF.ExtendExisting("Bardic Music")
eSF.AddHook(ET_OnNewDay, EK_NEWDAY_REST, EMNewDay, ())
eSF.AddHook(ET_OnD20PythonQuery, "Max Bardic Music", QueryMaxBardicMusic, ())
eSF.AddHook(ET_OnD20PythonQuery, "Current Bardic Music", QueryCurrentBardicMusic, ())
eSF.AddHook(ET_OnD20PythonSignal, "Deduct Bardic Music Charge", DeductBardicMusic, ())
