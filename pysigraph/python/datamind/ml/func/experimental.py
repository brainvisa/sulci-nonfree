from __future__ import print_function

from __future__ import absolute_import
import numpy as N
from six.moves import range


# ==============================================================================
# Experimental Functions
# ==============================================================================
# Provide:
# - OneWayAnovaFstat
# - SimpleLinRegFstat
# - OneWayAnovaEffect
# - SimpleLinRegEffect

# ------------------------------------------------------------------------------
# Fstat(s)
class GrpScattering(object):

    """
    Group scaterring:
    Re-order labels according to sorted X
    Parse Re-order labels and increment a buffer each time consecutive labels
    have the same value.
    A A A B B B => 0 1 2 2 3 4 : cpt=4
    A B A B A B => 0 0 0 0 0 0 : cpt=0
    A A B A B B => 0 1 1 1 1 2 : cpt=2
    """

    def __init__(self):
        pass

    def eval(self, X, y):
        """
        X : array of real values, typically a 2D array nb samples * nd features
        y : vector of group labels
        For each features in X calulate a  group scattering score
        """
        # If the contrast is not provided assume that X is a vector
        # of group labels, then build the desing matrix
        labels = y.ravel()
        scores = []
        for j in range(X.size // X.shape[0]):
            labels_sorted = labels[N.argsort(X[:, j])]
            label_cur = labels[0]
            cpt = 0
            for lab in labels_sorted[1:]:
                if lab == label_cur:
                    cpt += 1
                else:
                    label_cur = lab
            scores.append(cpt)
        return(N.array(scores))

if __name__ == "__main__":
    import datamind.data as data
    import numpy as N
    from datamind.io import DF

    print(
        "====================================================================")
    print("Basic usage of fit/reduce")
    d = data.twoclassses()
    y = d[:, ["class"]]
    X = d[:, 1:]
    self = GrpScattering()
    print(self.eval(X, y))
