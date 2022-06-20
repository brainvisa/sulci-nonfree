from __future__ import print_function

from __future__ import absolute_import
import numpy as N
from numpy.linalg import pinv
import datamind.stats.design as design

# import matplotlib.mlab as MM
# import LinearAlgebra as LA
import scipy.stats as SS

from datamind.ml import classif
from datamind.ml import scaling

def rank(x):
    """
    Returns the rank of a matrix.
    The rank is understood here as the an estimation of the number of
    linearly independent rows or columns (depending on the size of the
    matrix).
    [copied from an old version of matplotlib.mlab.rank
    this function is deprecated in latest version of matplotlib]
    """
    x = N.asarray(x)
    s = N.linalg.svd(x, compute_uv=False)
    maxabs = N.max(N.absolute(s))
    maxdim = max(x.shape)
    tol = maxabs * maxdim * 1e-13
    return N.sum(s > tol)



# ==============================================================================
# Linear Model based objective function
# ==============================================================================
# Provide:
# - OneWayAnovaFstat
# - SimpleLinRegFstat
# - OneWayAnovaEffect
# - SimpleLinRegEffect

# ------------------------------------------------------------------------------
# Fstat(s)
class Fstat(object):
    """
    Fstat based on FFF glm, Don't use this class directly, subclass it,
    and overload the function formatDesignMatAndGetFContrast(X) that
    format the design matrix (eventually do nothing) and build the contrast.
    See OneWayAnovaFstat for an example
    # See "eval" function
    """
    def eval(self, Y, X, Fcontrast=None):
        """
        Y : array of real values, typically a 2D array nb samples * nd features
        X : vector of group labels
            It can also be a design matrix, in this can the Fcontrast MUST
            be provided.
        For each features calulate an anova defined by X
        """
        # If the contrast is not provided assume that X is a vector
        # of group labels, then build the desing matrix
        if Fcontrast is None:
            X, Fcontrast = self.formatDesignMatAndGetFContrast(X)
        import nipy.neurospin.glm
        mf = nipy.neurospin.glm.glm(Y, X)
        # fff2=> mf.fit();
        self.beta = mf.beta
        conf = mf.contrast(Fcontrast, type='F', tiny=1e-10)
        self._dof1 = conf.dof
        self._dof2 = conf.dim
        # self._conf=conf
        self._fval = conf.stat()
        # self._fval=conf.test(tiny=1e-10,pval=False)
        
        self._pval = None
        self._zval = None
        return self._fval
        
    def getEstimatedCoeff(self) : return self.beta
    
    def pvalue(self):
        if self._pval is not None:
            return self._pval
        import scipy.stats as SS
        self._pval = SS.f.sf(self._fval, self._dof2, self._dof1)
        return self._pval

    def zvalue(self):
        if self._zval is not None:
            return self._zval
        import scipy.stats as SS
        p = self.pvalue()
        z = SS.norm.isf(p)
        # To avoid inf values kindly supplied by scipy
        th = z[N.where(z < N.inf)].max()
        self._zval = N.minimum(z, int(th+1)) 
        return self._zval

class OneWayAnovaFstat(Fstat):
    """
    One-way Anova
    
    Examples:
    d=data.twoclassses(stringClassLabels=True)
    X=d[:,"class"]
    Y=d[:,1:]
    aov=OneWayAnovaFstat()
    print("F-values------------")
    print(aov.eval(Y,X))
    print("p-values------------")
    print(aov.pvalue())
    print("z-values------------")
    print(aov.zvalue())
    
    # Get design matrix and Fcontrast
    dMat,Fcon=aov.formatDesignMatAndGetFContrast(X)
    # Give them to eval (to avoid useless re-computation)
    print(aov.eval(Y,dMat,Fcon))

    """
    def formatDesignMatAndGetFContrast(self, X):
        return design.factorCodingOneWayAnova(X, intercept=True, Fcontrast=True)


