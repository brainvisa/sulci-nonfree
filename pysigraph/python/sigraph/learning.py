# -*- coding: utf-8 -*-
# Copyright CEA (2000-2006)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
#
# This software is governed by the CeCILL license version 2 under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import numpy
import sigraph
import datamind
from soma import aims
from six.moves import range
if sigraph.Settings.debug:
    datamind.Settings.debug = True
import datamind.ml.classifier.optimizers
from sigraph.grid_io import *
from datamind.tools import *

# datamind plugin registration
from . import datamind_backend
# datamind plugin loading
datamind_backend.SigraphDataMindPlugin().load()
import datamind.ml.model


# replace Trainer.trainOne() function
def trainer_trainOne(self, it, obj):
    par = aims.carto.PyObjectfromObject(obj.get())
    mod = it.model()
    if not mod.isAdaptive():
        print('skip learning of non adaptive model')
        return
    if not mod.topModel():
        print('Trainer.trainOne: no TopModel')
        return
    labels = [x for x in mod.topModel().significantLabels()
              if x != 'unknown']
    print(' * labels : ', ' '.join(labels))
    err = 0
    mgraph = self.getGraphModel()
    modelfilename = mgraph['aims_reader_filename']
    prefix = os.path.dirname(modelfilename)
    try:
        fnamebase = mgraph['filename_base']
    except:
        fnamebase = '*'
    if fnamebase == '*':
        fnamebase = os.path.basename(modelfilename)
        p = fnamebase.rfind('.')
        if p >= 0:
            fnamebase = fnamebase[:p]
        fnamebase += '.data'
    if prefix == '':
        prefix = '.'
    prefix = os.path.join(prefix, fnamebase)
    # print('prefix:', prefix)
    m = self.mode()
    opt = {}
    opt['dimreduction'] = par.dimreduction_mode
    opt['optimized_dim'] = par.dimreduction_optim
    opt['predict_train'] = par.predict_train
    opt['selected_dim'] = par.selected_dim
    opt['gaussian_mode'] = par.gaussian_mode

    dimred_modes = ['None', 'univariate,SVC', 'univariate,duch_blend',
                    'univariate,matt_blend', 'univariate,random',
                    'univariate,permutation_wilcoxon',
                    'univariate,kullback_leibler', 'PLS1', 'SVD', 'ICA']
    optimized_dim_modes = ['None', 'rankloop', 'forwardbackward']

    if not (opt['dimreduction'] in dimred_modes):
        msg.error("dimreduction_mode : unknown mode '%s'" %
                  opt['dimreduction'])
        msg.info("try one of : %s" % str(dimred_modes))
        sys.exit(1)
    if not (opt['predict_train'] in [0, 1]):
        msg.error("'%d' bad value for predict_train : 0 or 1" %
                  opt['predict_train'])
        sys.exit(1)
    if not (opt['optimized_dim'] in optimized_dim_modes):
        msg.error("dimreduction_optim : unknown mode '%s'" %
                  opt['optimized_dim'])
        msg.info("try one of : %s" % str(optimized_dim_modes))
        sys.exit(1)

    if opt['dimreduction'] == 'None':
        opt['dimreduction'] = False
    if opt['optimized_dim'] == 'None':
        opt['optimized_dim'] = False

    print("options = ", opt)

    opt['labels'] = labels
    if m == sigraph.Trainer.GenerateOnly:
        self.generateDataBase(it, prefix)
        self.generateOnly(it, prefix, opt)
    elif m == sigraph.Trainer.GenerateAndTrain:
        self.generateDataBase(it, prefix)
        self.generateOnly(it, prefix, opt)
        self.readAndTrain(it, prefix, opt)
    elif m == sigraph.Trainer.ReadAndTrain:
        err += self.readAndTrain(it, prefix, opt)
    elif m == sigraph.Trainer.TrainDomain:
        self.trainDomain(it)
    elif m == sigraph.Trainer.TrainStats:
        self.trainStats(it)
    else:
        print('unknown Trainer mode', m)


def write_database_from_c_plus_plus(ad, prefix):
    import datamind.io.old_csvIO as io
    from datamind.ml import database
    descr = ad.getAdapDescr()
    learnable = descr.getSiDBLearnable()
    X = learnable.getX()
    Y = learnable.getY()
    INF = learnable.getINF()
    db = database.DbNumpy(X, Y, INF)
    w = io.WriterCsv()
    csv_filename = ad.getDataBaseName(prefix) + '.data'
    minf_filename = ad.getDataBaseName(prefix) + '.minf'
    # print('csv_filename:', csv_filename)
    # print('minf_filename:', minf_filename)
    sigraph_dic = {
        'split': learnable.getSplit(),
        'cycles': learnable.getCycles(),
    }
    header = {
        'X': descr.descriptorsNames().list(),
        'Y': ['Potential', 'Class'],
        'INF': ['cycle'],
        'user': ('sigraph', sigraph_dic)
    }
    w.write(csv_filename, db, header, minf_filename)
    descr.clearDB()


def trainer_readAndTrain(self, it, prefix, opt):
    return it.adaptive().train(prefix, opt)


def topAdaptive_train(self, prefix, opt):
    return self.model().train(prefix, opt)


def adaptiveTree_train(self, prefix, opt):
    res = []
    for c in self.children():
        res.append(c.train(prefix, opt))
        # mixer(res) FIXME
    return sum(res)


def trainer_generateOnly(self, it, prefix, opt):
    return it.adaptive().generateOnly(prefix, opt)


def topAdaptive_generateOnly(self, prefix, opt):
    return self.model().generateOnly(prefix, opt)


def adaptiveTree_generateOnly(self, prefix, opt):
    for c in self.children():
        c.generateOnly(prefix, opt)


def adaptiveLeaf_generateOnly(self, prefix, opt):
    write_database_from_c_plus_plus(self, prefix)

sigraph.Trainer.generateOnly = trainer_generateOnly
sigraph.TopAdaptive.generateOnly = topAdaptive_generateOnly
sigraph.AdaptiveTree.generateOnly = adaptiveTree_generateOnly
sigraph.AdaptiveLeaf.generateOnly = adaptiveLeaf_generateOnly
#


def plot_progression_bar(eval, user_data):
    user_data['progression_bar'].display(user_data['number'])
    user_data['number'] += 1


#
def adaptiveleaf_learn_mlp(self, train, test):
    from datamind.ml.classifier import optimizers
    from datamind.ml import database
    import datamind.ml.validation as validation
    import datamind.ml.classifier.ofunc as ofunc

    # FIXME : attention l'utilisation de 'test' 2 fois biaise les resultats
    part = validation.Single(db=[train, test])
    clf = datamind.ml.classifier.MlpSi(self)
    clf.fit(part)
    res = clf.predict(test)
    mse = res.compute_mse()
    subad = self.workEl()
    subad.setGenErrorRate(numpy.nan)
    # FIXME : il faudrait les taux de chaque classe
    subad.setGenGoodErrorRate(mse)
    subad.setGenBadErrorRate(mse)
    return mse

    # FIXME :  la methode clone ne marche pas sur les mlp : probleme avec
    # _dblearnable d'adapdescr.-> donc l'optimizer ne marche pas
    # clf = clf.clone()
    # val = validation.Single(train = part, test = test)
    # mo2 = optimizers.Iterate(10)
    # of = ofunc.ModelOFunc(clf, val, 'mse')
    # res = mo2.optimize(of)
    # if res.has_key('best_model'):
    #	self.update(res['best_model']._adaptiveleaf)
    # return res['value']

#


