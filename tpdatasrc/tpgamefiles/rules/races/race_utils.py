from __main__ import game
from toee import *
import char_editor

def SavingThrowLevel(classEnum, attachee, saveTypeEnum):
	classLvl = attachee.stat_level_get(classEnum)
	isSaveFavored = game.is_save_favored_for_class(classEnum, saveTypeEnum)
	if (isSaveFavored):
		value = 2 + classLvl  / 2 
	else:
		value = (classLvl ) / 3
	return value


