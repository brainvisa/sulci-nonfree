from __future__ import print_function

from __future__ import absolute_import
import numpy as N
import scipy.stats as ST
import scipy as SP
from six.moves import range


def MIFloatFunc(Xi, Xj, bins):
    """
    Compute the mutual information for 2 1d arrays (coudl be float)

    MIFloatFunc(Xi,Xj,bins)

    """
    probajoint = N.histogram2d(Xi, Xj, bins=bins)[0] / len(Xi)
    probamarge = N.outer(probajoint.sum(1), probajoint.sum(0))
    return N.nansum(probajoint * N.log(probajoint / probamarge))


class MIFloat(object):

    """
    Compute the mutual information for 2 1d arrays (coudl be float)

    Input :
    bins (defaut is 20)

    Output :
    MI

    Example
    func = MIFloat(30)
    func.eval(Xi,Xj)

    """

    def __init__(self, bins=20, verbose=False):
        self.bins = bins
        self.verbose = verbose

    def computeMI(self, Xi, Xj):
        """
        Compute the mutual information for 2 1d arrays (coudl be float)

        Input :
        Xi
        Xj
        bins (defaut is 10)
        verbose (defaut is False)

        Output :
        MI

        Example
        MutualInformationFloat(Xi,Xj)

        """

        bins = self.bins
        probajoint = N.histogram2d(Xi, Xj, bins=bins)[0] / len(Xi)
        probamarge = N.outer(probajoint.sum(1), probajoint.sum(0))
        return N.nansum(probajoint * N.log(probajoint / probamarge))

    def eval(self, Xi, Xj):

        if N.size(Xi.shape) == 1:
            Xi = N.reshape(Xi, [N.size(Xi), 1])
        if N.size(Xj.shape) == 1:
            Xj = N.reshape(Xj, [N.size(Xj), 1])
        nsize = N.size(Xi, 1)
        if nsize == 1:
            MI = self.computeMI(Xi, Xj)
            if self.verbose == True:
                print(MI)
        else:
            MI = N.zeros(nsize)
            for i in range(nsize):
                MI[i] = self.computeMI(Xi[:, i], Xj)
                if self.verbose == True:
                    print(MI[i])
        return MI


class multiMI(object):

    """
    Mutual Information : see Kraskov et al. 2004 "Estmating Mutual Information" for more details.
    Warning : Y must be Integer !!!

    MULTIVARIATE WAY

    Class for mutual information function, with "eval".

    Input :
    X : array (in R)
    Y : array of classes (in N)
    k : number of nearest neigboorhs to consider

    Output :
    MI(X,Y)


    Examples:
    import datamind.data as data
    from datamind.ml.func import *
    import numpy as N
    d=data.twoclassses()
    Y=d[:,"class"]
    X=d[:,1:]
    X = N.asarray(X)
    Y = N.asarray(Y)
    im_class = multiMI(3)
    for i in range(N.size(X,1)):
            print(im_class.eval(X[:,i],Y))
    """

    def __init__(self, k):
        """
Set the value of k for the KNN
        """
        self.k = k

    # CALCUL DE I(X,Y) :
    def eval(self, X, Y):
        k = self.k
        if N.size(X.shape) == 1:
            X = N.reshape(X, [N.size(X, 0), 1])
        if N.size(Y.shape) == 1:
            Y = N.reshape(Y, [N.size(Y, 0), 1])
        IY = self.calcul_IY(Y, k)
        indi, indj = self.calcul_knn(X, k)
        terme_4 = self.calcul_terme_4(indj, Y, k)
        IXY = IY + terme_4
        return IXY

    # CALCUL de YI :
    def calcul_IY(self, Y, k):
        taillex = N.size(Y, 0)
        terme_1 = -SP.special.digamma(N.int(k))
        terme_2 = SP.special.digamma(taillex)
        ymax = N.int(N.max(Y))
        ymin = N.int(N.min(Y))
        sumY = 0
        for i in range(ymin, ymax + 1):
            sum = N.size(N.where(Y[:, 0] == i))
            sumY = sumY  + \
                (N.float(sum) / N.float(taillex)) * SP.special.digamma(sum)
        terme_3 = sumY
        IY = terme_1 + terme_2 - terme_3
        return IY

    # CALCUL DES KNN
    def calcul_knn(self, X, k):
        import fff.graph as FG
        ind_i, ind_j, d = FG.graph_cross_knn(X, X, k + 1)
        ind_ib = ind_i[N.where(d != 0)]
        ind_jb = ind_j[N.where(d != 0)]
        return ind_ib, ind_jb

    # CALCUL DU TERME 4 DE L INFORMATION MUTUELLE
    def calcul_terme_4(self, indj, Y, k):
        nxl = 0
        taillex = N.size(Y, 0)
        for i in range(taillex):
            Yk = Y[indj[i * k:(i + 1) * k]]
            nxl = nxl + \
                SP.special.digamma(
                    N.int(N.size(N.where(Yk[:, 0] == Y[i, 0])[0]) + 1))
        nxl = N.float(nxl) / N.float(taillex)
        return nxl

    def setParams(self, k):
        self.k = k