class SimpleLinRegFstat(Fstat):
    """
    Simple linear regression = 1 regressor + intercept
    
    Examples:
    import N.random as rnd
    from datamind.core import DF
    rnd.normal()
    x1=rnd.normal(size=20)
    x2=rnd.normal(size=20)
    y=x1+2*x2+10+rnd.normal(size=20)
    d=DF(N.array([x1,x2,y]).T,colnames=["x1","x2","y"])
    # In a classification context Y+I is the design matrix, and X contains
    # the features: for each feature x in X fit x = beta1 Y+ beta0 I
    Y=d[:,["x1","x2"]]
    X=d[:,["y"]]
    lr=SimpleLinRegFstat()
    print("F-values------------")
    print(lr.eval(Y,X))
    print("p-values------------")
    print(lr.pvalue())
    print("z-values------------")
    print(lr.zvalue())
    
    Validity between naive formula (Y = Xi betai + Ei) and vectorized form  (X = Y beta + E) :
    import numpy as N
    from numpy.random import randn
    nbitems = 10
    dim = 20
    nu = 0.5
    alpha = N.zeros((dim))
    alpha[:5] = N.exp(N.log(0.5)*N.arange(5))
    X = randn(nbitems,dim)
    noise = nu*randn(nbitems)
    Y = N.dot(X,alpha) + noise
    
    from datamind.ml.func import SimpleLinRegFstat
    anov = SimpleLinRegFstat()
    fval= anov.eval(X, Y)
    fval2 = N.zeros((1,dim))
    for i in range(dim): fval2[0,i] = anov.eval(Y,X[:,i])
    print(fval, fval2)
    """
    def formatDesignMatAndGetFContrast(self, X):
        return design.addIntercept(X, Fcontrast=True)

# ------------------------------------------------------------------------------
# Effect
class Effect(object):
    """
    Effect based on FFF glm, Don't use this class directly, subclass it,
    and overload the function formatDesignMatAndGetFContrast(X) that
    format the design matrix (eventually do nothing) and build the contrast.
    See OneWayAnovaFstat for an example
    # See "eval" function
    """
    def eval(self, Y, X, Fcontrast=None):
        """
        Y : array of real values, typically a 2D array nb samples * nd features
        X : vector of group labels
            It can also be a design matrix, in this can the Fcontrast MUST
            be provided.
        For each features calulate an anova defined by X
        """
        # If the contrast is not provided assume that X is a vector
        # of group labels, then build the desing matrix
        if Fcontrast is None:
            X, Fcontrast = self.formatDesignMatAndGetFContrast(X)
        import nipy.neurospin.glm
        mf = nipy.neurospin.glm.glm(Y, X)
        #=>fff2 mf.fit();
        conf = mf.contrast(Fcontrast, type='F')
        return conf.effect#.ravel()
    
class OneWayAnovaEffect(Effect):
    """
    One-way Anova, return the effect
    
    Examples:
    # See OneWayAnovaFstat
    aovEffect=OneWayAnovaEffect()
    print("Effect ------------")
    print(aovEffect.eval(Y,X))
    """
    def formatDesignMatAndGetFContrast(self, X):
        return design.factorCodingOneWayAnova(X, intercept=True, Fcontrast=True)


class SimpleLinRegEffect(Effect):
    """
    Simple linear regression = 1 regressor + intercept, return the effect
    
    Examples:
    # See SimpleLinRegFstat
    lrEffects=SimpleLinRegEffect()
    print("Effect ------------")
    print(lr.eval(Y,X))
    
    """
    def formatDesignMatAndGetFContrast(self, X):
        return design.addIntercept(X, Fcontrast=True)


