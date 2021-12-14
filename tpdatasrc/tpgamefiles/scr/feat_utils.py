from templeplus.pymod import PythonModifier
from toee import *
import tpdp

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



