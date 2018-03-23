#Extra Music: SRD

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Extra Music"

def EMNewDay(attachee, args, evt_obj):

	BardicMusicCount = attachee.has_feat("Extra Music")


	#Extra Music grants 4 additional uses of Bardic Music each time the feat is taken
  #Max music per day remains 3, can't find a way to alter it. Works properly nonetheless.
	args.set_arg(0, args.get_arg(0) + BardicMusicCount * 4)

	return 0


eSF = PythonModifier()
eSF.ExtendExisting("Bardic Music")
eSF.AddHook(ET_OnNewDay, EK_NEWDAY_REST, EMNewDay, ())
