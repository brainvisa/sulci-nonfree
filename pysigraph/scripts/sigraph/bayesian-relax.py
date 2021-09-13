#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import numpy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io
from datamind.tools import *


keep_python_wrappers = []


def parseOpts(argv):
    description = 'Annealing to label sulci.'
    parser = OptionParser(description)
    parser.add_option('-c', '--config', dest='config',
                      metavar='FILE', action='store', default=None,
                      help='siRelax config file')
    parser.add_option('--nodes-weights', dest='nodes_weights_filename',
                      metavar='FILE', action='store', default=None,
                      help="csv storing nodes_weights : need 'sulci' and "
                      "'size_errors' columns name")
    parser.add_option('-l', '--labels', dest='labelsfile',
                      metavar='FILE', action='store', default=None,
                      help='file storing available labels for each sulci node')
    parser.add_option('-d', '--distrib_sulci', dest='distribname_sulci',
                      metavar='FILE', action='store', default=None,
                      help='shape sulcus distribution models')
    parser.add_option('--clf_sulci', dest='clfname_sulci',
                      metavar='FILE', action='store', default=None,
                      help='shape sulcus classifier models')
    parser.add_option('-n', '--distrib_nodes', dest='distribname_nodes',
                      metavar='FILE', action='store', default=None,
                      help='nodes position distribution models')
    parser.add_option('-m', '--graphmodel', dest='graphmodelname',
                      metavar='FILE', action='store',
                      default='bayesian_graphmodel.dat', help='bayesian model : '
                      'graphical model structure (default : %default)')
    parser.add_option('--norelations', dest='norelations',
                      action='store_true', default=False,
                      help='inhibite relations models')
    parser.add_option('-w', '--weights', dest='weights', metavar='FLOAT',
                      action='store', default=','.join([str(1./3)] * 4),
                      help='list with 4 elements given weights (ordered like this) '
                      'segment, sulci, relation and prior. Warning weights will be '
                      'normalized, so if a model is missing, you must give a null '
                      'weight for it (default : all models have the same weights).'
                      'ex: 0.2,0.8,0.')
    parser.add_option('-p', '--prior', dest='prior',
                      metavar='NAME', action='store', default=None,
                      help='use priors (default : no prior), see distrib FILE '
                      'content to know available priors.')
    parser.add_option('-f', '--labels-filter', dest='labels_filter',
                      metavar='FILE', action='store', default=None,
                      help='list of labels (one per line) fixed during recognition '
                      'based on true labels \'name\' field.')
    parser.add_option('--penalize-unknown', dest='penalize_unknown',
                      metavar='TYPE', action='store', default=None,
                      help='penalize unknown label (default : penalize through SPAM '
                      'model if given else not penalization). Choose TYPE among :\n'
                      '- node_number : P(X|L=unknown)=(1/Z)*(0.01)^(#unknown/50)\n'
                      '- node_size : P(X|L=unknown)=(1/Z)*(0.01)^(#unknown/10000)')
    parser.add_option('--no-weights', dest='noweights', action='store_true',
                      default=False, help='remove model weights (sulci,relations)')
    return parser, parser.parse_args(argv)


def clean_relations(cgraph):
    cliques = cgraph.cliques()

    # delete edges cliques
    todel = [c for c in cliques if (c.getSyntax() == 'fakeRel'
                                    or c['model_type'] == 'random_edge')]
    for c in todel:
        del cliques[c]
    for v in cgraph.vertices():
        cliques = v["cliques"]
        for c in todel:
            try:
                del cliques[c]
            except KeyError:
                pass


