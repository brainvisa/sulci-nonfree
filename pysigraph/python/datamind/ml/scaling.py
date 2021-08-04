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
from datamind.ml import wip
from datamind.ml.model import Model


class Scale(Model):

    def fit(self):
        wip.not_implemented()

    def scale(self, train):
        '''
Alias to predict function : apply dimension reduction of db'''
        self.predict(train)


class NormalizedXScale(Scale):

    '''
Basic scale on X data matrix thanks to data mean and standard deviation.'''

    def _fit_matrix_X(self, X):
        import numpy
        self._mean = X.mean(axis=0)
        Xc = X - self._mean
        self._std = Xc.std(axis=0)
        request = (self._std == 0)
        if request.sum() != 0:
            self._std[numpy.argwhere(self._std == 0)] = 1.
        return Xc / self._std

    def fit(self, train, copy=False):
        '''
Learn train database (X part) scaling and apply it onto it.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.
        '''
        from datamind.ml import database
        X = train.getX()
        Y = train.getY()
        X2 = self._fit_matrix_X(X)
        db = train.share_or_copy(copy)
        db.setX(X2)
        return db

    def _predict_matrix_X(self, X):
        return (X - self._mean) / self._std

    def predict(self, test, copy=False):
        '''
Scale test database (X part) according to learned scale.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.'''
        from datamind.ml import database
        X = test.getX()
        Y = test.getY()
        X2 = self._predict_matrix_X(X)
        db = test.share_or_copy(copy)
        db.setX(X2)
        return db

    def get_std(self):
        '''Return learned X standard deviations of each dimension'''
        return self._std

    def get_mean(self):
        '''Return learned X mean of each dimension.'''
        return self._mean


class NormalizedYScale(Scale):

    '''
Basic scale on Y data matrix thanks to data mean and standard deviation.'''

    def _fit_matrix_Y(self, Y):
        self._mean = Y.mean()
        Yc = Y - self._mean
        self._std = Yc.std()
        if self._std == 0:
            self._std = 1.
        return Yc / self._std

    def fit(self, train, copy=False):
        '''
Learn train database (Y part) scaling and apply it onto it.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.
        '''
        from datamind.ml import database
        X = train.getX()
        Y = train.getY()
        Y2 = self._fit_matrix_Y(Y)
        db = train.share_or_copy(copy)
        db.setY(Y2)
        return db

    def _predict_matrix_Y(self, Y):
        return (Y - self._mean) / self._std

    def predict(self, test, copy=False):
        '''
Scale test database (Y part) according to learned scale.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.'''
        from datamind.ml import database
        X = test.getX()
        Y = test.getY()
        Y2 = self._predict_matrix_Y(Y)
        db = test.share_or_copy(copy)
        db.setX(X2)
        return db

    def get_std(self):
        '''Return learned Y standard deviations of each dimension'''
        return self._std

    def get_mean(self):
        '''Return learned Y mean of each dimension.'''
        return self._mean


class NormalizedXYScale(Scale):

    '''
Basic scale on X and Y data matrix thanks to data mean and standard
deviation.'''

    def __init__(self):
        self._nx = NormalizedXScale()
        self._ny = NormalizedYScale()

    def fit(self, train, copy=False):
        '''
Learn train database (X and Y parts) scaling and apply it onto it.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.
        '''
        from datamind.ml import database
        X = train.getX()
        Y = train.getY()
        X2 = self._nx._fit_matrix_X(X)
        Y2 = self._ny._fit_matrix_Y(Y)
        db = train.share_or_copy(copy)
        db.setXY(X2, Y2)
        return db

    def predict(self, test, copy=False):
        '''
Scale test database (X and Y parts) according to learned scale.

copy:  If False return scaled database shares most data with current
   database except shared ones.

Return scaled database.'''
        from datamind.ml import database
        X = test.getX()
        Y = test.getY()
        X2 = self._nx._predict_matrix_X(X)
        Y2 = self._ny._predict_matrix_Y(Y)
        db = test.share_or_copy(copy)
        db.setXY(X2, Y2)
        return db

    def get_stdX(self):
        '''Return learned X standard deviations of each dimension'''
        return self._nx.get_std()

    def get_meanX(self):
        '''Return learned X mean of each dimension.'''
        return self._nx.get_mean()

    def get_stdY(self):
        '''Return learned Y standard deviations of each dimension'''
        return self._ny.get_std()

    def get_meanY(self):
        '''Return learned Y mean of each dimension.'''
        return self._ny.get_mean()
