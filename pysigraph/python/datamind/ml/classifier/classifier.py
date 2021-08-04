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

from __future__ import absolute_import
from datamind.ml.model import Model
from datamind.ml import wip
from datamind.tools import msg
from six.moves import range
from six.moves import zip


class Classifier(Model):

    def list_criterions(self):
        return WeightedBinaryClassifierResult.list_criterions()


class Regressor(Model):

    def list_criterions(self):
        return WeightedRegressorResult.list_criterions()


#
class Result(object):

    def list_criterions(cls):
        '''List all available criterions through this Result.'''
        return list(cls._compute_criterions.keys())
    list_criterions = classmethod(list_criterions)

    def compute(self, criterion, *args, **kwargs):
        '''
Compute given criterion (string).

*args, **kwarsg : pass to criterion evaluation function.'''
        try:
            compute_fun = self._compute_criterions[criterion]
            return compute_fun(self, *args, **kwargs)
        except TypeError as e:
            msg.error("Criterion computation function need some "
                      "additional parameters.")
            msg.info('doc : ')
            print(compute_fun.__doc__)
            raise TypeError(e)


class ClassifierResult(Result):

    def __init__(self, true_values=None, predict_values=None):
        '''
true_values is copied, predict_values is stored.
        '''
        import numpy
        self.true_values = numpy.array(true_values, copy=True)
        self.predict_values = numpy.array(predict_values, copy=False)


#
class BinaryClassifierResult(ClassifierResult):

    def __init__(self, true_values=None, predict_values=None):
        '''
Result of a predict process of a binary classifier (2 classes problem).
Only labels parameter is mandatory, other ones can be computed quite quickly
(true/false_positive/negative parameters) or very quickly (rates,
specificity and sensitivity), thanks to these parameters plus tested data.

true_values :            list or array of true classes
predict_values :         list of array of predicted classes.'''
        import numpy
        ClassifierResult.__init__(self, true_values, predict_values)
        self.true_positive = numpy.nan
        self.false_positive = numpy.nan
        self.true_negative = numpy.nan
        self.false_negative = numpy.nan
        self.sensitivity = numpy.nan
        self.specificity = numpy.nan
        self.raw_classification_rate = numpy.nan
        self.classification_rates = {}

    def compute_all(self):
        '''Evaluate all criterions.'''
        if self.true_values is None:
            return
        for c, f in BinaryClassifierResult._compute_criterions.items():
            f()

    def compute_raw_classification_rate(self):
        size = len(self.true_values)
        total_correct = (self.predict_values == self.true_values).sum()
        self.raw_classification_rate = float(total_correct) / size
        return self.raw_classification_rate

    def compute_classification_rates(self):
        '''
Compute classification rates for each class.
        '''
        import numpy
        classes = numpy.unique1d(self.true_values)
        for c in classes:
            self.compute_classification_rate(c)
        return self.classification_rates

    def compute_classification_rate(self, label):
        '''
Compute classification rate for one class.

label :   indice of class.
        '''
        ind = (self.true_values == label)
        p = self.predict_values[ind]
        t = self.true_values[ind]
        size = len(p)
        total_correct = (p == t).sum()
        self.classification_rates[label] = float(total_correct) / size
        return self.classification_rates[label]

    def setValues(self, sensitivity=None, specificity=None,
                  true_positive=None, false_positive=None,
                  true_negative=None, false_negative=None,
                  raw_classification_rate=None,
                  classification_rates=None):
        '''
true_positive :  positive predicted label which are really positive label.
false_positive : positive predicted label which are not positive label.
true_negative :  negative predicted label which are really negative label.
false_negative : negative predicted label which are not negative label.

raw_classification_rate :      classification rate of all predicted data
                           without weighting corrections.
classification_rates :          dictionnary of classification rate

sensitivity :    true_positive / (true_positive + false_negative).
specificity :    true_negative / (false_positive + true_negative).'''
        if true_positive != None:
            self.true_positive = true_positive
        if false_positive != None:
            self.false_positive = false_positive
        if true_negative != None:
            self.true_negative = true_negative
        if false_negative != None:
            self.false_negative = false_negative
        if sensitivity != None:
            self.sensitivity = sensitivity
        if specificity != None:
            self.specificity = specificity
        if raw_classification_rate != None:
            self.raw_classification_rate = raw_classification_rate
        if classification_rates != None:
            self.classification_rates = \
                classification_rates

    def __repr__(self):
        s = 'sensitivity : %f, specificity : %f, ' \
            % (self.sensitivity, self.specificity)
        s += 'true positive : %f, false positive : %f, ' \
            % (self.true_positive, self.false_positive)
        s += 'true negative : %f, false negative : %f, ' \
            % (self.true_negative, self.false_negative)
        s += 'raw classification rate : %f, ' \
            % (self.raw_classification_rate)
        s += 'classification rates : %s, ' \
            % str(self.classification_rates)
        return s

    _compute_criterions = {}
    _compute_criterions['raw_classification_rate'] = \
        compute_raw_classification_rate
    _compute_criterions['classification_rate'] = \
        compute_classification_rate


