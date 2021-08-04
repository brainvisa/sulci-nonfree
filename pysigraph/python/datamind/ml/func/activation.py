from __future__ import print_function

from __future__ import absolute_import
import numpy as N
from six.moves import range

class Activation(object):
    """
    Activation-based test.
    Significant response or activation to any of the different conditions compared to baseline effect. 
    If 2 classes or conditions (c0, c1), for each feature f, we compute the mean between Ac0(f) and Ac1(f)
    with Ac0(f) = mco(f)/sqrt(varc0(f)/nco) and Ac1(f) = mc1(f)/sqrt(varc1(f)/nc1),  
    mco(f) (mc1(f)) and  varc0(f) (varc1(f)) are respectively the mean and variance 
    over the nc0 (nc1) trials of condition c0 (c1) at feature f.
    
    Returns: activation scores for each features

    Examples:
    import datamind.data as data
    import numpy as N
    d=data.twoclassses(nbSamples=30,deltaMeans=N.arange(2,0,-.1),nbNoize=100)
    y=d[:,["class"]]
    X=d[:,1:]
    
    act=Activation()
    acts=act.eval(X,Y)
    print("acts for each variable (the 10 first are discriminatives)")
    print(acts)
    print("Variables ranks sorting with decreasing acts (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(acts)]).ravel())
    
    """
    def __init__(self):
        pass

    def eval(self, X, Y):
        """
        One sample |T-test| on the 2 populations (to extend to more than 2 pop)
        OUTPUT : _stats :mean of the 2 t-test scores
        """
        labels = N.unique(Y)      
        self._acts = N.zeros(X.shape[1])
        self._maxacts = N.zeros(X.shape[1])
        self._A0_i = N.zeros(X.shape[1])
        self._A1_i = N.zeros(X.shape[1])
        self._dof = N.zeros(X.shape[1])
        c0 = N.nonzero(Y == labels[0])[0]
        c1 = N.nonzero(Y == labels[1])[0]
        self._c = [len(c0), len(c1)]
        import fff.group as fg
        self._A0_i = abs(fg.onesample.stat(X[c0]))[0]
        self._A1_i = abs(fg.onesample.stat(X[c1]))[0]
        # self._acts = (self._A0_i + self._A1_i)/2.0
        for i in range(X.shape[1]) :
            self._acts[i] = max(self._A0_i[i], self._A1_i[i])
        if Y.ndim == 1:
            self._dof = float(Y.shape[0] - 1)
        else :
            self._dof = float(Y.shape[0] - Y.shape[1])
        self._pval = None
        self._zval = None
        return self._acts
    
    def pvalue(self):
        '''
        P(|Tc|>t) = P(Tc>t) + P(Tc<-t) = 2*P(Tc>t)
        P(|T0|>t0 U |T1|>t1 U...) = 1-P(|T0|<=t0 and |T1|<=t1 and...) = 1-P(|T0|<=t0)*P(|T1|<=t1)*...
                                  = 1 -(1-P(|T0|>t0))*((1-P(|T1|>t1))*...
        '''
        if self._pval is not None:
            return self._pval
        import scipy.stats as SS
        self._pval0 = 2*SS.t.sf(self._A0_i, self._c[0]-1)
        self._pval1 = 2*SS.t.sf(self._A1_i, self._c[1]-1)
        self._pval = 1-(1-self._pval0)*(1-self._pval1)
        print("min p:", min(self._pval))
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

class Activation_Anova(object):
    def __init__(self):
        pass

    def eval(self, X, Y):
        """
        """
        from datamind.ml import dimred
        from datamind.ml.func.activation import Activation
        from datamind.ml.func.glm import SimpleLinRegFstat
        drano = SimpleLinRegFstat()
        anos = drano.eval(X, Y)
        
        dract = Activation()
        acts = dract.eval(X, Y)
        self._acts_anos = acts+anos[0,:]
        if Y.ndim == 1:
            self._dof = float(Y.shape[0] - 1)
        else :
            self._dof = float(Y.shape[0] - Y.shape[1])
        self._pval = None
        self._zval = None
        return self._acts_anos
         
    def pvalue(self):
        if self._pval is not None:
            return self._pval
        import scipy.stats as SS
        self._pval = SS.t.sf(self._acts_anos, self._dof)
        print("min p:", min(self._pval))
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
    