class OptimizedSvm(datamind.ml.model.Model):

    def __init__(self, adap, prefix, nbfolds, dim, weights):
        self._prefix = prefix
        self._adap = adap
        self._dim = dim
        self._nbfolds = nbfolds
        self._weights = weights

    def _print_info(self, train):
        msg.write_list([' * ', ('Learning', 'green'), ' SVM with ',
                        (str(self._nbfolds), 'cyan'), '-cv (mode : ' +
                        self._adap.workEl().getSvmMode() + ') on ',
                        (str(train.size()), 'cyan'), ' vectors\n'])

    def fit(self, train):
        from datamind.ml.classifier import optimizers
        from datamind.ml import database
        import datamind.ml.validation as validation
        import datamind.ml.classifier.ofunc as ofunc

        self._print_info(train)
        # shuffle data
        X = train.getX()
        rrange = list(range(X.shape[0]))
        numpy.random.shuffle(rrange)
        X2 = X[rrange]
        Y2 = train.getY()[rrange]
        INF2 = train.getINF()[rrange]
        groups = train.getGroups()[rrange]
        s0, s1 = (groups == 0).sum(), (groups == 1).sum()
        train2 = database.DbNumpy(X2, Y2, INF2, groups)

        groups_ind = [numpy.where(groups == 0)[0].tolist(),
                      numpy.where(groups != 0)[0].tolist()]

        val = validation.NativeCV(db=train2, nbfolds=self._nbfolds)
        # val = validation.CrossValidation(train2, self._nbfolds,
        #		'groups_balancing', groups=groups_ind)
        # val = validation.WeightedCrossValidation(train2, self._nbfolds,
        #		'groups_balancing', groups=groups_ind,
        #		groups_weights=[int(s1 / s0), 1])
        computing_number = numpy.prod([len(v)
                                       for v in self.parameters_ranges().values()])
        pb = ProgressionBarPct(computing_number)
        pb_data = {'progression_bar': pb, 'number': 0}
        grid = optimizers.Grid(self.parameters_ranges(),
                               strategy=self.strategy(), fun=plot_progression_bar,
                               user_data=pb_data)
        of = ofunc.ModelOFunc(self._clf, val, self.criterion(),
                              criterion_parameters=self.criterion_parameters(groups))
        gridres = grid.optimize(of)
        self.write_results(gridres)
        msg.write_list([' * ', ('cv best value = ', 'green'),
                        ' = ', gridres['value'], '\n'])
        self._clf.setParams(gridres['best_params'])
        # import pylab
        # pylab.imshow(gridres['array res']['val'],
        #		interpolation='nearest')
        # pylab.colorbar()
        # pylab.show()
        self._clf.fit(train2)

    def predict(self, test):
        from datamind.ml import database
        test2 = database.DbNumpy(test.getX(), test.getY(),
                                 test.getINF(), test.getGroups())
        res = self._clf.predict(test2)
        r = self.getResult(test, res)
        msg.write_list([' * ', ('predict best values', 'green'),
                        ' = ', res, '\n'])
        return r

    def write_results(self, gridres):
        ao = self._adap.topModel().parentAO()
        model_file = ao['model_file']
        basename = model_file[:model_file.rfind('.')] + '_errors_grid'
        filename = os.path.join(self._prefix, basename)
        gw = GridWriter()
        pr = self.parameters_ranges()
        gw.write(filename, gridres['array res']['val'],
                 list(pr.keys()), list(pr.values()))
        msg.write_list([' * ', ('best params', 'green'),
                        ' = ', gridres['best_params'], '\n'])


class OptimizedSvmOneClass(OptimizedSvm):

    def __init__(self, adap, prefix, nbfolds, dim, weights):
        OptimizedSvm.__init__(self, adap, prefix, nbfolds, dim, weights)
        steps = 10
        self._grange = numpy.logspace(-4, 2, steps) / self._dim
        self._nurange = numpy.linspace(0, 1, steps)
        self._clf = self.clf()

    def parameters_ranges(self):
        return {'gamma': self._grange, 'nu': self._nurange}

    def criterion(self):
        return 'weighted_classification_rate'

    def criterion_parameters(self, groups):
        return {'weights': self._weights}

    def getResult(self, test, res):
        res.compute('weighted_classification_rate', self._weights)
        rate = 1. - res.weighted_classification_rate
        subad = self._adap.workEl()
        subad.setGenErrorRate(numpy.nan)
        # FIXME : il faudrait les taux de chaque classe
        subad.setGenGoodErrorRate(rate)
        subad.setGenBadErrorRate(rate)
        # FIXME : verifier ce qu'il faut renvoyer
        return rate

    def clf(self, weights=None):
        import datamind.ml.classifier as classifier
        return classifier.SvcSi(self._adap, weights=weights)

    def strategy(self):
        return 'min'


class OptimizedEsvr(OptimizedSvm):

    def __init__(self, adap, prefix, nbfolds, dim, weights):
        steps = 10
        OptimizedSvm.__init__(self, adap, prefix, nbfolds, dim, weights)
        self._grange = numpy.logspace(-4, 2, steps) / self._dim
        self._crange = numpy.logspace(-1, 3, steps)
        self._clf = self.clf()

    def parameters_ranges(self):
        return {'gamma': self._grange, 'C': self._crange}

    def criterion(self):
        return 'groups_wmse'

    def criterion_parameters(self, groups):
        return {'weights': self._weights, 'groups': groups}

    def getResult(self, test, res):
        groups = test.getGroups()
        res.compute('groups_wmse', groups, self._weights)
        subad = self._adap.workEl()
        good_rate = res.groups_mse_list[0]
        bad_rate = res.groups_mse_list[1]
        subad.setGenErrorRate(numpy.nan)
        subad.setGenGoodErrorRate(good_rate)
        subad.setGenBadErrorRate(bad_rate)
        # classif rate :
        t, p, g = res.true_values, res.predict_values, test.getGroups()
        from datamind.ml.classifier import BinaryClassifierResult
        res2 = BinaryClassifierResult(g, p >= 0.5)
        r0 = res2.compute('classification_rate', 0)
        r1 = res2.compute('classification_rate', 1)
        subad.setMisclassGoodRate(1. - r0)
        subad.setMisclassBadRate(1. - r1)
        return res.groups_wmse

    def clf(self, weights=None):
        import datamind.ml.classifier as classifier
        return classifier.SvrSi(self._adap, weights=weights)

    def strategy(self):
        return 'min'


class OptimizedCsvc(OptimizedSvm):

    def __init__(self, adap, prefix, nbfolds, dim, weights):
        steps = 10
        OptimizedSvm.__init__(self, adap, prefix, nbfolds, dim, weights)
        self._grange = numpy.logspace(-4, 2, steps) / self._dim
        self._crange = numpy.logspace(-1, 3, steps)
        self._clf = self.clf(weights)

    def parameters_ranges(self):
        return {'gamma': self._grange, 'C': self._crange}

    def criterion(self):
        return 'weighted_classification_rate'

    def criterion_parameters(self, groups):
        return {'weights': self._weights}

    def getResult(self, test, res):
        res.compute('weighted_classification_rate', self._weights)
        rates = res.compute_classification_rates()
        rate = res.weighted_classification_rate
        good_rate, bad_rate = rates[0], rates[1]
        subad = self._adap.workEl()
        subad.setGenErrorRate(numpy.nan)
        subad.setGenGoodErrorRate(1. - good_rate)
        subad.setGenBadErrorRate(1. - bad_rate)
        return rate

    def clf(self, weights):
        import datamind.ml.classifier as classifier
        return classifier.SvcSi(self._adap, weights=weights)

    def strategy(self):
        return 'max'


class OptimizedFakeSvm(OptimizedSvm):

    def __init__(self, adap, prefix, nbfolds, dim, weights):
        from datamind.ml import wip
        wip.fixme()


