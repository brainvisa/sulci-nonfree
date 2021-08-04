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

# FIXME : ajouter la notion de "reduire la dimension au sens d'une certaine
# fonction objectif

# Abstract classes


class DimensionReduction(Model):

    def __init__(self, train=None):
        '''
Dimension reduction class. If train database is not None, fit method is
called on it.'''
        Model.__init__(self)
        if train:
            self.fit(train)

    def reduce(self, db):
        '''
Alias to predict function : apply dimension reduction of db'''
        return self.predict(self, db)


class FeatureSelection(DimensionReduction):

    def fit(self, train, view=False):
        '''
Fit Feature selection model on train data.

train : database
view :  boolean
    True : return a view on train database.
    False : return a new database.'''
        wip.not_implemented()

    def reduce(self, db, view=False):
        '''
Fit Feature selection model on train data.

db :    database
view :  boolean
    True : return a view on train database.
    False : return a new database.'''
        wip.not_implemented()


class Embedding(DimensionReduction):
    pass


class DimensionReductionPipeline(DimensionReduction):

    def __init__(self, dr_list, train=None):
        '''
Dimension reduction pipeline : pipe several dimension reduction techniques
on a dataset.

dr_list : list of dimension reduction instance to pipe
train :   if not None, fit the pipeline model on this database.'''
        DimensionReduction.__init__(self)
        self._dr_list = dr_list
        if train:
            self.fit(train)

    def fit(self, train, *args, **kwargs):
        '''
Fit each dimension reduction techniques to the according nested database.

*args, **kwargs : arguments passed to fit/reduce method of each dimension
              dimension techniques.'''
        db_reduced = train
        for dr in self._dr_list:
            dr.fit(db_reduced, *args, **kwargs)
            db_reduced = dr.reduce(db_reduced, *args, **kwargs)
        return db_reduced

    def reduce(self, db, *args, **kwargs):
        '''
Reduce database thanks to fitted dimension reduction techniques.

*args, **kwargs : arguments passed to reduce methods of each dimension
              dimension techniques.'''
        db_reduced = db
        for dr in self._dr_list:
            db_reduced = dr.reduce(db_reduced, *args, **kwargs)
        return db_reduced
