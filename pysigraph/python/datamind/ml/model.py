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
from numpy import ones
from datamind.ml import wip
from datamind.ml.mlObject import MlObject
from six.moves import range

#


class Model(MlObject):

    def __init__(self):
        self._filter_name = 'nothing'
        pass

    def fit(self, train):
        '''Fit model on train data : X|Y -> Y = f(X).'''
        wip.not_implemented()

    def predict(self, test):
        '''Predict model on test data : f|X -> Y = f(X).'''
        wip.not_implemented()

    def _check_params(self, params):
        '''
Check if params are available parameters for the concerned model.'''
        unknown_params = []
        for p in params.keys():
            if not p in self.listParams():
                unknown_params.append(p)
        if len(unknown_params):
            raise ValueError("Unknown parameter(s) '%s'."
                             "See self.listParams() for available ones." %
                             unknown_params)

    def listParams(self):
        '''List available parameters for the present model.'''
        wip.not_implemented()

    def _setFilter(self, filter_name, random_seed=None):
        '''
        Indicate the the type of balancing to be applied to the data.
        '''
        self._filter_name = filter_name
        self._random_seed = random_seed

#


class PrependedModel(Model):

    def __init__(self):
        pass

    def computeResult(self, Y, target):
        return self._model.computeResult(Y, target)

    def _format_target(self, target):
        return self._model._format_target(target)

    def _check_params(self, params):
        '''
Check if params are available parameters for the prepended model.'''
        model_listParams = {}
        for p in params.keys():
            if not p in self.listParams():
                self._model._check_params(params)
                model_listParams.__setitem__(p, params[p])
                params.pop(p)
        if model_listParams.__len__():
            params.__setitem__('model', model_listParams)
        return params
#


class DimReductionModel(PrependedModel):

    '''
Prepend a dimension reduction step before feeding a model with these data.
    '''

    def __init__(self, dr, model, skip_dimred_fitting=False):
        '''
dr :     dimension reduction instance.
model :  model instance (classifier, regressor, clustering).
skip_dimred_fitting : dr is used only to reduce data, so dr must have been
                  learned before.'''
        self._dr = dr
        self._model = model
        self._skip_dimred_fitting = skip_dimred_fitting
        self._selected_dim = -1
        self._model_params = {}
        self._setFilter(self._model._filter_name)

    def fit(self, train):
        '''
Reduce train data (use get_reduced_train on dimension reduction model if
available) and fit model on it. 'dim' parameter is used to reduced
dimension.

train : train database.'''
        train_reduced = self.fit_reduce(train)
        return self._model.fit(train_reduced)

    def fit_reduce(self, train):
        if not self._skip_dimred_fitting:
            self._dr.fit(train)
        # FIXME : stocker les data reduites ?
        try:
            train_reduced = self._dr.get_reduced_train(train,
                                                       self._selected_dim)
        except AttributeError:
            train_reduced = self._dr.reduce(train,
                                            self._selected_dim)
        return train_reduced

    def predict(self, test):
        '''
Reduce test data thanks to dimension reduction model and predict model on
it. 'dim' parameter is used to reduced dimension.

test : test database.'''
        test_reduced = self._dr.reduce(test, self._selected_dim)
        # FIXME : stocker les data reduites ?
        return self._model.predict(test_reduced)

    def setParams(self, params):
        '''
Set Params (see self.listParams() for available parameters).

params : dictionary of parameters.'''
        # FIXME : ajouter les parametres du model aussi ?
        # par exemple en indexant les parametres, afin d'eviter les
        # conflits : i.e : {'dim' : 3, 'model.C' : 10}
        # ou plutot un id si self._model est un composite aussi
        params = self._check_params(params)
        self._selected_dim = params.get('dim', self._selected_dim)
        self._model_params = params.get('model', self._model_params)
        self._model.setParams({'model': self._model_params})

    def listParams(self):
        '''List avalaible parameters for dim reduction model'''
        return ['dim', 'model']


