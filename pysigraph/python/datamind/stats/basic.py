from __future__ import absolute_import
import numpy as N
import scipy.stats as SS


def scale(X, mean=None, std=None):
    """
    center and normalize X
    return X normalized, mean, std
    """
    import numpy as N
    if mean is None:
        mean = X.mean(axis=0)
    Xc = X - mean
    if std is None:
        std = Xc.std(axis=0)
    return Xc / std, mean, std


def pillai(eig, dim, df_grp, df_res):
    """
    dim : input space dimension
    df_grp: nb_groups-1
    df_res: nb_samples-nb_groups
    Return the:
    Pillai_score F_stat, p-value, z-value
    """
    test = N.sum(eig / (1 + eig))
    s = N.minimum(dim, df_grp)
    n = 0.5 * (df_res - dim - 1)
    m = 0.5 * (N.abs(dim - df_grp) - 1)
    tmp1 = 2 * m + s + 1
    tmp2 = 2 * n + s + 1
    approxF = (tmp2 / tmp1 * test) / (s - test)
    dof1 = s * tmp1
    dof2 = s * tmp2
    p = SS.f.sf(approxF, dof1, dof2)
    z = SS.norm.isf(p)
    return(test, approxF, p.item(), z.item())
