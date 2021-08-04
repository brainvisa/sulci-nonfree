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
# Multivariate

class Multivariate (object):
    
    def __init__(self):
        pass
    
    def eval_MR(self, Y, X):
        '''
        Multiple regression 
        '''        
        clf = classif.LinearRegression()
        clf.fit(X, Y) 
        self.beta = clf.getEstimatedCoeff()[0:X.shape[1],:]
        self.error = clf.getError()

class Multivariate_glm(Multivariate):
      
    """
    Multivariate Analysis of Variance For Regression problem.
    
    Input :
            X : n*p model matrix of full column rank with n is the number of
                            observation and p is the number of regressors
            y : n*m vector of observation on a response variable
            H : q*p hypothesis matrix, q hypothesis on the linear combination
                    of the rows (p) of beta. If H is None, we consider the complete
                    model =>  H is a vector of p one
            T  : multivariate test statistics
            val : type of statistic value returned
        init_anova : bool initialized to False. For the Manova method, we need to
                compute the ANOVA only one time. Once calculated, init_anova = True. 
    Output :
           Fisher statitiscs (F scores, p-value or z-value)
    Method :
            1) Linear Regression of the multivariate model Y = X*beta + error
                    Y is the matrix n*m of responses, where each 
                    column represents a distinct variable. 
            2) Calcul of the hypothesis sum of squares and product matrix SSPh and the
            error sum of squares and product matrix SSPe
                    SSPh = (Hbeta)^T [H (X^T X)^-1 H^T]^-1 (Hbeta)
                    SSPe = error^T error
            3) Eigenvalue of SSPh SSPe^-1 (eig(between variance/within variance))
            4) multivariate test statistics and its transposition to F-test
    Example
    """
    def __init__(self, H = None, T = 'Pillai', val = 'fval'):
        Multivariate.__init__(self)
        self.H = H
        self.T = T
        self.val = val
            
    def multivariate_test(self):
        r_HinvXtXHt = rank(self.HinvXtXHt) #rank of L(XtX)-1Lt
        r_HplusE = rank(self.SSPh + self.SSPe) #rank of (H+E)
        min_r = min(r_HinvXtXHt, r_HplusE) #s=min(m,q)
        df_err = self.error.shape[0] - r_HplusE #error degrees of freedom n-m

        if self.T == 'Pillai': 
            t = sum(self.proj_eigval/(1.0+self.proj_eigval))
            df1_t = df_err-r_HplusE + min_r 
            df2_t = abs(r_HplusE-r_HinvXtXHt)  + min_r 
            self._df1_f = min_r * df2_t
            self._df2_f = min_r * df1_t
            self._fval = (df1_t/df2_t)*(t / (min_r-t))
        elif  self.T == 'Hotelling-Lawley': 
            t = sum(self.proj_eigval)
            df1_t = (df_err-r_HplusE-1.0)*min_r + 2.0 
            df2_t = (abs(r_HplusE-r_HinvXtXHt)  + min_r ) * min_r**2
            self._df1_f = df2_t / min_r
            self._df2_f = df1_t
            self._fval = (df1_t * t) / df2_t
        elif  self.T == 'Wilks': 
            t = N.prod(1.0/(1.0 + self.proj_eigval))
            a = df_err - ((r_HplusE - r_HinvXtXHt + 1.0)/2.0)
            b = (r_HplusE * r_HinvXtXHt - 2)/4.0
            if r_HplusE**2 + r_HinvXtXHt**2 - 5.0 > 0 : 
                c = N.sqrt( (r_HplusE**2 * r_HinvXtXHt**2 - 4.0) / (r_HplusE**2 + r_HinvXtXHt**2 - 5.0))
            else :
                c = 1.0
            df1_t = a*c - 2*b
            df2_t = r_HplusE * r_HinvXtXHt
            self._df1_f = df2_t
            self._df2_f = df1_t
            self._fval = ( (1.0 - pow(t, 1.0/c)) / pow(t, 1.0/c)) * (df1_t/df2_t)
        elif  self.T == 'Roy': 
            t = max(self.proj_eigval)
            df1_t = df_err - max(r_HplusE, r_HinvXtXHt) + r_HinvXtXHt
            df2_t = max(r_HplusE, r_HinvXtXHt)
            self._df1_f = df2_t
            self._df2_f = df1_t
            self._fval = t * (df1_t/df2_t)
                            
        self._pval = None
        self._zval = None
        if self.val == 'pval' : 
            all_value = self.pvalue()
        elif self.val == 'zval' : 
            all_value = self.zvalue()
        else : 
            all_value = self._fval
        return all_value
                    
    def pvalue(self):
        if self._pval is not None:
            return self._pval
        self._pval = SS.f.sf(self._fval, self._df1_f, self._df2_f)
        return self._pval

    def zvalue(self):
        if self._zval is not None:
            return self._zval
        p = self.pvalue()
        z = SS.norm.isf(p)
        # To avoid inf values kindly supplied by scipy
        th = z[N.where(z < N.inf)].max()
        self._zval = N.minimum(z, int(th+1)) 
        return self._zval
    
    
    def anova(self, X, Y):
        self.anov = SimpleLinRegFstat()
        self._fval    = self.anov.eval(X, Y)
        if self.val == 'pval' : 
            all_value = self.anov.pvalue()
        elif self.val == 'zval' : 
            all_value = self.anov.zvalue()
        else : 
            all_value = self._fval
        return all_value
    
    def eval(self,Y,X, H = None):
        '''
Returns the eigenvalues of the MANOVA model
eig = eval(self,Y,X, H = None)
INPUT :
        Y : Array of shape(n,m), observation on a response variable
X : Array of shape (n,p) model matrix of full column rank where n is the number of
                                        observation and p is the number of regressors
        H=None : Array of shape (q,p) hypothesis matrix, q hypothesis on the linear combination
                        of the rows (p) of beta. 
        If H == None, w  H=ones(p) is used
OUTPUT :
        eig= (p) array of eigenvalues 
        '''       
        if N.size(X) == X.shape[0] :
            X = N.reshape(X, (X.shape[0], 1))
        
        # if Y is an array of shape (n, 1) or it is the first MANOV eval (init_anova == False), we compute an ANOVA. 
        # Be careful! For the anova call, we have to give first X and then Y. 
        if Y.shape[1] == 1 :
            return self.anova(X, Y)
        
        self.eval_MR(Y, X)
        
        if H != None : 
            if self.H.shape[1] != X.shape[1] :
                raise exceptions.ValueError('Hypothesis matrix H must have the shape (h, dim1(X)) ')
            self.H = H
        else :
            self.H = N.ones([1, X.shape[1]])
                                
                
        Hbeta = N.dot(self.H, self.beta)
        Hbetat = N.dot( N.transpose(self.beta), N.transpose(self.H))
        invXtX = pinv(N.dot(N.transpose(X), X))
        self.HinvXtXHt = N.dot(N.dot(self.H, invXtX), N.transpose(self.H))
        inv_HinvXtXHt = pinv(self.HinvXtXHt)
        self.SSPh = N.dot( N.dot( Hbetat, inv_HinvXtXHt), Hbeta)
        self.SSPe = N.dot(N.transpose(self.error), self.error)

        if self.T == 'Pillai': 
            proj_matrix =  N.dot(self.SSPh, pinv(self.SSPh + self.SSPe) ) 
            # t = N.trace(proj_matrix)
        elif  self.T == 'Hotelling-Lawley': 
            proj_matrix =  N.dot(pinv(self.SSPe), self.SSPh) 
            # t = N.trace(proj_matrix)
        elif  self.T == 'Wilks': 
            proj_matrix =  N.dot(self.SSPh, pinv(self.SSPh + self.SSPe) ) 
            # t = LA.determinant(self.SSPe) / LA.determinant(self.SSPh + self.SSPe)
        elif  self.T == 'Roy': 
            proj_matrix =  N.dot(pinv(self.SSPe), self.SSPh)
            # t = max(self.proj_eigval)
        # from pylab import eig
        # self.proj_eigval, eigvect = eig(proj_matrix)
        self.proj_eigval, eigvect = self.svd(proj_matrix)
        self.proj_eigval = N.array(N.real(self.proj_eigval))
        self.proj_eigval = self.proj_eigval[N.nonzero( abs(self.proj_eigval) > pow(10, -1) ) [0]]
        self.proj_eigval = N.array(sorted(self.proj_eigval, reverse = True))
        self.proj_eigval = self.proj_eigval[0:self.H.shape[0]]

        return self.multivariate_test()
        
    def svd(self, X):
        # computational trick to compute svd. U, S, V=linalg.svd(X)
        import scipy as Sc
        K = N.dot(X.T, X)
        S, V = Sc.linalg.eigh(K)     
        S = N.sqrt(N.maximum(S, 1e-30))
        S_sort = -N.sort(-S)[:X.shape[0]]
        S_argsort = N.argsort(-S).tolist()
        V = V.T[S_argsort,:]
        V = V[:X.shape[0],:]
        return S_sort, V    
        
    def setParams(self, Params):
        self.H = Params[0]
        self.T = Params[1]
        self.val = Params[2]

