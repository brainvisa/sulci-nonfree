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
from numpy import zeros, inf, array, arange, sqrt
from datamind.ml.mlObject import MlObject
from datamind.ml import wip
from six.moves import range
from six.moves import zip


class Optimizer(MlObject):

    '''Base class for model optimizers.'''

    def _set_strategy(self):
        def islower(err, err_best): return (err < err_best)

        def isgreater(err, err_best): return (err > err_best)
        best_val = isbetter = None
        if self._strategy == 'min':
            best_val = inf
            isbetter = islower
        elif self._strategy == 'max':
            best_val = -inf
            isbetter = isgreater
        return best_val, isbetter


class Avoid(Optimizer):

    '''
This is not really an optimizer since it only calls model's fit function,
but it sould be handy when you want to inhibit the optimizer step of a
pipeline process.'''

    def __init__(self): pass

    def optimize(self, ofunc):
        eval = ofunc.eval()
        return eval


class Iterate(Optimizer):

    '''
This Optimizer is a generic model optimizer. It is based on another
optimizer. It clones a model n times and then each of the resulting models
is initialized, learned. Finally, the better one is kept and return with
the corresponding error.

n :            number of tries.
strategy :     'min' : keep minimal valuated model.
               'max' : keep maximal valuated model.'''

    def __init__(self, n, optimizer=None, strategy='min'):
        self._n = n
        self._strategy = strategy

    def optimize(self, ofunc):
        best_model = None
        model = ofunc.getModel()
        best_val, isbetter = self._set_strategy()
        for i in range(self._n):
            m = model.clone()
            m.init()
            ofunc.setModel(m)
            res = ofunc.eval()
            val = res['value']
            if isbetter(val, best_val):
                best_val, best_model = val, m
        return {'best_model': best_model, 'value': best_val}


class Grid(Optimizer):

    def __init__(self, ranges, fun=None,
                 user_data=None, strategy='min'):
        '''
Cover a discrete parameter space defined by 'ranges' to optimize a set
of model parameters over train and test databases.

ranges : list (one per parameter) of list (value taken by this parameter).
strategy :     'min' : minimize objective function over grid.
           'max' : maximize objective function over grid.
fun :          user function called for each grid element with
           user_data parameter.
user_data :    data passed to user function.'''
        from datamind.ml.explorer import GridExplorer
        self._ranges = ranges
        self._gridExplorer = GridExplorer(ranges)
        self._strategy = strategy
        self._fun = fun
        self._user_data = user_data

    def _local_optimize(self, indexes, params, user_data):
        ofunc = user_data['ofunc']
        eval = ofunc.eval(params.copy())
        val = eval['value']
        isbetter = user_data['isbetter']
        user_data['array res'][tuple(indexes)] = (val,)
        if user_data['fun']:
            user_data['fun'](eval, user_data['user_data'])
        if isbetter(val, user_data['best_val']):
            user_data['best_val'] = val
            user_data['best_params'] = params.copy()
            user_data['ofunc res'] = eval

    def optimize(self, ofunc):
        import numpy
        best_val, isbetter = self._set_strategy()
        best_params = {}
        best_params.update(list(zip(list(self._ranges.keys()),
                               (x[0] for x in self._ranges.values()))))
        t = numpy.dtype([('val', float)])
        shape = [len(v) for v in self._ranges.values()]
        a = zeros(shape, dtype=t)
        user_data = {'ofunc': ofunc, 'best_val': best_val,
                     'best_params': best_params, 'ofunc res': None,
                     'isbetter': isbetter, 'array res': a,
                     'fun': self._fun, 'user_data': self._user_data}
        self._gridExplorer.explore(self._local_optimize, user_data)

        return {'best_params': user_data['best_params'],
                'value': user_data['best_val'],
                'ofunc res': user_data['ofunc res'],
                'array res': user_data['array res']}


# FIXME : a virer, je laisse ca la parce que ca serait sympa de generer
# automatiquement les ranges de parametre
class __Grid_old(Optimizer):

    def _initialize(self, centers, nvalues, step):
        if centers.size != nvalues.size or \
                centers.size != step.size or \
                nvalues.size != step.size:
            raise AssertionError
        self._step = step
        self._max = centers + ((nvalues + 1) / 2.) * step
        self._min = centers - ((nvalues - 1) / 2.) * step
        self._dim = centers.size


class RefineGrid(Grid):

    """
    Reiterate by refinement a grid search of the best model parameters :
    We first use a coarse grid. After identifying a "better" region
    on the grid, a finer grid search on that region can be conducted.

    _step_ranges : list (for each parameters) of the ranges of the successive\
                   steps necessary to create the refined grid around the\
                   optimzed model parameters.
    _nbRound : number of iteration or length of the _step_ranges
    """

    def setRound(self, nbRound):
        self._nbRound = nbRound

    def optimize(self, ofunc):
        old_best_params = []
        ranges_init = self._ranges.copy()
        array_res = {'ranges': [], 'best_params': {}, 'res_temp': []}
        self.type_params = float

        while True:

            nbround = 0

            ranges_val = list(self._ranges.values())

            res = Grid(self._ranges, strategy=self._strategy).optimize(ofunc)

            best_params = list(res['best_params'].values())
            array_res['ranges'].append(self._ranges.copy())
            array_res['best_params'] = res['best_params']
            array_res['res_temp'].append(res['array res'].copy())
            for i in range(len(best_params)):
                interval = []
                if len(old_best_params) > 0 and \
                        best_params[i] == old_best_params[i]:
                    nbround = nbround + 1
                else:
                    ind = ranges_val[i].index(best_params[i])
                    if ind - 1 >= 0:
                        interval.append(ranges_val[i][ind - 1])
                    interval.append(best_params[i])
                    if ind + 1 < len(ranges_val[i]):
                        interval.append(ranges_val[i][ind + 1])

                    self._ranges[list(self._ranges.keys())[i]] = \
                        self.param_sequence_create(interval)

            if nbround == len(best_params):
                for i in range(len(best_params)):
                    self._ranges[list(self._ranges.keys())[i]] = [best_params[i]]
                break

            old_best_params = best_params

        self._ranges = ranges_init
        return array_res

    def setTypeParams(self, type_params):
        self.type_params = type_params

    def param_sequence_create(self, interval):
        seq = [self.type_params(interval[0])]
        for i in range(len(interval) - 1):
            # center = (interval[i+1] - interval[i]) /2.0
            # seq.extend( [ interval[i] + center, interval[i+1]])
            center = sqrt(interval[i + 1] * interval[i])
            seq.extend(
                [self.type_params(center), self.type_params(interval[i + 1])])
        return seq
