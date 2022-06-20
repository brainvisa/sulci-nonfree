# Copyright CEA (2000-2006)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
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
from datamind.ml import dimreduction
from datamind.ml import database
import numpy
from six.moves import range


#
class SvdNumpy(dimreduction.DimensionReduction):

    '''
SVD : Singular Value Decomposition for dimension reduction.

How to use it on a test set ?
>>> svd = SvdNumpy()
>>> svd.fit(train)
>>> r_test = svd.reduce(test, ndim)   #ndim : number of preserved dimensions

How to use it on your train set ?
>>> r_train = svd.get_reduced_train(train, ndim)
    '''

    def fit(self, train):
        '''
Fit singular value decomposition (svd) on train data.

Find u, d, v,   with d : diagonal matrix,
                 v : unitary matrix according to
                     m = u * d * v^t, where m is train.getX().

train : train database.

Return u matrix matching transformed train matrix by svd. Null covariance
eigenvalues don't project data in u matrix, use predict method if you
want to nullify transformed data along principal dimensions of null eigen
values.'''
        m = train.getX()
        # m = u * d * vt
        u, d, vt = numpy.linalg.svd(m, False)
        self._u = u
        self._d = numpy.array(d)
        self._vt = vt
        return self._u

    def reduce(self, test, n=-1, copy=False):
        '''
Apply SVD on test database and reduce data to the first n principal
components. Null standard deviation are handled cleanly.

test : the test database.
n :    the number of selected principal components.
copy:  If False return reduced database shares most data with current
   database except reduced ones.

Return a new database of transformed and reduced dataset.
        '''
        X = test.getX()

        # prevent null std deviation
        d = numpy.array(self._d)
        ind = numpy.argwhere(d == 0)
        d[ind] = 1.
        di = 1. / d
        di[ind] = 0.

        # m2 = m * v * d^-1
        # FIXME : faire la selection avant la multiplication)
        X2 = numpy.multiply(numpy.dot(X, self._vt.T), di)
        if n > 0:
            if n > X2.shape[1]:
                n = int(X2.shape[1] / 2.0)
            X2 = X2[:, :n]
        db = test.share_or_copy(copy)
        db.setX(X2)
        return db

    def get_reduced_train(self, train, n=-1, copy=False):
        '''
Get reduced train database without any computing, because reduced database
has already been computed when fitting the model. The train parameter is
only used to put Y values (classes, regressors) into database.

train : train database.
n :     the number of selected principal components.
copy:   If False return reduced database shares most data with current
    database except reduced ones.

Return a new reduced train database.
        '''
        X2 = self._u
        if n > 0:
            if n > X2.shape[1]:
                n = int(X2.shape[1] / 2.0)
            X2 = X2[:, :n]
        db = train.share_or_copy(copy)
        db.setX(X2)
        return db

    def get_udv(self):
        '''
Return u, d, v^t, according to m = u * d * v^t.
        '''
        return self._u, self._d, self._vt

    def get_components_from_infopct(self, pct):
        '''
Return number of components to keep to preserve a certain amount of
information in your data.

pct : percentage (between 0 and 1) of information you want to preserve.
        '''
        # d provides standard deviation information
        # Information amount is encoded by variance in gaussian case.
        d2 = self._d ** 2
        p = d2.cumsum() / d2.sum()
        w = numpy.argwhere(p >= pct)
        if pct > 1. or len(w) == 0:
            return None
        else:
            return w[0, 0]

    def getTransformation(self):
        '''
Compute and return transformation M computed by svd.

So U = X * M  where  the database X = (U * D * V^t).
                 and M = V * D^-1
        '''
        u, d, vt = self.get_udv()
        ind = numpy.argwhere(d == 0)
        d[ind] = 1.
        di = 1. / d
        di[ind] = 0.

        return numpy.dot(vt.T, numpy.diag(di))