class Manova_RSS(Multivariate):
    '''
    Manova_RSS is a Multivariate_glm which returns the residual sum of squares
    '''
    def __init__(self):
        Multivariate.__init__(self)
       
    def anova(self, X, Y):
        self.anov = SimpleLinRegFstat()
        self.anov.eval(X, Y)
        self.beta = self.anov.getEstimatedCoeff()
        return self.RSS(X, Y)
    
    def RSS(self, X, Y):
        err = Y - N.dot(X,  N.transpose(self.beta))
        if len(err.shape) == 1:
            rss = N.sum(err*err)
        else :
            rss = N.sum(N.sum(err*err))
        return rss 
    
    def eval(self, Y, X):
        if N.size(X) == X.shape[0] :
            X = N.reshape(X, (X.shape[0], 1))
        
        # if Y is an array of shape (n, 1) or it is the first MANOV eval (init_anova == False), we compute an ANOVA. 
        # Be careful! For the anova call, we have to give first X and then Y. 
        if Y.shape[1] == 1 :
            return self.anova(X, Y)
        self.eval_MR(Y, X)
        return self.RSS(Y, X)
        

class ManovaLDA(object):
        """
        Manova multivariate F statistic (based on Pillai-Barlett trace)
        It is based on LDA
        Example:
        import datamind.data as data
        d=data.twoclassses(stringClassLabels=True)
        X=d[:,"class"]
        Y=d[:,1:]
        manova=ManovaLDA()
        manova.eval(Y,X)
        manova.pvalue()
        manova.zvalue()
        """
        def __init__(self,priors=None):
                self.priors = priors
        def eval(self, Y, X):
                import datamind.ml.classif
                lda = datamind.ml.classif.LDA(priors=self.priors)
                lda.fit(Y, X)
                self.approxF, self.p, self.z = lda.getStat()
                return self.approxF
        def pvalue(self): return self.p
        def zvalue(self): return self.z


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

