from templeplus.ui import load_doc, key_identifier
import templeplus.ui
from templeplus.hooks import replace_func, CFUNCTYPE

doc = None
el_output = None
el_input = None
last_line = None
history = []
history_pos = 0
eval_locals = dict()

def focus_input():
    el_input.Focus()


def input_keydown():
    global event, doc, history_pos
    key = key_identifier(event.parameters["key_identifier"])
    if key == key_identifier.ESCAPE:
        doc.Hide()
    elif key == key_identifier.RETURN:
        text = el_input.value
        history.append(text)
        history_pos = len(history)
        el_input.value = ''
        code = compile(text, '<console>', 'single')
        eval(code, globals(), eval_locals)
    elif key == key_identifier.UP:
        print history_pos, history
        if 0 <= history_pos - 1 < len(history):
            el_input.value = history[history_pos - 1]
            del history[history_pos - 1]
            history_pos = len(history)
    elif key == key_identifier.DOWN:
        pass


def init():
    global doc, el_input, el_output
    if doc:
        return
    doc = load_doc("ui/console.rml")
    el_input = doc.GetElementById("input")
    el_output = doc.GetElementById("output")

    # doc.AddEventListener("mousedown", focus_input)
    el_input.AddEventListener("keydown", input_keydown)


def show():
    init()
    doc.Show()


def append(text):
    # we may be called before the UI has loaded fonts, etc.
    # i those cases we drop the messages
    if not templeplus.ui.initialized:
        return

    init()
    global last_line, doc, el_output
    if not last_line:
        last_line = doc.CreateTextNode("")
        el_output.AppendChild(last_line)
    last_line.text += text

    # Scroll to the end of the page when a new line is inserted
    el_output.scroll_top = el_output.scroll_height - el_output.client_height
    return

    # Investigate if this is faster since it has to re-measure less text
    lines = text.splitlines(True)
    for line in lines:
        last_line.text += line

        if line.endswith("\n"):
            last_line = doc.CreateTextNode("")
            el_output.AppendChild(last_line)

def install_hooks():
    voidfunc = CFUNCTYPE(None)
    replace_func(0x101DF7C0, voidfunc(show))
