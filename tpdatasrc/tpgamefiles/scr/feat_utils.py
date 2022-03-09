from toee import *
import tpdp
from spell_utils import getBonusHelpTag

#This file contains common functions for feats

#query return 1
def queryReturnOne(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

#get ability modifier
def getAbilityModifier(attachee, ability):
    return (attachee.stat_level_get(ability) - 10) / 2

def getFeatName(args):
    return args.get_cond_name()

def getFeatTag(featName):
    return "TAG_{}".format(featName.upper().replace(" ", "_"))

def getFeatHelpTag(featName):
    featTag = getFeatTag(featName)
    return "~{}~[{}]".format(featName, featTag)

def applyFeatBonus(attachee, args, evt_obj):
    bonusValue = args.get_param(0)
    if not bonusValue:
        bonusValue = args.get_arg(2)
    bonusType = args.get_param(1)
    featName = getFeatName(args)
    featHelpTag = getFeatHelpTag(featName)
    evt_obj.bonus_list.add(bonusValue, bonusType, "{} : {}".format(bonusHelpTag, featHelpTag))
    return 0

