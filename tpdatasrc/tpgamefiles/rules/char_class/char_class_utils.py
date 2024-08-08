# note: disabled class skills (such as swim, ride etc.) will not appear in the game even if listed in the class_skills tuples
from __main__ import game
from toee import *
import char_editor

def IsClassSkill(class_skills, skillEnum):
	for s in class_skills:
		if s == skillEnum:
			return 1
	return 0
	
def IsClassFeat(class_feats, featEnum):
	for l,t in class_feats.items():
		for f in t:
			if (f == featEnum):
				return l
	return 0

def SavingThrowLevel(classEnum, attachee, saveTypeEnum):
	classLvl = attachee.stat_level_get(classEnum)
	isSaveFavored = game.is_save_favored_for_class(classEnum, saveTypeEnum)
	if (isSaveFavored):
		value = 2 + classLvl  / 2 
	else:
		value = (classLvl ) / 3
	return value

def GetHighestBaseClass( attachee ):
	classes = attachee.get_character_base_classes()
	highest_class = stat_level_barbarian
	highest_level = -1
	
	for classEnum in classes:
		level = attachee.stat_level_get(classEnum)
		if level > highest_level:
			highest_level = level
			highest_class = classEnum
	
	return highest_class

def GetHighestArcaneClass( attachee ):
	return attachee.highest_arcane_class
	
def GetHighestSpontaneousArcaneClass( attachee ):
	return attachee.highest_spontaneous_arcane_class
	
def GetHighestVancianArcaneClass( attachee ):
	return attachee.highest_vancian_arcane_class
	

def GetHighestDivineClass( attachee ):
	return attachee.highest_divine_class

def GetBaseCasterLevel(caster_levels, classLvl):
	if classLvl >= len(caster_levels):
		base_cl = caster_levels[-1]
	elif classLvl < 1:
		base_cl = caster_levels[0]
	else:
		base_cl = caster_levels[classLvl]
	return base_cl
	
def GetSpellsKnownAddedCount( spells_known, spellListLvl, spellLvl):
	highestEntry = -1
	newNumKnown = -999
	prevNumKnown = -999
	if (spellListLvl == 1): #no previous number
		vals = spells_known[1]
		if (len(vals) > spellLvl ):
			return vals[spellLvl]
		else:
			return 0
	# print "spellListLvl is " + str(spellListLvl)
	for p,q in spells_known.items():
		# print "iterated level: " + str(p)
		if (p == spellListLvl):
			# print "found it; q is " + str(q)
			if (spellLvl < len(q) ):
				newNumKnown = q[spellLvl]
			else:
				return 0
		elif (p == spellListLvl - 1):
			# print "prev num known levels " + str(q)
			if (spellLvl < len(q) ):
				prevNumKnown = q[spellLvl]
			else:
				prevNumKnown = 0
		if (p > highestEntry and p < spellListLvl):
			highestEntry = p
	# print "newNumKnown is " + str(newNumKnown) + "prevNumKnown is: " + str(prevNumKnown)
	if (newNumKnown == -999):
		if (highestEntry == -1):
			return 0
		vals = spells_known[highestEntry]
		
		if (len(vals) > spellLvl ):
			newNumKnown = vals[spellLvl]
		else:
			return 0
		# get the previous class level's num known
		if (highestEntry > 1):
			vals = spells_known[highestEntry-1]
			if (len(vals) > spellLvl ):
				prevNumKnown = vals[spellLvl]
				return newNumKnown - prevNumKnown
			else:
				return newNumKnown
		else:
			return newNumKnown
	else:
		return newNumKnown - prevNumKnown
	return 0