def clean_available_labels(cgraph, availablelabels, fixed_labels):
    # keep only "available labels"
    for v in cgraph.vertices():
        remove_ok = True
        add_ok = True
        if v.getSyntax() != 'fold':
            continue
        ind = v['index']
        cliques = v['cliques']
        try:
            true_label = v['name']
        except KeyError:
            pass
        if fixed_labels and true_label in fixed_labels:
            av = [true_label]
            v['label'] = true_label
            remove_ok = add_ok = False
        elif availablelabels:
            av = availablelabels[ind]
            if fixed_labels:
                s1 = set(av)
                s2 = set(fixed_labels)
                av = list(set.difference(s1, s2))

        # clean possible labels
        pl = v['possible_labels'].get()
        possible_labels = aims.vector_STRING.fromObject(pl)
        possible_labels_save = aims.vector_STRING(possible_labels)

        # add only labels from the intersection of
        # availablelabels and possible_labels
        if availablelabels:
            for i in range(len(possible_labels)):
                del possible_labels[0]
            for s in av:
                if (not remove_ok and not add_ok) or \
                        (s in possible_labels_save):
                    possible_labels.append(s)
        if add_ok and not (v['label'] in possible_labels):
            if len(possible_labels) > 0:
                print("Warning : node " + str(v['index']) +
                      ", '" + v['label'] +
                      "' label forbidden, replace it by '" +
                      possible_labels[0] + "'")
                v['label'] = possible_labels[0]
            else:
                print("Warning : node " + str(v['index']) +
                      "available labels empty, fill it " +
                      "with current label '" +
                      v['label'] + "'")
                possible_labels.append(v['label'])

        # clean cliques
        todel = [c for c in cliques
                 if (c['model_type'] == 'random_vertex')
                 and not (c['label'] in possible_labels)]
        for c in todel:
            c.removeVertex(v)
        # FIXME : faut-il nettoyer les cliques de relation?
        # on peut pour accelerer, mais ce n'est pas obligatoire
        # il faut verier que les 2 labels de la relation ne sont
        # present ni sur le vertex considere ni


class PythonMergeAdaptive(sigraph.AdaptiveLeaf):
    def __init__(self, models, priors, prior_ind, weights):
        '''
models/weights dictionnary indexed by node/sulcus/relation
        '''
        sigraph.AdaptiveTree.__init__(self)
        self._models = models
        self._weights = weights
        if prior_ind != -1 and priors:
            self._prior = priors[prior_ind]
        else:
            self._prior = 1.

    def prop(self, cl):
        en = 0
        for name, m in self._models.items():
            en += self._weights[name] * m.prop(cl)
        vertices = cl.vertices()
        if self._weights['prior']:
            en -= self._weights['prior'] * numpy.log(self._prior)
        return en


class PythonSulcusOnlyAdaptive(sigraph.AdaptiveLeaf):
    '''
 model for nodes + sulcus.
    '''

    def __init__(self, datagraph, mgraph, graphobject, node_model,
                 prior_ind, weights, node_weight):
        sigraph.AdaptiveTree.__init__(self)
        self._datagraph = datagraph
        self._mgraph = mgraph
        self._graphobject = graphobject
        if node_model:
            self._node_model = PythonNodeAdaptive(datagraph,
                                                  node_model, node_weight)
        else:
            self._node_model = None
        if prior_ind != -1:
            self._prior = node_model['priors'][prior_ind]
        else:
            self._prior = 1.
        self._weights = weights

    def prop(self, cl):
        w1, w2 = self._weights['node'], self._weights['sulcus']
        en = w2 * self.prop_sulcus(cl)
        if self._node_model:
            en += w1 * self._node_model.prop(cl)
        vertices = cl.vertices()
        if self._weights['prior']:
            en -= self._weights['prior'] * numpy.log(self._prior)
        return en

    def graphObject(self):
        return self._graphobject


class PythonDistribSulcusOnlyAdaptive(PythonSulcusOnlyAdaptive):
    '''
model for node + sulcus as density function
    '''

    def __init__(self, datagraph, mgraph, graphobject, node_model,
                 sulcus_model, prior_ind, weights, node_weight):
        PythonSulcusOnlyAdaptive.__init__(self, datagraph, mgraph,
                                          graphobject, node_model, prior_ind, weights, node_weight)
        self._sulcus_distr = sulcus_model['density']
        self.init_cache()

    def init_cache(self):
        # threshold very bad energies
        self._thrsh_li = self._sulcus_distr.appf(0.9)

        # freq if missing sulcus
        nb = self._mgraph['nbase_graphs']
        ao = self.graphObject()
        if ao.has_key('noinstance_count'):
            p = ao['noinstance_count']
            f = 1. - (p / nb)
        else:
            f = 1.
        freq = self._sulcus_distr.appf(f)
        # avoid inf values
        if freq[1] == 0:
            freq = -1000., 0.
        self._freq_li = freq

    def prop_sulcus(self, cl):
        cliquedescr = self.cliqueDescr()
        ao = self.graphObject()
        vec = aims.vector_DOUBLE()
        valid = cliquedescr.makeVector(cl, vec, ao)
        if not valid:
            logli, li = self._freq_li
            # scale nodes and sulcus likelihood to the same values
            neglogli = - (logli / len(vec))
            return neglogli
        cliquedescr.preProcess(vec, ao)
        vec = vec.arraydata()
        logli, li = self._sulcus_distr.likelihood(vec)
        if logli < self._thrsh_li[0]:
            logli, li = self._thrsh_li
        return - logli


