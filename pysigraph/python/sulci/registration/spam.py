
from .procrust import Registration, MixtureGlobalRegistration, \
    MixtureLocalRegistration
from sulci.models import distribution, distribution_aims
import numpy
# used in standalone use (2024)
import numpy as np
from soma import aims, aimsalgo
from sulci.features.descriptors import descriptorFactory
import argparse
import sys


class SpamRegistration(Registration):

    '''
Spam registration with spherical priors on translation and rotation.

spam :   one spam
g : gravity center of sulcus
X :      data (voxels position)
R_angle_var :  angle prior variance from identity rotation
R_dir_var :    variance of rotation direction from R_dir_mean mean direction
R_dir_mean :   mean rotation direction
t_var :  prior variance from null translation

if None value are specified the corresponding prior is removed.
    '''

    def __init__(self, spam, g, X, R_angle_var=numpy.pi / 8.,
                 R_dir_var=numpy.pi / 8., R_dir_mean=None, t_var=0.5,
                 verbose=0):
        Registration.__init__(self, verbose=verbose)
        self._spam = spam
        self._g = g
        self._X = X
        if R_angle_var is None:
            self._angle_prior = None
        else:
            self._angle_prior = distribution.VonMises()
            self._angle_prior.set_mu(0.)
            self._angle_prior.set_kappa(1. / R_angle_var)
            self._angle_prior.update()
        if (R_dir_var is None) or (R_dir_mean is None):
            self._dir_prior = None
        else:
            self._dir_prior = distribution.VonMisesFisher(p=3)
            self._dir_prior.set_mu(R_dir_mean)
            self._dir_prior.set_kappa(1. / R_dir_var)
            self._dir_prior.update()
        if t_var is None:
            self._t_prior = None
        else:
            self._t_prior = distribution.Gaussian()
            id = numpy.identity(3)
            self._t_prior.setMeanCov(numpy.zeros(3), t_var * id)

    def setPriors(self, translation_prior, direction_prior, angle_prior):
        self._t_prior = translation_prior
        self._dir_prior = direction_prior
        self._angle_prior = angle_prior

    def energy(self):
        X2 = self._R * (self._X - self._g) + self._t + self._g
        X2vox = X2 / numpy.expand_dims(
            self._spam.img_density().getVoxelSize()[:3], 1)
        w_norm = numpy.linalg.norm(self._w)
        if w_norm:
            dir = self._w / w_norm
        else:
            dir = self._w
        s_logli, s_li = self._spam_energy(X2vox)
        if self._angle_prior:
            theta = numpy.arccos((numpy.trace(self._R) - 1.) / 2.)
            a_logli, a_li = self._angle_prior.likelihood(theta)
        else:
            a_logli, a_li = 0., 1.
        if self._dir_prior:
            dir = numpy.asarray(dir).ravel()
            d_logli, d_li = self._dir_prior.likelihood(dir)
        else:
            d_logli, d_li = 0., 1.
        if self._t_prior:
            t_logli, t_li = self._t_prior.likelihood(self._t.T)
        else:
            t_logli, t_li = 0., 1.
        if self._verbose:
            print('en:', -s_logli, -a_logli, -d_logli, -t_logli)
        en = - (s_logli + a_logli + d_logli + t_logli)
        return en

    def _spam_energy(self, X):
        return self._spam.prodlikelihoods(X.T)


class SpamGroupRegistration(SpamRegistration):

    def __init__(self, spam, g, X, weights, groups,
                 R_angle_var=numpy.pi / 2., R_dir_var=numpy.pi / 4.,
                 R_dir_mean=None, t_var=3., verbose=0):
        SpamRegistration.__init__(self, spam, g, X, R_angle_var,
                                  R_dir_var, R_dir_mean, t_var, verbose)
        self._weights = numpy.asarray(weights).ravel()
        self._groups = groups

    def _spam_energy(self, X):
        s_logli, s_li = self._spam.weighted_prodlikelihoods_groups(
            X.T, self._weights, self._groups)
        s_logli = s_logli / self._weights.sum()
        return s_logli, numpy.exp(s_logli)


