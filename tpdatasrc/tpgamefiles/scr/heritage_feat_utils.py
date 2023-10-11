from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# This file handles heritage feats

def heritageTypeList():
    typeList = [
    "Celestial Sorcerer Heritage",
    "Draconic Heritage Black",
    "Draconic Heritage Blue",
    "Draconic Heritage Brass",
    "Draconic Heritage Bronze",
    "Draconic Heritage Copper",
    "Draconic Heritage Gold",
    "Draconic Heritage Green",
    "Draconic Heritage Red",
    "Draconic Heritage Silver",
    "Draconic Heritage White",
    "Fey Heritage",
    "Fiendish Heritage",
    "Infernal Sorcerer Heritage"
    ]
    return typeList

# PHB II
def getCelestialSorcererHeritageFeatList():
    featList = [
    "Celestial Sorcerer Aura",
    "Celestial Sorcerer Heritage",
    "Celestial Sorcerer Lance",
    "Celestial Sorcerer Lore",
    "Celestial Sorcerer Wings"
    ]
    return featList

# Draconic Feat List
# Note: Draconic Aura and Double Aura are not part of the draconic feats!
# Missing: Complete Arcane: Claw, Flight, Legacy(maybe a skip), Power, Presence
# Missing Dragon Magic: [Colour] Dragon Lineage (started), Draconic Knowledge (skip as there are no knowledge skills?), Draconic Senses
# Missing Dragon Magic Multiclass options: Dragonfire Assault (Power Attack), Dragonfire Channeling (Turn/Rebuke),
# Dragonfire Inspiration (Bardic Music), Dragonfire Strike (Sneak/Skirmish/Sudden Strike)
# Missing Races of the Dragon: Persuasion
def getDraconicHeritageFeatList():
    featList = [
    "Draconic Arcane Grace",
    "Draconic Armor",
    "Draconic Breath",
    "Draconic Claw ",
    "Draconic Flight"
    "Draconic Heritage Black",
    "Draconic Heritage Blue",
    "Draconic Heritage Brass",
    "Draconic Heritage Bronze",
    "Draconic Heritage Copper",
    "Draconic Heritage Gold",
    "Draconic Heritage Green",
    "Draconic Heritage Red",
    "Draconic Heritage Silver",
    "Draconic Heritage White",
    "Draconic Knowledge",
    "Draconic Legacy",
    "Draconic Persuasion",
    "Draconic Power",
    "Draconic Presence",
    "Draconic Resistance",
    "Draconic Senses",
    "Draconic Skin",
    "Draconic Toughness",
    "Draconic Vigor",
    "Dragon Lineage Black",
    "Dragon Lineage Blue",
    "Dragon Lineage Brass",
    "Dragon Lineage Bronze",
    "Dragon Lineage Copper",
    "Dragon Lineage Gold",
    "Dragon Lineage Green",
    "Dragon Lineage Red",
    "Dragon Lineage Silver",
    "Dragon Lineage White",
    "Dragonfire Assault",
    "Dragonfire Channeling",
    "Dragonfire Inspiration",
    "Dragonfire Strike"
    ]
    return featList

# Complete Mage
def getFeyHeritageFeatList():
    featList = [
    "Fey Heritage",
    "Fey Legacy",
    "Fey Power",
    "Fey Presence",
    "Fey Skin"
    ]
    return featList

# Complete Mage
def getFiendishHeritageFeatList():
    featList = [
    "Fiendish Heritage",
    "Fiendish Legacy",
    "Fiendish Power",
    "Fiendish Presence",
    "Fiendish Resistance"
    ]
    return featList

# PHB II
def getInfernalSorcererHeritageFeatList():
    featList = [
    "Infernal Sorcerer Eyes",
    "Infernal Sorcerer Heritage",
    "Infernal Sorcerer Howl",
    "Infernal Sorcerer Resistance",
    ]
    return featList

def hasDifferentHeritageFeat(attachee, featToAquire):
    for heritageFeat in heritageTypeList():
        if not featToAquire == heritageFeat:
            if attachee.has_feat(heritageFeat):
                return True
    return False

# Counts the number of heritage feats a character has
# Used by several feats
def countHeritageFeats(attachee, heritageType):
    if heritageType == heritage_celestial_sorcerer:
        featList = getCelestialSorcererHeritageFeatList()
    elif heritageType in range(heritage_draconic_black, heritage_draconic_white + 1):
        featList = getDraconicHeritageFeatList()
    elif heritageType == heritage_fey:
        featList = getFeyHeritageFeatList()
    elif heritageType == heritage_fiendish:
        featList = getFiendishHeritageFeatList()
    elif heritageType == heritage_infernal_sorcerer:
        featList = getInfernalSorcererHeritageFeatList()
    numberOfFeats = 0
    for feat in featList:
        if attachee.has_feat(feat):
            numberOfFeats += 1
    return numberOfFeats

# Draconic heritage mappings
# [colourString, elementType, breathWeaponShape]

def getDictDraconicHeritage():
    dictDraconicHeritage = {
    heritage_draconic_black: ["Black", D20DT_ACID, dragon_breath_shape_line],
    heritage_draconic_blue: ["Blue", D20DT_ELECTRICITY, dragon_breath_shape_line],
    heritage_draconic_brass: ["Brass", D20DT_FIRE, dragon_breath_shape_line],
    heritage_draconic_bronze: ["Bronze", D20DT_ELECTRICITY, dragon_breath_shape_line],
    heritage_draconic_copper: ["Copper", D20DT_ACID, dragon_breath_shape_line],
    heritage_draconic_gold: ["Gold", D20DT_FIRE, dragon_breath_shape_cone],
    heritage_draconic_green: ["Green", D20DT_ACID, dragon_breath_shape_cone],
    heritage_draconic_red: ["Red", D20DT_FIRE, dragon_breath_shape_cone],
    heritage_draconic_silver: ["Silver", D20DT_COLD, dragon_breath_shape_cone],
    heritage_draconic_white: ["White", D20DT_COLD, dragon_breath_shape_cone]
    }
    return dictDraconicHeritage

def getDraconicHeritageColourString(heritage):
    dictDraconicHeritage = getDictDraconicHeritage()
    return dictDraconicHeritage[heritage][0]

def getDraconicHeritageElement(heritage):
    dictDraconicHeritage = getDictDraconicHeritage()
    return dictDraconicHeritage[heritage][1]

def getDraconicHeritageBreathShape(heritage):
    dictDraconicHeritage = getDictDraconicHeritage()
    return dictDraconicHeritage[heritage][2]
