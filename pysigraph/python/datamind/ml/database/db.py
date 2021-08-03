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
from datamind.ml.mlObject import MlObject
from datamind.ml import wip
from . import iterators
from . import selectors
import numpy
from datamind.tools import *
from six.moves import range


class MetaInfo(MlObject):

    '''Class to store database metainformation.'''

    def __init__(self, cols_name=None):
        self.cols_name = cols_name


class Db(MlObject):

    '''
Abstract class of Data for machine learning
Rem grpCol may become yCols'''

    def __init__(self):
        wip.not_implemented()

    # python methods
    def __repr__(self):
        return str(type(self))

    def __len__(self):
        '''get the number of rows'''
        return self._rows_number

    def size(self):
        '''get the number of rows'''
        return self.__len__()

    def share_or_copy(self, copy=False):
        if copy:
            return self.copy()
        else:
            return self.share()

    def reader(self):
        '''
Return reader instance knowing how to create a db with a
given backen from a file.'''
        return None

    def shape(self):
        '''get the database shape (rows, columns)'''
        wip.not_implemented()

    # Data accessors
    def getX(self):
        '''get the independant matrix data'''
        wip.not_implemented()

    def getY(self):
        '''get the dependant matrix data'''
        wip.not_implemented()

    def getYlevels(self):
        '''if Y is a qualitive vector, get levels of Y'''
        wip.not_implemented()

    # Row / vectors accessors
    def getRow(self, ind):
        '''get one column'''
        wip.not_implemented()

    def getRowsNumber(self):
        '''Get number of rows.'''
        return self._rows_number

    def getRows(self):
        '''get all columns'''
        wip.not_implemented()

    def getRowIterator(self, start=0):
        '''get a row iterator'''
        return iterators.RowIterator(self, start)

    def getSubjects(self, row=None):
        '''convert columns to features'''
        wip.not_implemented()

    # Col / features accessors
    def getCols(self):
        '''get all cols'''
        wip.not_implemented()

    def getColsNumber(self):
        '''Get number of columns.'''
        return self._cols_number

    def getFeatures(self, col=None):
        '''Convert cols to features.'''
        wip.not_implemented()

    def getColsIndicesFromNames(self, names):
        '''Return columns indices from a list of column names.'''
        wip.not_implemented()
        return [_cols_name[n] for n in names]

    # Misc accessors
    def getSignature(self):
        '''return the basename of the file witout extension'''
        wip.not_implemented()

    def setSignature(self, name):
        wip.not_implemented()

    def appendSignature(self, name, set=True):
        '''
Append & return name to the signature. If set is True set the signature.'''
        wip.not_implemented()

    # data selection
    def select(self, *args, **kargs):
        '''
Get a view on database selecting rows and/or columns by there names or their
indices.

select(rows = None, cols = None, invert = False, where = None):
Classical relational database way to do selection. When a parameter is None,
it don't have any effect.

rows :       list of indices of rows. (ex : [1, 4, 5]).
cols :       list of names of indices of columns. (ex : [1, 'name', 5])
invert :     invert selection.
where :      additional logical predicate.


select(sel):
Filter database with 'sel'.

sel :        a selector instance (internal object used to optimize some
         selections).  '''
        import numpy
        sel = rows = [None]
        cols = [None]
        where = [None]
        invert = [False]
        l = [rows, cols, invert, where]
        h = {'rows': rows, 'cols': cols,
             'invert': invert, 'where': where}
        for i, a in enumerate(args):
            l[i][0] = a
        for k, v in kargs.items():
            if h[k][0] != None:
                raise TypeError(
                    "select() got multiple values for " +
                    "keyword argument '" + k + "'")
            h[k][0] = v
        if (isinstance(rows[0], list) or rows[0] == None) and\
                (isinstance(cols[0], list) or cols[0] == None):
            s = selectors.Classic(rows[0], cols[0],
                                  invert[0], where[0])
            return s.select(self)
        if (isinstance(rows[0], numpy.ndarray) or rows[0] == None) and \
                (isinstance(cols[0], numpy.ndarray) or cols[0] == None):
            s = selectors.Array(rows[0], cols[0], invert[0])
            return s.select(self)
        if not isinstance(sel[0], selectors.Selector):
            raise TypeError(
                'First parameter should a list of rows or a selector.')
        else:
            return sel[0].select(self)

    def getIndexesByGrp(self):
        '''return lines index for each group'''
        wip.not_implemented()

    # file I/O
    def save(self, file, as_csv=True):
        wip.not_implemented()

    # filter methods
    def _filter(self, filter_name, nbfolds=None, split=None, groups=None, random_seed=None):
        '''
filter_name :
- groups_balancing : gives groups of data to be balanced.
- discrete_balancing : balance classes (assuming Y part of your data
are classes).
- continuous_balancing : define classes based on your continuous Y data,
balance these groups.
- mixture : simply mix the points (for one class by preference).
- nothing : do nothing (for one class by preference).

Balancing data is important to avoid bias in favour of some classes
by chance.

groups :  used to define groups to balance in each fold.
class_ind : list of lists of the indices of each class
        '''
        class_ind = []
        self._nbfolds = nbfolds
        self._split = split
        self._filter_name = filter_name
        if self._filter_name == 'groups_balancing':
            class_ind = groups
        elif self._filter_name == 'continuous_balancing':
            class_ind = self.continuous_balancing()
        elif self._filter_name == 'discrete_balancing':
            class_ind = self.discrete_balancing()
        elif self._filter_name == 'mixture':
            class_ind = self.mixture(random_seed)
        elif self._filter_name == 'nothing':
            class_ind = self.do_nothing(random_seed)
        else:
            msg.warning("No filter named '%s'" %
                        self._filter_name + ". Disable balancing.")
            self._filter_name = 'nothing'
            class_ind = self.do_nothing(random_seed)

        if self._nbfolds == 'auto':
            self._nbfolds = -1
        if self._split == 'auto':
            self._split = -1

        check = True
        if self._filter_name == 'continuous_balancing' or \
                self._filter_name == 'discrete_balancing' or \
                self._filter_name == 'groups_balancing':
            if not(self.check_cut(class_ind)):
                check = False
                if self._filter_name == 'mixture':
                    class_ind = self.mixture()
        if self._nbfolds == -1 or self._split == -1 or not(check):
            self.define_cut(class_ind)
        return self.db_filter(class_ind)

    def discrete_balancing(self):
        '''Discrete targets'''
        classes = numpy.unique(self.getY())
        class_ind = []
        for i in range(len(classes)):
            class_ind.append([])
            class_ind[i] = numpy.nonzero(self.getY() == classes[i])[0].tolist()
        return class_ind

    def continuous_balancing(self):
        '''Continuous targets : Find another solution!!!!'''
        y = numpy.round(self.getY())
        class_ind = []
        classes = numpy.unique(y)

        for i in range(len(classes)):
            class_ind.append([])
            class_ind[i] = numpy.nonzero(y == classes[i])[0].tolist()
        return class_ind

    def mixture(self, random_seed=None):
        class_ind = []
        if random_seed:
            perm = random_seed
        else:
            perm = list(range(self._rows_number))
            numpy.random.shuffle(perm)

        class_ind.append(perm)
        return class_ind

    def do_nothing(self, random_seed=None):
        class_ind = []
        if random_seed:
            perm = random_seed
        else:
            perm = list(range(self._rows_number))
        class_ind.append(perm)
        return class_ind

    def check_cut(self, class_ind):
        '''Check or Define the cut of the database.'''
        check = True
        for i in range(len(class_ind)):
            nb_points = len(class_ind[i])
            if self._nbfolds == 1:
                if (1. / nb_points) > self._split:
                    check = False
                    msg.warning("\nThere is a pb with "
                                "the given test's part.")
            elif self._nbfolds > 1:
                if nb_points < self._nbfolds:
                    check = False
                    msg.warning("\n There is a pb with "
                                "the given number of folds.")
            if nb_points == 1:
                msg.warning("\n One class has only one element."
                            " So mixture filter has been choose.")
                self._filter_name = 'mixture'
                return False
        return check

    def define_cut(self, class_ind):
        '''Define the cut of the database.'''
        if self._filter_name == 'continuous_balancing' or \
           self._filter_name == 'discrete_balancing':
            class_len = [len(class_ind[i])
                         for i in range(len(class_ind))]
            if self._nbfolds == -1 or self._split == None:
                self._nbfolds = min(class_len)
            else:
                self._split = 1.0 / min(class_len)
        elif self._filter_name == 'mixture' or \
                self._filter_name == 'nothing':
            density = float(max(self.getY()[:, 0]) -
                            min(self.getY()[:, 0]) / self._rows_number)
            if self._nbfolds == -1 or self._split == None:
                self._nbfolds = int(round(density))
            else:
                self._split = 1. / density

        if self._nbfolds <= 1:
            self._nbfolds = 2
        elif self._split >= 1:
            self._split = 0.5

        if self._split == None:
            info = "\nThe option 'auto' give %i number of folds." \
                % self._nbfolds
        else:
            info = "\nThe option 'auto' give a test set of"\
                " %d of the data database." % self._split
        msg.info(info)

    def db_filter(self, class_ind):
        '''
Then cut the database and create the train/test set(s).
        '''
        db_list_filtered = []
        fold_start = numpy.zeros(self._nbfolds + 1, int)

        for i in range(self._nbfolds):

            train_ind = []
            test_ind = []

            for j in range(len(class_ind)):
                nb_points = len(class_ind[j])

                if self._nbfolds == 1:
                    test_nbP = nb_points * self._split
                    if numpy.floor(test_nbP) != test_nbP:
                        if j % 2 == 0:
                            test_nbP =\
                                numpy.floor(test_nbP)
                        else:
                            test_nbP =\
                                numpy.ceil(test_nbP)
                    test_nbP = int(test_nbP)
                    train_nbP = nb_points - test_nbP
                    temp = class_ind[j]

                    test_ind = test_ind +\
                        class_ind[j][0:test_nbP]
                    train_ind = train_ind +\
                        class_ind[j][test_nbP:nb_points]

                else:
                    fold_start = [k * nb_points /
                                  self._nbfolds
                                  for k in range(self._nbfolds + 1)]
                    begin = fold_start[i]
                    end = fold_start[i + 1]
                    train_ind = train_ind +\
                        class_ind[j][0:begin] +\
                        class_ind[j][end:]
                    test_ind = test_ind +\
                        class_ind[j][begin:end]

            db_list_filtered.append({'train_ind': train_ind,
                                     'test_ind': test_ind})

        return db_list_filtered
