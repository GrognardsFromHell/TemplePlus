from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Spike Stones Extender"

spikeStonesExtender = PythonModifier()
spikeStonesExtender.ExtendExisting("sp-Spike Stones")
spikeStonesExtender.AddSpellDismissStandardHook() #Should be dismissable like spike stones



