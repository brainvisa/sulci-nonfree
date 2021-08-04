from __future__ import print_function

from __future__ import absolute_import
import numpy as N
from datamind.core import DF
import os

# ------------------------------------------------------------------------------
# UTILS


def buildDesignMat(X, formula=None, interaction=None):
    """
    Build the Design from  X (DataFrame).
    This X comes from a list of tuples result of a SQL request or
    a csv file).

    Categorial variable coding:
    Categorial variable are coded using the "overparameterized model":
    One indicator variable is created for each level of the categorical
    variable

    TODO: formula (like in R) ex.: "sex+age+sex:age"
    TODO: interaction
    """
    Xout = None
    Xout_names = []
    for col in X.colnames():
        print(col)
        # col="ResponseEstimatedAt4weeksAveraged"
        if type(X[0, col]) is str:
            levels = N.unique(X[:, col].tolist())
            l = []
            for lev in levels:
                Xout_names.append(col + "_" + lev)
                l.append([int(rep == lev) for rep in X[:, col].tolist()])
            Xoutcur = N.array(l).T
        else:
            Xout_names.append(col)
            Xoutcur = X[:, col]
        if Xout is None:
            Xout = Xoutcur
        else:
            Xout = N.concatenate(
                [Xout, Xoutcur.reshape(Xoutcur.shape[0], 1)], 1)
        Xout = DF(Xout, colnames=Xout_names)
    return Xout


def factorCodingOneWayAnova(x, coding="dummy", intercept=False, Fcontrast=False):
    """
    Code caterorial regressor for one-way anova
    x : an array, which is flatened before any processing
    method: (default "Overparameterized")
    Example:
    factorCodingOneWayAnova(['G1', 'G1', 'G2', 'G2', 'G3', 'G3'])
    #array([[1, 0, 0],
        #[1, 0, 0],
        #[0, 1, 0],
        #[0, 1, 0],
        #[0, 0, 1],
        #[0, 0, 1]])

    factorCodingOneWayAnova(['G1', 'G1', 'G2', 'G2', 'G3', 'G3'],intercept=True,Fcontrast=True)
    #(array([[1, 0, 1],
            #[1, 0, 1],
            #[0, 1, 1],
            #[0, 1, 1],
            #[0, 0, 1],
            #[0, 0, 1]]),
    #array([[ 1.,  0.,  0.],
        #[ 0.,  1.,  0.]]))
    """
    import numpy as N
    x = N.asarray(x).flatten()
    levels = N.unique(x)
    fcont = None
    if coding == "dummy":
        X = None
        for lev in levels:
            reg = (lev == x).astype("int").reshape(x.shape[0], 1)
            if X is None:
                X = reg
            else:
                X = N.concatenate([X, reg], 1)
        fcont = N.identity(X.shape[1])
        if intercept is True:
            X[:, -1] = 1
            dim = X.shape[1] - 1
            fcont = N.concatenate(
                [N.identity(dim), N.zeros(dim).reshape(dim, 1)], 1)
        if Fcontrast:
            return X, fcont
        return X
    print("Coding ", coding, " Not implemented")


def factorCodingTwoSamplesTtest(x, coding="dummy", intercept=False, Tcontrast=False):
    """
    Code caterorial regressor for two samples t-test
    x : an array, which is flatened before any processing
    method: (default "Overparameterized")
    Example:
    factorCodingTwoSamplesTtest(['G1', 'G1', 'G2', 'G2'])

    factorCodingTwoSamplesTtest(['G1', 'G1', 'G2', 'G2', 'G1', 'G2'],Tcontrast=True)
    """
    X = factorCodingOneWayAnova(
        x, coding=coding, intercept=False, Fcontrast=False)
    if N.unique(N.asarray(x).ravel()).size != 2:
        raise ValueError("x should have exactly 2 levels")
    if Tcontrast:
        return X, N.array([1, -1])
    return X


def addIntercept(x, Fcontrast=False):
    import datamind.core.utilsStruct as utils
    x = utils.as2darray(x)
    n = x.shape[0]
    nreg = x.shape[1]
    # add the intercept
    intercept = utils.as2darray(N.ones(n))
    x = N.concatenate([x, intercept], axis=1)
    # build the contrast
    if not Fcontrast:
        return x
    fcon = N.concatenate([N.identity(nreg), N.zeros(nreg).reshape(nreg, 1)], 1)
    return x, fcon