if __name__ == "__main__":
    import datamind.data as data
    import numpy as N
    # d=data.twoclassses(nbSamples=60,nbNoize=1,stringClassLabels=True)[10:,:]
    # Y=d[:,["class"]]
    # X=d[:,1:]
    
    # act=Activation()
    # acts=act.eval(X,Y)
    # print("acts for each variable (the 10 first are discriminatives)")
    # print(acts[:10])
    # print("Variables ranks sorting with decreasing acts (the 10 first are discriminatives)")
    # print(N.fliplr([N.argsort(acts)]).ravel()[:10])
    
    # act_ano=Activation_Anova()
    # acts_anos=act_ano.eval(X,Y)
    # print("acts_anos for each variable (the 10 first are discriminatives)")
    # print(acts_anos[:10])
    # print("Variables ranks sorting with decreasing acts_anos (the 10 first are discriminatives)")
    # print(N.fliplr([N.argsort(acts_anos)]).ravel()[:10])
    
    import numpy.random as NR
    nbitems = 100
    dim = 1000
    signal = 1.0
    
    # generate the data
    # Y = N.zeros((nbitems,1))
    # Y[:60] = 1.0
    # alpha = N.zeros((dim))
    # alpha[:10] = signal#N.exp(N.log(0.5)*N.arange(10))
    # SN = NR.randn(nbitems,dim)
    # SP = N.zeros((nbitems,dim))
    # SP[:] =  alpha
    # X = SN + SP*N.reshape(Y, (nbitems,1))
    # GLM
    # act=Activation()
    # acts=act.eval(X,Y)
    # print("acts for each variable (the 10 first are discriminatives)")
    # print(acts[:10])
    # print("Variables ranks sorting with decreasing acts (the 10 first are discriminatives)")
    # print(N.fliplr([N.argsort(acts)]).ravel()[:10])
    
    # signal = 0.6
    # dim = 60000
    # Y = N.zeros((nbitems,1))
    # Y[:60] = 1.0
    # alpha = N.zeros((dim))
    # alpha[:10] = signal#N.exp(N.log(0.5)*N.arange(10))
    # SN = NR.randn(nbitems,dim)
    # SN[:, 10:20] = signal
    # SP = N.zeros((nbitems,dim))
    # SP[:] =  alpha
    # X = SN + SP*N.reshape(Y, (nbitems,1))
    
    
    # act=Activation()
    # acts=act.eval(X,Y)
    # acts_p=act.pvalue()
    # print("acts for each variable (the 10 first are discriminatives)")
    # print(acts[:20])
    # print("Variables ranks sorting with decreasing acts (the 10 first are discriminatives)")
    # acts_sort = N.fliplr([N.argsort(acts)]).ravel()[:20]
    # print(acts_sort)
    # print("pvalue:",acts_p[acts_sort])
    
    # act_ano=Activation_Anova()
    # acts_anos =act_ano.eval(X,Y)
    # print("acts_anos for each variable (the 10 first are discriminatives)")
    # print(acts_anos[:20])
    # print("Variables ranks sorting with decreasing acts_anos (the 10 first are discriminatives)")
    # print(N.fliplr([N.argsort(acts_anos)]).ravel()[:20])
    
    # from datamind.ml.func.activation import *
    # from datamind.ml.func.glm import SimpleLinRegFstat
    # drano = SimpleLinRegFstat()
    # anos = drano.eval(X, Y)
    # print("anos for each variable (the 10 first are discriminatives)")
    # print(anos[:20])
    # print("Variables ranks sorting with decreasing anos (the 10 first are discriminatives)")
    # print(N.fliplr([N.argsort(acts)]).ravel()[:20])
    
    import pickle
    Swd = '/neurospin/lnao/Pmad/Cecilia/database/'
    # database = Swd+'voxels/simul/db_r_rsimu_umu_signal8e-1.pic'
    # database = Swd+'voxels/0031/db_spmt_WordReaders_31.pic'
    # database = Swd+'voxels/0029/db_lateralisationBin_29.pic'
    # database = Swd+'voxels/0025/db_pb_GD_ego_25.pic'
    # database = Swd+'voxels/0029/db_pb_GD_ego_29.pic'
    database = Swd+'voxels/0029/db_spmt_sexe_29.pic'
    # database = Swd+'voxels/0031/db_lecture_ok_31.pic'
    # database = Swd+'voxels/0021/db_spmt_lecture_ok_21.pic'

    fic = open(database, 'rb')
    db_data = pickle.load(fic)
    fic.close()
    
    X = db_data['db_X'][:180,:]
    Y = db_data['db_Y'][:180]
    
    # act=Activation()
    # acts=act.eval(X,Y)
    # acts_p=act.pvalue()
    # ind = N.nonzero(acts_p<=0.01)[0]
    # print("acts for each variable (the 10 first are discriminatives)")
    # print(acts[ind])
    # print(acts_p[ind])
    
    from datamind.ml import dimred
    from datamind.ml.func.glm import SimpleLinRegFstat
    fdr = dimred.UnivRanking(func=Activation(), zval=3.0)
    # fdr = dimred.UnivRanking(func=SimpleLinRegFstat(), zval=3.0)
    fdr.fit(X, Y)
    dbX = fdr.reduce(X)
    print("min pvalue:", min(fdr.getPvalues()))
    nbf = len(fdr.getSelectedFeatures())
    print("nbf:", nbf)
    print("z:", fdr.getZvalues()[fdr.getSelectedFeatures()])
