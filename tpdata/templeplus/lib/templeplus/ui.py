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
    "AlegreyaSC-Regular.otf"
]


def loadFonts():
    print "Registering fonts"

    for font in fonts:
        print "Loading font", font
        LoadFontFace("/fonts/" + font)

print str(loadFonts)