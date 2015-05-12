import sys

def dump_args(*args):
    for i in range(0, len(args)):
        print "Arg %d: %s" % (i, repr(args[i]))

def connect(port=9814):
    sys.path.append('pycharm-debug.egg')

    import pydevd
    import pydevd_file_utils
    pydevd.settrace('localhost', port=port, stdoutToServer=True, stderrToServer=True)
