from _rocketcore import *
from _rocketcontrols import *
from _hooks import replace_func
from ctypes import *

mainContext = contexts['main']

fonts = [
    "Alegreya-Regular.otf",
    "Alegreya-Bold.otf",
    "Alegreya-Italic.otf",
    "Alegreya-BoldItalic.otf",
    "AlegreyaSC-Bold.otf",
    "AlegreyaSC-BoldItalic.otf",
    "AlegreyaSC-Italic.otf",
    "AlegreyaSC-Regular.otf"
]

def init():
    loadFonts()
    hookFunctions()

def loadFonts():
    print "Registering fonts"

    for font in fonts:
        print "Loading font", font
        LoadFontFace("/fonts/" + font)

showOptionsOrg = None

def showOptions(unk):
    print "Showing options", unk
    showOptionsOrg(unk)

def hookFunctions():
    FUNC = CFUNCTYPE(None, c_int)
    global showOptionsOrg
    showOptionsOrg = replace_func(0x10119D20, FUNC(showOptions))
