from __future__ import print_function

from __future__ import absolute_import
import numpy as N
import scipy.stats as ST
from six.moves import range


class Wilcoxon(object):


    """
    Wilcoxon non parametric test.
    Calculates the Wilcoxon signed-rank test for the null hypothesis that two
    samples come from the same distribution. A non-parametric T-test.
    (need N > 20)

    Returns: t-statistic, two-tailed p-value

    Examples:
    import datamind.data as data
    import numpy as N
    d=data.twoclassses(nbSamples=30,deltaMeans=N.arange(2,0,-.1),nbNoize=100)
    y=d[:,["class"]]
    X=d[:,1:]

    wilc=Wilcoxon()
    wilcs=wilc.eval(X,y)
    print("wilcs for each variable (the 10 first are discriminatives)")
    print(wilcs)
    print("Variables ranks sorting with decreasing wilcs (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(wilcs)]).ravel())

    """

    def __init__(self):
        pass

    def eval(self, X, Y):
        """
        """
        labels = N.unique(Y)
        wilcs = N.zeros(X.shape[1])
        for i in range(X.shape[1]):
            stat, pvalue = ST.wilcoxon(X[:, i], N.reshape(Y, Y.shape[0]))
            wilcs[i] = stat

        return wilcs

if __name__ == "__main__":
    import datamind.data as data
    import numpy as N
    d = data.twoclassses(
        nbSamples=60, nbNoize=1, stringClassLabels=True)[10:, :]
    Y = d[:, ["class"]]
    X = d[:, 1:]

    wilc = Wilcoxon()
    wilcs = wilc.eval(X, Y)
    print("wilcs for each variable (the 10 first are discriminatives)")
    print(wilcs)
    print(
        "Variables ranks sorting with decreasing wilcs (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(wilcs)]).ravel())

    import numpy.random as NR
    nbitems = 100
    dim = 63000
    signal = 0.8

    # generate the data
    Y = N.zeros((nbitems, 1))
    Y[:60] = 1.0
    alpha = N.zeros((dim))
    alpha[:10] = signal  # N.exp(N.log(0.5)*N.arange(10))
    SN = NR.randn(nbitems, dim)
    SP = N.zeros((nbitems, dim))
    SP[:] = alpha
    X = SN + SP * N.reshape(Y, (nbitems, 1))
    # GLM
    wilc = Wilcoxon()
    wilcs = wilc.eval(X, Y)
    print(
        "Variables ranks sorting with decreasing wilcs (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(wilcs[:10])]).ravel())
    print("wilcs for each variable (the 10 first are discriminatives)")
    print(wilcs[N.fliplr([N.argsort(wilcs[:10])]).ravel()])
