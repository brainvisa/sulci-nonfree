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

from datamind.ml.database import DbNumpy
from datamind.tools import msg
from datamind.ml import wip
from datamind.ml.database import iterators
from datamind.ml.database import selectors


class DbSi(DbNumpy):

    def __init__(self, X=None, Y=None, INF=None, groups=None,
                 sigraph_header=None):
        DbNumpy.__init__(self, X, Y, INF, groups)
        if sigraph_header is not None:
            self._split = sigraph_header['split']
            self._cycles = sigraph_header['cycles']
        else:
            self._split = 0
            self._cycles = 0

    def setSplit(self, split):
        self._split = split

    def setCycles(self, cycles):
        self._cycles = cycles

    def getSplit(self):
        return self._split

    def getCycles(self):
        return self._cycles

    def share(self):
        X, Y, INF, groups = self.getX(), self.getY(), \
            self.getINF(), self.getGroups()
        dbsi = DbSi(X, Y, INF, groups)
        dbsi.setSplit(self._split)
        dbsi.setCycles(self._cycles)
        return dbsi

    def copy(self, X=True, Y=True, INF=True,
             groups=True, split=True, cycles=True):
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
        dbsi = DbSi(X, Y, INF, groups)
        if split:
            dbsi.setSplit(self._split)
        if cycles:
            dbsi.setCycles(self._cycles)
        return dbsi

    def filter(self, mode="outp", split=True):
        '''
mode :     'class_id', 'outp' or 'none' (or None)
split :    split data in train / test database according to sigraph split
       value (see self.getSplit).
        '''
        import numpy
        if self._split is None:
            msg.warning('filter : nothing done, because no split '
                        'defined in database, use setSplit to set it.')
            return None
        if split == False:
            return self
        split = self._split
        X, Y, INF = self.getX(), self.getY(), self.getINF()
        outp = Y[:, 0][:, numpy.newaxis]
        class_id = Y[:, 1][:, numpy.newaxis]
        size = X.shape[0]
        train_ind = range(split)
        test_ind = range(split, size)
        train_groups = class_id[train_ind].ravel()
        test_groups = class_id[test_ind].ravel()
        if mode is "outp":
            Y = outp
        elif mode is "class_id":
            Y = class_id
        elif mode is None or mode is 'none':
            Y = None
        else:
            msg.error("unknown mode '%s'" % mode)
            return None
        if X is not None:
            Xa = X[train_ind]
            Xe = X[test_ind]
        else:
            Xa = Xe = None
        if Y is not None:
            Ya = Y[train_ind]
            Ye = Y[test_ind]
        else:
            Ya = Ye = None
        if INF is not None:
            INFa = INF[train_ind]
            INFe = INF[test_ind]
        else:
            INFa = INFe = None

        train = DbSi(Xa, Ya, INFa, train_groups)
        test = DbSi(Xe, Ye, INFe, test_groups)
        train.setCycles(self._cycles)
        test.setCycles(self._cycles)
        return train, test
