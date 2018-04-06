#Extra Music: SRD

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Music"

def EMNewDay(attachee, args, evt_obj):

	BardicMusicCount = attachee.has_feat("Extra Music")

	#Extra Music grants 4 additional uses of Bardic Music each time the feat is taken
	args.set_arg(0, args.get_arg(0) + BardicMusicCount * 4)

	return 0
	
def QueryMaxBardicMusic(attachee, args, evt_obj):

    #Total uses = bard level + extra music count * 4
	MaxMusicCount = attachee.has_feat("Extra Music") * 4
	MaxMusicCount += attachee.stat_level_get(stat_level_bard)
	evt_obj.return_val = MaxMusicCount
	return 0

eSF = PythonModifier()
eSF.ExtendExisting("Bardic Music")
eSF.AddHook(ET_OnNewDay, EK_NEWDAY_REST, EMNewDay, ())
eSF.AddHook(ET_OnD20PythonQuery, "Max Bardic Music", QueryMaxBardicMusic, ())