#
class CcaNumpy(dimreduction.DimensionReduction):

    '''
CCA : Canonical Correlation Analysis.

Optimized correlation between inputs X and outputs Y.

           E[xy]
cor = ------------------
      sqrt(E[x^2]E[y^2])

with x = X * wx^t
    y = Y * wy^t

How to use it ?
>>> cca = CcaNumpy()
>>> cca.fit(train)
>>> r_train = cca.reduce(train, ndim)
>>> r_test = cca.reduce(test, ndim)
'''

    def fit(self, train):
        '''
Learn CCA best correlation and vector basis from train data.

train : train database.'''
        X, Y = numpy.matrix(train.getX()), numpy.matrix(train.getY())

        # normalize Y
        Y = (Y - Y.mean()) / Y.std()

        # Correlations
        Cxx = X.T * X
        CxxI = Cxx.I
        Cyy = Y.T * Y
        CyyI = Cyy.I
        Cxy = X.T * Y
        Cyx = Cxy.T

        # solve smallest eigenproblem.
        if X.shape[1] < Y.shape[1]:
            m = (CxxI * Cxy * CyyI * Cyx)
        else:
            m = (CyyI * Cyx * CxxI * Cxy)
        vals, vects = numpy.linalg.eigh(m)
        r = list(range(vals.size))
        r.reverse()
        cor = numpy.sqrt(vals[:, r])  # biggest eigenvalue

        # Cxy * wy = cor * lambda_x * Cxx * wx
        # Cyx * wx = cor * lambda_y * Cyy * wy
        # with lambda_x = 1/lambda_y
        #               = sqrt((wy^t * Cyy * wy) / (wx^t * Cxx * wx))
        if X.shape[1] < Y.shape[1]:
            wx = vects[:, r]
            w = numpy.linalg.pinv(Cxy) * cor[0] * Cxx * wx
            lambda_x = 1. / numpy.linalg.norm(w)
            wy = w * lambda_x
        else:
            wy = vects[:, r]
            w = numpy.linalg.pinv(Cyx) * cor[0] * Cyy * wy
            lambda_y = 1. / numpy.linalg.norm(w)
            wx = w * lambda_y

        self._wx = wx
        self._wy = wy
        self._cor = cor

    def reduce(self, test, n, copy=False):
        '''
Apply CCA on test database and reduce data to the n most correlated
components.

test : test database.
n :    number of selected correlated components.
copy:  If False return reduced database shares most data with current
   database except reduced ones.

Return a new database of transformed and reduced dataset.'''

        X, Y = numpy.asmatrix(test.getX()), numpy.matrix(test.getY())
        X2 = (X * self._wx)[:, list(range(n))]
        Y2 = (Y * self._wy)[:, list(range(n))]

        db = test.share_or_copy(copy)
        db.setXY(X2, Y2)
        return db

    def get_correlation(self):
        '''Return CCA optimized correlation values.'''
        return self._cor

    def get_canonical_correlations_number(self):
        '''Return number of canonical correlations vectors.'''
        return len(self._cor)


#
class FeatureExtractionLdaNumpy(dimreduction.DimensionReduction):

    '''Feature Extraction based on LDA (Linear Discriminant Analysis).'''

    def fit(self, train):
        '''Learn LDA projection matrix from train database.'''
        from datamind.ml import classifier
        self._lda = classifier.LdaNumpy()
        self._lda.fit(train)

    def reduce(self, test, n, copy=False):
        '''
Apply LDA on test database and reduce data to the n most discriminative
components.

test : the test database.
n :    the number of selected latent components.
copy:  If False return reduced database shares most data with current
   database except reduced ones.

Return a new database of transformed and reduced dataset.'''
        X = numpy.asmatrix(test.getX())
        X2 = X * self._lda.getTransformation()
        db = test.share_or_copy(copy)
        db.setX(X2[:, list(range(n))])
        return db

    def getTransformation(self):
        '''Get LDA projection axes (transformation matrix).'''
        return self._lda.getTransformation()


