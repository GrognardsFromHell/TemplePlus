class PythonModifier:
    def __init__(self, name, numArgs, preventDup = true):
        self.__register_mod__(self, name, numArgs, preventDup) # implemented in C
        if (preventDup):
        self.__add_hook_prevent_dup() # this is common enough to do in the C as a speical case...
    def AddHook(self, eventType, eventKey, callback):
        self.__add_hook__(self, eventType, eventKey, callback)