class PythonClassifierSulcusOnlyAdaptive(PythonSulcusOnlyAdaptive):
    '''
model for node + sulcus as density function
    '''

    def __init__(self, datagraph, mgraph, graphobject, node_model,
                 sulcus_model, prior_ind, weights, node_weight):
        PythonSulcusOnlyAdaptive.__init__(self, datagraph, mgraph,
                                          graphobject, node_model, prior_ind, weights, node_weight)
        self._sulcus_clf = sulcus_model['clf']

    def prop_sulcus(self, cl):
        cliquedescr = self.cliqueDescr()
        ao = self.graphObject()
        vec = aims.vector_DOUBLE()
        valid = cliquedescr.makeVector(cl, vec, ao)
        if not valid:
            return 1
        cliquedescr.preProcess(vec, ao)
        vec = vec.arraydata()
        p, label = self._sulcus_clf.predict(vec)
        if p == 0:
            logli = 1000
        else:
            logli = numpy.log(p)
        return - logli


class PythonNodeAdaptive(sigraph.AdaptiveLeaf):
    _cache = {}

    def __init__(self, datagraph, node_model, node_weight):
        sigraph.AdaptiveTree.__init__(self)
        self._datagraph = datagraph
        self._distr = node_model['density']
        self._node_weight = node_weight

    def prop(self, cl):
        '''
alpha :   flatten coefficient.
P(X|D) :  SPAM distribution

               P(X|D)^alpha
P(X|D alpha) = ---------------------
             _
            /
            |  P(X|D)^alpha dX
          _/                 
        '''
        cache = PythonNodeAdaptive._cache
        neglogli = 0.
        vertices = cl.vertices()
        label = cl['label']
        for v in vertices:
            if v['label'] != label:
                continue
            ind = v['index']
            if cache.has_key((ind, label)):
                neglogli -= cache[(ind, label)]
            else:
                logli, li = self._distr.likelihood(
                    self._datagraph, v)
                # flatten distribution :
                alpha = (1. - self._node_weight)
                alpha_norm = self._distr.powered_integral(alpha)
                logli = logli * alpha - numpy.log(alpha_norm)
                cache[(ind, label)] = logli
                neglogli -= logli
        return neglogli


class PythonPenalizeNodeNumberAdaptive(sigraph.AdaptiveLeaf):
    def __init__(self):
        '''
proba : proba d'avoir node_number nodes unknown

E = - #n * log(proba) / node_number
P = proba^(#n / node_number)
        '''
        sigraph.AdaptiveTree.__init__(self)
        proba = 0.01
        node_number = 50
        self._weight = - numpy.log(proba) / node_number

    def prop(self, cl):
        vertices = cl.vertices()
        label = cl['label']
        n = 0
        for v in vertices:
            if v['label'] == label:
                n += 1
        return self._weight * n


class PythonPenalizeNodeSizeAdaptive(sigraph.AdaptiveLeaf):
    def __init__(self):
        sigraph.AdaptiveTree.__init__(self)
        proba = 0.01
        node_size = 10000  # FIXME : verifier
        self._weight = - numpy.log(proba) / node_size

    def prop(self, cl):
        vertices = cl.vertices()
        label = cl['label']
        size = 0
        for v in vertices:
            if v['label'] == label:
                size += v['size']
        return self._weight * size