def adaptiveleaf_learn_svm(self, prefix, train, test, opt):
    subadaptive = self.workEl()
    models = {'classifier': OptimizedCsvc,
              'oneclass': OptimizedSvmOneClass,
              'regression': OptimizedEsvr,
              'quality': OptimizedFakeSvm,  # FIXME
              'decision': OptimizedFakeSvm}  # FIXME

    dim = train.getX().shape[1]
    nbfolds = opt['nbfolds']
    model_class = models[subadaptive.getSvmMode()]
    groups = train.getGroups()
    s0, s1 = (groups == 0).sum(), (groups == 1).sum()
    if (not s0) or (not s1):
        msg.error("training database : empty class : " +
                  "C0 : %d items, C1 : %d items" % (s0, s1))
        exit(1)
    w0, w1 = 1. / (2. * s0), 1. / (2. * s1)
    weights = {0: w0, 1: w1}

    model = model_class(self, prefix, nbfolds, dim, weights)
    model.fit(train)
    if opt['predict_train']:
        msg.write_list([' ', ('train', 'red'), ' : \n'])
        if len(train.getX()) != 0:
            model.predict(train)
    msg.write_list([' ', ('test', 'red'), ' : \n'])
    if len(test.getX()) != 0:
        return model.predict(test)
    else:
        return 0.


#
def matrix_sdp_log(m):
    d, p = numpy.linalg.eigh(m)
    # log(m) = p * diag(log(d)) * p.T
    return p * numpy.multiply(numpy.log(d), p).T


def matrix_sdp_exp(m):
    d, p = numpy.linalg.eigh(m)
    # exp(m) = p * diag(exp(d)) * p.T
    return p * numpy.multiply(numpy.exp(d), p).T


def matrix_sdp_power_a(m, a):
    d, p = numpy.linalg.eigh(m)
    # m^a = p * diag(d^a) * p.T
    return p * numpy.multiply(d ** a, p).T


def regular_covariance_decomposition(data):
    '''
Compute covariance of data matrix and compute eigen decomposition
preventing negative eigenvalues.

data :         data matrix

C = P * D * P.T  where C : data covariance matrix
                       D : diagonal matrix
                       P : rotation matrix
C is Hermitian but singular so negative eigenvalues may appears in D.

return D, P, ndim,  with vector-shaped D and removed negative values,
                    ndim : number of preserve dimensions
    '''

    n = data.shape[0]
    cov = data.T * data / (n - 1)
    # C = U * D * V.T ~= P * D * P.T
    u, d, vt = numpy.linalg.svd(cov)  # FIXME

    # nullify dimension >= max_rank
    cov_max_rank = min(n, data.shape[1])
    d[cov_max_rank:] = 0

    # set 10e-1 to forget metric along these axis
    neg_dim = (d <= 10e-1)
    ndim = len(d) - neg_dim.sum()
    d[neg_dim] = 0
    return d, numpy.asmatrix(vt.T), ndim


def bootstrap_covariance_decomposition(data, n_draws=1000):
    '''
Compute robust covariance matrix of a matrix. The process is based on
processing n_draws covariance matrix from bootstrap samples and then
a mean covariance matrix is computed from log-euclidian metric.

data :     data matrix
n_draws :  number of bootstrap draws

C = P * D * P.T  where C : data covariance matrix
                       D : diagonal matrix
                       P : rotation matrix
return D, P, ndim,  with vector-shaped D and removed negative values,
                    ndim : number of preserve dimensions
    '''
    n = data.shape[0]
    dim = data.shape[1]
    idx = numpy.random.randint(n, size=(n_draws, n))
    ndims = []
    mats = []

    # compute bootstraped covariance matrix decompositions
    for id in idx:
        data2 = data[id]
        d, vt, ndim = regular_covariance_decomposition(data2)
        ndims.append(ndim)
        mats.append((d, vt))
    ndims = numpy.array(ndims)
    ndim = ndims.min()

    # compute robust log-euclidian mean of covariance matrix
    cov_mean = numpy.zeros((data.shape[1], data.shape[1]))
    for d, vt in mats:
        d[ndim:] = 1
        logd = numpy.log(d)
        # logd[ndim:] = -10000. # prevent nan and inf #FIXME
        logd[ndim:] = 0.  # prevent nan and inf
        cov_mean += vt.T * numpy.multiply(logd, vt.T).T
    cov_mean /= len(mats)
    e, q = numpy.linalg.eigh(cov_mean)
    # C = q * e * q.T = (q * v) * (v * e * v)  * (v * q.T)
    # with v = identity(dim)[::-1] and v * v = Id, v.T = v
    v = numpy.asmatrix(numpy.identity(dim)[::-1])
    e = e[::-1]  # e = v * e * v
    e[ndim:] = 0
    q = q * v
    # take exponential of logcovariance mean
    return numpy.exp(e), numpy.asmatrix(q), ndim


class ScaleModel(object):

    '''SiNormalizedXScale + Model'''

    def __init__(self, scale, model):
        self._scale = scale
        self._model = model

    def fit(self, train):
        from datamind.ml import scaling

        # data prescaling
        scaled_train = self._scale.fit(train)
        scaled_train = self._scale.predict(train)  # to handle null std

        # fit
        self._model.fit(scaled_train)

    def predict(self, test):
        scaled_test = self._scale.predict(test)
        self._model.predict(scaled_test)


class OneGaussian(datamind.ml.model.Model):

    '''Abstract class'''

    def __init__(self):
        datamind.ml.model.Model.__init__(self)
        self._dim = None  # data dim
        self._ndim = None  # data reduced dimensionality

    def fit(self, db):
        from datamind.ml import dimreduction, database

        data = numpy.asmatrix(db.getX())
        self._dim = data.shape[1]

        # covariance matrix and metric
        self._ndim = self.compute_metric(db)

    def inputDim(self):
        return self._dim

    def getIntrinsecDim(self):
        return self._ndim

    def norm(self):
        return numpy.sqrt(self.det() * (2 * numpy.pi) ** self._ndim)

    def lognorm(self):
        return 0.5 * (numpy.log(self.det()) +
                      self._ndim * numpy.log(2 * numpy.pi))

    def density(self, x):
        '''Computed according to intrinsec dimension'''
        return self.density_no_normalized(x) / self.norm()

    def logdensity(self, x):
        return self.logdensity_no_normalized(x) - self.lognorm()

    def likelihood(self, db):
        data = numpy.asmatrix(db.getX())
        return numpy.prod([self.density(x.T) for x in data])

    def loglikelihood(self, db):
        data = numpy.asmatrix(db.getX())
        return numpy.sum([self.logdensity(x.T) for x in data])


class OneNormalizedGaussian(OneGaussian):

    def __init__(self):
        OneGaussian.__init__(self)

    def fit(self, db):
        from datamind.ml import dimreduction, database

        # remove dimensions with null std
        data = numpy.asmatrix(db.getX())
        cond = numpy.asarray(data.std(axis=0)).ravel() != 0
        good = numpy.argwhere(cond).T.ravel()
        bad = numpy.argwhere(cond != 1).T.ravel()
        self._selected_dims = numpy.hstack((good, bad))
        data = data[:, cond]
        db_reduced = db.share()
        db_reduced.setX(data)
        self._dim = data.shape[1]

        # covariance matrix and metric
        self._ndim = self.compute_metric(db_reduced)

    def density_no_normalized(self, x):
        # exp -(1/2) * x^t * x
        return numpy.exp(-0.5 * numpy.dot(x.T, x)[0, 0])

    def logdensity_no_normalized(self, x):
        return -0.5 * numpy.dot(x.T, x)[0, 0]

    def compute_metric(self, db):
        # All is already done during scaling
        return db.getX().shape[1]

    def metric(self):
        return numpy.identity(self._dim)

    def sqrt_metric(self):
        return numpy.identity(self._dim)

    def det(self):
        return 1


