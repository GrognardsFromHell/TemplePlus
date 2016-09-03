import tpdp

class PythonModifier(tpdp.ModifierSpec):
    def AddHook(self, eventType, eventKey, callShit, argsTuple ):
        self.add_hook(eventType, eventKey, callShit, argsTuple)
    def ExtendExisting(self, condName):
        self.extend_existing(condName)
    def AddItemForceRemoveHandler(self): # in charge of backing up condition args
        self.add_item_force_remove_callback()
    def MapToFeat(self, feat_enum, feat_list_max = -1, feat_cond_arg2 = 0):
        self.add_to_feat_dict(feat_enum, feat_list_max, feat_cond_arg2)