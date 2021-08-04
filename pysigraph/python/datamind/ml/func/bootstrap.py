from __future__ import print_function

from __future__ import absolute_import
from . import glm
import numpy as N
from . import roc
# ------------------------------------------------------------------------------
# Boostraped objectif function
# To add a new function you need:
# 1) An objection function: an object with the eval(X) method
# 2) An averaging function see average and averageNormStdDev belows
#
# TO DO
# 1) Inherit from Bootstrap class
# 2) In the constructor, specify the objective function
# Example :
# class BootstrapOneWayAnovaFstat(Bootstrap):
#  def __init__(self,nbIte=30):
#        Bootstrap.__init__(self,func=glm.OneWayAnovaFstat(),nbIte=nbIte)
#


def average(X):
    return X.mean(0)


def averageNormStdDev(X):
    return X.mean(0) / X.std(0)


class Bootstrap(object):
    def __init__(self, func, nbIte=30, averageFunc=average,
                resampleWithinEachGroup=False):
        self.nbIte = nbIte
        self.func = func
        self.averageFunc = averageFunc
        self.resampleWithinEachGroup = resampleWithinEachGroup

    def eval(self, X, Y):
        import datamind.ml.resampling as resample
        if self.resampleWithinEachGroup:
            boot = resample.Boostrap(Y, self.nbIte, resampleWithinEachGroup=True)
        else:
            boot = resample.Boostrap(Y, self.nbIte)
        scores = []
        for rnd_index in boot:
            # print(rnd_index)
            Xi = X[rnd_index,:]
            Yi = Y[rnd_index,:]
            scores.append(self.func.eval(Xi, Yi))
        scores = N.asarray(scores)
        return self.averageFunc(scores)


class BootstrapOneWayAnovaFstat(Bootstrap):

    """
    Bootstrap and average OneWayAnova F-stat

    Examples:
    d=data.twoclassses(stringClassLabels=True)
    X=d[:,["class"]]
    Y=d[:,1:]
    print("Classical Anova Fstat ------------------")
    aov=glm.OneWayAnovaFstat()
    print(aov.eval(Y,X))
    print("Bootstraped Anova Fstat -----------------")
    bootaov=BootstrapOneWayAnovaFstat(nbIte=30)
    print(bootaov.eval(Y,X))
    """
    def __init__(self, nbIte=30):
        Bootstrap.__init__(self, func=glm.OneWayAnovaFstat(), nbIte=nbIte)


class BootstrapOneWayAnovaEffect(Bootstrap):

    """
    Bootstrap and average OneWayAnova Effect then normalize by its std-dev

    Examples:
    print("Classical Anova effect ------------------")
    aov=glm.OneWayAnovaEffect()
    print(aov.eval(Y,X))

    print("Bootstraped Anova Effect -----------------")
    bootaov=BootstrapOneWayAnovaEffect(nbIte=30)
    print(bootaov.eval(Y,X))
    """
    def __init__(self, nbIte=30):
        Bootstrap.__init__(self, func=glm.OneWayAnovaEffect(), nbIte=nbIte,
            averageFunc=averageNormStdDev)


class BootstrapRocAuc(Bootstrap):

    """
    Bootstrap and average OneWayAnova Effect then normalize by its std-dev

    Examples:
    print("Classical ROC AUC  -----------------")
    rocauc=roc.RocAuc()
    print(rocauc.eval(Y,X))

    print("Bootstraped ROC AUC -----------------")
    bootrocauc=BootstrapRocAuc(nbIte=30)
    print(bootrocauc.eval(Y,X))
    """
    def __init__(self, nbIte=30):
        Bootstrap.__init__(self, func=roc.RocAuc(), nbIte=nbIte)

if __name__ == "__main__":
    import datamind.data as data
    print(
        "Anova: Two classes small toy set ===================================")
    d = data.twoclassses(stringClassLabels=True)
    X = d[:, ["class"]]
    Y = d[:, 1:]
    print("Classical Anova Fstat ------------------")
    aov = glm.OneWayAnovaFstat()
    print(aov.eval(Y, X))

    print("Bootstraped Anova Fstat -----------------")
    bootaov = BootstrapOneWayAnovaFstat(nbIte=30)
    print(bootaov.eval(Y, X))

    print("Classical Anova effect ------------------")
    aov = glm.OneWayAnovaEffect()
    print(aov.eval(Y, X))

    print("Bootstraped Anova Effect -----------------")
    bootaov = BootstrapOneWayAnovaEffect(nbIte=30)
    print(bootaov.eval(Y, X))

    print("Classical ROC AUC  -----------------")
    rocauc = roc.RocAuc()
    print(rocauc.eval(Y, X))

    print("Bootstraped ROC AUC -----------------")
    bootrocauc = BootstrapRocAuc(nbIte=30)
    print(bootrocauc.eval(Y, X))
