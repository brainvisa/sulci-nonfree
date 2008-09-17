import pickle, numpy, scipy.stats

from classifier import Classifier, classifier_map
import datamind.ml.classif

################################################################################
class AbstractCRvm(Classifier):
	def __init__(self):
		Classifier.__init__(self)

	def fit(self, X, Y):
		self._clf.fit(X, Y)

	def predict(self, x):
		'''
    return posterior probability of class 0 : P(class 0|X) and label
		'''
		# Warning : le classifieur ressort la proba a posteriori du
		# modele et non la classe.
		G = self._clf.cross_Gram(x[None]) # predict only one vector
		p = self._clf.testing(G)[0]
		label = numpy.round(p) # 0 or 1
		return 1 - p, label

	def predict_db(self, X):
		G = self._clf.cross_Gram(X)
		P = self._clf.testing(G)
		labels = numpy.round(P) # array of 0 or 1
		return 1 - P, labels


class LinearCRvm(AbstractCRvm):
	def __init__(self):
		AbstractCRvm.__init__(self)
		self._clf = datamind.ml.classif.CRVM(kw=0.,
					niter=300, delta=1.e-6)
		self._name = 'linear_crvm'

	def toTuple(self):
		return self._clf._Gram, self._clf.weight, self._clf.alpha, \
			self._clf.precision, self._clf.covariance, \
			self._clf.Xtrain, self._clf.heuristic

	def fromTuple(self, obj):
		self._clf._Gram, self._clf.weight, self._clf.alpha, \
			self._clf.precision, self._clf.covariance, \
			self._clf.Xtrain, self._clf.heuristic = obj

	@classmethod
	def getParameters(self, obj): return []


class GaussianCRvm(AbstractCRvm):
	def __init__(self, kw=1.):
		AbstractCRvm.__init__(self)
		self._clf = datamind.ml.classif.CRVM(kw=kw,
					niter=300, delta=1.e-6)
		self._name = 'gaussian_crvm'
		self._kw = kw

	def toTuple(self):
		return self._kw, self._clf._Gram, self._clf.weight, \
			self._clf.alpha, self._clf.precision, \
			self._clf.covariance, self._clf.Xtrain, \
			self._clf.heuristic

	def fromTuple(self, obj):
		self._kw, self._clf._Gram, self._clf.weight, self._clf.alpha, \
			self._clf.precision, self._clf.covariance, \
			self._clf.Xtrain, self._clf.heuristic = obj

	@classmethod
	def getParameters(self, obj):
		return [obj[0]]


class OptimizedGaussianCRvm(AbstractCRvm):
	def __init__(self, k=10., kw=1.):
		AbstractCRvm.__init__(self)
		self._k = k
		if kw:
			self._kw = kw
			self._clf = datamind.ml.classif.CRVM(kw=self._kw,
						niter=300, delta=1.e-6)
		else:
			self._kw = None
			self._clf = None
		self._name = 'optimized_gaussian_crvm'
	
	def fit(self, X, Y):
		from fff.graph import WeightedGraph
		G_knn = WeightedGraph(len(X))
		G_knn.knn(X, self._k)
		G_knn.set_euclidian(X)
		self._kw = G_knn.weights.mean()
		self._clf = datamind.ml.classif.CRVM(kw=self._kw,
					niter=300, delta=1.e-6)
		AbstractCRvm.fit(self, X, Y)
		
	def toTuple(self):
		return self._k, self._kw, self._clf._Gram, self._clf.weight, \
			self._clf.alpha, self._clf.precision, \
			self._clf.covariance, self._clf.Xtrain, \
			self._clf.heuristic

	def fromTuple(self, obj):
		self._k, self._kw, self._clf._Gram, self._clf.weight, \
			self._clf.alpha, self._clf.precision, \
			self._clf.covariance, self._clf.Xtrain, \
			self._clf.heuristic = obj

	@classmethod
	def getParameters(self, obj):
		return [obj[0], obj[1]]

################################################################################
classifier_map.update({\
	'linear_crvm' : LinearCRvm,
	'gaussian_crvm' : GaussianCRvm,
	'optimized_gaussian_crvm' : OptimizedGaussianCRvm,
})

