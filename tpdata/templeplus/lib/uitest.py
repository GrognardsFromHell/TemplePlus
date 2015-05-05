
from templeplus import ui

def run():
    ctx = ui.mainContext
    doc = ctx.LoadDocument('test.rml')
    doc.Show()

