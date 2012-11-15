# -*- coding: utf-8 -*-
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

from datamind.ml import classifier
from datamind.ml import database
from datamind.tools import *
import sigraph



class WrapperSi(classifier.Classifier):
	def __init__(self, adaptiveleaf, weights = None, **user_data):
		'''
    Abstract Class to wrap sigraph adaptiveleaf/subadaptive.

    adaptiveleaf :  an adaptiveleaf.
    weights :       dictionary of (classes, weights). Weights are double values
                    and can be normalized or not.
                    ex : {'0' : 10., '1' : 1.}
    		'''
		if not isinstance(adaptiveleaf, sigraph.AdaptiveLeaf):
			raise ValueError, \
				"'adaptiveleaf' parameter should be an " \
				" AdaptiveLeaf instance."
		self._adaptiveleaf = adaptiveleaf
		self._weights = weights
		self._user_data = user_data
		if weights is not None :self._setWeights(weights)

	def init(self):
		self._adaptiveleaf.init()

	def predict(self, test):
		res = self._adaptiveleaf.workEl().test(self._convert(test))
		return self._getResult(res)
	
	def setParams(self, params):
		print "warning", params

	def _convert(self, db):
		import numpy
		X = db.getX()
		Y = db.getY()
		INF = db.getINF()
		# groups are not used in C++ until now
		if X.flags.fortran or not X.flags.contiguous:
			X = numpy.array(X, copy=True, order='C')
		if Y.flags.fortran or not Y.flags.contiguous:
			Y = numpy.array(Y, copy=True, order='C')
		sidb = sigraph.SiDBLearnable(X, Y, INF)
		sidb.setLabels(["cycle"])
		return sidb

	def _setWeights(self, weights):
		subad = self._adaptiveleaf.workEl()
		for c, w in weights.items():
			subad.setSvmWeight(c, w)

	def _getResult(self, res) :
		import numpy
		true_values = numpy.array(res.true_values, copy=True)
		predict_values = numpy.array(res.predict_values, copy=True)
		return self.computeResult(true_values, predict_values)


class MlpSi(WrapperSi):
	def __init__(self, *args, **kwargs):
		WrapperSi.__init__(self, *args, **kwargs)

	def _convert(self, db):
		sidb = WrapperSi._convert(self, db)
		cycles = db.getCycles()
		sidb.setCycles(cycles)
		return sidb
	
	def setParams(self, params):
		print "warning : currently MLP(sigraph) have not any free " + \
			"parameters."

	def fit(self, part):
		train = self._convert(part.getTrain())
		test = self._convert(part.getTest())
		return self._adaptiveleaf.workEl().learn(\
			self._adaptiveleaf, train, test)

	def clone(self):
		return MlpSi(self._adaptiveleaf.clone(), **self._user_data)

	def computeResult(self, true_values, predict_values):
		from datamind.ml.classifier import RegressorResult
		return RegressorResult(true_values, predict_values)


class SvmSi(WrapperSi):
	def __init__(self, *args, **kwargs):
		WrapperSi.__init__(self, *args, **kwargs)
		e = self._adaptiveleaf.workEl()
		e.setSigma = lambda x : e.setGamma(1./x)
		self._pset = {'gamma' : e.setGamma, 'sigma' : e.setSigma,
			'C' : e.setC, 'epsilon' : e.setEpsilon, 'nu' : e.setNu}
	
	def setParams(self, params):
		for k, v in params.items() :
			self._pset[k](v)

	def fit(self, train):
		return self._adaptiveleaf.workEl().learn(\
				self._convert(train))
		
	def clone(self):
		return SvmSi(self._adaptiveleaf.clone(), **self._user_data)

	def cv(self, train, nbfolds) :
		e = self._adaptiveleaf.workEl()
		res = e.crossvalidation(self._convert(train), int(nbfolds))
		return self._getResult(res)


class SvcSi(SvmSi):
	def __init__(self, *args, **kwargs):
		SvmSi.__init__(self, *args, **kwargs)

	def computeResult(self, true_values, predict_values):
		'''Compute classifier results.'''
		from datamind.ml.classifier import \
				WeightedBinaryClassifierResult
		return WeightedBinaryClassifierResult(true_values,
						predict_values)


class SvrSi(SvmSi):
	def __init__(self, *args, **kwargs):
		SvmSi.__init__(self, *args, **kwargs)

	def computeResult(self, true_values, predict_values):
		'''Compute regressor results.'''
		from datamind.ml.classifier import GroupsRegressorResult
		return GroupsRegressorResult(true_values, predict_values)
