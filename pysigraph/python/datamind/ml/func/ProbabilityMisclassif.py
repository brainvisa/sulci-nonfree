from __future__ import absolute_import
import numpy as N
import scipy as SP
from six.moves import range


class ProbabilityErrorSubset(object):

    """
    Score of misclassification rate (cf Optimal Feature Selection for NEarest Centroid Classifiers, With Applications to Gene Expression Mircoarrays - A.R. Dabney and J.D. Storey).


    Examples:
    from datamind.ml.func import *
    from datamind.ml.classif import *
    func = ProbabilityErrorSubset()
    X = NR.rand(100,50)
    Y = (4*X[:,10]).astype('int')
    Y = N.reshape(Y,[N.size(Y),1])
    score = func.eval(X,Y)

    """

    # CALCUL DE I(X,Y) :

    def eval(self, Xtrain, Ytrain):
        """
        Function computing Pr(error,s) for X, Y and a sbuset.

        Input:
        X
        Y
        Subset

        Output
        Pr(error,s)
        """

        nY = N.unique(Ytrain)
        import pylab as PY
        cov = N.asmatrix(PY.cov(N.transpose(Xtrain)))

        probaY = N.zeros(N.size(nY))
        for i in range(N.size(probaY)):
            sizei = N.size(N.where(Ytrain == nY[i])[0])
            probaY[i] = N.float(sizei) / N.float(N.size(Ytrain, 0))

        # CALCUL DES CENTRES POUR UN SUBSET
        mu = []
        for i in range(N.size(nY)):
            Yi = N.where(Ytrain == nY[i])[0]
            Xi = Xtrain
            mu.append(N.average(Xi[Yi, :], 0))
        mu = N.asarray(mu)

        # CALCUL DES DISTANCES DE MAHALANOBIS
        dist_mahal = N.zeros([N.size(nY), N.size(nY)])
        for i in range(N.size(nY)):
            for j in range(N.size(nY)):
                mui = N.asmatrix(mu[i])
                muj = N.asmatrix(mu[j])
                dist = ((muj - mui)) * cov.I * (muj - mui).T
                dist_mahal[i, j] = N.sqrt(dist)

        # VALEUR POUR PHI
        value_phi = N.zeros([N.size(nY), N.size(nY)])
        for i in range(N.size(nY)):
            for j in range(N.size(nY)):
                value = dist_mahal[i, j]**2 + 2 * N.log(probaY[j] / probaY[i])
                if dist_mahal[i, j] != 0:
                    value = value / (2 * dist_mahal[i, j])
                    value_phi[i, j] = value
        for i in range(N.size(nY)):
            for j in range(N.size(nY)):
                if i == j:
                    value_phi[i, j] = N.max(value_phi)

        # VALEUR DU TERME DE LA SOMME
        value_sum = 0
        for j in range(N.size(nY)):
            minij = N.min(value_phi[:, j])
            value_sum_j = (1 - SP.stats.norm.cdf(minij)) * probaY[j]
            value_sum = value_sum + value_sum_j

        return value_sum