# ------------------------------------------------------------------------------
# Tstat(s)
class Tstat(object):
    """
    Tstat based on FFF glm, Don't use this class directly, subclass it,
    and overload the function formatDesignMatAndGetFContrast(X) that
    format the design matrix (eventually do nothing) and build the contrast.
    See TwoSamplesTtest for an example
    # See "eval" function
    """
    def eval(self, Y, X, Tcontrast=None):
        """
        Y : array of real values, typically a 2D array nb samples * nd features
        X : vector of group labels
            It can also be a design matrix, in this can the Tcontrast MUST
            be provided.
        For each features calulate an anova defined by X
        """
        # If the contrast is not provided assume that X is a vector
        # of group labels, then build the desing matrix
        if Tcontrast is None:
            X, Tcontrast = self.formatDesignMatAndGetTContrast(X)
        import nipy.neurospin.glm
        mf = nipy.neurospin.glm.glm(Y, X)
        # fff2=> mf.fit();
        cont = mf.contrast(Tcontrast, type='t', tiny=1e-10)
        self._dof = cont.dof
        self._tval = cont.stat()
        # =>fff2 self._tval=cont.test(pval=False)
        self._pval = None
        self._zval = None
        return self._tval
      
    def formatDesignMatAndGetTContrast(self, X):
        return design.addIntercept(X, Fcontrast=True)  
    
    def pvalue(self):
        if self._pval is not None:
            return self._pval
        import scipy.stats as SS
        self._pval = SS.t.sf(self._tval, self._dof)
        return self._pval

    def zvalue(self):
        if self._zval is not None:
            return self._zval
        import scipy.stats as SS
        p = self.pvalue()
        z = SS.norm.isf(p)
        # To avoid inf values kindly supplied by scipy
        th = z[N.where(z < N.inf)].max()
        self._zval = N.minimum(z, int(th+1)) 
        return self._zval

class TwoSamplesTtest(Tstat):
    """
    TwoSamples T-test
    
    Examples:
    d=data.twoclassses(stringClassLabels=True)
    X=d[:,"class"]
    Y=d[:,1:]
    
    ttest=TwoSamplesTtest()
    self=ttest
    print("T-values------------")
    print(ttest.eval(Y,X))
    print("p-values------------")
    print(ttest.pvalue())
    print("z-values------------")
    print(ttest.zvalue())
    self=ttest
    
    print("TwoSamplesTtest: (2) Two classes small toy set, explicit contrast --")
    print("A>B == -B<A ?")
    from datamind.core import DF
    print(TwoSamplesTtest(groupOfInterest=DF.code("A")).eval(Y,X)==\
    -TwoSamplesTtest(groupOfInterest=DF.code("B")).eval(Y,X))
    """
    def __init__(self, groupOfInterest=None):
        """
        groupOfInterest: the label of the group of interest.
        Example: if x is [1,1,1,0,0,0]
            groupOfInterest=1 will test 1>0
            groupOfInterest=0 will test 0>1
        """
        self.groupOfInterest = groupOfInterest
    def formatDesignMatAndGetFContrast(self, X):
        dMat = design.factorCodingTwoSamplesTtest(X, intercept=False, Tcontrast=False)
        if self.groupOfInterest is None:
            tcont = N.array([1, -1])
        else:
            labels = N.unique(X)
            which = self.groupOfInterest == labels
            if N.sum(which) != 1:
                raise exceptions.ValueError("groupOfInterest was not found in x")
            tcont = which.astype("int")
            tcont[tcont == 0] = -1
        return dMat, tcont
# ------------------------------------------------------------------------------


