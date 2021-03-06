from __future__ import print_function
from __future__ import absolute_import
import os
import numpy
from soma import aims
from sulci.common import io
from six.moves import range

# class Transformation(object): pass


class RigidTransformation(aims.Transformation3d):

    def __init__(self, R, t):
        aims.Transformation3d.__init__(self)
        self._R = R
        self._t = t

    def to_motion(self):
        m = numpy.identity(4)
        m[0:3, 3] = self._t.T
        m[:3, :3] = self._R
        motion = aims.Motion(m.flatten())
        return motion

    def write(self, filename):
        motion = self.to_motion()
        aims.Writer().write(motion, filename)


class LocalRigidTransformations(aims.Transformation3d):

    def __init__(self, transformations):
        aims.Transformation3d.__init__(self)
        trs = []
        for i, trans in enumerate(transformations):
            if isinstance(trans, tuple):
                R, t = trans
                trans = RigidTransformation(R, t)
            trs.append(trans)
        self._transformations = trs

    def get_transformations(self):
        return self._transformations

    def write(self, filenames):
        for i in range(len(self._transformations)):
            trans = self._transformations[i]
            filename = filenames[i]
            trans.write(filename)


class SulcusWiseRigidTransformations(aims.Transformation3d):

    def __init__(self, transformations=None):
        aims.Transformation3d.__init__(self)
        self._transformations = {}
        if not transformations:
            return
        for sulcus, trans in transformations.items():
            self.add_transformation(sulcus, trans)

    def add_transformation(self, sulcus, transformation):
        self._transformations[sulcus] = transformation

    def write(self, directory, summary_filename):
        try:
            os.makedirs(directory)
        except OSError as e:
            print("directory '%s' already exists" % directory)
        h = {}
        for sulcus, trans in self._transformations.items():
            if trans is None:
                continue
            filename = "motion_%s.trm" % sulcus
            filename = os.path.join(directory, filename)
            h[sulcus] = os.path.join(
                os.path.basename(os.path.dirname(filename)),
                os.path.basename(filename))
            trans.write(filename)
        io.write_pp('transformations', summary_filename, h)
