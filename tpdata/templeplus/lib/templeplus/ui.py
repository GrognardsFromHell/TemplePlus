from __future__ import absolute_import
from _rocketcore import *
from _rocketcontrols import *
from templeplus import add_font

mainContext = contexts['main']
initialized = False

def init():
    global initialized
    loadFonts()
    hookFunctions()
    initialized = True


def loadFonts():
    print "Registering fonts"

    # Alegreya
    add_font("/fonts/Alegreya-Regular.otf", "Alegreya")
    add_font("/fonts/Alegreya-Bold.otf", "Alegreya", bold=True)
    add_font("/fonts/Alegreya-Italic.otf", "Alegreya", italic=True)
    add_font("/fonts/Alegreya-BoldItalic.otf", "Alegreya", bold=True, italic=True)

    # Alegreya Small Caps
    add_font("/fonts/AlegreyaSC-Regular.otf", "AlegreyaSC")
    add_font("/fonts/AlegreyaSC-Bold.otf", "AlegreyaSC", bold=True)
    add_font("/fonts/AlegreyaSC-Italic.otf", "AlegreyaSC", italic=True)
    add_font("/fonts/AlegreyaSC-BoldItalic.otf", "AlegreyaSC", bold=True, italic=True)

    # Open Sans
    add_font("/fonts/OpenSans-Regular.ttf", "OpenSans")
    add_font("/fonts/OpenSans-Bold.ttf", "OpenSans", bold=True)
    add_font("/fonts/OpenSans-Italic.ttf", "OpenSans", italic=True)

    # Source Code Pro
    add_font("/fonts/SourceCodePro-Regular.otf", "SourceCodePro")
    add_font("/fonts/SourceCodePro-Bold.otf", "SourceCodePro", bold=True)

def hookFunctions():
    import ui.options

    ui.options.init()

    import ui.console
    ui.console.install_hooks()


def load_doc(docUrl):
    doc = mainContext.LoadDocument(docUrl)
    return doc
