from templeplus import ui


def show():
    ctx = ui.mainContext
    doc = ctx.LoadDocument('ui/options.rml')
    doc.Show()
