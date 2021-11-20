from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Stunning Fist Charge Handler"

# Extending Stunning Fist, so new feats can interact with the Stunning Fist Charges

def extraStunningFistCharges(attachee, args, evt_obj):
    print "extraStunningFistCharges Hook"
    additionalStunningCharges = attachee.d20_query("PQ_Get_Extra_Stunning_Fist_Charges")
    print "additionalStunningCharges: {}".format(additionalStunningCharges)
    args.set_arg(0, args.get_arg(0) + additionalStunningCharges)
    print "total charges for today: {}".format(args.get_arg(0))
    return 0

def deductStunningFistCharge(attachee, args, evt_obj):
    chargesToDeduct = evt_obj.data1
    chargesLeft = args.get_arg(0)
    if chargesLeft:
        chargesLeft -= chargesToDeduct
        args.set_arg(0, chargesLeft)
    return 0

def getStunningFistCharges(attachee, args, evt_obj):
    chargesLeft = args.get_arg(0)
    evt_obj.return_val = chargesLeft
    return 0

def stunningFistTooltip(attachee, args, evt_obj):
    #Adding a tooltip, so you can see how many charges are left
    chargesLeft = args.get_arg(0)
    evt_obj.append("Stunning Fist uses left: {}".format(chargesLeft))
    return 0

stunningFistExtend = PythonModifier()
stunningFistExtend.ExtendExisting("feat_stunning_fist")
stunningFistExtend.MapToFeat(feat_stunning_fist)
stunningFistExtend.AddHook(ET_OnNewDay, EK_NEWDAY_REST, extraStunningFistCharges, ())
stunningFistExtend.AddHook(ET_OnD20PythonSignal, "PS_Deduct_Stunning_Fist_Charge", deductStunningFistCharge, ())
stunningFistExtend.AddHook(ET_OnD20PythonQuery, "PQ_Get_Stunning_Fist_Charges", getStunningFistCharges, ())
stunningFistExtend.AddHook(ET_OnGetTooltip, EK_NONE, stunningFistTooltip, ())