class WeightedRelationAdaptive(sigraph.AdaptiveLeaf):
    def __init__(self, relation_model, weight):
        sigraph.AdaptiveTree.__init__(self)
        self._relation_model = relation_model
        self._weight = weight

    def prop(self, cl):
        return self._weight * self._relation_model.prop(cl)

    def graphObject(self):
        return self._relation_model.graphObject()


def replace_models(datagraph, mgraph, (nodes_model, sulci_model),
                   prior, weights, nodes_weights):
    sulci_models = sulci_model['vertices']
    sulci_model_type = sulci_model['type']
    if nodes_model:
        nodes_models = nodes_model['vertices']
    else:
        nodes_models = None
    if prior and nodes_models:
        pn = nodes_model['priors_nodes_hash']
        prior_ind = pn[prior]
    else:
        prior_ind = -1

    for v in mgraph.vertices():
        if v.getSyntax() != 'model_node':
            continue
        oldmodel = v['model']
        if not oldmodel.isAdaptive():
            continue
        sulcus = v['label']
        sulcus_model = sulci_models[sulcus]
        if nodes_models:
            if nodes_models.has_key(sulcus):
                node_model = nodes_models[sulcus]
            else:
                node_model = None
        else:
            node_model = None
        if nodes_weights:
            try:
                node_weight = nodes_weights[sulcus]
            except KeyError:
                node_weight = 0.
        else:
            node_weight = 0.
        if sulci_model_type == 'distributions':
            m = PythonDistribSulcusOnlyAdaptive(datagraph, mgraph,
                                                oldmodel.graphObject(), node_model,
                                                sulcus_model, prior_ind, weights, node_weight)
        elif sulci_model_type == 'classifiers':
            m = PythonClassifierSulcusOnlyAdaptive(datagraph,
                                                   mgraph, oldmodel.graphObject(), node_model,
                                                   sulcus_model, prior_ind, weights, node_weight)
        else:
            print("unknown type '%s'" % sulci_model_type)
        m.setParent(oldmodel.parent())
        m.setCliqueDescr(oldmodel.model().cliqueDescr())
        keep_python_wrappers.append(m)
        x = aims.Object.ptrToObject(m)
        v['model'] = x


def merge_relations(mgraph, weights, relations):
    # wrapped relations models
    for e in mgraph.edges():
        oldmodel = e['model']
        # only adaptive models are wrapped
        if not oldmodel.isAdaptive():
            continue
        m = WeightedRelationAdaptive(oldmodel, weights['relation'])
        m.setParent(oldmodel.parent())
        m.setCliqueDescr(oldmodel.model().cliqueDescr())
        keep_python_wrappers.append(m)
        x = aims.Object.ptrToObject(m)
        e['model'] = x


def merge_nodes_and_sulci(datagraph, mgraph, nodes_model, prior,
                          weights, nodes_weights):
    if nodes_model:
        nodes_models = nodes_model['vertices']
    else:
        nodes_models = None
    if prior and nodes_models:
        pn = nodes_model['priors_nodes_hash']
        prior_ind = pn[prior]
    else:
        prior_ind = -1

    # merge sulci models
    for v in mgraph.vertices():
        if v.getSyntax() != 'model_node':
            continue
        oldmodel = v['model']
        if not oldmodel.isAdaptive():
            continue
        sulcus = v['label']
        if nodes_models:
            if nodes_models.has_key(sulcus):
                node_model = nodes_models[sulcus]
            else:
                node_model = None
        else:
            node_model = None
        if nodes_weights:
            try:
                node_weight = nodes_weights[sulcus]
            except KeyError:
                node_weight = 0.
        else:
            node_weight = 0.
        models_to_merge = {'sulcus': oldmodel}
        if node_model:
            m = PythonNodeAdaptive(datagraph, node_model,
                                   node_weight)
            models_to_merge['node'] = m
            priors = node_model['priors']
        else:
            priors = None
        merge = PythonMergeAdaptive(models_to_merge, priors,
                                    prior_ind, weights)
        merge.setParent(oldmodel.parent())
        merge.setCliqueDescr(oldmodel.model().cliqueDescr())
        keep_python_wrappers.append(merge)
        x = aims.Object.ptrToObject(merge)
        v['model'] = x


