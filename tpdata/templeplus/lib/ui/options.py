from templeplus import ui
from templeplus.hooks import *

hideUtilBarWindows = None
hideUtilBar = None
showOptionsOrg = None

def showOptions(fromMainMenu):
    print "Showing options", fromMainMenu
    # showOptionsOrg(fromMainMenu)
    hideUtilBarWindows(1)
    hideUtilBar()

def init():
    print "Initializing"
    FUNC = CFUNCTYPE(None, c_int)
    global showOptionsOrg
    showOptionsOrg = replace_func(0x10119D20, FUNC(showOptions))

    global hideUtilBarWindows
    hideUtilBarWindows = native_func(0x101156B0, None, c_int)

    global hideUtilBar
    hideUtilBar = native_func(0x1010EEC0, None)

def show():
    ctx = ui.mainContext
    doc = ctx.LoadDocument('ui/options.rml')
    doc.Show()
