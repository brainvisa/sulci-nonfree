from __future__ import absolute_import
import numpy as N

"""
Just a basic function of X and Y (function for test)
func = cst_X * N.sum(X)  + cst_Y * N.sum(Y)
"""


class Testfunction(object):

    def __init__(self):
        self.cst_X = 0
        self.cst_Y = 0

    # CALCUL DE I(X,Y) :
    def eval(self, X, Y):
        x = self.cst_X * N.sum(X)
        y = self.cst_Y * N.sum(Y)
        res = x + y
        return res

    def setParams(self, Params):
        self.cst_X = Params[0]
        self.cst_Y = Params[1]
