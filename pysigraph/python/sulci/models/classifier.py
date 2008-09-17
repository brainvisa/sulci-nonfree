import pickle, numpy, scipy.stats


class Classifier(object):
	def write(self, filename):
		fd = open(filename, 'w')
		obj = self.toTuple()
		pickle.dump(obj, fd)
		fd.close()

	def read(self, filename):
		fd = open(filename, 'r')
		obj = pickle.load(fd)
		fd.close()
		#try:
		self.fromTuple(obj)
		#except TypeError: raise IOError

	def name(self): return self._name


################################################################################
class FiltredClassifier(Classifier):
	def __init__(self, clf=None, dims=None):
		Classifier.__init__(self)
		self._clf = clf
		self._dims = dims

	def fit(self, X, Y):
		self._clf.fit(X[:, self._dims], Y)

	def predict(self, x):
		return self._clf.predict(x[self._dims])

	def predict_db(self, X):
		return self._clf.predict_db(X[:, self._dims])

	def toTuple(self):
		t = self._clf.toTuple()
		return (t, self._clf.name(), self._dims)

	def fromTuple(self, obj):
		t, name, self._dims = obj
		Clf = classifierFactory(name)
		p = Clf.getParameters(t)
		self._clf = Clf(*p)
		self._clf.fromTuple(t)


################################################################################

classifier_map = { \
	'filtred' : FiltredClassifier,
}

################################################################################
def classifierFactory(name):
	return classifier_map[name]
