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
import datamind.ml.database
from . import iterators
from . import db


class View(db.Db):

    def __init__(self, parent, rows=None, cols=None):
        '''
Create a view (relational database meaning). No copy is done. A view is not
fully attached to his parent, it can be describe with a list of rows and a
tuple of list of 3 kinds of columns, you may want to reparent it later.

parent :    a View or a Db instance.
rows :      indexes of rows.
cols :      tuple of list of indexes of columns (X, Y and Info ones).'''
        if rows == [] or cols == []:
            raise ValueError("Can't create View based on " +
                             "empty row or column list")

        if not isinstance(parent, datamind.ml.database.Db):
            raise TypeError("'parent', sould be a database.")
        self._rows = rows
        self._cols = cols
        if self._rows != None:
            self._rows.sort()
            self._rows_number = len(self._rows)
        if self._cols != None:
            self._cols.sort()
            self._cols_number = len(self._cols)
        self._bounded_parent(parent)

    # python methods
    def __repr__(self):
        wip.not_implemented()
        return str(type(self))

    def __len__(self):
        '''get the number of rows'''
        if self._rows is not None:
            return len(self._rows)
        return len(self._parent)

    def share(self):
        '''
Return a database sharing internal data with current database.
        '''
        return Partial_View(self._parent, self._rows, self._cols)

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
        return database.DbNumpy(X, Y, INF, groups)

    def getColsNumber(self):
        if self._cols:
            return len(self._cols)
        return self._parent.getColsNumber()

    def shape(self):
        '''get the view shape (rows, columns)'''
        return len(self), self.getColsNumber()

    # private methods
    def _bounded_parent(self, parent):
        self._parent = parent
        if self._rows == None:
            self._rows_number = self._parent.getRowsNumber()
        if self._cols == None:
            self._cols_number = self._parent.getColsNumber()

    # View methods
    def reparent(self, parent):
        '''Set another parent (db/view) to current view.'''
        self._bounded_parent(parent)

    # Other database methods
    def getX(self):
        g = self._parent.getX()
        if self._rows:
            g2 = g[self._rows]
        else:
            g2 = g
        if self._cols:
            g3 = g2[:, self._cols]
        else:
            g3 = g2
        return g3

    def getY(self):
        g = self._parent.getY()
        if self._rows:
            g2 = g[self._rows]
        else:
            g2 = g
        return g2

    def getINF(self):
        g = self._parent.getINF()
        if self._rows:
            g2 = g[self._rows]
        else:
            g2 = g
        return g2

    def getGroups(self):
        g = self._parent.getGroups()
        if self._rows:
            g2 = g[self._rows]
        else:
            g2 = g
        return g2

    # Row / vectors accessors
    def getRow(self, ind):
        return self._parent.getRow(self._rows[ind])

    def getRowsNumber(self):
        '''Get number of rows (value is cached).'''
        return self._rows_number

    # Iterators
    def getRowIterator(self, start=0):
        '''get a row iterator'''
        return iterators.RowIterator(self, start)

    def getParent(self):
        return self._parent

    def getRowsIndex(self):
        return self._rows


class Partial_View(View):

    def __init__(self, parent, rows=None, cols=None):
        View.__init__(self, parent, rows, cols)
        self._x, self._y, self._info, self._groups = None, None, None, None

    # Other database methods
    def setX(self, X):
        self._x = X

    def setY(self, Y):
        self._y = Y

    def setXY(self, X, Y):
        self.setX(X)
        self.setY(Y)

    def setINF(self, INF):
        self._info = INF

    def setGroups(self, groups):
        self._groups = groups

    def getX(self):
        if self._x is None:
            return View.getX(self)
        else:
            return self._x

    def getY(self):
        if self._y is None:
            return View.getY(self)
        else:
            return self._y

    def getINF(self):
        if self._info is None:
            return View.getINF(self)
        else:
            return self._info

    def getGroups(self):
        if self._groups is None:
            return View.getGroups(self)
        else:
            return self._groups
