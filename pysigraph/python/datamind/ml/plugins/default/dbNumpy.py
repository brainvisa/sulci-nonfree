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
from datamind.ml import database
import numpy


class DbNumpy(database.Db):

    '''
Database implementation based on Numpy arrays.
    '''

    def __init__(self, x=None, y=None, info=None, groups=None):
        '''
x :      features data matrix.
y :      data matrix to fit (classes / regression values).
info :   info table instance.
groups : array of group label to groups continuous Y data. Should be used
     for data to be regressed and owning with some a priori on group
     structure in these data. Used to take weights into account or
     to balance data (filter / crossvalidation).

     By default (groups=None), vectors are grouped by their Y value :
     classes in case of classification and set of same Y values for
     regression.

     ex : groups = [1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1]
          define 2 groups of indices : [0, 3, 4, 8, 9, 11]
                                   and [1, 2, 5, 6, 7, 10]

x, y, info and groups reference are stored in this database.
        '''
        self._x = x
        self._y = y
        self._info = info
        self._groups = groups
        if (groups is not None and len(groups.shape) != 1):
            raise ValueError(
                "groups parameter must have only one axis.")
        self._check_shape_2d('X', x)
        self._check_shape_2d('Y', y)
        self._check_shape_2d('INF', info)
        self._compute_dims()
        self._check_dims()
        self._compute_cols_number()

    def _compute_dims(self):
        '''
Compute number of rows from data (seeking those which are not None).
        '''
        list = [c for c in [self._x, self._y, self._info, self._groups]
                if c is not None]
        if len(list) == 0:
            self._rows_number = 0
        else:
            self._rows_number = list.pop().shape[0]

    def _check_shape_2d(self, name, matrix):
        if matrix is None:
            return
        if len(matrix.shape) != 2:
            raise ValueError("%s : bad shape, expect 2d." % name)

    def _check_dims(self):
        if not self._rows_number:
            return
        list = [c for c in [self._x, self._y, self._info, self._groups]
                if c is not None]
        list.pop()
        error = False
        for l in list:
            if l.shape[0] != self._rows_number:
                error = True
        if error:
            raise ValueError("x, y, info and groups must "
                             "have the same number of rows.")

    def _compute_cols_number(self):
        self._cols_number = 0
        list = [c for c in [self._x, self._y, self._info]
                if c is not None]
        for l in list:
            self._cols_number += l.shape[1]

    # python methods
    def __repr__(self):
        return database.Db.__repr__(self)

    def shape(self):
        '''
Return shape of database (including X, Y and INF data matrix).
        '''
        return self._rows_number, self._cols_number

    def share(self):
        '''
Return a database sharing internal data with current database.
        '''
        X, Y, INF, groups = self.getX(), self.getY(), \
            self.getINF(), self.getGroups()
        return DbNumpy(X, Y, INF, groups)

    def copy(self, X=True, Y=True, INF=True, groups=True):
        '''
Return a copy of current database and its internal data.

X, Y, INF, groups are boolean. If True, the matching parameter
is not copied, None is set instead.
        '''
        if X:
            X = self.getX().copy()
        else:
            X = None
        if Y:
            Y = self.getY().copy()
        else:
            Y = None
        if INF:
            INF = self.getINF().copy()
        else:
            INF = None
        if groups:
            groups = self.getGroups().copy()
        else:
            groups = None
        return DbNumpy(X, Y, INF, groups)

    # Data accessors
    def getX(self):
        '''get the independant matrix data'''
        return self._x

    def getY(self):
        '''get the dependant matrix data'''
        return self._y

    def getINF(self):
        '''get metainformation matrix data'''
        return self._info

    def getGroups(self):
        '''
Return groups if exist, else call getY(). The return value should
represent classes (for discrete Y and classification purpose) or groups
(for continuous Y and regression purpose). '''
        if self._groups is not None:
            return self._groups
        Y = self.getY()
        if Y is not None and len(Y.shape) != 1:
            return None
        return Y

    def getRowNumber(self):
        return self._rows_number

    def getColsNumber(self):
        return self._cols_number

    def getRow(self, ind):
        return self._x[ind], self._y[ind], \
            self._info[ind], self._group[ind]

    def setX(self, X):
        self._x = X
        self._check_dims()
        self._compute_cols_number()

    def setY(self, Y):
        self._y = Y
        self._check_dims()
        self._compute_cols_number()

    def setXY(self, X, Y):
        self._x = X
        self._y = Y
        self._check_dims()
        self._compute_cols_number()

    def setINF(self, INF):
        self._info = INF
        self._check_dims()
        self._compute_cols_number()

    def setGroups(self, groups):
        self._groups = groups
        self._check_dims()
