
import ctypes

def replace_func(address, replacement):
    """
    Replaces the native function starting at address with the given Python function.

    :type address: int
    :param address: The memory address to replace (will automatically be rebased to temple.dll memory space)
    :param replacement: A ctype function pointer that will be used in place of the original function.
    :return A Python callable representing the original function.
    """
    pass


def rebase(address):
    """
    Rebases an address from the original temple.dll image base to the actual temple.dll image base.
    :type address: int
    :param address: The address to rebase.
    """
    pass
