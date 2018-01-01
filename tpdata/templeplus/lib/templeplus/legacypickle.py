
import pickle

"""
    This subclass is sadly necessary to correctly unpickle pre Python 2.5 argument lists.
    ToEE serializes it's argument list for the obj constuctor as "None", which is invalid
    since Python 2.5. We convert the None into an empty tuple, which is what ToEE actually
    meant to do.
"""
class CustomUnpickler(pickle.Unpickler):
    def __init__(self, file):
        pickle.Unpickler.__init__(self, file)
        self.dispatch[pickle.REDUCE] = CustomUnpickler.load_reduce

    def load_reduce(self):
        print("In load reduce!")
        stack = self.stack
        args = stack.pop()
        func = stack[-1]
        print func, args
        if args:
            value = func(*args)
        else:
            value = func()
        print "Created Value:", value
        stack[-1] = value

def load(file):
    return CustomUnpickler(file).load()

def test(p):
    print "Hello World", p
