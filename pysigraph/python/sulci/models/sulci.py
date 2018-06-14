from __future__ import print_function
import six


class SulciModel(object):

    def __init__(self, graphmodel, segments_distrib=None,
                 relations_distrib=None, sulci_distrib=None,
                 labels_prior=None, global_rotation_prior=None,
                 local_rotations_prior=None):
        self._graphmodel = graphmodel
        self._segments_distrib = segments_distrib
        self._relations_distrib = relations_distrib
        self._sulci_distrib = sulci_distrib
        self._labels_prior = labels_prior
        self._global_rotation_prior = global_rotation_prior
        self._local_rotations_prior = local_rotations_prior

    def labels(self):
        if self._segments_distrib:
            return list(six.iterkeys(self._segments_distrib['vertices']))
        elif self._relations_distrib:
            pairs = self._segments_distrib['vertices'].keys()
            return set([pairs[0] for x in l] + [pairs[1] for x in l])
        else:
            print("error : not implemented")
            import sys
            sys.exit(1)

    def graphmodel(self): return self._graphmodel

    def segments_distrib(self): return self._segments_distrib

    def relations_distrib(self): return self._relations_distrib

    def sulci_distrib(self): return self._sulci_distrib

    def labels_prior(self): return self._labels_prior

    def global_rotation_prior(self): return self._global_rotation_prior

    def local_rotations_prior(self): return self._local_rotations_prior