class OneFullGaussian(OneGaussian):

    def __init__(self):
        OneGaussian.__init__(self)
        self._invcov = None
        self._invcovsqrt = None
        self._det = None

    def density_no_normalized(self, x):
        # exp - (1/2) * x^t C.I * x with C : full covariance matrix
        d = (x.T * self._invcov * x)[0, 0]
        if d >= 700:
            d = 700
        elif d <= -700:
            d = -700
        return numpy.exp(-0.5 * d)

    def logdensity_no_normalized(self, x):
        return -0.5 * (x.T * self._invcov * x)[0, 0]

    def covariance_decomposition(self, data):
        return regular_covariance_decomposition(data)

    def compute_metric(self, db):
        data = numpy.asmatrix(db.getX())
        # covariance matrix
        d, p, ndim = self.covariance_decomposition(data)
        d[ndim:] = 1.
        d = 1. / d
        d[ndim:] = 0.
        self._invcov = p * numpy.multiply(d, p).T
        self._invcovsqrt = p * numpy.multiply(d ** 0.5, p).T
        self._det = numpy.prod(d[:ndim])
        return ndim

    def metric(self):
        return self._invcov

    def sqrt_metric(self):
        return self._invcovsqrt

    def det(self):
        return self._det


class OneBootstrapFullGaussian(OneFullGaussian):

    def __init__(self):
        OneGaussian.__init__(self)

    def covariance_decomposition(self, data):
        return bootstrap_covariance_decomposition(data)


class SiOneGaussian(datamind.ml.model.Model):

    def __init__(self, adap, model):
        '''model : OneGaussian model.'''
        datamind.ml.model.Model.__init__(self)
        self._adap = adap
        self._onegaussian_model = model

    def fit(self, db):
        from datamind.ml import dimreduction, database

        # fit gaussian
        self._onegaussian_model.fit(db)
        data = numpy.asmatrix(db.getX())

        # Gaussian parameters
        subad = self._adap.workEl()
        gaussnet = subad.net()
        ndim = self._onegaussian_model.getIntrinsecDim()
        gaussnet.init(ndim, 1, 1)
        gaussian = gaussnet.gauss(0)
        gaussian.init(ndim, True)
        for d in range(ndim):
            gaussian.setCenterCoord(d, 0)
        gaussnet.setWeight(0, 1)
        subad.setDefaultValue(0)
        logli = self._onegaussian_model.loglikelihood(db)
        print("mean log likelihood : ", logli / data.shape[0])


class ModelSiSaver(object):

    def __init__(self, adap, model):
        self._adap = adap
        self._model = model
        from datamind.ml import scaling
        self._savers_list = {SiOneGaussian: SiOneGaussianSiSaver,
                             ScaleModel: ScaleModelSiSaver,
                             scaling.NormalizedXScale: NormalizedXScaleSiSaver}

    def get_saver(self, adap, model):
        savers = self._savers_list[model.__class__]
        return savers.get_subsaver(adap, model)

    @classmethod
    def get_subsaver(clf, adap, model):
        return clf(adap, model)

    def save(self):
        pass


class SiOneGaussianSiSaver(ModelSiSaver):

    def __init__(self, adap, model):
        ModelSiSaver.__init__(self, adap, model)

    @classmethod
    def get_subsaver(self, adap, model):
        _local_savers = {
            OneNormalizedGaussian: SiOneNormalizedGaussianSiSaver,
            OneFullGaussian: SiOneFullGaussianSiSaver,
            OneBootstrapFullGaussian: SiOneFullGaussianSiSaver}
        submodel = model._onegaussian_model
        return _local_savers[submodel.__class__](adap, model)

    def save(self):
        # store sqrt of metric tensor if needed
        self._save_dimred()
        self._save_postscaling()

    def _save_postscaling(self):
        # data postscaling
        dim = self._model._onegaussian_model.inputDim()
        mean = numpy.zeros(dim)
        std = numpy.zeros(dim) + 1.
        mean_aims = aims.vector_DOUBLE(mean)
        std_aims = aims.vector_DOUBLE(std)
        self._adap.workEl().setStats(mean_aims, std_aims)


class SiOneNormalizedGaussianSiSaver(SiOneGaussianSiSaver):

    def __init__(self, adap, model):
        SiOneGaussianSiSaver.__init__(self, adap, model)

    def _save_dimred(self):
        adap = self._model._adap
        onegaussian_model = self._model._onegaussian_model
        dim = onegaussian_model._dim
        selected_dims = onegaussian_model._selected_dims
        subad = adap.workEl()
        w = subad.net().weight(0)
        if w < -10e10:
            subad.setGenGoodErrorRate(1)
            subad.setGenBadErrorRate(1)
        else:
            subad.setGenGoodErrorRate(0.)
            subad.setGenBadErrorRate(0.)
        dimred = sigraph.RanksDimReductor(selected_dims, dim)
        adap.setDimReductor(dimred)


class SiOneFullGaussianSiSaver(SiOneGaussianSiSaver):

    def __init__(self, adap, model):
        SiOneGaussianSiSaver.__init__(self, adap, model)

    def _save_dimred(self):
        adap = self._adap
        subad = adap.workEl()
        subad.setGenGoodErrorRate(0.)
        subad.setGenBadErrorRate(0.)
        onegaussian_model = self._model._onegaussian_model
        sqrt_metric = onegaussian_model.sqrt_metric()
        ndim = onegaussian_model.getIntrinsecDim()

        # store sqrt of metric tensor
        m = numpy.array(sqrt_metric.T, copy=True, order='C')
        shape = aims.vector_S32(m.shape)
        m = aims.vector_FLOAT(m.ravel().tolist())
        dimred = sigraph.MatrixDimReductor(m, shape, ndim)
        adap.setDimReductor(dimred)


class NormalizedXScaleSiSaver(ModelSiSaver):

    def __init__(self, adap, model):
        ModelSiSaver.__init__(self, adap, model)

    def save(self):
        mean = aims.vector_FLOAT(self._model.get_mean())
        std = aims.vector_FLOAT(self._model.get_std())
        self._adap.setMean(mean)
        self._adap.setStd(std)


class ScaleModelSiSaver(ModelSiSaver):

    def __init__(self, adap, model):
        ModelSiSaver.__init__(self, adap, model)
        scale = model._scale
        submodel = model._model
        self._scale_saver = self.get_saver(self._adap, model._scale)
        self._submodel_saver = self.get_saver(self._adap, model._model)

    def save(self):
        self._scale_saver.save()
        self._submodel_saver.save()


def saveModel(adap, model):
    saver = ModelSiSaver(adap, model).get_saver(adap, model)
    saver.save()


def adaptiveleaf_learn_gaussian(self, prefix, trainsi, testsi, opt):
    from datamind.ml import database, scaling

    subadaptive = self.workEl()
    models = {'normalized': OneNormalizedGaussian,
              'full': OneFullGaussian,
              'bootstrap_full': OneBootstrapFullGaussian}

    # FIXME : store type of model somewhere.

    scale = scaling.NormalizedXScale()
    submodel = SiOneGaussian(self, models[opt['gaussian_mode']]())
    model = ScaleModel(scale, submodel)

    # train + test
    data = numpy.concatenate((trainsi.getX(), testsi.getX()))
    db = database.DbNumpy(data, None)
    model.fit(db)
    saveModel(self, model)
    return float('nan')


