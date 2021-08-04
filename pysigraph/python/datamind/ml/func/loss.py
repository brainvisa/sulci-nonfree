# ---------------------------------------------------------------------------
# LOSS functions
from __future__ import absolute_import
import numpy as N


def zeroOne_err(pred, true, sd=False):
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    diffs = pred != true
    if sd:
        float(N.sum(diffs)) / pred.size, N.std(diffs)
    else:
        return float(N.sum(diffs)) / pred.size


def zeroOne_acc(pred, true, sd=False):
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    eq = pred == true
    if sd:
        return float(N.sum(eq)) / pred.size, N.std(eq)
    else:
        return float(N.sum(eq)) / pred.size


def ms_err(pred, true):
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    return N.mean(N.square(pred - true))


def explained_var(pred, true):
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    return (N.var(true) - N.var(true - pred)) / N.var(true) * 100.0


def weighted_classif_rate(pred, true):
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    labels = N.unique(true)
    true_labels = [N.nonzero(true == lab)[0] for lab in labels]
    classif_rates = [float(N.sum(pred[true_lab] == true[true_lab])) / len(true_lab)
                     for true_lab in true_labels]
    return float(N.sum(classif_rates)) / len(labels)


def sens_spec_rate(pred, true):
    '''
    sensibility TP/(TP+FN) or TP/P  and specificity TN/(TN+FP) ot TN /N
    '''
    pred = N.asarray(pred).ravel()
    true = N.asarray(true).ravel()
    if pred.size != true.size:
            raise ValueError('pred and true do not have the same size')
    labels = N.unique(true)
    true_labels = [N.nonzero(true == lab)[0] for lab in labels]
    classif_rates = [float(N.sum(pred[true_lab] == true[true_lab])) / len(true_lab)
                     for true_lab in true_labels]
    return classif_rates