def eval_MI(X, Y, k):
    """
    Mutual Information : see Kraskov et al. 2004 "Estmating Mutual Information" for more details.
    Warning : Y must be Integer !!!

    Function for direct evaluation of Mutual Information

    Input :
    X : array (in R)
    Y : array of classes (in N)
    k : number of nearest neigboorhs to consider

    Output :
    MI(X,Y)

    Examples:
    import datamind.data as data
    from datamind.ml.func import *
    import numpy as N
    d=data.twoclassses()
    Y=d[:,"class"]
    X=d[:,1:]
    X = N.asarray(X)
    Y = N.asarray(Y)
    MI_XY = MutualInformation.eval_MI(X,Y,3)
    """
    im_class = MI(k)
    IXY = im_class.eval(X, Y)
    return IXY


class univMI(object):

    """
    Mutual Information : univariate way, return array

    Mutual Information : see Kraskov et al. 2004 "Estmating Mutual Information" for more details.
    Warning : Y must be Integer !!!

    MULTIVARIATE WAY

    Class for mutual information function, with "eval".

    Input :
    X : array (in R)
    Y : array of classes (in N)
    k : number of nearest neigboorhs to consider

    Output :
    MI(X,Y)


    Examples:
    import datamind.data as data
    from datamind.ml.func import *
    import numpy as N
    d=data.twoclassses()
    Y=d[:,"class"]
    X=d[:,1:]
    X = N.asarray(X)
    Y = N.asarray(Y)
    im_class = univMI(3)
    res = im_class.eval(X,Y)
    """

    def __init__(self, k):
        """
Set the value of k for the KNN
        """
        self.k = k

    # CALCUL DE I(X,Y) :
    def eval(self, X, Y):
        im = multiMI(self.k)
        results = N.zeros(N.size(X, 1))
        for i in range(N.size(X, 1)):
            Xi = X[:, i]
            Xi = N.reshape(Xi, [N.size(Xi), 1])
            value = im.eval(Xi, Y)
            results[i] = value

        return results


class MIKnnFloat(object):

    """
    Mutual Information : see Kraskov et al. 2004 "Estmating Mutual Information" for more details.
    Warning : Y could be float !

    MULTIVARIATE WAY

    Class for mutual information function, with "eval".

    Input :
    X : array (in R)
    Y : array of classes (in R)
    k : number of nearest neigboorhs to consider
    method : estimate 1 or 2 (see Kaskov for more detail - 1 is defaut)

    Output :
    MI(X,Y)


    Examples:
    import datamind.data as data
    from datamind.ml.func import *
    import numpy as N
    d=data.twoclassses()
    Y=d[:,"class"]
    X=d[:,1:]
    X = N.asarray(X)
    Y = N.asarray(Y)
    im_class = MIKnnFloat(3)
    for i in range(N.size(X,1)):
            print(im_class.eval(X[:,i],Y))
    """

    def __init__(self, k, method=1):
        """
Set the value of k for the KNN
        """
        self.k = k
        self.method = method

    # CALCUL DE I(X,Y) :
    def eval(self, X, Y):
        if self.method == 1:
            mi = self.ComputeEstimate1(X, Y)
        if self.method == 2:
            mi = self.ComputeEstimate2(X, Y)
        return mi

    def ComputeEstimate1(self, X, Y):
        k = self.k
        sum = 0
        for indice in range(N.size(X, 0)):
            normx_i = Distance(X, indice)
            normy_i = (Y - Y[indice])[:, 0]
            knn_i, epsilon_i = ComputeEpsilon(normx_i, normy_i, k)
            nx_i = N.size(N.where(normx_i < epsilon_i / 2)[0])
            digamma_x = SP.special.digamma(nx_i + 1)
            ny_i = N.size(N.where(normy_i < epsilon_i / 2)[0])
            digamma_y = SP.special.digamma(ny_i + 1)
            sum = sum + digamma_x + digamma_y
        mutual_information = SP.special.digamma(
            k) + SP.special.digamma(N.size(X, 0))
        mutual_information = mutual_information - \
            N.float(sum) / N.float(N.size(X, 0))
        return mutual_information

    def ComputeEstimate2(self, X, Y):
        k = self.k
        sum = 0
        for indice in range(N.size(X, 0)):
            normx_i = self.Distance(X, indice)
            normy_i = (Y - Y[indice])[:, 0]
            knn_i, epsilon_i = self.ComputeEpsilon(normx_i, normy_i, k)
            nx_i = N.size(N.where(normx_i <= normx_i[knn_i])[0])
            digamma_x = SP.special.digamma(nx_i)
            ny_i = N.size(N.where(normy_i <= normy_i[knn_i])[0])
            digamma_y = SP.special.digamma(ny_i)
            sum = sum + digamma_x + digamma_y
        mutual_information = SP.special.digamma(
            k) + SP.special.digamma(N.size(X, 0)) - 1. / k
        mutual_information = mutual_information - \
            N.float(sum) / N.float(N.size(X, 0))
        return mutual_information

    def ComputeEpsilon(self, normx_i, normy_i, k):
        normz_i = N.max(N.vstack((normx_i, normy_i)), 0)
        normz_i = N.reshape(normz_i, [N.size(normz_i), 1])
        knn_i = N.argsort(normz_i, 0)[k]
        epsilon_i = 2 * normz_i[knn_i]
        return knn_i, epsilon_i

    def Distance(self, S, indice):
        normi = S - S[indice, :]
        normi = N.sqrt(N.sum(normi * normi, 1))
        return normi
