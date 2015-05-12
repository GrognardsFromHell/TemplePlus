from templeplus.ui import load_doc

debug_doc = None


def show():
    global debug_doc
    if debug_doc:
        debug_doc.Close()
        debug_doc = None
        return
    show_url("debug.rml")

def show_url(url):
    global debug_doc
    if debug_doc:
        debug_doc.Close()
        debug_doc = None

    debug_doc = load_doc("ui/debug/" + url)
    debug_doc.Show()

