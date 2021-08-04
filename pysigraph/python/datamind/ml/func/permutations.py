from __future__ import print_function

from __future__ import absolute_import
import numpy as N
import numpy.random as NR
from six.moves import range

"""
Set of functions using permutations and objective function to compute a p-value used
for evaluating if a feature is informative
"""


def permutation(X, Y, func):
    """
    Function evaluating an objective function with permutations on X
    Need :
    X
    Y
    objective function
    Return :
    value of the ojective function with permutations
    """
    rd = N.argsort(NR.rand(N.size(X, 0)))
    Xi = X[rd,:]
    value = func.eval(Xi, Y)
    return value


def p_value_distrib(value,Distrib,inverse = False):
    """
    Compute the pvalue of value in a distribution.

    Example :
    p_value_distrib(value,Distrib)

    """
    if inverse == False :
        under = N.asarray(N.where(Distrib > value))[0]
    if inverse == True :
        under = N.asarray(N.where(Distrib < value))[0]
    p_value = N.float(N.size(under))/N.float(N.size(Distrib))
    return p_value


def p_value_information(X,Y,func,P,inverse = False):
    """
    Function evaluating an p-value of an objective function with permutations on X
    Need :
    X
    Y
    objective function
    P :  the number of permutations
    inverse : if inverse == True (False by defaut), compute the head of the distribution
    Return :
    p-value of the ojective function with permutations
    """
    value = func.eval(X, Y)
    Distrib = []
    for l in range(P) :
        Distrib.append(permutation(X, Y, func))
    Distrib = N.asarray(Distrib)
    p_value = p_value_distrib(value, Distrib, inverse)
    return p_value


def is_informatif(X,Y,func,P,alpha,inverse = False):
    """
    Function evaluating if X as information with treshold on the p-value
    Need :
    X
    Y
    objective function
    P :  the number of permutations
    alpha : treshold on the p-value
    Return :
    boolean  : is informatif
    """
    p_value = p_value_information(X, Y, func, P, inverse)
    if (p_value < alpha) :
        informatif = True
    else :
        informatif = False
    return informatif








class PermutationsPvalue (object):
    """
    Univariate : If X is a matrix, then return a vector with the value for each column of X.
    Function evaluating an p-value of an objective function with permutations on X
    Need :
    X
    Y
    objective function
    P :  the number of permutations
    same = False : if same = True, we use the same distribution of permutations over all the voxels (faster)
    inverse ( = False) : if true, compute the pvalue in teh head of the distribution.
    verbose
    Return :
    p-value of the ojective function with permutations
    """
    def __init__(self,func,P,same = False,inverse = False,verbose = False):
        self.func = func
        self.P = P
        self.same = same
        self.inverse = inverse
        self.verbose = verbose

    def eval(self, X, Y):
        func = self.func
        P = self.P
        same = self.same
        inverse = self.inverse
        p_value = N.zeros(N.size(X, 1))
        if same == False :
            for i in range(N.size(X, 1)):
                Xi = X[:, i]
                Xi = N.reshape(Xi, [N.size(Xi, 0), 1])
                score = func.eval(Xi, Y)
                Distrib = []
                for l in range(P) :
                    Distrib.append(permutation(Xi, Y, func))
                Distrib = N.asarray(Distrib)
                p_value[i] = p_value_distrib(score, Distrib, inverse)
                if self.verbose == True :
                    print(i, p_value[i])


        if same == True :
            Distrib = []
            for l in range(P) :
                Xi = X[:, 0]
                Xi = N.reshape(Xi, [N.size(Xi, 0), 1])
                Distrib.append(permutation(Xi, Y, func))

            for i in range(N.size(X, 1)):
                Xi = X[:, i]
                Xi = N.reshape(Xi, [N.size(Xi, 0), 1])
                value = func.eval(Xi, Y)
                p_value[i] = p_value_distrib(value, Distrib, inverse)
                if self.verbose == True :
                    print(i, p_value[i]			)




        self.pvalue = p_value
        return p_value

    def getPvalues(self):
        return self.pvalue
