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
import sigraph
from datamind.tools import *

# FIXME : ajouter un mechanisme permettant de controler le parcours :
# par ex : - arreter le parcours parce qu'on a trouver ce qu'on voulait
#          - passer au vertex/edge/element de l'arbre suivant
#          - remonter au niveau plus haut
# PB : pour le moment l'approche retenue impose un parcours complet de l'arbre.
# Idee : - mots clefs a returner depuis les fonctions utilisateurs :
#          'up', 'next', 'stop', 'down' ?

__doc__	= '''
    General help to fill fundict parameter of covering function of this module.

    The general prototype for each user_function is :
        fun(el, fundict, user_data) where 'el' is the current element of
        the graph model or adaptive hierarchy structure.
        For edge_* function, el is an aimssip.Edge.
        For vertex_* function, el is an aimssip.Vertex.
	For topadaptive_* function, el is an TopAdaptive.
	For adaptivetree_* function, el is an AdaptiveTree.
	For adpativeleaf function, el is an AdaptiveLeaf.

    These functions are called at each corresponding step of the covering.
    The postfix 'before' indicates that the call is done before cover down into
    the graph/adaptive structure. The postfix 'after' indicated that the call
    is down after climbing upto the element of the structure. Because
    adaptiveleaf is the deeper structure, you have access to only one user
    function call at this step.

    You can override this funtions : edge_before, edge_after, vertex_before,
    vertex_after, topadaptive_before, topadaptive_after, adaptivetree_before,
    adaptivetree_after, adaptiveleaf.'''


def add_filter_options(parser):
    '''
Add 2 options : --labelsfilter and --filtermode to give filtering power to
your program.

parser :   OptionParser instance from optparse module.'''
    import optparse

    def filter_callback(option, opt_str, value, parser):
        if value != None:
            value = value.split(',')
        setattr(parser.values, option.dest, value)

    def mode_callback(option, opt_str, value, parser):
        if value in ['strict', 'flexible', 'regexp']:
            setattr(parser.values, option.dest, value)
        else:
            error = "'%s' invalid filter mode " % str(value)
            raise optparse.OptionValueError(error)

    parser.add_option('-f', '--labelsfilter', dest='labels_filter',
                      metavar='FILTER', action='callback', nargs=1,
                      default=None, callback=filter_callback, type='string',
                      help='only models matching this filter are covered.' +
                      "ex : 'label1,label2' or 'label' or 'regexp1,regexp2' " +
                      "(here order is important)")
    parser.add_option('--filtermode', dest='filter_mode', metavar='FILTER',
                      action='callback', default='strict', nargs=1, type='string',
                      callback=mode_callback, help='strict, flexible or regexp')


def filtred(labels, filter, mode):
    '''
labels :  list of labels of current vertex or edge
filter :  list of labels (coma separated values, i.e : 'label1,label2' or
          'label1') used to build the filter if mode is 'strict' or
          'flexible'. For regexp, filter is a list of regexp (csv) where
mode :    * 'strict' : return True if one of the label in filter are not in
          current labels list or vice-versa.
          * 'flexible' : return True if one of the label in filter are not
          not current labels list.
          * 'regexp' : '''
    if not filter:
        return False
    import numpy
    if not mode:
        mode = 'strict'

    def all_in(labels1, labels2):
        return numpy.array([not l in labels2 for l in labels1]).any()
    if mode == 'strict':
        return all_in(filter, labels) or all_in(labels, filter)
    elif mode == 'flexible':
        return all_in(filter, labels)
    elif mode == 'regexp':
        import re
        if len(labels) != len(filter):
            return True
        try:
            return numpy.array([not re.compile(f).match(labels[i])
                                for i, f in enumerate(filter)]).any()
        except:
            import sys
            msg.error("'" + ','.join(filter) +
                      "' invalid regular expression.")
            sys.exit(1)

    else:
        msg.warning(" '" + str(mode) + "' invalid filtering mode, " +
                    "use 'strict' mode instead.")


def save_adaptive_callback(ad, user_data):
    ao = ad.topModel().parentAO()
    print(ao)
    mw = user_data['frgwriter']
    mw.dataDirectory(user_data['graph'])
    mw.parseModel(ao, "model_file", "model")