def add_clique_unknown_model(datagraph, mgraph, nodes_models, prior_ind,
                             node_weight, weights):
    # add unknown clique
    cliques = datagraph.cliques()
    unknown_cliques = [c for c in datagraph.cliques()
                       if c['model_type'] == 'random_vertex' and
                       c['label'] == 'unknown']
    if len(unknown_cliques) == 0:
        unknown_clique = sigraph.VertexClique()
        unknown_clique['model_type'] = 'random_vertex'
        unknown_clique['label'] = 'unknown'
        unknown_clique['graph'] = datagraph
        keep_python_wrappers.append(unknown_clique)
    elif len(unknown_cliques) == 1:
        unknown_clique = unknown_cliques[0]
        cliques.add(unknown_clique)
    else:
        msg.warning('several unknown cliques, take the first')
        unknown_clique = unknown_cliques[0]
        cliques.add(unknown_clique)
    for v in datagraph.vertices():
        if v.getSyntax() != 'fold':
            continue
        pl = v['possible_labels'].get()
        possible_labels = aims.vector_STRING.fromObject(pl)
        if 'unknown' in possible_labels:
            unknown_clique.addVertex(v)


def add_node_unknown_model(datagraph, mgraph, nodes_models, prior_ind,
                           node_weight, weights):
    node_model = nodes_models['unknown']
    priors = node_model['priors']
    m = PythonNodeAdaptive(datagraph, node_model, node_weight)
    models_to_merge = {'node': m}
    merge = PythonMergeAdaptive(models_to_merge, priors, prior_ind, weights)
    v = [v for v in mgraph.vertices() if v['label'] == 'unknown'][0]
    keep_python_wrappers.append(merge)
    x = aims.Object.ptrToObject(merge)
    v['model'] = x


def add_penalize_node_number_unknown_model(datagraph, mgraph, nodes_models,
                                           prior_ind, node_weight, weights):
    node_model = nodes_models['unknown']
    priors = node_model['priors']
    m = PythonPenalizeNodeNumberAdaptive()
    models_to_merge = {'node': m}
    merge = PythonMergeAdaptive(models_to_merge, priors, prior_ind, weights)
    v = [v for v in mgraph.vertices() if v['label'] == 'unknown'][0]
    keep_python_wrappers.append(merge)
    x = aims.Object.ptrToObject(merge)
    v['model'] = x


def add_penalize_node_size_unknown_model(datagraph, mgraph, nodes_models,
                                         prior_ind, node_weight, weights):
    node_model = nodes_models['unknown']
    priors = node_model['priors']
    m = PythonPenalizeNodeSizeAdaptive()
    models_to_merge = {'node': m}
    merge = PythonMergeAdaptive(models_to_merge, priors, prior_ind, weights)
    v = [v for v in mgraph.vertices() if v['label'] == 'unknown'][0]
    keep_python_wrappers.append(merge)
    x = aims.Object.ptrToObject(merge)
    v['model'] = x


def handle_unknown_model(datagraph, mgraph, nodes_model, prior,
                         weights, nodes_weights, penalize_unknown):
    if nodes_model:
        nodes_models = nodes_model['vertices']
    else:
        nodes_models = None
    if prior and nodes_models:
        pn = nodes_model['priors_nodes_hash']
        prior_ind = pn[prior]
    else:
        prior_ind = -1

    if nodes_weights:
        try:
            unknown_weight = nodes_weights['unknown']
        except KeyError:
            unknown_weight = 0.
    else:
        unknown_weight = 0.
    if not nodes_models:
        return

    if penalize_unknown is None:
        add_node_unknown_model(datagraph, mgraph, nodes_models,
                               prior_ind, unknown_weight, weights)
    elif penalize_unknown == 'node_number':
        add_penalize_node_number_unknown_model(datagraph, mgraph,
                                               nodes_models, prior_ind, unknown_weight, weights)
    elif penalize_unknown == 'node_size':
        add_penalize_node_size_unknown_model(datagraph, mgraph,
                                             nodes_models, prior_ind, unknown_weight, weights)

    add_clique_unknown_model(datagraph, mgraph, nodes_models,
                             prior_ind, unknown_weight, weights)


