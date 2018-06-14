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
import sigraph
import datamind.ml.classifier.optimizers
import string
import os
import sys


class GridPlotter(object):

    def _ranges2labels(self, ranges):
        return [('%2.1e' % i) for i in ranges]

    # Ne marche que sur une grille 2D
    def plot(self, filename, a, names, ranges, title='Classification '
             'rates from cross-validation'):
        import numpy
        import pylab
        labels1 = self._ranges2labels(ranges[0])
        labels2 = self._ranges2labels(ranges[1])

        f = pylab.figure(1)
        pylab.xticks(range(len(labels1)), labels1)
        pylab.yticks(range(len(labels2)), labels2)
        pylab.imshow(a[numpy.arange(len(a), 0, -1) - 1],
                     interpolation='nearest')
        if a.min() != a.max():
            pylab.colorbar()
        pylab.xlabel(names[0] + ' Values')
        pylab.ylabel(names[1] + ' Values')
        pylab.title(title)
        pylab.savefig(filename)
        f.clear()

    def plot_average(self, filename, a, names, ranges):
        self.plot(filename, a, names, ranges, title='Average '
                  'classification rates from cross-validation')

    def plot_best(self, filename, best_parameters, names, ranges):
        import numpy
        import pylab
        labels1 = self._ranges2labels(ranges[0])
        labels2 = self._ranges2labels(ranges[1])

        f = pylab.figure(1)
        try:
            for params, size in best_parameters.items():
                # FIXME : pour des axes logs uniquement ici
                pylab.scatter([numpy.log(params[0])],
                              [numpy.log(params[1])], s=(size * 10))
        except:
            print("skip '" + filename + "' (insufficient data)")
            return
        pylab.xticks(numpy.log(ranges[0]), labels1)
        pylab.yticks(numpy.log(ranges[1]), labels2)
        pylab.grid(True)
        pylab.xlabel(names[0] + ' Values')
        pylab.ylabel(names[1] + ' Values')
        pylab.title('Best parameters')
        pylab.savefig(filename)
        f.clear()


class GridReader(object):

    def read(self, filename):
        import cPickle
        return cPickle.load(open(filename))


class GridWriter(object):
    # Fichier : Liste de (nom/range) + array

    def write(self, filename, a, names, ranges):
        import cPickle
        o = (filename, a, names, ranges)
        cPickle.dump(o, open(filename, 'w'))