if __name__ == "__main__":
        import datamind.data as data
        d = data.twoclassses(stringClassLabels=True)
        X = d[:, ["class"]]
        Y = d[:, 1:]
        
        print("OneWayAnovaFstat: Two classes small toy set ========================")
        aov = OneWayAnovaFstat()
        print("F-values------------")
        print(aov.eval(Y, X))
        print("p-values------------")
        print(aov.pvalue())
        print("z-values------------")
        print(aov.zvalue())
        
        # Get design matrix and Fcontrast
        dMat, Fcon = aov.formatDesignMatAndGetFContrast(X)
        # Give them to eval (to avoid useless re-computation)
        print(aov.eval(Y, dMat, Fcon))
        
        print("Compare with R ----")
        d.write("/tmp/testR.csv")
        import rpy
        a = rpy.r.eval(rpy.r.parse(text="""
                d=read.table(\"/tmp/testR.csv\",header=T);
                print(summary(aov(I0_2~class,data=d)));
                print(summary(aov(I1_1~class,data=d)));
                print(summary(aov(I2_0.5~class,data=d)));
                print(summary(aov(N0~class,data=d)));
                print(summary(aov(N1~class,data=d)));
                """))
        
        print("TwoSamplesTtest: (1) Two classes small toy set ========================")
        ttest = TwoSamplesTtest()
        self = ttest
        print("T-values------------")
        print(ttest.eval(Y, X))
        print("p-values------------")
        print(ttest.pvalue())
        print("z-values------------")
        print(ttest.zvalue())
        self = ttest
        
        print("TwoSamplesTtest: (2) Two classes small toy set, explicit contrast --")
        print("A>B == -B<A ?")
        from datamind.core import DF
        print(TwoSamplesTtest(groupOfInterest=DF.code("A")).eval(Y, X) ==\
        -TwoSamplesTtest(groupOfInterest=DF.code("B")).eval(Y, X))
        
        print("T-values^2 - F-values: -------")
        print(aov.eval(Y, X).ravel()-ttest.eval(Y, X)**2)

        print("OneWayAnovaEffect on toy data set =================================")
        aovEffect = OneWayAnovaEffect()
        print("Effect ------------")
        print(aovEffect.eval(Y, X))

        print("Manova on toy data set =================================")
        manova = ManovaLDA()
        print("Manova ------------")
        print(manova.eval(Y, X))
        
        print("OneWayAnovaFstat on Iris data set ==================================")
        iris = data.iris()
        X = iris[:, "Species"]
        Y = iris[:, :-1]
        aov = OneWayAnovaFstat()
        print("F-values------------")
        print(aov.eval(Y, X))
        print("p-values------------")
        print(aov.pvalue())
        print("z-values------------")
        print(aov.zvalue())
        
        print("Compare with R -----")
        iris.write("/tmp/iris.csv")
        a = rpy.r.eval(rpy.r.parse(text="""
                d=read.table(\"/tmp/iris.csv\",header=T);
                print(summary(aov(Sepal.Length~Species,data=d)));
                print(summary(aov(Sepal.Width~Species,data=d)));
                print(summary(aov(Petal.Length~Species,data=d)));
                print(summary(aov(Petal.Width~Species,data=d)));
                """))

        print("SimpleLinRegFstat toy data set =====================================")
        import numpy.random as rnd
        from datamind.core import DF
        rnd.normal()
        x1 = rnd.normal(size=20)
        x2 = rnd.normal(size=20)
        y = x1+2*x2+10+rnd.normal(size=20)
        d = DF(N.array([x1, x2, y]).T, colnames=["x1", "x2", "y"])
        # In a classification context Y+I is the design matrix, and X contains
        # the features: for each feature x in X fit x = beta1 Y+ beta0 I
        Y = d[:, ["x1", "x2"]]
        X = d[:, ["y"]]
        lr = SimpleLinRegFstat()
        print("F-values------------")
        print(lr.eval(Y, X))
        print("p-values------------")
        print(lr.pvalue())
        print("z-values------------")
        print(lr.zvalue())
        
        print("Compare with R (square the t-values) -----")
        d.write("/tmp/testR.csv")
        a = rpy.r.eval(rpy.r.parse(text="""
                d=read.table(\"/tmp/testR.csv\",header=T);
                print(summary(lm(x1~y,data=d)));
                print(summary(lm(x2~y,data=d)))
                """))
 
        print("SimpleLinRegEffect toy data set ===================================")
        lrEffects = SimpleLinRegEffect()
        print("Effect ------------")
        print(lr.eval(Y, X))

