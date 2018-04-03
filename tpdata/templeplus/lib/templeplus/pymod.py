import tpdp

class PythonModifier(tpdp.ModifierSpec):
    def AddHook(self, eventType, eventKey, callbackFcn, argsTuple ):
        self.add_hook(eventType, eventKey, callbackFcn, argsTuple)
    def ExtendExisting(self, condName):
        self.extend_existing(condName)
    def AddItemForceRemoveHandler(self): # in charge of backing up condition args
        self.add_item_force_remove_callback()
    def MapToFeat(self, feat_enum, feat_list_max = -1, feat_cond_arg2 = 0):
        self.add_to_feat_dict(feat_enum, feat_list_max, feat_cond_arg2)
    def AddSpellCountdownStandardHook(self):
        self.add_spell_countdown_standard()    
    def AddAoESpellEndStandardHook(self):
        self.add_aoe_spell_ender()