from __future__ import print_function

from __future__ import absolute_import
import numpy as N
import scipy.stats as ST
from six.moves import range


class Friedman(object):


    """
    Friedman non parametric test equivalent of the repeated-measures Anova.
    Statistic test for the Comparison of k variables on n multiple data sets.
    It Computes the ranks of the k variables for each data sets and checks if all the variables are equivalent.

    Returns: average ranks of each variables if data sets are tied and ranks of each variables for each data sets otherwise
            k-tailed p-value according to the F-distribution if n>1 (because it derives a better statistic, less conservative than the Friedman's Chi2)

    Examples:

    fried=Friedman()
    r, pf=fried.eval(scores, ties)
    print("ranks for each variable ")
    print(r)
    print("Are the variables significantly different?")
    print(pf)

    """

    def __init__(self):
        pass

    def eval(self, scores):
        """
        INPUT :
        scores = (n*k) shape. Scores of the k variables for the n data sets
        OUTPUT :
        r = (k) shape average ranks of the k variables
        p = (1) shape. p-values of Friedman's F distribution computed with Chi2 distribution
        """
        dim = scores.shape
        if len(dim) < 2:
            raise ValueError('scores must be 2D array')
        n = dim[0]
        k = dim[1]
        self._n = n
        self._k = k

        # sort the variables scores for each data sets. The best scoring
        # variable gets the rank 1.
        self._ranks = N.zeros((n, k))
        for i in range(n):
            ind = N.argsort(-scores[i])
            self._ranks[i, ind] = list(range(1, k + 1))
        print("self._ranks0:", self._ranks)
        self.check_equalRank(scores)
        print("self._ranks1:", self._ranks)
        self._ranks = N.mean(self._ranks, 0)

        # Compute the Friedman statistics and the p-value
        self._fried_F = None
        self._pF = None
        self._fried_chi2 = (12.0 * n / (k * (k + 1))) * (
            N.sum(self._ranks**2) - ((k * (k + 1)**2) / 4.0))
        # self._fried_chi2 = [ ( (12.0*nbs)/(k*(k+1))) * ( N.sum( ( self._ranks[i] - ( (k+1)/2.0))**2)) for i in range(nbr)]
        # self._fried_chi2 = [ (( 12.0/(nbs*k*(k+1))) * N.sum(
        # (self._ranks[i]*nbs)**2)) - ( 3*nbs*(k+1)) for i in range(nbr)]
        self._pchi2 = ST.chi2.sf(self._fried_chi2, k - 1)

        self._fried_F = N.abs(
            ((n - 1) * self._fried_chi2) / ((n * (k - 1)) - self._fried_chi2))
        self._pF = ST.f.sf(self._fried_F, k - 1, (k - 1) * (n - 1))
        return self._ranks, self._pF

    def check_equalRank(self, scores):
        for i in range(self._n):
            score = scores[i]
            u = N.unique(score)
            if len(u) < self._k:
                for j in range(len(u)):
                    ind = N.nonzero(score == u[j])[0]
                    if len(ind) > 1:
                        mean_r = N.mean(self._ranks[i, ind])
                        self._ranks[i, ind] = [mean_r for k in range(len(ind))]

    def getPchi2(self):
        return self._pchi2

    def getPF(self):
        return self._pF

    def pairwise_comp(self, indices_set, indices_set_comp=None):
        """
        INPUT :
        indices_set = list of the variables indices to compare 2 by 2
        indices_set_comp = list of list of the the variables indices to compare to each indices_set indices
        ex : indices_set = [0, 1] indices_set_comp = [[2],[3,4]]=>(0,2);(1,3);(3,4) comparisons
        OUTPUT :
        pval = list of tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        """
        nbind = len(indices_set)
        if indices_set_comp == None:
            r1 = list(range(nbind - 1))
            r2 = list(range(i + 1, nbind))
        else:
            r1 = []
            r2 = []
            for i in range(nbind):
                r1.extend([indices_set[i]] * len(indices_set_comp[i]))
                r2.extend(indices_set_comp[i])

        p = []
        z = []
        # for i in range(nbind-1):
            # for j in range(i+1,nbind):
                # zval = (self._ranks[indices_set[j]]-self._ranks[indices_set[i]])/N.sqrt( (nbind*(nbind+1))/ (6.0*self._n))
                # z.append((indices_set[j], indices_set[i], zval))
                # p.append((indices_set[j], indices_set[i],
                # ST.norm.sf(N.abs(zval))))
        for i in range(len(r1)):
            zval = (self._ranks[r1[i]] - self._ranks[r2[i]]) / N.sqrt(
                (nbind * (nbind + 1)) / (6.0 * self._n))
            z.append((r1[i], r2[i], zval))
            p.append((r1[i], r2[i], ST.norm.sf(N.abs(zval))))
        self._z = z
        self._p = p
        return p


if __name__ == "__main__":
    import numpy.random as NR
    n = 5
    scores = N.zeros((5, 4))
    scores[:, 0] = NR.random(5) * 100
    # scores[:, 1] = scores[:, 0]+50
    scores[:, 1] = NR.random(5)
    scores[:, 2] = NR.random(5) * 100
    scores[:, 3] = scores[:, 2]

    fried = Friedman()
    r, pf = fried.eval(scores)
    print("ranks for each variable ")
    print(r)
    print("Are the variables significantly different?")
    print(pf)

    print("\n")

    p = fried.pairwise_comp(list(range(4)))
    print("p-values of the pairwise comparisons")
    print(p)
    from .post_hoc import *
    ph = Nemenyi()
    H_select = ph.eval(p, 0.05)
    print("alpha adjusted with Nemenyi post-hoc test:")
    print(ph.getAlphaAdjust())
    print("Null Hypotheses rejected:")
    print(H_select)

    print("\n")

    ph = Holm()
    H_select = ph.eval(p, 0.05)
    print("alpha adjusted with Holm post-hoc test:")
    print(ph.getAlphaAdjust())
    print("Null Hypotheses rejected:")
    print(H_select)

    print("\n")

    ph = Shaffer_stat()
    H_select = ph.eval(p, 0.05)
    print("alpha adjusted with Shaffer static post-hoc test:")
    print(ph.getAlphaAdjust())
    print("Null Hypotheses rejected:")
    print(H_select)

    print("\n")

    ph = Berg_Hom()
    H_select = ph.eval(p, 0.05)
    print("alpha adjusted with Berg_Hom static post-hoc test:")
    print(ph.getAlphaAdjust())
    print("Null Hypotheses rejected:")
    print(H_select)
