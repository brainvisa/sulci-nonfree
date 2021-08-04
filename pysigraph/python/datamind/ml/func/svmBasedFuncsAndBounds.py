from __future__ import print_function

from __future__ import absolute_import
import numpy as N


class SvmLinWeights(object):

    """
    Return the weights of a linear multivariate SVM (fitted on all X's features)
    sum_i (alpha_i * SV_i)
    Can be used as a ranking function
    """

    def __init__(self, scale=True, C=1, type_svm='C-classification'):
        """
        scale: the data ? (default yes)
        C: the cost (default 1)
        type_svm (see R library e1071):
            *  'C-classification'
            *  'nu-classification'
            *  'one-classification' (for novelty detection)
            *  'eps-regression'
            *  'nu-regression'
        """
        self.scale = scale
        self.C = C
        import rpy
        rpy.r.eval(rpy.r.parse(text="""
        library(e1071);
        svmLin.weigths<-function(X,Y,scale = TRUE,cost=1,
            type_svm='C-classification'){
            if(type_svm=='C-classification'||type_svm=='nu-classification')
                Y=as.factor(Y)
            mod=svm(X,Y,scale=FALSE,kernel='linear',fitted=FALSE,cost=cost,type=type_svm)
            W = t(mod$SV) %*% mod$coefs
            return(W)
        }"""
                               ))

    def eval(self, X, Y):
        import rpy
        W = rpy.r.svmLin_weigths(X, Y, scale=self.scale, cost=self.C)
        # Two classes pb return |W|
        if W.shape[1] == 1:
            return N.abs(W.ravel())
        else:
            raise UserWarning("More than 2 class problem not implemented")

if __name__ == "__main__":
    import datamind.data as data
    import numpy as N
    d = data.twoclassses(deltaMeans=N.arange(2, 1, -0.1), nbNoize=1000)
    y = d[:, ["class"]]
    X = d[:, 1:]
    f = SvmLinWeights()
    scores = f.eval(X, y)
    print("Ranks of features (10 first are discriminatives)")
    print(N.argsort(scores)[::-1][:100])
    f = SvmLinWeights(C=10**-5)
    scores = f.eval(X, y)
    print("Ranks of features (10 first are discriminatives)")
    print(N.argsort(scores)[::-1][:100])