class SpamMixtureRegistration(Registration):

    def __init__(self, X, weights, spams_mixture, groups,
                 is_affine=False, verbose=0):
        Registration.__init__(self, verbose=verbose)
        if len(weights) != len(spams_mixture):
            raise ValueError("number of spams/weights" +
                             "mismatch. (%d != %d)" % (len(spams_mixture),
                                                       len(weights)))
        self._X = X
        self._weights = numpy.asmatrix(weights)
        self._spams_mixture = spams_mixture
        self._groups = groups
        self._size = len(spams_mixture)
        self._is_affine = is_affine

    def energy(self):
        X2 = self._R * self._X + self._t
        en = 0.
        spams = self._spams_mixture.get_models()
        for i, spam in enumerate(spams):
            w = numpy.asarray(self._weights[i]).ravel()
            logli, li = spam.weighted_prodlikelihoods_groups(X2.T,
                                                             w, self._groups)
            en -= logli
        if self._is_affine:
            # add gamma prior over each scaling factor with mean
            # equal to 1 and with scale parameter k = 1600
            # (std ~= 0.035)
            # P(x|k) = x^(k-1) * exp(-kx) * (k^k) / gamma(k)
            # log(P(x|k)) = (k-1)*log(x) - k*x + C(k)
            en -= (1599 * numpy.log(self._D)
                   - 1600 * self._D).sum()
        return en


MixtureGlobalRegistration._algo_map.update({
    distribution_aims.SpamMixtureModel: SpamMixtureRegistration,
})

MixtureLocalRegistration._algo_map.update({
    distribution_aims.SpamMixtureModel: SpamGroupRegistration,
})


# -------
# Added in 2024 to allow use of SPAM registration in a more "standalone" way
# (as a commandline or simple function)

def build_spam_image(filenames):
    vsum = None
    n = 0
    for fname in filenames:
        vol = aims.read(fname)
        vol[vol.np != 0] = 1
        vol = vol.astype('FLOAT')
        if vsum is None:
            vsum = vol
        else:
            vsum += vol
        n += 1
    vsum /= n
    return vsum, n


def create_data_matrix(data):
    if isinstance(data, aims.Graph):
        return create_data_matrix_fromgraph(data)
    return create_data_matrix_fromvol(data)


def create_data_matrix_fromgraph(graph_data):
    X = []
    groups = []
    id = 0
    vdict = dict([(v['index'], v) for v in graph_data.vertices()])
    graph_data_vertices = [vdict[index] for index in sorted(vdict.keys())]
    # using multiple data types causes an error
    # data_type = ('voxels_aims_ss', 'voxels_bottom')
    data_type = 'voxels_aims_ss'
    descriptor = descriptorFactory(data_type)
    # motion = aims.AffineTransformation3d()
    # WARNING: hard-coded transform to ICBM template ref
    motion = aims.GraphManip.getICBM2009cTemplateTransform(graph_data)

    for xi in graph_data_vertices:
        if xi.getSyntax() != 'fold':
            continue
        # if self._not_selected(xi):
        #     continue
        Xs = descriptor.data(motion, xi)
        if Xs is not None:
            X.append(Xs)
            groups += [id] * len(Xs)
        id += 1
    return np.vstack(X).astype(float), np.array(groups)


def create_data_matrix_fromvol(data):
    Xv = np.where(data.np != 0)
    vs = data.getVoxelSize()[:3]
    X = np.array(Xv[:3]).T * vs
    return X, np.zeros(X.shape[0])


def dilate_spam_mask(spam_vol, dil=5.):
    mask = spam_vol.astype('S16')
    mask.fill(0)
    mask[spam_vol.np > 1e-5] = 32767
    mg = aimsalgo.MorphoGreyLevel_S16()
    mask = mg.doDilation(mask, dil)
    return mask


def mask_data(X, mask):
    vs = mask.getVoxelSize()[:3]
    dim = np.array(mask.getSize()[:3])
    Xvox = np.round(X / vs).astype(int)
    Xfilt = np.logical_and((Xvox >= 0).sum(axis=1).astype(bool),
                           (Xvox < dim).sum(axis=1).astype(bool))
    Xvox = Xvox[Xfilt]
    Xvox = Xvox.T
    Xvox = tuple(Xvox) + (np.zeros((Xvox.shape[1]), dtype=int), )
    X2 = X[Xfilt][mask[Xvox].astype(bool)]
    return X2


def move_image_slightly(data, axis, angle, translation, gravity_center):
    # used for testing
    q = aims.Quaternion()
    q.fromAxis(axis, angle)
    transl = aims.AffineTransformation3d.translationTransform
    tr = transl(translation) \
        * transl(gravity_center) * aims.AffineTransformation3d(q) \
        * transl(gravity_center).inverse()
    resampler = aims.ResamplerFactory_S16.getResampler(0)
    resampled = aims.Volume(data)
    resampler.resample(data, tr, 0, resampled)
    return resampled, tr


