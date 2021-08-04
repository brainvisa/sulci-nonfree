from __future__ import print_function

from __future__ import absolute_import
import numpy as N
import scipy as S
from six.moves import range
from six.moves import zip


class Post_Hoc(object):


    """
    Post-Hoc analysis define the appropriate level(s) of significance alpha of a family of hypotheses H1,...,Hm.
    It finds the pairwise p_values (p1,...,pm) corresponding to the hypotheses which are statistically significant by adjusting alpha.

    Returns: average ranks of each variables if data sets are tied and ranks of each variables for each data sets otherwise
            k-tailed p-value according to the F-distribution if n>1 (because it derives a better statistic, less conservative than the Friedman's Chi2)

    Examples:

    ph=Post_Hoc()
    r, pf=ph.eval(pval)
    print("ranks for each variable ")
    print(r)
    print("Are the variables significantly different?")
    print(pf)

    """

    def __init__(self):
        pass

    def eval(self, H, alpha):
        """
        INPUT:
        H = list of m tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        alpha = Initial level of significance
        OUTPUT:
        H_select = list of tuples of shape 3 (i, j, pval) rejected, ie null hypotheses rejected, ie variables i and j are significantly different.
        """
        pass

    def getAlphaAdjust(self):
        return self.alpha_adjust


class Nemenyi(Post_Hoc):

    def eval(self, H, alpha):
        """
        It adjusts alpha by dividing it by the nb of comparison m=k(k+1)/2-k and it rejects the hypotheses Hi<=alpha/m.
        INPUT:
        H = list of m tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        alpha = Initial level of significance
        OUTPUT:
        H_select = list of tuples of shape 3 (i, j, pval) rejected, ie null hypotheses rejected, ie variables i and j are significantly different.
        """
        m = len(H)
        H_select = []
        alpha_adjust = []
        alpha_adjust.append(alpha / m)
        for i in range(m):
            if H[i][2] <= alpha / m:
                H_select.append(H[i])
        self.alpha_adjust = alpha_adjust
        return H_select


class Holm(Post_Hoc):

    def eval(self, H, alpha):
        """
        It adjusts the value of alpha in a step down procedure. It orders the p-values p1,..,pm (smallest to largest)=>po1, ..., pom and
        Ho1, ..., Hom the corresponding hypotheses. It rejects Ho1,..,Ho(i-1) if is the smallest integer such that poi>alpha/(m-i+1) because the
        reject of an hypotheses changes the number of hypotheses to test and hence the value of alpha.
        INPUT:
        H = list of m tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        alpha = Initial level of significance
        OUTPUT:
        H_select = list of tuples of shape 3 (i, j, pval) rejected, ie null hypotheses rejected, ie variables i and j are significantly different.
        """
        m = len(H)
        H_select = []
        alpha_adjust = []

        pval = [H[i][2] for i in range(m)]
        ind_sort = N.argsort(N.array(pval))
        Ho = [H[i] for i in ind_sort]

        for i in range(m):
            if Ho[i][2] <= alpha / (m - (i + 1) + 1):
                H_select.append(Ho[i])
                alpha_adjust.append(alpha / (m - (i + 1) + 1))
        self.alpha_adjust = alpha_adjust
        return H_select


class Shaffer_stat(Post_Hoc):

    def eval(self, H, alpha):
        """
        It adjusts the value of alpha in a step down procedure by taking account the logical relation among the family of hypotheses.
        It orders the p-values p1,..,pm (smallest to largest)=>po1, ..., pom and Ho1, ..., Hom the corresponding hypotheses.
        It rejects Hoi if poi<=alpha/ti where ti is the maximum number of hypotheses which can be true given that (i-1) hypotheses are false.
        INPUT:
        H = list of m tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        alpha = Initial level of significance
        OUTPUT:
        H_select = list of tuples of shape 3 (i, j, pval) rejected, ie null hypotheses rejected, ie variables i and j are significantly different.
        """
        m = len(H)
        H_select = []
        alpha_adjust = []

        pval = [H[i][2] for i in range(m)]
        ind_sort = N.argsort(N.array(pval))
        Ho = [H[i] for i in ind_sort]

        T = []
        delta = 1 + (8 * m)
        k = (1 + N.sqrt(delta)) / 2.0
        T = self.getT_rec(T, k)
        T = N.unique(T).tolist()
        T.sort(reverse=True)
        if T.count(0) != 0:
            T.remove(0)

        for i in range(m):
            t = T[N.argmax(N.array(T) <= m - i)]
            if Ho[i][2] <= alpha / t:
                H_select.append(Ho[i])
                alpha_adjust.append(alpha / t)
        self.alpha_adjust = alpha_adjust
        return H_select

    def getT_rec(self, T, step):
        """
        Recursive formulae to obtaine t values : T(k) = Union j=1 a k {Comb(j,2)+x : x in T(k-j)}
        k = (1+sqrt(delta))/2.0 with delta = 1+(4*2*m)
        """
        if step < 2:
            return [0]
        else:
            for i in range(1, step + 1):
                T.extend(
                    self.sum_f(self.getT_rec([], step - i), N.int(S.comb(i, 2, exact=1))))

        return T

    def sum_f(self, l, add):
        for i in range(len(l)):
            l[i] += add
        return l


