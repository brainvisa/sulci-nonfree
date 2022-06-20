# Copyright CEA-INRIA (2000-2008)
#
#  This software and supporting documentation were developed by
# CEA,DSV,I2BM,Neurospin and INRIA Saclay, PARIETAL
# CEA Saclay, Batiment 145, Point Courrier 156
# 91191 Gif sur Yvette  FRANCE
#
# This software is governed by the CeCILL license version 2 under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.


from __future__ import absolute_import
import numpy
import numpy.random
from datamind.ml import wip
from datamind.ml import classifier
#from datamind.ml import classif
from datamind.ml import database
from numpy.linalg import pinv, eig
from datamind.ml import scaling
from six.moves import range


class LdaNumpy(classifier.Classifier):

    '''
LDA (Linear Discriminant Analysis) : classifier for 2 classes.

It can be used to do linear classification or linear feature extraction
(See FeatureExtractionLdaNumpy).

Map X original feature vector to a reduced feature z such as z = A^t * x,
Maximizing The Fisher's class separability criterion :

       tr(A^t * Sb * A )
F(A) = -----------------      with Sb : between covariance matrix,
       tr(A^t * Sw * A )           Sw : within covariance matrix.'''

    def __init__(self):
        classifier.Classifier.__init__(self)

    def fit(self, train):
        '''Fit LDA on train database.'''
        import exceptions
        X, Y = numpy.asmatrix(train.getX()), train.getY()
        classes = numpy.unique(Y)
        if len(classes) == 2:
            ind0, ind1 = classes
        else:
            raise exceptions.RuntimeError('LDA can handle only ' +
                                          '2 clases. Your classes : ' + str(classes))

        # Compute within and between covariance matrix
        Xm = X.mean(axis=0)
        Y0 = (Y == ind0).ravel()
        s0 = Y0.sum()
        X0 = X[Y0]
        X0m = X0.mean(axis=0)
        Y1 = (Y == ind1).ravel()
        X1 = X[Y1]
        s1 = Y1.sum()
        X1m = X1.mean(axis=0)
        Hw0 = X0 - X0m
        Hw1 = X1 - X1m
        Hb0 = X0m - Xm
        Hb1 = X1m - Xm
        Sw = Hw0.T * Hw0 + Hw1.T * Hw1
        Sb = Hb0.T * Hb0 * s0 + Hb1.T * Hb1 * s1

        # Solve eigenproblem to maximize F(A)
        m = Sw.I * Sb
        vals, vects = numpy.linalg.eig(m)

        # ordering according to decreasing discriminating power
        r = vals.argsort()[::-1]

        self._Sw = Sw
        self._Sb = Sb
        self._vals = vals[r]
        self._A = vects[:, r]

    def predict(self, test):
        X, Y = test.getX(), test.getY()
        target = None  # FIXME
        wip.not_implemented()
        return WeightedBinaryClassifierResult(Y, target)

    def getTransformation(self):
        return self._A

    def getEnergy(self):
        '''Return optimized energy.'''
        w = self._A[:, 0]
        return ((w.T * self._Sb * w) / (w.T * self._Sw * w))[0, 0]