#
def adaptiveleaf_learn_mixgaussian(self, prefix, train, test, opt):
    models = {'normalized': OneNormalizedGaussian,
              'full': OneFullGaussian,
              'bootstrap_full': OneBootstrapFullGaussian}

    from datamind.ml import dimreduction, database
    from datamind.ml import scaling

    adap = self
    subad = self.workEl()
    subad.reset()

    # data prescaling
    scale = SiNormalizedXScale(self)
    scaled_train = scale.fit(train)
    scaled_train = scale.predict(train)  # to handle null std
    data_scaled = numpy.asmatrix(scaled_train.getX())

    Y = scaled_train.getY()
    if hasattr(numpy, 'unique1d'):
        classes = numpy.unique1d(Y)
    else:
        classes = numpy.unique(Y)
    for c in classes:
        ind = (Y == c).ravel()
        db = database.DbNumpy(train.getX()[ind], train.getY()[ind])

        # FIXME : store type of model somewhere.
        model = models[opt['gaussian_mode']]()
        model.fit(db)

        # store metric tensor
        metric = model.metric()
        m = numpy.array(metric, copy=True, order='C')
        shape = aims.vector_S32(m.shape)
        m = aims.vector_DOUBLE(m.ravel().tolist())
        subad.addMatrix(m)
        subad.addSqrtDet(model.det())

    # FIXME eval generalisation rate
    subad.setGenGoodErrorRate(1)
    subad.setGenBadErrorRate(1)
    adap.setDimReductor(None)

    # post scaling
    dim = scaled_train.getX().shape[1]
    mean = numpy.zeros(dim)
    std = numpy.zeros(dim) + 1.
    mean_aims = aims.vector_DOUBLE(mean)
    std_aims = aims.vector_DOUBLE(std)
    adap.workEl().setStats(mean_aims, std_aims)
    return float('nan')


#
import datamind.ml.scaling


class SiScale(datamind.ml.scaling.Scale):

    def __init__(self, model):
        self._model = model


class SiNormalizedXScale(SiScale):

    def __init__(self, model):
        from datamind.ml import scaling
        SiScale.__init__(self, model)
        self._scale = scaling.NormalizedXScale()

    def fit(self, train):
        scaled_train = self._scale.fit(train)
        mean = aims.vector_FLOAT(self._scale.get_mean())
        std = aims.vector_FLOAT(self._scale.get_std())
        self._model.setMean(mean)
        self._model.setStd(std)
        return scaled_train

    def predict(self, test):
        return self._scale.predict(test)


class SiNormalizedBalancedXScale(SiScale):

    def __init__(self, model):
        from datamind.ml import scaling
        SiScale.__init__(self, model)
        self._scale = scaling.NormalizedXScale()

    def fit(self, train):
        from datamind.ml import database

        # duplicate data of class 0
        X = train.getX()
        Yc = train.getY()
        c0 = (Yc == 0).ravel()
        c1 = (Yc == 1).ravel()
        X0r = numpy.repeat(X[c0], c1.sum() / c0.sum(), axis=0)
        Xr = numpy.concatenate((X0r, X[c1]))
        repeated0_train = database.DbNumpy(Xr, None)

        # scaling
        scaled_trainr = self._scale.fit(repeated0_train)
        mean = aims.vector_FLOAT(self._scale.get_mean())
        std = aims.vector_FLOAT(self._scale.get_std())
        self._model.setMean(mean)
        self._model.setStd(std)
        return self._scale.predict(train)

    def predict(self, test):
        return self._scale.predict(test)


class SiNormalizedGenericScale0(SiScale):

    def __init__(self, model):
        from datamind.ml import scaling
        SiScale.__init__(self, model)

    def fit(self, train, classes):
        from datamind.ml import database

        ind = (classes == 0).ravel()
        # db = train.share_or_copy(copy)
        #               db.setX(X2)
        #
        train0 = database.DbNumpy(train.getX()[ind], train.getY()[ind])
        scaled_train0 = self._scale.fit(train0)
        self.register()
        return self._scale.predict(train)

    def predict(self, test):
        return self._scale.predict(test)


class SiNormalizedXYScale0(SiNormalizedGenericScale0):

    def __init__(self, model):
        from datamind.ml import scaling
        SiNormalizedGenericScale0.__init__(self, model)
        self._scale = scaling.NormalizedXYScale()

    def register(self):
        self._model.setMean(aims.vector_FLOAT(self._scale.get_meanX()))
        self._model.setStd(aims.vector_FLOAT(self._scale.get_stdX()))


class SiNormalizedXScale0(SiNormalizedGenericScale0):

    def __init__(self, model):
        from datamind.ml import scaling
        SiNormalizedGenericScale0.__init__(self, model)
        self._scale = scaling.NormalizedXScale()

    def register(self):
        self._model.setMean(aims.vector_FLOAT(self._scale.get_mean()))
        self._model.setStd(aims.vector_FLOAT(self._scale.get_std()))


#
class SiDimensionReduction(datamind.ml.dimreduction.DimensionReduction):

    def __init__(self, model):
        datamind.ml.dimreduction.DimensionReduction.__init__(self)
        self._model = model
        self._dr = None

    def postNormalization(self, reduced_train, classes):
        # mean / std on reduce database|class 0
        ind = (classes == 0).ravel()
        data = reduced_train.getX()[ind]
        mean = aims.vector_DOUBLE(data.mean(axis=0))
        std = aims.vector_DOUBLE(data.std(axis=0))
        # mean = aims.vector_DOUBLE([0] * train.getX().shape[1])
        # std = aims.vector_DOUBLE([1] * train.getX().shape[1])
        self._model.workEl().setStats(mean, std)

    def getGroupsDb(self, db):
        from datamind.ml import database
        db.share()
        db.setY(db.getGroups()[None].T)
        return db

    def substituteY(self, db1, db2):
        db1.share()
        db1.setY(db2.getY())
        return db1


# class SiDisplay(object):
#	def __init__(self): pass


# class SiFileDisplay(SiDisplay):
#	def __init__(self, filename):
#		SiDisplay.__init__(self)
#		self._filename

#	def show(self):
#		import pylab
#		pylab.savefig(self._filename)

#	def
#		import pylab, time
#		import matplotlib
#		pylab.figure()
#		pylab.plot(X2, [0] * len(X2), 'bo')
#		pylab.plot(X1, [0] * len(X1), 'ro')
#		pylab.plot(range, p2, 'b-')
#		pylab.plot(range, p1, 'r-')
#		pylab.show()
#		time.sleep(100)


class SiUnivariateDimensionReduction(SiDimensionReduction):

    def __init__(self, model, ranker):
        SiDimensionReduction.__init__(self, model)
        self._ranker = ranker

    def fit(self, train):
        train2 = self.getGroupsDb(train)

        # scaling
        groups = train.getGroups()
        self._scale = SiNormalizedXScale0(self._model)
        self._scaled_train = self._scale.fit(train2, groups)

        # dim reduction
        self._dr = UnivariateDimensionReduction(self._ranker)
        self._dr.fit(self._scaled_train)
        scores = self._dr.getScores()
        ranks = self._dr.getRanks()
        self._selected_dim = self._ranker.selected_dim(scores)
        print("scores = ", scores)
        print("ranks = ", self._dr.getRanks())

    def register(self, n):
        if not self._dr:
            model.setDimReductor(None)
            return
        if n == 'auto':
            n = self._selected_dim
        selected_dims = aims.vector_S32(self._dr.getRanks())
        dimred = sigraph.RanksDimReductor(selected_dims, n)
        self._model.setDimReductor(dimred)

    def get_reduced_train(self, train, n=-1):
        if n == 'auto':
            n = self._selected_dim
            print("selected dim (auto) = ", n)
        reduced_train = self._dr.reduce(self._scaled_train, n)
        groups = train.getGroups()
        self.postNormalization(reduced_train, groups)
        r = self.substituteY(reduced_train, train)
        return r

    def reduce(self, test, n=-1):
        if n == 'auto':
            n = self._selected_dim
        scaled_test = self._scale.predict(test)
        reduced_test = self._dr.reduce(scaled_test, n)
        return self.substituteY(reduced_test, test)


