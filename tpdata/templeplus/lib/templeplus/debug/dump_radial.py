import debug
import json

# Needed to serialize obj handles
class TempleJson(json.JSONEncoder):
    def default(self, obj):
        return repr(obj)


def run():
    radials = debug.dump_radial()

    # replace children with real children
    for radial in radials:
        for node in radial["nodes"]:
            realChildren = []
            for child in node["children"]:
                realChildren.append(radial["nodes"][child])
            node["children"] = realChildren
        radial["nodes"] = filter(lambda x: x["parent"] == -1, radial["nodes"])

    with open("radials.json", "wt") as fh:
        json.dump(radials, fh, cls=TempleJson)

