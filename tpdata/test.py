
xd = 0
yd = 0

def onDragStart(event):
    global xd, yd
    elem = event.target_element
    xd = elem.absolute_left - event.parameters["mouse_x"]
    yd = elem.absolute_top - event.parameters["mouse_y"]

def onDrag(event):
    elem = event.target_element
    x = event.parameters["mouse_x"]
    y = event.parameters["mouse_y"]
    elem.style.left = "%dpx" % (x + xd)
    elem.style.top = "%dpx" % (y + yd)

def onDragEnd(event):
    global xd, yd
    x = event.parameters["mouse_x"]
    y = event.parameters["mouse_y"]
    print "End @  ", x, y