#
class FeatureExtractionPls1Numpy(dimreduction.DimensionReduction):

    '''
Feature Extraction based on PLS1 (Partial Least Squares for one
dimensional Y).

How to use it on a test set ?
>>> dr = FeatureExtractionPls1Numpy()
>>> dr.fit(train)
>>> r_test = dr.reduce(test, ndim)   #ndim : number of preserved dimensions

How to use it on your train set ?
>>> r_train = dr.get_reduced_train(train, ndim)

X and Y must be normalized (see NormalizedXYScale).'''

    def __init__(self, train=None):
        self.__doc__ = FeatureExtractionPls1Numpy.__doc__
        from datamind.ml import classifier
        self._pls = classifier.Pls1Numpy()
        if train:
            self.fit(train)

    def fit(self, train, weights=None):
        '''
Learn PLS1 best latent directions from train data.

train : train database.
weights : '''
        self._pls.fit(train, weights)

    def get_reduced_train(self, train, n=-1, copy=False):
        '''
Get reduced train database without any computing, because reduced database
has already been computed when fitting the model. The train parameter is
only used to put Y values (classes, regressors) into database.

train : train database.
n :     the number of selected latent components.
copy:  If False return reduced database shares most data with current
   database except reduced ones.

Return a new reduced train database.'''
        X2 = numpy.asarray(self._pls.getTransformedData())
        if n > X2.shape[1]:
            n = int(X2.shape[1] / 2.0)
        if n > 0:
            X2 = X2[:, :n]
        db = train.share_or_copy(copy)
        db.setX(X2)
        return db

    def reduce(self, test, n=-1, copy=False):
        '''
Apply PLS1 on test database and reduce data to the n most discriminant
components.

test : test database.
n :    number of selected latent components.
copy:  If False return reduced database shares most data with current
   database except reduced ones.
        '''
        X, Y = numpy.asmatrix(test.getX()), test.getY()
        X2 = numpy.asarray((X * self._pls.getTransformation()))
        if n > X2.shape[1]:
            n = int(X2.shape[1] / 2.0)
        if n > 0:
            X2 = X2[:, :n]
        db = test.share_or_copy(copy)
        db.setX(X2)
        return db

    def getTransformation(self):
        '''Get LDA projection axes (transformation matrix).'''
        return self._pls.getTransformation()

    def get_components_from_errorpct(self, test, pct):
        '''
Return number of latent variables to keep according to error evolution.

pct : percentage (between 0 and 1) of enhancement on error to stop dimension
addition.

Minimal number returned is 1 dimension.
        '''
        mse = self._pls.optimize(test)
        return self.get_components_from_mse(mse, pct)

    def get_components_from_mse(self, mse, pct):
        '''
Return number of latent variables to keep according to error evolution.

mse : array of mean square errors (may be compute by
                                get_components_from_errorpct)'''
        diff = 1. - mse[1:] / mse[:-1]
        return numpy.argwhere(diff < pct)[0, 0] + 1

    def get_pls_model(self):
        '''Return PLS1 model used to do feature extraction.'''
        return _pls