class SiMultivariateDimensionReduction(SiDimensionReduction):

    def __init__(self, model):
        SiDimensionReduction.__init__(self, model)

    def register(self, n):
        if not self._dr:
            self._model.setDimReductor(None)
            return
        m = numpy.array(self._dr.getTransformation().T,
                        copy=True, order='C')
        shape = aims.vector_S32(m.shape)
        m = aims.vector_FLOAT(m.ravel().tolist())
        if n == -1:
            n = shape[0]
        dimred = sigraph.MatrixDimReductor(m, shape, n)
        self._model.setDimReductor(dimred)


class SiSvd(SiMultivariateDimensionReduction):

    def __init__(self, model):
        SiMultivariateDimensionReduction.__init__(self, model)

    def fit(self, train):
        from datamind.ml import dimreduction

        # scaling
        self._scale = SiNormalizedXScale(self._model)
        self._scaled_train = self._scale.fit(train)

        # dimension reduction
        # selected_pct = 0.97
        try:
            self._dr = dimreduction.SvdNumpy(self._scaled_train)
            # selected_dim = dr.get_components_from_infopct(selected_pct)
        # svd does not work converged, original train/test databases
        # are used
        except numpy.linalg.linalg.LinAlgError:
            msg.warning("SVD does not converged, "
                        "skip dimreduction step.")
            self._dr = None

    def get_reduced_train(self, train, n=-1):
        if self._dr:
            reduced_train = self._dr.get_reduced_train(
                self._scaled_train, n)
        else:
            reduced_train = train
        groups = train.getGroups()
        self.postNormalization(reduced_train, groups)
        return reduced_train

    def reduce(self, test, n=-1):
        scaled_test = self._scale.predict(test)
        if self._dr:
            reduced_test = self._dr.reduce(scaled_test, n)
        else:
            reduced_test = test
        return reduced_test


class SiIca(SiMultivariateDimensionReduction):

    def __init__(self, model):
        SiMultivariateDimensionReduction.__init__(self, model)

    def fit(self, train):
        from datamind.ml import dimreduction, plugins
        import mdp.signal_node
        plugins.plugin_manager.load_plugin('mdp')

        # FIXME : je ne crois pas que ca serve
        # train2 = self.getGroupsDb(train)
        train2 = train

        # scaling
        self._scale = SiNormalizedBalancedXScale(self._model)
        scaled_train = self._scale.fit(train2)

        # precomputing with svd
        selected_pct = 0.999
        try:
            self._dr1 = dimreduction.SvdNumpy(scaled_train)
            selected_dim1 = self._dr1.get_components_from_infopct(
                selected_pct)
            reduced_trainr1 = self._dr1.get_reduced_train(
                scaled_train, selected_dim1)
            self._reduced_train1 = self._dr1.reduce(scaled_train,
                                                    selected_dim1)
            self._selected_dim1 = selected_dim1
        except numpy.linalg.linalg.LinAlgError:
            msg.warning("SVD does not converged, "
                        "skip dimreduction step.")
            reduced_trainr1 = scaled_trainr
            self._reduced_train1 = scaled_train

        # FIXME : enregister la transformation svd aussi
        # dimension reduction
        try:
            self._dr = dimreduction.IcaMdp(reduced_trainr1)
        # svd does not work converged, original train/test databases
        # are used.
        except mdp.signal_node.NodeException as m:
            msg.warning("ICA exception, skip dimreduction "
                        "step : " + repr(m))

    def get_reduced_train(self, train, n=-1):
        if self._dr:
            reduced_train = self._dr.reduce(self._reduced_train1, n)
        else:
            reduced_train = train
        groups = train.getGroups()
        self.postNormalization(reduced_train, groups)
        return self.substituteY(reduced_train, train)

    def reduce(self, test, n=-1):
        # FIXME : je ne crois pas que ca serve
        # test2 = self.getGroupsDb(test)
        test2 = test
        scaled_test = self._scale.predict(test2)
        if self._dr1:
            reduced_test1 = self._dr1.reduce(scaled_test,
                                             self._selected_dim1)
        else:
            reduced_test1 = test
        if self._dr:
            reduced_test2 = self._dr.reduce(reduced_test1, n)
        else:
            reduced_test2 = test
        return self.substituteY(reduced_test2, test)


class SiPls1(SiMultivariateDimensionReduction):

    def __init__(self, model):
        SiMultivariateDimensionReduction.__init__(self, model)

    def fit(self, train):
        from datamind.ml import dimreduction, database

        # scaling
        groups = train.getGroups()
        self._scale = SiNormalizedXYScale0(self._model)
        self._scaled_train = self._scale.fit(train, groups)

        # weights
        w0 = 1. / (groups == 0).sum()
        w1 = 1. / (groups == 1).sum()
        W = (groups * w1 + (1 - groups) * w0).ravel()

        # dimension reduction
        # selected_error_pct = 0.05
        self._dr = dimreduction.FeatureExtractionPls1Numpy()
        self._dr.fit(self._scaled_train, weights=W)
        # mse = dr.get_pls_model().optimize(scaled_test)
        # selected_dim = dr.get_components_from_errorpct(scaled_test,
        #					selected_error_pct)

    def get_reduced_train(self, train, n=-1):
        reduced_train = self._dr.get_reduced_train(
            self._scaled_train, n)
        groups = train.getGroups()
        self.postNormalization(reduced_train, groups)
        return self.substituteY(reduced_train, train)

    def reduce(self, test, n=-1):
        # FIXME : je ne crois pas que ca serve
        # test2 = self.getPotentialDb(test)
        test2 = test
        scaled_test = self._scale.predict(test2)
        reduced_test = self._dr.reduce(scaled_test, n)
        return self.substituteY(reduced_test, test)


#
class Ranker(object):

    def __init__(self):
        pass

    def rank(self, X, Y):
        from datamind.ml import wip
        wip.fixme()

    def selected_dim(self, scores):
        selected_dim = 15
        return selected_dim


class SvcRanker(Ranker):

    def __init__(self):
        Ranker.__init__(self)

    def rank(self, X, Y):
        # fix find_library so that it finds libsvm in a non-system path
        from soma.utils import find_library
        find_library.patch_ctypes_find_library()
        from datamind.ml import database, plugins, validation
        from datamind.ml.classifier import optimizers
        import datamind.ml.classifier.ofunc as ofunc
        import svm
        plugins.plugin_manager.load_plugin('libsvm')

        # create train/test database
        split = (len(X) * 2) / 3
        Y = Y[:, numpy.newaxis]
        X1, Y1 = X[:split][:, numpy.newaxis], Y[:split]
        X2, Y2 = X[split:][:, numpy.newaxis], Y[split:]

        train = database.DbNumpy(X1, Y1)
        test = database.DbNumpy(X2, Y2)

        # parameters
        steps = 10
        grange = numpy.logspace(-4, 2, steps)
        crange = numpy.logspace(-1, 3, steps)
        nbfolds = (train.getY() == 0).sum()
        w0 = 1. / (train.getY() == 0).sum()
        w1 = 1. / (train.getY() == 1).sum()
        ws = w0 + w1
        weights = {'0': w0 / ws, '1': w1 / ws}

        # C-SVC model + (C, gamma) optimization.
        clf = datamind.ml.classifier.CSvcLibSvm(svm.RBF, weights)
        val = validation.NativeCV(db=train, nbfolds=nbfolds)
        of = ofunc.ModelOFunc(clf, val,
                              criterion='weighted_classification_rate',
                              criterion_parameters={'weights': 'auto'})
        grid = optimizers.Grid({'C': crange, 'gamma': grange},
                               strategy='max')
        gridres = grid.optimize(of)
        clf.setParams(gridres['best_params'])
        clf.fit(train)
        res = clf.predict(test)
        res.compute('weighted_classification_rate', weights='auto')
        return res.weighted_classification_rate


class RandomRanker(Ranker):

    def __init__(self):
        Ranker.__init__(self)

    def rank(self, X, Y):
        return numpy.random.uniform(0, 1)


