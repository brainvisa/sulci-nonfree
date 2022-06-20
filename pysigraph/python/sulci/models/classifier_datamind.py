from __future__ import absolute_import
import pickle
import numpy
import scipy.stats

from .classifier import Classifier, classifier_map

#


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
        G = self._clf.cross_Gram(x[None])  # predict only one vector
        p = self._clf.testing(G)[0]
        label = numpy.round(p)  # 0 or 1
        return 1 - p, label

    def predict_db(self, X):
        G = self._clf.cross_Gram(X)
        P = self._clf.testing(G)
        labels = numpy.round(P)  # array of 0 or 1
        return 1 - P, labels


#
classifier_map.update({
})