def spam_register(spam_vol, skel_data, do_mask=True, eps=1e-5,
                  R_angle_var=np.pi / 8, t_var=5.,
                  in_log=True, calibrate_distrib=None, verbose=False):
    ''' Register data using SPAM maps.
    Data may be a graph or a binary image (skeleton).

    For now if data is a volume, data and spam_vol should be in the same
    referential (no initial transform), and if data is a graph, it is
    transformed to ICBM template space, thus spam_vol should be in this ICBM
    reference space. We may improve this in the future.

    Parameters
    ----------
    spam_vol: Volume_FLOAT
        SPAM probability map
    skel_data: Volume or Graph object or numpy array
        data to be registered. Can be a numpy coordinates array (vox x 3)
        in mm.
    do_mask: bool
        if True, a mask will be made from the SPAM volume, dilated 5mm, and
        data to be registered which are out of this mask is erased in order to
        avoid polluting the energy function, and increase speed.
    eps: float
        minimm energy difference to stop the optimizatioon
    R_angle_var: float
        std deviation constraint on the rotation angle
    t_var: float
        std deviation constraint on the optimized translation
    in_log: bool
        if True (default) SPAMs likelihoods are averaged in logs, which means
        they are the product of the likelihoods of each voxel. A voxel out of
        the SPAM (with zero or near zero likelyhood) will thus have a very
        large defavorable impact on the overall likelihood (its log is a large
        negative number). Thus the algorithm will concentrate to fit all the
        data inside the distribution.
        If False, likelihoods are averaged in linear space. A
        voxel out of the distribution will be 0 (but still count in the
        averaging) but will have less impact then in log space, moreover "far"
        voxels which will always keep out of the distribution will have no
        influence on the optimization, except for the global likelyhood scaling
        (see calibrate_scaling, below). Thus the algorithm will concentrate on
        fitting some data along the most probable regions, disregarding data
        which are outside. It may then be used to fit part of a skeleton in the
        SPAM, using a whole-brain (or whole-hemisphere) skeleton.
    calibrate_distrib: None or float
        if not None, the SPAM distribution will be rescaled according to the
        initial likelyhood of the data, in order to count as the given weight
        relatively to constraints energies (angle, axis and position
        constraints energies). In log mode, the scaling is applied to the
        linear prodlikelihoods, thus not directly to the energy (which is the
        log).
    verbose: bool
        if True, print energy and powell parameters info during optimization.
    '''
    is_affine = False
    user_func = (lambda self, x: None)
    user_data = None
    mode = 'powell'
    # shift = np.exp(-50)


    if isinstance(skel_data, np.ndarray):
        X = skel_data
        gravity_center = np.expand_dims(np.average(X, axis=0), axis=1)
    else:
        X, groups = create_data_matrix(skel_data)
        mc = aims.MassCenters_FLOAT(spam_vol)
        mc.doit()
        gravity_center = np.expand_dims(np.array(mc.infos()['0'][0][0]),
                                        axis=1)

    if do_mask:
        mask = dilate_spam_mask(spam_vol)
        # aims.write(mask, '/tmp/mask.nii.gz')
        Xregion = mask_data(X, mask)
        print('masked:', len(Xregion), ', from:', len(X))
    else:
        Xregion = X

    if not in_log:
        spam_vol.header()['from_log'] = True
    spam = distribution_aims.Spam()
    spam.set_img_density(spam_vol)
    spam.update()

    if calibrate_distrib is not None:
        vs = spam_vol.getVoxelSize()[:3]
        logli, li = spam.prodlikelihoods((Xregion / vs))
        # scale linear likelihoods
        if in_log:
            w = calibrate_distrib / li
        else:
            w = calibrate_distrib / logli
        spam_vol *= w
        spam.set_img_density(spam_vol)
        spam.update()

    algo = SpamRegistration(spam, gravity_center, Xregion.T,
                            R_angle_var=R_angle_var, t_var=t_var,
                            verbose=verbose)
    R = np.matrix(np.eye(3))
    t = np.matrix(np.zeros((3, 1)))

    algo.set_initialization(R, t)
    R, t = algo.optimize(eps, user_func, user_data, mode, is_affine)

    out_tr = aims.AffineTransformation3d()
    out_tr.affine()[:3, :3, 0, 0] = R
    transl = aims.AffineTransformation3d.translationTransform
    out_tr = transl(t) * transl(gravity_center) * out_tr \
        * transl(gravity_center).inverse()

    return out_tr