#
class WeightedBinaryClassifierResult(BinaryClassifierResult):

    def __init__(self, true_values=None, predict_values=None):
        BinaryClassifierResult.__init__(self, true_values,
                                        predict_values)
        import numpy
        self.weighted_classification_rate = numpy.nan

    def compute_all(self, weights=None):
        BinaryClassifierResult.compute(self)
        if self.true_values is None:
            return
        if self.weights is None:
            return
        criterions = WeightedBinaryClassifierResult._compute_criterions
        for c, f in criterions.items():
            if c == 'weighted_classification_rate':
                f(weights)
            else:
                f()

    def compute_weighted_classification_rate(self, weights=None):
        '''
Compute weighted classification rate.

weights : None :    do nothing.
      'auto' :  reweighted each class according to balance its size
      List :    one weight for each data sample
                (ex : [1.1, 3.5, 4.2...])
      Dict :    dictionnary of weights for each classes. You must
                specify all classes else strange results can occure.
                (ex : {1 : 0.2, 'a' : 0.5, 2.2 : 0.1})'''
        import numpy
        if weights is None:
            return None
        elif weights == 'auto':
            weights = {}
            print("uni:", numpy.unique(self.true_values))
            for c in numpy.unique(self.true_values):
                print("sum:", (self.true_values == c).sum())
                weights[c] = 1. / (self.true_values == c).sum()
        if type(weights) is dict:
            e = numpy.array(list(weights.values()))
            s = e.sum()
            if s != 1:
                weights = dict(list(zip(list(weights.keys()), e / s)))
            z = numpy.zeros_like(self.true_values).astype('double')
            for c in numpy.unique(self.true_values):
                z += (self.true_values == c) * weights[c]
            weights = z
        try:
            weights.__iter__
        except:
            raise ValueError('invalid weights')
        ind = (self.true_values == 0).ravel()
        diff = (self.true_values == self.predict_values).astype('i2')
        self.weighted_classification_rate = float(numpy.dot(
            diff, weights)) / weights.sum()
        return self.weighted_classification_rate

    def setValues(self, sensitivity=None, specificity=None,
                  true_positive=None, false_positive=None,
                  true_negative=None, false_negative=None,
                  raw_classification_rate=None,
                  c0_classification_rate=None,
                  c1_classification_rate=None,
                  weighted_classification_rate=None):
        BinaryClassifierResult.setValues(self,
                                         sensitivity, specificity, true_positive, false_positive,
                                         true_negative, false_negative, raw_classification_rate,
                                         c0_classification_rate, c1_classification_rate)
        if weighted_classification_rate != None:
            self.weighted_classification_rate = \
                weighted_classification_rate

    def __repr__(self):
        s = BinaryClassifierResult.__repr__(self)
        s += ', weighted classification rate : %f' \
            % self.weighted_classification_rate
        return s

    _compute_criterions = dict(BinaryClassifierResult._compute_criterions)
    _compute_criterions['weighted_classification_rate'] = \
        compute_weighted_classification_rate


#
class RegressorResult(Result):

    def __init__(self, true_values=None, predict_values=None):
        '''
Results of regressor prediction.

true_values :            list of true values
predict_values :         list of predicted values.

true_values is copied, predict_values is stored.
        '''
        import numpy
        self.true_values = numpy.array(true_values, copy=True)
        self.predict_values = numpy.array(predict_values, copy=False)
        self.mse = numpy.nan

    def compute_all(self):
        if self.true_values is None:
            return
        self.compute_mse()
        # FIXME : completer

    def compute_mse(self):
        total_error = sumv = sumy = sumvv = sumyy = sumvy = 0.
        size = len(self.true_values)
        for i in range(size):
            v = self.predict_values[i]
            y = self.true_values[i]
            sumv += v
            sumy += y
            sumvv += v * v
            sumyy += y * y
            sumvy += v * y
            d = (v - y)
            total_error += d * d
        self.mse = total_error / size
        return self.mse

    def setValues(self, mse=None):
        ''' mse :        mean square error.'''
        if mse != None:
            self.mse = mse

    def __repr__(self):
        s = 'mse : %f' % (self.mse)
        return s

    _compute_criterions = {}
    _compute_criterions['mse'] = compute_mse