class DuchBlendRanker(Ranker):

    def __init__(self):
        Ranker.__init__(self)

    def rank(self, X, Y):
        # sort
        ind = X.argsort().tolist()
        X = X[ind]
        Y = Y[ind]

        yc = Y[0]  # current class
        cn = 0        # current number of neighbours of the same class.
        s = 0
        for i in range(len(X)):
            if Y[i] == yc:
                cn += 1
            else:
                s += cn * (cn - 1)
                cn = 0
                yc = Y[i]
        s += cn * (cn - 1)  # last group.
        return -s


class MattBlendRanker(Ranker):

    def __init__(self):
        Ranker.__init__(self)

    def rank(self, X, Y):
        # sort
        ind = X.argsort().tolist()
        X = X[ind]
        Y = Y[ind]
        W = numpy.array([10., 1.])
        s = 0
        sigma = X.std() / 5.
        if sigma == 0:
            sigma = 1
        for i in range(len(X)):
            s2 = 0
            for j in range(len(X)):
                if Y[i] != Y[j]:
                    e = numpy.exp(((X[i] - X[j]) / sigma))
                    s2 += W[Y[i]] * e
            s2 /= len(X) * W.sum()
            s += s2 / (len(X) * W.sum())
        print(s)
        return s


class PermutationWilcoxonRanker(Ranker):

    def __init__(self):
        Ranker.__init__(self)

    def rank(self, X, Y):
        n = 10000

        def wilcoxon(Y):
            '''Mann-Whitney U test / Mann-Whitney-Wilcoxon test'''
            return numpy.abs(
                numpy.argwhere(Y == 0).T.reshape(-1).sum() -
                numpy.argwhere(Y == 1).T.reshape(-1).sum())

        ind = X.argsort().tolist()
        Y = Y[ind]

        e = wilcoxon(Y)

        res = numpy.zeros(n)
        for i in range(n):
            numpy.random.shuffle(Y)
            res[i] = wilcoxon(Y)

        p = len(res[res >= e]) / float(n)
        return p

    def selected_dim(self, scores):
        pvalues = scores
        tolareted_error = 0.01
        dim = len(scores)
        th = tolareted_error / dim
        selected_dims = (pvalues < th)
        return len(numpy.argwhere(selected_dims))


class KullbackLeiblerRanker(Ranker):

    def __init__(self, store=False):
        Ranker.__init__(self)
        self._store = store

    def rank(self, X, Y):

        ind0 = (Y == 0).ravel()
        ind1 = (Y == 1).ravel()
        X0 = X[ind0]
        X1 = X[ind1]

        def KullbackLeibler(p1, p2):
            e = 1.e-100
            return (p1 * numpy.log((p1 + e) / (p2 + e))).sum()

        def KullbackLeibler_data(X1, X2):
            import scipy.stats
            try:
                kde1 = scipy.stats.kde.gaussian_kde(X1)
                kde2 = scipy.stats.kde.gaussian_kde(X2)
            except scipy.linalg.basic.LinAlgError:
                return 0.
            mi = min(X1.min(), X2.min())
            ma = max(X1.max(), X2.max())
            n = 10000
            range = numpy.linspace(mi, ma, n)
            p1 = kde1.evaluate(range)
            p2 = kde2.evaluate(range)
            if self._store:
                self._range = range
                self._p1 = p1
                self._p2 = p2
            return KullbackLeibler(p1, p2)

        k = KullbackLeibler_data(X0, X1)
        try:
            return 1. / k
        except ZeroDivisionError:
            return float('inf')

        def getDistributions(self):
            if not self._store:
                return None
            return self._range, self._p1, self._p2


class UnivariateDimensionReduction(datamind.ml.dimreduction.DimensionReduction):

    '''
Generic univariate dimension reduction method based on a customizable
ranking method.'''

    def __init__(self, ranker):
        '''
ranker : methods which take 2 vectors (X | Y) and give a score to eval
     given dimension according to a given criterion : data independance
     for instance.'''
        self._ranker = ranker

    def fit(self, train):
        # get databases
        X = train.getX()
        Y = train.getY().ravel()
        dim = X.shape[1]

        # rank
        ranks = numpy.zeros((dim, 2))
        ranks[:, 0] = numpy.arange(dim)
        for i in range(dim):
            Xs = X[:, i].ravel()
            ranks[i, 1] = self._ranker.rank(Xs, Y)

        # Sort feature index by errors
        ind = ranks[:, 1].argsort()
        self._ranks = ranks[ind]

    def reduce(self, test, n=-1, copy=False):
        from datamind.ml import database
        X = test.getX()
        ranks = self.getRanks()
        if n > 0:
            selected_dims = ranks[:n]
        else:
            selected_dims = ranks
        db = test.share_or_copy(copy)
        db.setX(X[:, selected_dims])
        return db

    def getRanks(self):
        return self._ranks[:, 0].astype('i2')

    def getScores(self):
        return self._ranks[:, 1]


#
def dimensionReductionFactory(model, mode):
    if mode.startswith('univariate'):
        m, submode = mode.split(',')
        if submode == 'SVC':
            ranker = SvcRanker()
        elif submode == 'permutation_wilcoxon':
            ranker = PermutationWilcoxonRanker()
        elif submode == 'kullback_leibler':
            ranker = KullbackLeiblerRanker()
        elif submode == 'duch_blend':
            ranker = DuchBlendRanker()
        elif submode == 'matt_blend':
            ranker = MattBlendRanker()
        elif submode == 'random':
            ranker = RandomRanker()
        else:
            msg.error("Unknown univariate submode '" + mode + "'")
        dr = SiUnivariateDimensionReduction(model, ranker)
    elif mode == 'SVD':
        dr = SiSvd(model)
    elif mode == 'ICA':
        dr = SiIca(model)
    elif mode == 'PLS1':
        dr = SiPls1(model)
    else:
        msg.error("Unknown multivariate mode '" + mode + "'")
    return dr


def dimension_reduction(mode, selected_dim, model, train, test):
    '''Dimension reduction of train / test database.'''

    # Select dimreductor
    dr = dimensionReductionFactory(model, mode)
    dr.fit(train)
    reduced_train = dr.get_reduced_train(train, selected_dim)
    reduced_test = dr.reduce(test, selected_dim)
    dr.register(selected_dim)

    return reduced_train, reduced_test


def optimizedDim(mode, model, train, test, opt):
    '''Generic optimization of dimension according to a feature ranking
or feature transformation + ranking (linear transformation).'''
    mode = opt['dimreduction']
    prefix = opt['prefix']
    nbfolds = opt['nbfolds']
    if not mode:
        msg.error('You must used dimension reduction to '
                  'optimized dimension.')

    selected_dim = -1
    trainsi, testsi = dimension_reduction(mode, selected_dim,
                                          model, train, test)

    dr = dimensionReductionFactory(model, mode)
    dr.fit(train)

    dim = train.getX().shape[1]

    errors = numpy.zeros(dim) + float('inf')
    dimrange = list(range(1, dim))

    for d in dimrange:
        # dimension reduction
        reduced_train = dr.get_reduced_train(train, d)
        reduced_test = dr.reduce(test, d)
        dr.register(d)

        # classification
        errors[d] = model.learn_svm(prefix, reduced_train,
                                    reduced_test, opt)

    optimal_dim = errors.argmin()
    msg.write_list([' * ', ('optimal dimension', 'green'),
                    ' = ', optimal_dim, '\n'])

    # write result
    import fcntl

    basename = "optimized_dim_errors.data"
    filename = os.path.join(prefix, basename)
    fd = open(filename, 'a')
    fcntl.lockf(fd.fileno(), fcntl.LOCK_EX)
    if fd.tell() == 0:
        fd.write('labels\t' + '\t'.join([str(d) for d in dimrange]) + '\n')
    txt = ','.join(opt['labels']) + '\t'
    txt += '\t'.join([str(t) for t in errors[dimrange].tolist()])
    txt += '\n'
    fd.write(txt)
    fcntl.lockf(fd.fileno(), fcntl.LOCK_UN)
    fd.close()

    # fit
    reduced_train = dr.get_reduced_train(train, optimal_dim)
    reduced_test = dr.reduce(test, optimal_dim)
    dr.register(optimal_dim)

    return model.learn_svm(prefix, reduced_train, reduced_test, opt)


