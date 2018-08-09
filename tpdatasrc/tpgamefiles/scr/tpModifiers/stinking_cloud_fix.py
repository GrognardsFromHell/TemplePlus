#Fix for bug 184  Stinking Cloud: Nauseated creatures still make Attacks of opportunity but not normal attacks.

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Stinking Cloud AOO Fix"

def StinkingCloudEffectAOOPossible(attachee, args, evt_obj):
	# No making AOOs when Nauseated
	evt_obj.return_val = 0
	return 0

stinkingCloudEffect = PythonModifier()
stinkingCloudEffect.ExtendExisting("sp-Stinking Cloud Hit")
stinkingCloudEffect.AddHook(ET_OnD20Query, EK_Q_AOOPossible, StinkingCloudEffectAOOPossible, ())
