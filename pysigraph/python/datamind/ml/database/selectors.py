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
import numpy


class Selector(object):

    def __init__(self, invert):
        self._rows = None
        self._cols = None
        self._cols_name = None
        self._invert = invert

    def _invert_selection(self, db):
        if (self._rows):
            self._rows = numpy.setdiff1d(
                numpy.arange(db.getRowsNumber()), self._rows)
        if (self._cols):
            self._cols = numpy.setdiff1d(
                numpy.arange(db.getColsNumber()), self._cols)

    def select(self, db):
        from . import views
        if self._invert:
            self._invert_selection(db)
        try:
            return views.View(db, self._rows, self._cols)
        except ValueError:
            return None


class Classic(Selector):

    def __init__(self, rows=None, cols=None,
                 invert=False, where=None):
        Selector.__init__(self, invert)
        if rows != None:
            self._rows_filter(rows)
        if cols != None:
            self._cols_filter(cols)

    def _rows_filter(self, rows):
        self._rows = []
        for r in rows:
            t = type(r)
            if (t == int):
                self._rows.append(r)
            else:
                raise TypeError("'" + str(type(r)) +
                                "' : unsupported type")

    def _cols_filter(self, cols):
        self._cols = []
        for c in cols:
            if (type(c) == int):
                self._cols.append(c)
            elif (type(c) == str):
                self._cols_name.append(c)
            else:
                raise TypeError("'" + str(type(c)) +
                                "' : unsupported type")

    def select(self, db):
        if self._cols_name != None:
            wip.not_implemented("cols_name selection")
            # self._cols += db.getIndicesFromName()
        return Selector.select(self, db)


class RowsIndices(Selector):

    def __init__(self, rows, invert=False):
        Selector.__init__(self, invert)
        self._rows = rows


class ColsIndices(Selector):

    def __init__(self, cols, invert=False):
        Selector__init__(self, invert)
        self._cols = cols


class ColsNames(Selector):

    def __init__(self, cols_name, invert=False):
        Selector.__init__(self, invert)
        wip.not_implemented("cols_name selection")


class Array(Selector):

    def __init__(self, rows=None, cols=None, invert=False):
        Selector.__init__(self, invert)
        if rows != None:
            self._check_dtype(rows)
            self._rows = self._filter(rows)
        if cols != None:
            self._check_dtype(cols)
            self._cols = self._filter(cols)

    def _check_dtype(self, boolarray):
        import numpy
        if boolarray.dtype != numpy.dtype('|b1'):
            raise TypeError("'%s' : unsupported dtype"
                            % bool_array.dtype)

    def _filter(self, boolarray):
        import numpy
        return numpy.where(boolarray)[0].tolist()


class Inter(Selector):

    '''Intersection between a list of selectors and/or views.'''

    def __init__(self, selectors, invert=False):
        Selector.__init__(self, invert)

        # rows
        r = selectors.pop()._rows
        for s in selectors:
            r = numpy.intersect1d(r, s._rows)
        self._rows = r

        # cols
        c = selectors.pop()._cols
        for s in selectors:
            c = numpy.intersect1d(r, s._cols)
        self._cols = c