#
class FeatureExtractionANOVA(dimreduction.DimensionReduction):

    def __init__(self, train=None, score_type='f', threshold=0.05, bonferroni=False, FDR=False, mask=None):
        if train:
            self.fit(train)
        self._mask = mask
        self.bonferroni = bonferroni
        self.score_type = score_type
        self.threshold = threshold
        self.FDR = FDR
    '''
	def fit(self, train):
		from datamind.ml import classifier
		X = train.getX()
		Y = train.getY()
		#self.scaling = scaling.NormalizedXScale()
		#X = self.scaling._fit_matrix_X(X)

		if self._mask != None :
			quantity_mask = numpy.sum(self._mask[train.getRowsIndex(), :], 0) - 1
			selected_dim = numpy.nonzero(quantity_mask > 0)[0]
			X_mask = X[:, selected_dim]
	        else : X_mask = X

		train_inversed = database.DbNumpy(numpy.double(Y), X_mask)
		self._lr = classifier.LRNumpy()
		self._lr.fit(train_inversed)

		self._beta = self._lr.getTransformation()
		self._error = self._lr.getError()

		MCSCres = numpy.sum(self._error**2,0)
		MCSCreg = numpy.sum(X_mask**2,0) - MCSCres
		dofReg = float(numpy.shape(Y)[1])
		dofRes = float(numpy.shape(Y)[0]-dofReg-1)
		self._F = MCSCreg/MCSCres*dofRes/dofReg

		from scipy.stats import f
		self._pval = f.sf(self._F,dofReg,dofRes)

		if self.bonferroni == True:
			self._pval*= train.getX().shape[1]
                if self.FDR:
                  	from fff.FDR import fdr
			F = fdr(self.threshold)
			self._pval = F.all_fdr(self._pval)
	'''

    def reduce(self, test, n=-1, pval=0.01, copy=False):
        X = test.getX()
        sF = -numpy.sort(-self._F)
        if n != -1:
            if n > X.shape[1]:
                n = int(X.shape[1] / 2.0)
            self._featuresSelected = numpy.nonzero(self._F > sF[n])[0]
        else:
            self._featuresSelected = numpy.nonzero(
                self._pval <= self.threshold)[0]

        X = X[:, self._featuresSelected]
        db = test.share_or_copy(copy)
        db.setX(X)
        return db

    def fit(self, train):
        from datamind.ml.func import glm
        lr = glm.SimpleLinRegFstat()
        self.f_values = numpy.reshape(
            lr.eval(train.getX(), train.getY()), (lr.eval(train.getX(), train.getY()).shape[1]))
        self.p_values = numpy.reshape(lr.pvalue(), lr.pvalue().shape[1])
        self.z_values = numpy.reshape(lr.zvalue(), lr.zvalue().shape[1])
        if self.bonferroni == True:
            self.p_values *= train.getX().shape[1]
        if self.FDR:
            from fff.FDR import fdr
            F = fdr(self.threshold)
            self.p_values = F.all_fdr(self.p_values)
        self._F = self.f_values
        self._pval = self.p_values

    def getFeaturesSelected(self):
        return self._featuresSelected

    def getPval(self):
        return self.p_values[self._featuresSelected]

    def setInfo(self, mask=None):
        self._mask = mask

    def setBonferroni(self, bonferroni=False):
        self.bonferroni = bonferroni

    def setFDR(self, FDR=False):
        self.FDR = FDR


class SRDANumpy(dimreduction.DimensionReduction):

    def __init__(self, train=None, priors=None, k=-1, alpha=0.1, n=-1):
        if train:
            self.fit(train)
        self.priors = priors
        self.k = k
        self.alpha = alpha
        self.n = n

    def fit(self, train):
        from datamind.ml import classifier
        self._srda = classifier.SRDA(self.k, self.priors, self.alpha, self.n)
        self._srda.fit(train)

        self._transfMatrix = self._srda.getTransformation()

    def reduce(self, test, n=-1, copy=False):
        X = test.getX()
        targetTransf = numpy.dot(X, self._transfMatrix)
        db = test.share_or_copy(copy)
        db.setX(X)
        return db

    def getTransformation(self):
        return self._srda.getTransformation()


class LDANumpy(dimreduction.DimensionReduction):

    def __init__(self, train=None, priors=None):
        if train:
            self.fit(train)
        self.priors = priors

    def fit(self, train):
        from datamind.ml import classifier
        self._lda = classifier.LDA(self.priors)
        self._lda.fit(train)

        self._transfMatrix = self._lda.getTransformation()

    def reduce(self, test, n=-1, copy=False):
        X = test.getX()
        targetTransf = numpy.dot(X, self._transfMatrix)
        db = test.share_or_copy(copy)
        db.setX(X)
        return db

    def getTransformation(self):
        return self._lda.getTransformation()

