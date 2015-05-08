
def accept():
    pass

def decline():
    pass

#
# librocket does not support inline blocks with width: auto
# so we emulate that by hand here
#
def resizetabs(doc):
    for tab in doc.GetElementsByTagName("tab"):
        spans = tab.GetElementsByTagName("span")
        if len(spans) == 1:
            w = int(spans[0].client_width)
            tab.style.width = "%dpx" % (w, )
