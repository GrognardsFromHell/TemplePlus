from templeplus.pymod import PythonModifier
from toee import *
import tpdp


def DetGoodRadial(attachee, args, evt_obj):

	return 0

def DetGoodActionFrame(attachee, args, evt_obj):
	return 0

detGoodEnum = 2200
detGood = PythonModifier("Feat Detect Good", 0)
detGood.MapToFeat("Detect Good")
detGood.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, DetGoodRadial, ())
detGood.AddHook(ET_OnD20PythonActionFrame, detGoodEnum , DetGoodActionFrame, ())