#
class WeightedRegressorResult(RegressorResult):

    def __init__(self, true_values=None, predict_values=None):
        import numpy
        RegressorResult.__init__(self, true_values, predict_values)
        self.wmse = numpy.nan

    def compute_all(self):
        RegressorResult.compute(self)
        if self.true_values is None:
            return
        # FIXME : completer

    def compute_wmse(self, weights):
        import numpy
        try:
            it = weights.__iter__
        except:
            raise TypeError("weights must be iterable")
        if len(numpy.shape(weights)) != 1:
            raise ValueError("weights must be 1-shaped")
        true_values = self.true_values.ravel()
        total_error = sumv = sumy = sumvv = sumyy = sumvy = 0.
        size = len(self.true_values)
        total_w = numpy.array(weights).sum()
        t = numpy.asarray(self.true_values)
        p = numpy.asarray(self.predict_values)
        self.wmse = numpy.dot((t - p) ** 2, weights) / total_w
        return self.wmse

    def setValues(self, mse=None, wmse=None):
        '''
Results of weighted regressor prediction.

mse :        mean square error.
wmse :       weighted mean square error.'''

        RegressorResult.setValues(self, mse)
        if wmse != None:
            self.wmse = wmse

    def __repr__(self):
        s = RegressorResult.__repr__(self)
        s += ', weighted mse : %f' % self.wmse
        return s

    _compute_criterions = \
        dict(RegressorResult._compute_criterions)
    _compute_criterions['wmse'] = compute_wmse


#
class GroupsRegressorResult(RegressorResult):

    def __init__(self, true_values=None, predict_values=None):
        import numpy
        RegressorResult.__init__(self, true_values, predict_values)
        self.groups_mse_list = {}
        self.groups_wmse = numpy.nan

    def compute_all(self):
        RegressorResult.compute(self)
        if self.true_values is None:
            return
        # FIXME : completer

    def compute_groups_mse_list(self, groups):
        import numpy
        t = numpy.asarray(self.true_values)
        p = numpy.asarray(self.predict_values)
        id_list = numpy.unique1d(groups)
        groups_ind = {}
        for id in id_list:
            groups_ind[id] = numpy.where(groups == id)[0]
        for g, ind in groups_ind.items():
            mse = ((t[ind] - p[ind]) ** 2).mean()
            self.groups_mse_list[g] = mse
        return self.groups_mse_list

    def compute_groups_wmse(self, groups, weights):
        '''
Compute weighted mean square error based on given data groups.

groups :  list of groups id.
weights : dictionnary (groups id / weight).
      Weights can be normalized or not.

ex :      groups = [0, 1, 0, 0, 0, 1, 1, 1, 0]
      weights = {0 : 0.3, 1 : 0.7}

            ___
            \\
            /__   mse(g) * size(g) * weight(g)
         g in groups
grp_wmse = _________________________________________
            ___
            \\
            /__    size(g) * weight(g)
         g in groups

if for each g, w(g) = k / size(g), with k = Cte, then
grp_wmse = E_g(mse(g)), with E : expectantion over groups.
        '''
        import numpy
        self.compute_groups_mse_list(groups)
        wmse = 0.
        total_w = 0.
        size = {}
        for g, w in weights.items():
            size[g] = len(numpy.where(groups == g)[0])
            total_w += w * size[g]
        for g, mse in self.groups_mse_list.items():
            wmse += mse * size[g] * weights[g]
        wmse /= total_w
        self.groups_wmse = wmse
        return wmse

    def setValues(self, mse=None, groups_mse_list=None,
                  groups_wmse=None):
        '''
Results of weighted regressor prediction.

mse :         mean square error.
wmse_group :  weighted mean square error based on groups.
        '''
        RegressorResult.setValues(self, mse)
        if groups_mse_list != None:
            self.groups_mse_list = groups_mse_list
        if groups_wmse != None:
            self.groups_wmse = groups_wmse

    def __repr__(self):
        s = RegressorResult.__repr__(self)
        s += ', groups mse list : %s' % str(self.groups_mse_list)
        s += ', groups weighted mse : %f' % self.groups_wmse
        return s

    _compute_criterions = \
        dict(RegressorResult._compute_criterions)
    _compute_criterions['groups_mse_list'] = compute_groups_mse_list
    _compute_criterions['groups_wmse'] = compute_groups_wmse
