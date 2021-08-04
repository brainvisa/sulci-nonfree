from __future__ import print_function

# ------------------------------------------------------------------------------
# identity func & Identity class: do nothing tools


def identity(*args, **kwargs):
    """
    identity function return its arguments
    """
    ret = list(args) + list(kwargs.values())
    if len(ret) == 1:
        return ret[0]
    return ret


class Identity(object):

    """
    identity class : all method call return its arguments
    """

    def __getattr__(self, attr):
        return identity

if __name__ == "__main__":
    ide = Identity()
    print(ide.titi(1))
    print(ide.tutu(1, 2))