class Berg_Hom(Post_Hoc):

    def eval(self, H, alpha):
        """
        It adjusts the value of alpha in a step down procedure by taking account the logical relation among the family of hypotheses
        and the hypotheses already rejected. It is based on the idea to find all elementary hypotheses which can not be rejected.
        It rejects Hi if i does not belong to the acceptance set A=Union{Ej in E exhaustive set : min{P_Ej}>alpha/|Ej| }
        INPUT:
        H = list of m tuples of shape 3 (i, j, pval) with i and j, the indices of the pairwise variables and pval the pvalue of the pairwise (i,j) comparison.
        alpha = Initial level of significance
        OUTPUT:
        H_select = list of tuples of shape 3 (i, j, pval) rejected, ie null hypotheses rejected, ie variables i and j are significantly different.
        """
        m = len(H)
        self._H = H
        H_select = []
        alpha_adjust = []
        delta = 1 + (8 * m)
        k = (1 + N.sqrt(delta)) / 2.0

        indices_set = []
        for i in range(len(self._H)):
            if indices_set.count(self._H[i][0]) == 0:
                indices_set.append(self._H[i][0])
            if indices_set.count(self._H[i][1]) == 0:
                indices_set.append(self._H[i][1])
        indices_set = N.sort(indices_set).tolist()
        E = self.exhaustive_set(indices_set, k)
        self.E = E

        A = []
        for i in range(len(E)):
            p = [E[i][j][2] for j in range(len(E[i]))]
            # print("E[i]:",E[i], " p:",p, " min(p):",min(p), "
            # 0.05/len(E[i]):",0.05/len(E[i]))
            if min(p) > (alpha / len(E[i])):
                A.extend(E[i])
            else:
                alpha_adjust.append(alpha / len(E[i]))
        A_temp = []
        for i in range(len(A)):
            if A_temp.count(A[i]) == 0:
                A_temp.append(A[i])
        A = A_temp
        print("Acceptance set:", A)

        for i in range(m):
            if H[i] not in A:
                H_select.append(H[i])
        self.alpha_adjust = alpha_adjust
        return H_select

    def exhaustive_set(self, indices_set, k):
        """
        A set is called exhaustive when all these hypotheses could be true.
        The exhaustive set of k variables is all teh exhaustive sets of hypotheses using these variables.
        The below algorithm performs a division of the variables into 2 subsets, in which the last variable k
        is inserted in the second subset, the first subset cannot be empty and a subset must contain at least 2 variables.
        """
        E = []
        E1 = []
        E2 = []
        # print("indices_set:",indices_set)
        nbv = len(indices_set)
        if nbv <= 1:
            return E
        E.append(self.pairwise_set(indices_set))
        # print("Etemp:",E)
        s = ''
        s = s.join(N.array(indices_set, dtype=str).tolist())
        # print("s:",s)
        subset = list(self.powerset(s))
        # print("subset:",subset)
        S1, S2 = self.subsets(subset, k)
        for i in range(len(S1)):
            E1 = self.exhaustive_set(N.sort(S1[i]).tolist(), k)
            E2 = self.exhaustive_set(N.sort(S2[i]).tolist(), k)
            if len(E1) > 0:
                E.extend(E1)
            if len(E2) > 0:
                E.extend(E2)
            for k in range(len(E1)):
                for l in range(len(E2)):
                    E12 = []
                    E12.extend(E1[k])
                    E12.extend(E2[k])
                    E.append(E12)
        return E

    def pairwise_set(self, indices_set):
        """
        It computes the set of all possible and distinct pairwise comparisons using variables in indices_set
        """
        sub_H = []
        nbind = len(indices_set)
        ind_H = [(self._H[i][0], self._H[i][1]) for i in range(len(self._H))]
        for i in range(nbind - 1):
            for j in range(i + 1, nbind):
                ind = ind_H.index((indices_set[j], indices_set[i]))
                sub_H.append(self._H[ind])
        return sub_H

    def powerset(self, s):
        """
        It computes all the possible subsets of s
        """
        d = dict(list(zip((1 << i for i in range(len(s))), (set([e]) for e in s))))
        subset = set()
        yield subset
        for i in range(1, 1 << len(s)):
            subset = subset ^ d[i & -i]
            yield subset
        # return subset

    def subsets(self, subset, k):
        """
        It computes all the couple of subsets C1 and C2 such that |C1|>=2 and |C2|>=2 and len(indices_set) belongs to C2
        """
        S1 = []
        S2 = []
        indices_set = []

        for i in range(len(subset)):
            l = list(subset[i])
            indices_set.extend(int(l[j]) for j in range(len(l)))
        indices_set = N.unique(indices_set).tolist()
        for i in range(len(subset)):
            l = list(subset[i])
            l = N.array(l, int).tolist()
            if len(l) > 0 and l.count(k - 1) != 0:
                diff = N.setdiff1d(indices_set, l).tolist()
                if len(diff) > 0:
                    S2.append(l)
                    S1.append(diff)
        # print("S1:",S1)
        # print("S2:",S2)

        return S1, S2
