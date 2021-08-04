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
import six


class OFunc(MlObject):

    '''Base class of objective functions.'''
    pass


class ModelOFunc(OFunc):

    '''
Objective function based on a model and one or more databases.

model :      the base model.
validation : validation : cross validation, single validation...
criterion :  eval criterion, depends on model (classifier or regressor).
criterion_parameters :
             dictionnary of parameters passed to criterion computation
             function.
user_data :  any data user want to pass to its eval function.
fun :        user function defining eval objective function based on model
             rates, it follows this prototype :
                 val = fun(ofunc, res, eval_data, user_data)
             where ofunc is the current objective function, res : the result
             of validation method, eval_data are data passed to eval ofunc
             method (optimizer data for instance) and user_data are data
             passed initializing ofunc (classifier data for instance).

Currently you can change model after creating this objective function. It's
usefull for optimizers that interacts deeply with model (for instance when
model methods are needed during optimization.'''

    def __init__(self, model, validation, criterion,
                 criterion_parameters={}, user_data=None, fun=None):
        self._model = model
        self._validation = validation
        self._criterion = criterion
        self._criterion_parameters = criterion_parameters
        self._user_data = user_data
        self._fun = fun

    def getModel(self):
        return self._model

    def setModel(self, model):
        self._model = model

    def setParams(self, params):
        self._model.setParams(params)

    def getValidation(self):
        return self._validation

    def eval(self, params=None, eval_data=None):
        '''
Eval value of objective function at given parameters values. Each call of
this method takes along to a learning/fitting (with or without cross
validation) call of the internal model.

params :     parameters values of objective function.
eval_data :  optional data passed to user function if it was defined in
         ofunc constructor.'''
        fitres = val = None
        if params:
            self.setParams(params)
        res = self._validation.run(self._model)
        if res == -1:
            return {'value': -1, 'other': None}
        if self._fun:
            return self._fun(self, res, eval_data, user_data)
        try:
            res_predict = res['predict'][0]
            value = res_predict.compute(self._criterion,
                                        **self._criterion_parameters)
            return {'value': value, 'other': res}
        except KeyError:
            six.reraise(
                ValueError,
                ValueError("'" + self._criterion
                           + "' bad eval strategy."))


class FromOptimizer(OFunc):

    '''
Combine One optimizer with an ofunc to create an other ofunc. This trick
is usefull to plug several optimizers on another ones.'''

    def __init__(self, optimizer, ofunc):
        '''
optimizer :  an optimizer
ofunc :      an objective function.  '''
        self._optimizer = optimizer
        self._ofunc = ofunc

    def eval(self, params=None):
        '''
Here value of objective function is the returned value of the optimizer on
the internal objective function definied in constructor.

params :     parameters values of objective function.'''
        if params:
            self._ofunc.setParams(params)
        res = self._optimizer.optimize(self._ofunc)
        return res

    def getOFunc(self):
        return self._ofunc
