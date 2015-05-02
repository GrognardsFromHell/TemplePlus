
import pickle

"""
    This subclass is sadly necessary to correctly unpickle pre Python 2.5 argument lists.
    ToEE serializes it's argument list for the obj constuctor as "None", which is invalid
    since Python 2.5. We convert the None into an empty tumple, which is what ToEE actually
    meant to do.
"""
class CustomUnpickler(pickle.Unpickler):
    def __init__(self, file):
        pickle.Unpickler.__init__(self, file)
        self.dispatch[pickle.REDUCE] = CustomUnpickler.load_reduce

    def load_reduce(self):
        stack = self.stack
        args = stack.pop()
        func = stack[-1]
        if args:
            value = func(*args)
        else:
            value = func()
        stack[-1] = value

def load(file):
    return CustomUnpickler(file).load()
