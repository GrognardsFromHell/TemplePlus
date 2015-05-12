from __future__ import absolute_import
from _rocketcore import *
from _rocketcontrols import *

mainContext = contexts['main']

fonts = [
    "Alegreya-Regular.otf",
    "Alegreya-Bold.otf",
    "Alegreya-Italic.otf",
    "Alegreya-BoldItalic.otf",
    "AlegreyaSC-Bold.otf",
    "AlegreyaSC-BoldItalic.otf",
    "AlegreyaSC-Italic.otf",
    "AlegreyaSC-Regular.otf",
    "OpenSans-Regular.ttf",
    "OpenSans-Bold.ttf",
    "OpenSans-Italic.ttf"
]

def init():
    loadFonts()
    hookFunctions()

def loadFonts():
    print "Registering fonts"

    for font in fonts:
        print "Loading font", font
        LoadFontFace("/fonts/" + font)

def hookFunctions():
    import ui.options
    ui.options.init()

def load_doc(docUrl):
    doc = mainContext.LoadDocument(docUrl)
    return doc
