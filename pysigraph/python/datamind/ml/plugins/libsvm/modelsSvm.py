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

from __future__ import print_function

# fix find_library so that it finds libsvm in a non-system path
from __future__ import absolute_import
from soma.utils import find_library
find_library.patch_ctypes_find_library()
import svm
import numpy
from datamind.ml import classifier


class BaseLibSvm(classifier.Classifier):

    '''Base class of SVM models based on libsvm python bindings.'''

    def __init__(self, svm_type, kernel_type, seeds=0):
        classifier.Classifier.__init__(self)
        self._seeds = seeds
        self._svm_model = None
        self._params = dict(svm.svm_parameter.default_parameters)
        self._params['kernel_type'] = kernel_type
        self._params['svm_type'] = svm_type
        self._params['gamma'] = 1
        self.setParams({})

    def _check_databases(self, X, Y):
        '''
Check database : shape verification on X and Y.
Raise ValueErrors if failed.'''
        import exceptions
        if len(X.shape) != 2:
            raise exceptions.ValueError('X must have 2 dimension.')
        if Y is not None and len(Y.shape) != 1 and Y.shape[1] != 1:
            raise exceptions.ValueError('Y must have 1 dimension.')

    def fit(self, train):
        '''
Fit SVM Model.

train :  train database.'''
        import time
        X, Y = train.getX(), train.getY()
        self._check_databases(X, Y)
        prob = svm.svm_problem(Y.ravel(), X)
        t1 = time.time()
        self._svm_model = svm.svm_model(prob, self._svm_params)
        t2 = time.time()
        return

    def predict(self, test):
        '''
Predict svm Model on test database.

test :   test database.

Return prediction results on test database : including predicted values'''
        X, Y = test.getX(), test.getY()
        self._check_databases(X, Y)
        if Y is not None:
            Y = Y.ravel()
        target = [self._svm_model.predict(x) for x in X]
        return self.computeResult(Y, target)

    def cv(self, train, nbfolds):
        '''
train :    train database.
nbfolds :  number of folds of crossvalidation.

Return crossvalidation results (depends on model).'''
        X, Y = train.getX(), train.getY()
        self._check_databases(X, Y)
        Y = Y.ravel()
        prob = svm.svm_problem(Y, X)
        numpy.random.seed(self._seeds)
        target = svm.cross_validation(prob,
                                      self._svm_params, nbfolds)
        return self.computeResult(Y, target)

    def setParams(self, params):
        '''
params : dictionary of parameters

see self.listParams() for available parameters.'''
        # FIXME : add other parameters
        params = params.get('model', params)
        self._check_params(params)
        self._params['gamma'] = params.get('gamma', self._params['gamma'])
        self._params['C'] = params.get('C', self._params['C'])
        self._params['p'] = params.get('epsilon', self._params['p'])
        self._svm_params = svm.svm_parameter(**self._params)

    def listParams(self):
        '''List avalaible parameters for all SVM models.'''
        return ['gamma', 'C', 'epsilon']

    def clone(self):
        '''
Copy current SVM model (only copy parameters, not model itself).

Return copy.'''
        return BaseLibSvm(self._svm_type, self._kernel_type)


class CSvcLibSvm(BaseLibSvm):

    '''
C-SVC Model.

kernel_type : from python module svm, one of LINEAR, POLY, RBF, SIGMOID.
weight :      dictionary of weights : (i.e, {0 : 5., 1 : 0.1})
'''

    def __init__(self, kernel_type, weights=None):
        BaseLibSvm.__init__(self, svm.C_SVC, kernel_type)
        if weights:
            self._params['nr_weight'] = len(weights)
            self._params['weight_label'] = \
                numpy.array(list(weights.keys())).astype('int')
            print(
                self._params['weight_label'], self._params['weight_label'].dtype)
            self._params['weight'] = list(weights.values())
            self.setParams({})
        self._setFilter('discrete_balancing')

    def _format_target(self, target):
        '''Force dtype of target (predicted values).'''
        import numpy
        return numpy.array(target, dtype='i4')

    def computeResult(self, Y, target):
        '''Compute classifier results.'''
        from datamind.ml.classifier import \
            WeightedBinaryClassifierResult
        return WeightedBinaryClassifierResult(Y,
                                              self._format_target(target))

    def signature(self):
        '''Return model signature.'''
        return {'model': 'C SVC'}

    def listParams(self):
        '''List avalaible parameters for C-SVC model.'''
        return ['gamma', 'C']

    def list_criterions(self):
        '''List optimizable criterions.'''
        return WeightedBinaryClassifierResult.list_criterions()


class ESvrLibSvm(BaseLibSvm):

    def __init__(self, kernel_type):
        BaseLibSvm.__init__(self, svm.EPSILON_SVR, kernel_type)
        self._setFilter('continuous_balancing')

    def _format_target(self, target):
        '''Force dtype of target (predicted values).'''
        import numpy
        return numpy.array(target)

    def computeResult(self, Y, target):
        '''Compute regressor results.'''
        from datamind.ml.classifier import WeightedRegressorResult
        return WeightedRegressorResult(Y, self._format_target(target))

    def signature(self):
        '''Return model signature.'''
        return {'model': 'Epsilon SVR'}

    def listParams(self):
        '''List avalaible parameters for E-SVR model.'''
        return ['gamma', 'C', 'epsilon']

    def list_criterions(self):
        '''List optimizable criterions.'''
        return WeightedRegressorResult.list_criterions()
