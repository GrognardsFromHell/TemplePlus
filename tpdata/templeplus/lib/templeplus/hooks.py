from ctypes import *

import _hooks

rebase = _hooks.rebase
replace_func = _hooks.replace_func


def native_func(address, restype, *argtypes):
    """
    Creates a python wrapper for a native function from temple.dll
    :param address: memory address of function assuming non-rebased temple.dll (starting at 0x10000000),
                    will automatically be rebased to the real base address of temple.dll.
    :param restype: result type of the function (check ctypes docs) or None
    :param argtypes: variable number of argument types (check ctypes docs)
    :return: Callable Python object representing the native function
    """
    func_type = CFUNCTYPE(restype, *argtypes)
    return func_type(rebase(address))