if __name__ == '__main__':

    verbose = False
    eps = 1e-5
    do_mask = True
    R_angle_std = np.pi / 8
    t_std = 5.
    in_log = True
    calibrate_distrib = None

    parser = argparse.ArgumentParser(
        ''' Register data using SPAM maps.
Data may be a graph or a binary image (skeleton).

For now if data is a volume, data and spam_vol should be in the same
referential (no initial transform), and if data is a graph, it is
transformed to ICBM template space, thus spam_vol should be in this ICBM
reference space. We may improve this in the future.
''')
    parser.add_argument(
        '-i', '--input',
        help='input data file. May be a volume (skeleton) or a graph file.')
    parser.add_argument(
        '-s', '--spam', help='SPAM probability map')
    parser.add_argument(
        '-o', '--output', help='output transformation (.trm)')
    parser.add_argument(
        '-m', '--mask', action=argparse.BooleanOptionalAction, default=do_mask,
        help='if set, a mask will be made from the SPAM volume, dilated 5mm, '
        'and data to be registered which are out of this mask is erased in '
        'order to avoid polluting the energy function, and increase speed.')
    parser.add_argument(
        '--eps', type=float, default=eps,
        help='minimm energy difference to stop the optimization (default: '
        f'{eps})')
    parser.add_argument(
        '-a', '--angle', type=float, default=R_angle_std,
        help='angle prior: std deviation constraint on the rotation angle. '
        f'(default: PI / 8)')
    parser.add_argument(
        '-t', '--translation', type=float, default=t_std,
        help='translation prior: std deviation constraint on the optimized '
        f'translation (default: {t_std})')
    parser.add_argument(
        '--log', action=argparse.BooleanOptionalAction, default=in_log,
        help=
''' if True (default) SPAMs likelihoods are averaged in logs, which means
they are the product of the likelihoods of each voxel. A voxel out of
the SPAM (with zero or near zero likelyhood) will thus have a very
large defavorable impact on the overall likelihood (its log is a large
negative number). Thus the algorithm will concentrate to fit all the
data inside the distribution.
If False, likelihoods are averaged in linear space. A
voxel out of the distribution will be 0 (but still count in the
averaging) but will have less impact then in log space, moreover "far"
voxels which will always keep out of the distribution will have no
influence on the optimization, except for the global likelyhood scaling
(see calibrate_scaling, below). Thus the algorithm will concentrate on
fitting some data along the most probable regions, disregarding data
which are outside. It may then be used to fit part of a skeleton in the
SPAM, using a whole-brain (or whole-hemisphere) skeleton.''')
    parser.add_argument(
        '-c', '--calibrate-distrib', default=calibrate_distrib,
        type=float,
        help=
'''if not None, the SPAM distribution will be rescaled according to the
initial likelyhood of the data, in order to count as the given weight
relatively to constraints energies (angle, axis and position
constraints energies). In log mode, the scaling is applied to the
linear prodlikelihoods, thus not directly to the energy (which is the
log).''')
    parser.add_argument(
        '-v', '--verbose', action='store_true',
        help='if True, print energy and powell parameters info during '
        'optimization.')

    options = parser.parse_args(sys.argv[1:])

    # spam_file = f'/neurospin/dico/data/deep_folding/current/mask/2mm/L/{selected_sulci[0]}.nii.gz'
    # skel_f = '/neurospin/dico/data/deep_folding/current/datasets/pclean/binarized_skeletons/L/Lbinarized_skeleton_ammon.nii.gz'

    skel_f = options.input
    spam_file = options.spam
    out_trm = options.output
    eps = options.eps
    do_mask = options.mask
    R_angle_std = options.angle
    t_std = options.translation
    in_log = options.log
    calibrate_distrib = options.calibrate_distrib
    verbose = options.verbose

    skel_data = aims.read(skel_f)
    spam_vol = aims.read(spam_file).astype('FLOAT')

    out_tr = spam_register(spam_vol, skel_data, do_mask=do_mask, eps=eps,
                           R_angle_var=R_angle_std,
                           t_var=t_std,
                           in_log=in_log, calibrate_distrib=calibrate_distrib,
                           verbose=verbose)
    aims.write(out_tr, out_trm)