def save(model, filename, filter=None, filter_mode=None):
    '''
model :       graph model instance
filename :    filename to store model
filter :      filter (see filtred doc)
filter_mode : filter mode (see filtred doc).'''
    w = sigraph.FrgWriter(filename)
    if filter is None:
        w.write(model)
    else:
        fundict = {'topadaptive_before': save_adaptive_callback}
        data = {'frgwriter': w}
        cover(model, fundict, data, filter, filter_mode)


def cover_vertices(model, fundict, user_data=None,
                   filter=None, filter_mode=None):
    '''
Cover all vertices (sulci) of a graph model and call fundict functions
when it is required.

model :      graph model.
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.
filter :      filter (see filtred doc)
filter_mode : filter mode (see filtred doc).'''
    res = []
    vertices = model.vertices()
    for v in vertices:
        labels = [v['label']]
        if filtred(labels, filter, filter_mode):
            continue
        fundict.get('vertex_before', lambda s, u: None)(v, user_data)
        mod = v['model']
        if type(mod) == sigraph.Model:
            continue
        res.append(mod.cover(fundict, user_data))
        fundict.get('vertex_after', lambda s, u: None)(v, user_data)
    return res


def cover_edges(model, fundict, user_data=None,
                filter=None, filter_mode=None):
    '''
Cover all edges (sulci relations) of a graph model and call fundict
functions when it is required.

model :      graph model.
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.
filter :      filter (see filtred doc)
filter_mode : filter mode (see filtred doc).'''
    res = []
    edges = model.edges()
    for e in edges:
        labels = [e['label1'], e['label2']]
        if filtred(labels, filter, filter_mode):
            continue
        fundict.get('edge_before', lambda s, u: None)(e, user_data)
        try:
            mod = e['model']
        except TypeError:
            import exceptions
            raise exceptions.RuntimeError('invalid model type ' +
                                          'for ' + str(labels))
        if not mod:
            import exceptions
            raise exceptions.RuntimeError('no model for ' +
                                          str(labels))
        if type(mod) == sigraph.Model:
            continue
        res.append(mod.cover(fundict, user_data))
        fundict.get('edge_after', lambda s, u: None)(e, user_data)
    return res


# cover all models
def cover(model, fundict, user_data=None,
          filter=None, filter_mode=None):
    '''
Cover all models (sulci and sulci relations) of a graph model and call
fundict functions when it is required.

model :      graph model.
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.'''
    res = []
    res.append(cover_edges(model, fundict, user_data,
                           filter, filter_mode))
    res.append(cover_vertices(model, fundict, user_data,
                              filter, filter_mode))


def cover_count(model, filter=None, filter_mode=None):
    def f(e, data):	data['n'] += 1
    fundict = {'edge_before': f, 'vertex_before': f}
    data = {'n': 0}
    cover(model, fundict, data, filter, filter_mode)
    return data['n']


def cover_adaptive_count(model, filter=None, filter_mode=None):
    def f(e, data): data['n'] += 1
    fundict = {'adaptiveleaf': f}
    data = {'n': 0}
    cover(model, fundict, data, filter, filter_mode)
    return data['n']


def topAdaptive_cover(self, fundict, user_data=None):
    '''
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.'''
    fundict.get('topadaptive_before', lambda s, u: None)(self, user_data)
    res = self.model().cover(fundict, user_data)
    fundict.get('topadaptive_after', lambda s, u: None)(self, user_data)
    return res


def adaptiveTree_cover(self, fundict, user_data=None):
    '''
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.'''
    res = []
    fundict.get('adaptivetree_before', lambda s, u: None)(self, user_data)
    for c in self.children():
        res.append(c.cover(fundict, user_data))
    fundict.get('adaptivetree_after', lambda s, u: None)(self, user_data)
    return res


def adaptiveLeaf_cover(self, fundict, user_data=None):
    '''
fundict:     see __doc__ of cover module for prototype and usage.
user_data :  user data passed to user function.'''
    fundict.get('adaptiveleaf', lambda s, u: None)(self, user_data)

sigraph.TopAdaptive.cover = topAdaptive_cover
sigraph.AdaptiveTree.cover = adaptiveTree_cover
sigraph.AdaptiveLeaf.cover = adaptiveLeaf_cover

del topAdaptive_cover, adaptiveTree_cover, adaptiveLeaf_cover
