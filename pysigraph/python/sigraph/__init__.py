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

'''
U{sigraph library<../../sigraph-3.1/doxygen/index.html>} bindings

Model and data graphs for learning, automatic recognition, simulated
annealing, sulcal graphs identification, and far more funny features.

Main general purpose classes:
  - L{CGraph}
  - L{MGraph}
  - L{Model}, L{Adaptive} and subclasses
  - L{SubAdaptive} and subclasses
  - L{Anneal}
  - L{Learner}, L{Trainer}
  - L{Clique}
  - L{CliqueDescr} and subclasses

Main specialized classes:
  - L{FoldDescr2}
'''

from __future__ import print_function

import soma.aims
from sigraph import sigraphsip
import string
import os
import sys
from soma.importer import ExtendedImporter

# get everything out of namespace sigraph
ExtendedImporter().importInModule('', globals(), locals(), 'sigraphsip')
ExtendedImporter().importInModule(
    '', globals(), locals(), 'sigraphsip', ['sigraphsip.sigraph'])

del ExtendedImporter

soma.aims.__fixsipclasses__(locals().items())

# load plugins
__pluginsdir__ = os.path.join(os.path.dirname(__file__), 'plugins')
__plugins__ = []
sys.path.insert(0, __pluginsdir__)
pf = filter(lambda x: x.endswith('.py')
            or os.path.isdir(os.path.join(__pluginsdir__, x)),
            os.listdir(__pluginsdir__))
for x in pf:
    p = os.path.basename(x)
    if not os.path.isdir(os.path.join(__pluginsdir__, x)):
        i = p.rfind('.')
        if i >= 0:
            p = p[:i]
    try:
        exec('import ' + p)
        __plugins__.append(p)
    except Exception as e:
        print('loading of sigraph.' + p, 'failed:')
        print(e)
        del e
del os, x, p, i, sys, pf

convertersObjectToPython = {
    'PN7sigraph12VertexCliqueE': sigraphsip.sigraph.VertexClique.fromObject,
    'set of CliquePtr': Clique.fromSetPtrObject,
    'PN7sigraph5ModelE': Model.fromObject,
    'PSt3setIPN7sigraph6CliqueESt4lessIS2_ESaIS2_EE':
                    sigraphsip.sigraph.Clique.fromSetPtrObject,
}
soma.aims.convertersObjectToPython.update(convertersObjectToPython)

del soma, sigraph, convertersObjectToPython


class Settings(object):

    '''
    Settings : sigraph global settings are defined here.

    debug : if True some debug is activated (and also datamind one).
    '''
    debug = False

# docs

MGraph.__doc__ = '''
Model graph.

A model graph contains vertices and edges that are model
elements (L{Model}). They provide potentials to assess a clique graph
(L{CGraph}) and can be adaptive. Learning techniques work on model graphs.

U{Link to the C++ API<../../sigraph/doxygen/classsigraph_1_1MGraph.html>}
'''

# MGraph.checkCompatibility.__doc__ = '''
# Checks if the model graph is compatible with the data graph.

#@param graph: data graph
#@type graph: L{Graph}
#@return: check result status
#@rtype: L{VersionCheck} object
#'''

CGraph.__doc__ = '''
Clique graph.

A clique graph is a data graph that can be specialized (see L{FGraph} for
instance). It adds to the L{Graph} structure a set of L{Clique}s which are
used with a model graph (L{MGraph}) to set potentials, during a simulated
annealing (L{Anneal}) algorithm for instance.
'''