#
class MultiDimReductionModel(DimReductionModel):

    '''
Prepend several dimension reductions step before feeding a model with these data.
    '''

    def __init__(self, dr_list, model, skip_dimred_fitting=False):
        '''
dr :     list of dimension reduction instances.
model :  model instance (classifier, regressor, clustering).
skip_dimred_fitting : dr is used only to reduce data, so dr must have been
                  learned before.'''
        DimReductionModel.__init__(
            self, None, model, skip_dimred_fitting=False)
        self._dr_list = dr_list
        self._selected_dims = -1 * ones(len(self._dr_list))

    def fit(self, train):
        '''
Reduce train data (use get_reduced_train on dimension reduction model if
available) and fit model on it. 'dim' parameter is used to reduced
dimension.

Check that tha the reduced dimensions are sorted in descending order

train : train database.'''
        train_reduced = train
        for i in range(len(self._dr_list)):
            self._selected_dim = self._selected_dims[i]
            self._dr = self._dr_list[i]
            train_reduced = self.fit_reduce(train_reduced)
        return self._model.fit(train_reduced)

    def predict(self, test):
        '''
Reduce test data thanks to dimension reduction model and predict model on
it. 'dim' parameter is used to reduced dimension.

test : test database.'''
        test_reduced = test
        for i in range(len(self._dr_list)):
            test_reduced = self._dr_list[i].reduce(
                test_reduced, self._selected_dims[i])
        return self._model.predict(test_reduced)

    def check_selected_dims(self):

        self._selected_dims = sorted(self._selected_dims, reverse=True)

    def setParams(self, params):
        params = self._check_params(params)
        self._selected_dim = params.get('dim', self._selected_dims)
        self._model_params = params.get('model', self._model_params)
        self._model.setParams({'model': self._model_params})

#


class OptimizerModel(PrependedModel):

    """
    Prepend an optimization step before feeding a model with these data.

    How to use it?
    >> Define a model :
            clf_optModel = classifier.ESvrLibSvm(RBF)
    >> Define an OFunc 'FromOptimizer' (offo):
            - Define an optimizer :
                    ex : opt = optimizers.RefineGrid(ranges,strategy)
            - Define an OFunc :
                    ex : of = ofunc.ModelOFunc(clf, val, criterion)
            => offo = ofunc.FromOptimizer(opt, of)
    optModel = OptimizerModel(offo, clf_optModel)
    """

    def __init__(self, of, model):
        self._model = model
        self._offit = of
        self._model_params = {}
        self._setFilter(self._model._filter_name)
        self._compteur = 0

    def fit(self, train):
        '''
We fit the model with the optimized parameters calculated in the
eval function of the OFunc 'of'.

self._compteur : count the number of call of the function fit.
        Because we want to calculate the optimal parameters one time
        (For example for the first train fold of a cv)
        '''
        res = None
        self._offit.getOFunc().getValidation().setDB(train)

        if self._compteur == 0:
            res = self._offit.eval()
            params = {'model': res['best_params']}
            self._model.setParams(params)
            self._compteur = self._compteur + 1
        self._model.fit(train)
        return res

    def predict(self, test):
        return self._model.predict(test)

    def setParams(self, params):
        params = self._check_params(params)
        self._model_params = params.get('model', self._model_params)
        self._model.setParams({'model': self._model_params})

    def listParams(self):
        '''List avalaible parameters for OFunc model'''
        return ['model']

#


class ValidationModel(PrependedModel):

    """
    Prepend a validation step before feeding a model with these data.

    How to use it?
    >> Define a model :
            clf_model = classifier.ESvrLibSvm(RBF)
    >> Define a validation :
            val = validation.LeaveOneOut(db)
    valModel = ValidationModel(val, clf_model)
    """

    def __init__(self, val, model):
        self._model = model
        self._val = val
        self._model_params = {}
        self._setFilter(self._model._filter_name)

    def fit(self, train):
        '''
We fit the model with the validation 'val'.
        '''
        self._val._db = train
        self._val.run(self._model)
        self._model.fit(train)

    def predict(self, test):
        return self._model.predict(test)

    def setParams(self, params):
        params = self._check_params(params)
        self._model_params = params.get('model', self._model_params)
        self._model.setParams({'model': self._model_params})

    def listParams(self):
        '''List avalaible parameters for Validation model'''
        return ['model']
