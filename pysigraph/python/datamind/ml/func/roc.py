from __future__ import print_function

from __future__ import absolute_import
import numpy as N
from six.moves import range


class RocAuc(object):

    """
    Area under Curve (AUC) of ROC (Receiver Operating Characteristic) curve.
    Best == 1
    Random == 0.5

    - groupOfInterest is the label on which we want to compute the AUC.
      It must be one os the value returned by N.unique(y)
      If groupOfInterest=="both" (default): return(max(auc, 1-auc))
    - sort (default increasing) how x values are sorted. Other
      possible value is "decreasing".

      If you are interested on how a x variable do a good seperation
      of the two groups just let the default value ("both")

    Examples:
    import datamind.data as data
    import numpy as N
    d=data.twoclassses(nbSamples=30,deltaMeans=N.arange(2,0,-.1),nbNoize=100)
    y=d[:,["class"]]
    X=d[:,1:]

    auc=RocAuc()
    aucs=auc.eval(X,y)
    print("AUCs for each variable (the 10 first are discriminatives)")
    print(aucs)
    print("Variables ranks sorting with decreasing AUCs (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(aucs)]).ravel())

    print("Focus on the detection of group 0: AUCs & rank of 10 first variables")
    aucs=RocAuc(groupOfInterest=0).eval(X,y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])

    print("Focus on the detection of group 1: AUCs & rank of 10 first variables")
    aucs=RocAuc(groupOfInterest=1).eval(X,y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])
    print("=> Group 1 samples have bigger x values => small AUCs")
    print("=> Reverse the sorting order")
    aucs=RocAuc(groupOfInterest=1,sort="decreasing").eval(X,y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])
    self=RocAuc(groupOfInterest=1,sort="decreasing")
    """

    def __init__(self, groupOfInterest="both", sort="increasing"):
        self.groupOfInterest = groupOfInterest
        self.sort = sort

    def eval(self, X, y):
        """
        """
        labels = y.ravel()
        levels = N.unique(labels)
        if self.groupOfInterest == "both":
            groupOfInterest = levels[0]
        else:
            groupOfInterest = self.groupOfInterest
        nb_positive = N.sum(y == groupOfInterest)
        deltaX = 1. / (labels.size - nb_positive)
        deltaY = 1. / nb_positive
        import exceptions
        if levels.size != 2:
            raise exceptions.ValueError('y must have 2 labels')
        aucs = []
        for j in range(X.size // X.shape[0]):
            # j=0
            ycoord = 0
            auc = 0
            labels_sorted = labels[N.argsort(X[:, j])]
            if self.sort == "decreasing":
                labels_sorted = N.fliplr([labels_sorted]).ravel()
            for l in groupOfInterest == labels_sorted:
                if l:
                    ycoord += deltaY
                else:
                    auc += ycoord * deltaX
                # print(l,ycoord,auc)
            if self.groupOfInterest == "both":
                auc = N.maximum(auc, 1 - auc)
            aucs.append(auc)
        return(N.array(aucs))


if __name__ == "__main__":
    import datamind.data as data
    import numpy as N
    d = data.twoclassses(
        nbSamples=30, deltaMeans=N.arange(2, 0, -.1), nbNoize=100)
    y = d[:, ["class"]]
    X = d[:, 1:]

    auc = RocAuc()
    aucs = auc.eval(X, y)
    print("AUCs for each variable (the 10 first are discriminatives)")
    print(aucs)
    print(
        "Variables ranks sorting with decreasing AUCs (the 10 first are discriminatives)")
    print(N.fliplr([N.argsort(aucs)]).ravel())

    print(
        "Focus on the detection of group 0: AUCs & index of the 10 first ranked variables")
    aucs = RocAuc(groupOfInterest=0).eval(X, y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])

    print(
        "Focus on the detection of group 1: AUCs & index of the 10 first ranked variables")
    aucs = RocAuc(groupOfInterest=1).eval(X, y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])
    print("=> Group 1 samples have bigger x values => small AUCs")
    print("=> Reverse the sorting order")
    aucs = RocAuc(groupOfInterest=1, sort="decreasing").eval(X, y)
    print(aucs[:10])
    print(N.fliplr([N.argsort(aucs)]).ravel()[:10])
    self = RocAuc(groupOfInterest=1, sort="decreasing")