def merge_models(datagraph, mgraph, nodes_model, prior,
                 weights, nodes_weights, relations):
    if relations:
        merge_relations(mgraph, weights, relations)
    merge_nodes_and_sulci(datagraph, mgraph, nodes_model, prior,
                          weights, nodes_weights)


def remove_weights(graph):
    for v in graph.vertices():
        model = v['model']
        if not model:
            continue
        topmodel = model.topModel()
        if not topmodel:
            continue
        topmodel.setWeight(1)
    for e in graph.edges():
        model = e['model']
        if not model:
            continue
        topmodel = model.topModel()
        if not topmodel:
            continue
        topmodel.setWeight(1)


def main():
    # read options
    parser, (options, args) = parseOpts(sys.argv)
    if None in [options.config]:
        parser.print_help()
        sys.exit(1)
    if options.distribname_sulci and options.clfname_sulci:
        print("Only one option among --distrib_sulci and clf_sulci " +
              "can be specified.")
        parser.print_help()
        sys.exit(1)

    # weights
    w = numpy.array([float(x) for x in options.weights.split(',')])
    w /= w.sum()
    weights = {'node': w[0], 'sulcus': w[1],
               'relation': w[2], 'prior': w[3]}
    if len(weights) != 4:
        msg.error("must give 4 weights.")
        sys.exit(1)

    if options.nodes_weights_filename:
        import datamind.io as datamind_io
        r = datamind_io.ReaderCsv()
        d = r.read(options.nodes_weights_filename)
        errors = numpy.asarray(d[:, 'size_errors'])
        sulci = d[:, 'sulci'].tolist()
        nodes_weights = dict(zip(sulci, errors))
    else:
        nodes_weights = None

    # read
    print("reading...")
    if options.labelsfile:
        availablelabels = io.read_availablelabels(options.labelsfile)
    else:
        availablelabels = None
    if options.distribname_sulci:
        sulci_model = io.read_bayesian_model(
            options.graphmodelname, options.distribname_sulci)
    elif options.clfname_sulci:
        sulci_model, dummy1, dummy2 = io.read_clf_bayesian_model(
            options.graphmodelname, options.clfname_sulci)
    else:
        sulci_model = None
    if options.distribname_nodes:
        nodes_model = io.read_bayesian_model(
            options.graphmodelname, options.distribname_nodes)
    else:
        nodes_model = None
    if options.labels_filter:
        fd = open(options.labels_filter)
        fixed_labels = [l[:-1] for l in fd.readlines()]
        fd.close()
    else:
        fixed_labels = None

    rg = sigraph.FRGraph()
    fg = sigraph.FGraph()

    cfg = sigraph.AnnealConfigurator()
    cfg.loadConfig(options.config)
    cfg.loadGraphs(rg, fg)

    # init anneal
    an = sigraph.Anneal(fg, rg)
    cfg.initAnneal(an, cfg.plotFile)
    sigraph.AnnealExtension(an)
    an.setVoidMode(sigraph.Anneal.VOIDMODE_NONE, 0)

    # prepare models : remove relations/labels if needed
    print("adapt models...")
    cgraph = an.cGraph()  # datagraph
    cgraph.loadAllMissingElements()  # load buckets

    # cleaning
    if options.noweights:
        remove_weights(an.rGraph())
    if sulci_model or options.norelations:
        clean_relations(cgraph)
    if availablelabels:
        clean_available_labels(cgraph,
                               availablelabels, fixed_labels)

    if sulci_model:
        replace_models(cgraph, an.rGraph(), [nodes_model, sulci_model],
                       options.prior, weights, nodes_weights)
    else:
        merge_models(cgraph, an.rGraph(), nodes_model, options.prior,
                     weights, nodes_weights, not options.norelations)
    handle_unknown_model(cgraph, an.rGraph(), nodes_model, options.prior,
                         weights, nodes_weights, options.penalize_unknown)

    # fit
    an.reset()
    while not an.isFinished():
        print("---")
        an.nIter()
        an.fitStep()

    # write graph output with new labels
    w = sigraph.FoldWriter(cfg.output)
    w.write(fg)


if __name__ == '__main__':
    main()
