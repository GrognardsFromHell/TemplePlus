from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Lingering Song, Song and Silence: p. 40

print "Registering Lingering Song"
	
def QueryMaxBardicMusicExtraRounds(attachee, args, evt_obj):

    #5 Extra Rounds from this feat
	evt_obj.return_val += 5
	return 0

LingeringSong = PythonModifier("Lingering Song", 2) #Extra, Extra
LingeringSong.MapToFeat("Lingering Song")
LingeringSong.AddHook(ET_OnD20PythonQuery, "Bardic Ability Duration Bonus", QueryMaxBardicMusicExtraRounds, ())