def ForwardBackwardOptimizedDim(model, train, test, opt):
    '''
1) Begin with all features.
2) For all features try removing it and evaluate error on a test db.
   Remove the worst feature.
3) For all features try adding it and evaluate error on a test db.
   Add the best feature.
4) Repeat 2-3 until a minimum is reached.'''

    def eval(prefix, train, test, dims, opt):
        from datamind.ml import database

        # Reduced database
        reduced_train = train.share()
        reduced_train.setX(train.getX()[:, dims])
        reduced_test = test.share()
        reduced_test.setX(test.getX()[:, dims])

        # classification
        return model.learn_svm(prefix, reduced_train,
                               reduced_test, opt)

    nbfolds = opt['nbfolds']
    prefix = opt['prefix']

    dim = train.getX().shape[1]
    kept_dims = list(range(dim))
    remove_dims = []
    new_error = old_error = eval(prefix, train, test, kept_dims, opt)
    dims_changed = True
    while dims_changed:
        errors = numpy.zeros(len(kept_dims))
        dims_changed = False

        # remove dimension
        for i, d in enumerate(range(len(kept_dims))):
            dims = kept_dims[:d] + kept_dims[d + 1:]
            errors[i] = eval(prefix, train, test, dims, opt)
        i, new_error = errors.argmin(), errors.min()
        if new_error < old_error:
            old_error = new_error
            d = kept_dims[i]
            remove_dims.append(d)
            kept_dims.remove(d)
            dims_changed = True
        sys.stdout.flush()
        sys.stdout.flush()

        # add dimension
        for i, d in enumerate(remove_dims):
            dims = kept_dims + [d]
            errors[i] = eval(prefix, train, test, dims, opt)
        d, new_error = errors.argmin(), errors.min()
        if new_error < old_error:
            old_error = new_error
            remove_dims.remove(d)
            kept_dims.append(d)
            dims_changed = True
        sys.stdout.flush()
        sys.stdout.flush()

    msg.write_list([' * ', ('optimal dimension', 'green'),
                    ' = ', len(dims), '\n'])
    msg.write_list([' * ', ('dimensions list', 'green'),
                    ' = ', str(dims), '\n'])
    return eval(prefix, train, test, kept_dims, opt)


#
def setStandardScaling(model, train, test):
    '''
Scaling based on train and test class 0 elements. Set parameters on model.
    '''
    train_classes = train.getGroups()
    test_classes = test.getGroups()
    train0 = train.getX()[train_classes == 0]
    if test.getX().shape[0] != 0:
        test0 = test.getX()[test_classes == 0]
    else:
        test0 = numpy.array([]).reshape((0, train0.shape[1]))
    data = numpy.concatenate((train0, test0))
    mean = data.mean(axis=0)
    std = data.std(axis=0)
    mean_aims = aims.vector_DOUBLE(mean)
    std_aims = aims.vector_DOUBLE(std)
    model.workEl().setStats(mean_aims, std_aims)
    model.setDimReductor(None)
    return data, mean, std


def precomputing(model, train, test, opt):
    mode = opt['dimreduction']
    selected_dim = opt['selected_dim']
    if mode:
        trainsi, testsi = dimension_reduction(mode, selected_dim,
                                              model, train, test)
    else:
        setStandardScaling(model, train, test)
        trainsi, testsi = train, test
    return trainsi, testsi


def read_database(ad, prefix):
    from datamind.ml import reader
    filename = ad.getDataBaseName(prefix) + '.minf'
    print(" * read '%s'" % filename)
    return reader.Reader('Sigraph').read(filename)


def adaptiveLeaf_train(self, prefix, opt):
    opt['prefix'] = prefix
    from datamind.ml import database

    # loading of the database
    try:
        dbsi, header = read_database(self, prefix)
        # selection Y/class_id
    except:
        name = self.getDataBaseName(prefix)
        msg.warning("reading of '" + name + "(.data, .minf)' failed.")
        return 0.
    # Switch outp / class_id
    if isinstance(self.workEl(), sigraph.SubAdMlp):      # MLP
        filter_mode = 'outp'
    elif isinstance(self.workEl(), sigraph.SubAdGauss):  # Gaussian
        filter_mode = None
    elif isinstance(self.workEl(), sigraph.SubAdMixGauss):  # Gaussians Mix
        filter_mode = 'class_id'
    elif isinstance(self.workEl(), sigraph.sisvm.SubAdSvm):  # SVM
        svm_mode = self.workEl().getSvmMode()
        h = {'classifier': 'class_id', 'oneclass': 'class_id',
             'regression': 'outp', 'quality': 'class_id',
             'decision': 'class_id'}
        filter_mode = h[svm_mode]

    train, test = dbsi.filter(mode=filter_mode, split=True)
    groups = train.getGroups()
    if groups is not None:
        nbfolds = (groups == 0.).sum()
        opt['nbfolds'] = nbfolds

    if isinstance(self.workEl(), sigraph.SubAdMlp):      # MLP
        trainsi, testsi = precomputing(self, train, test, opt)
        err = self.learn_mlp(trainsi, testsi)
    elif isinstance(self.workEl(), sigraph.SubAdGauss) or \
            isinstance(self.workEl(), sigraph.SubAdLogGauss):  # Gaussian
        err = self.learn_gaussian(prefix, train, test, opt)
    elif isinstance(self.workEl(), sigraph.SubAdMixGauss):  # Gaussians Mix
        err = self.learn_mixgaussian(prefix, train, test, opt)
    elif isinstance(self.workEl(), sigraph.sisvm.SubAdSvm):  # SVM
        if opt['optimized_dim'] == 'rankloop':
            err = optimizedDim(opt['dimreduction'], self,
                               train, test, opt)
        elif opt['optimized_dim'] == 'forwardbackward':
            err = ForwardBackwardOptimizedDim(self, train, test,
                                              opt)
        elif not opt['optimized_dim']:
            trainsi, testsi = precomputing(self, train, test, opt)
            err = self.learn_svm(prefix, trainsi, testsi, opt)
    else:
        print('type = ', type(self.workEl()))
        print(" * unknown subadaptive -> skip")
        return 0.
    msg.write_list([' * ', ('err', 'green'), ' = ', err, '\n'])
    return err


# plug some methods into adaptive hierarchy
sigraph.Trainer.trainOne = trainer_trainOne
sigraph.Trainer.readAndTrain = trainer_readAndTrain
sigraph.TopAdaptive.train = topAdaptive_train
sigraph.AdaptiveTree.train = adaptiveTree_train
sigraph.AdaptiveLeaf.train = adaptiveLeaf_train
sigraph.AdaptiveLeaf.learn_mlp = adaptiveleaf_learn_mlp
sigraph.AdaptiveLeaf.learn_svm = adaptiveleaf_learn_svm
sigraph.AdaptiveLeaf.learn_gaussian = adaptiveleaf_learn_gaussian
sigraph.AdaptiveLeaf.learn_mixgaussian = adaptiveleaf_learn_mixgaussian

del trainer_trainOne, trainer_readAndTrain, topAdaptive_train
del adaptiveTree_train, adaptiveLeaf_train
del adaptiveleaf_learn_mlp, adaptiveleaf_learn_svm, adaptiveleaf_learn_gaussian
del adaptiveleaf_learn_mixgaussian