class Pls1Numpy(classifier.Regressor):

    '''
PLS (Partial Least Squares) : only for one dimensional Y.

Usefull for linear regression or linear feature extraction (like ACP)
based on studying covariance between principal axis and Y
(see FeatureExtractionPls1Numpy).

X and Y must be normalized (see NormalizedXYScale).

This PLS code is inspired from PyChem module and from the book
"The Element of Statistical Learning", T. Hastie, R. Tibshirani,
J. Friedman.'''

    def __init__(self):
        classifier.Regressor.__init__(self)

    def fit(self, train, weights=None):
        '''
Fit PLS on train database.


train : train database : X matrix must be standardized.

Method :
1) Compute the Axis named Z of maximum corelation between X and Y.
2) Orthogonalize X with respect to Z -> new X
3) Go to (1) and iterate until X rank is null.

The mth direction W_m solves :

    max      Corr^2 (y, Xa) * Var(Xa)
 ||a|| = 1
W_l^t * S * a = 0,       where S is the sample covariance matrix of the Xj
for l = 1...m-1'''
        X = numpy.matrix(train.getX(), copy=True)
        Y = numpy.matrix(train.getY(), copy=True)
        if weights is not None:
            WE = numpy.asarray(weights).ravel()
        if Y.shape[1] != 1:
            raise exceptions.RuntimeError('PLS1 can handle only ' +
                                          'one dimensional Y. Your Y have ' +
                                          str(Y.shape[1]) + 'dimensions')
        thetas, Zs, Ws, Ps = [], [], [], []
        import matplotlib.mlab as MM
        # r = MM.rank(X)
        # while r != 0:
        for i in range(X.shape[1]):

            if weights is not None:
                # W = X.T * diag(WE) * Y
                W = (X.T * numpy.multiply(WE, Y.T).T)
            else:
                W = (X.T * Y)
            W = W / numpy.sqrt(W.T * W)  # correlations
            Z = X * W
            nZ = Z.T * Z
            p = (X.T * Z) / nZ
            theta = ((Z.T * Y) / nZ)[0, 0]
            Y -= theta * Z
            # Orthogonalized X according to Z
            X -= Z * p.T
            Ws.append(W)
            Zs.append(Z)
            Ps.append(p)
            thetas.append(theta)
            r = MM.rank(X)
        # Transformed data
        self._data = numpy.hstack(Zs)
        self._P = numpy.hstack(Ps)
        self._weights = numpy.hstack(Ws)
        self._thetas = numpy.matrix(thetas).T
        self._transform = self.computeTransformation(train.getX(),
                                                     self._data)

    def computeCoeff(self, ind):
        '''
Compute and return the Beta vector of PLS coefficients with respect to the
mth axis.

ind : index of axis (from 0 to dim(X))

Beta = W * (P.T * W)^-1 * thetas

where w_j = correlations weights of j axis.'''
        W = self._weights[:, :ind]
        thetas = self._thetas[:ind]
        P = self._P[:, :ind]
        return W * (P.T * W).I * thetas

    def getTransformedData(self):
        '''Return Transformed train data.'''
        return self._data

    def predict(self, test, ind=-1):
        '''
Estimate Y from X on test database.

test :   test database
ind :    index of axis (from 0 to dim(X))
     -1 to take all dimensions.

X and Y must be normalized.

Y = X * Beta    where Beta = W * (P.T * W)^-1 * thetas'''
        from datamind.ml.classifier import WeightedRegressorResult
        X, Y = test.getX(), test.getY()
        if ind == -1:
            ind = X.shape[1]
        target = X * self.computeCoeff(ind)
        return WeightedRegressorResult(Y, target)

    def optimize(self, test):
        X, Y = test.getX(), test.getY()
        errors = []
        for d in range(1, X.shape[1] + 1):
            res = self.predict(test, d)
            errors.append(res.compute_mse()[0, 0])
        return numpy.array(errors)

    def computeTransformation(self, X1, X2):
        u, d, vt = numpy.linalg.svd(numpy.asmatrix(X1), False)
        ind = numpy.argwhere(d == 0)
        d[ind] = 1.
        di = 1. / d
        di[ind] = 0.
        return vt.T * (numpy.diag(di)) * u.T * numpy.asmatrix(X2)

    def getTransformation(self):
        '''To apply PLS transformation on a new dataset.'''
        return self._transform


class LRNumpy(classifier.Regressor):

    '''Linear Regression : Y = X * Beta  => Beta = X^-1 * Y'''

    def __init__(self):
        classifier.Regressor.__init__(self)

    def fit(self, train):
        X, Y = train.getX(), train.getY()
        self._beta = numpy.dot(numpy.linalg.pinv(X), Y)
        '''
		X_mean = numpy.mean(X, 0)
		X_mean = numpy.reshape( X_mean, (1, X_mean.shape[0]))
		beta_mean = numpy.concatenate((beta, X_mean), 0)

		Y_meanEffect = numpy.concatenate( (Y, \
					numpy.ones( (Y.shape[0], 1), 'd')), 1)
		'''
        self._error = Y - numpy.dot(X, self._beta)

    def getTransformation(self):
        '''Return Transformed train data.'''
        return self._beta

    def getError(self):
        return self._error

    def predict(self, test):
        X, Y = test.getX(), test.getY()
        target = numpy.dot(X, self._beta)
        return classifier.WeightedRegressorResult(Y, target)


