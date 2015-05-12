import templeplus
from toee import game

def create_btn(document, name, map, x, y):
    btn = document.CreateElement("button")
    btn.AppendChild(document.CreateTextNode(name))

    def goto_point():
        if not game.leader:
            return
        print "Going to point", name
        game.fade_and_teleport(0, 0, 0, int(map), int(x), int(y))

    btn.AddEventListener("click", goto_point)
    return btn


def loadPoints(document):
    list = document.GetElementById("list")
    list.inner_rml = ''

    points = templeplus.tio_read('rules/jumppoint.tab')
    lines = points.split("\n")
    for line in lines:
        if line.strip() == '':
            continue
        (id, name, map, x, y) = line.split("\t")
        name = name.replace("-", " ")
        btn = create_btn(document, name, map, x, y)
        list.AppendChild(btn)
