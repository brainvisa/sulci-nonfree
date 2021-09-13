#!/usr/bin/env python

from __future__ import print_function
import sys
import os
import numpy
import pprint
import re
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from datamind.tools import *
import datamind.io.old_csvIO as datamind_io
from datamind.ml.database import DbNumpy


################################################################################
class DatabaseCreator(object):
    def __init__(self, prefix, sulcus, node_dbcreator, rel_dbcreator):
        self._node_dbcreator = node_dbcreator
        self._rel_dbcreator = rel_dbcreator
        if node_dbcreator:
            node_dbcreator.setPrefix(prefix)
            node_dbcreator.setSulcusFilter(sulcus)
        if rel_dbcreator:
            rel_dbcreator.setPrefix(prefix)
            rel_dbcreator.setSulcusFilter(sulcus)

    def add_graph(self, g):
        subject = g['filename_base']
        if self._node_dbcreator:
            self._node_dbcreator.add_graph(g, subject)
        if self._rel_dbcreator:
            self._rel_dbcreator.add_graph(g, subject)

    def save_to_csv(self, summary_file):
        # save each database in csv
        if self._node_dbcreator:
            self._node_dbcreator.save_to_csv()
        if self._rel_dbcreator:
            self._rel_dbcreator.save_to_csv()

        # summary file
        minf = {}
        datatype = []
        if self._node_dbcreator:
            minf.update(self._node_dbcreator.getMinfFiles())
            datatype.append(self._node_dbcreator.datatype())
        if self._rel_dbcreator:
            minf.update(self._rel_dbcreator.getMinfFiles())
            datatype.append(self._rel_dbcreator.datatype())
        if len(datatype) == 1:
            datatype = datatype[0]
        h = {'data': datatype, 'files': minf}
        io.write_databases(summary_file, h)


################################################################################
class AbstractDatabaseCreator(object):
    def __init__(self):
        self._h = {}
        self._prefix = ''
        self._sulcus_filter = None

    def getMinfFiles(self):
        minf = {}
        for label, d in self._h.items():
            filename = d['files'][1]
            relfilename = re.sub('^%s%s' %
                                 (os.path.dirname(self._prefix),
                                  os.path.sep), '', filename)
            minf[label] = relfilename
        return minf

    def setPrefix(self, prefix):
        self._prefix = prefix

    def setSulcusFilter(self, sulcus):
        self._sulcus_filter = sulcus

    def datatype(self):
        return 'undefine'


class NodeDatabaseCreator(AbstractDatabaseCreator):
    def __init__(self):
        AbstractDatabaseCreator.__init__(self)
        self._writer = datamind_io.WriterCsv()

    def save_to_csv(self):
        print("save node databases...")
        bar = ProgressionBarPct(len(self._h), '#', color='blue')
        for i, (label, d) in enumerate(self._h.items()):
            bar.display(i)
            subjects = []
            for subject, data in d['subjects'].items():
                subjects += [subject] * data['number']
            X = self._toX(d)
            Y = None
            INF = numpy.zeros((X.shape[0], 2), dtype='S32')
            INF[:, 1] = subjects
            INF[:, 0] = label
            db = DbNumpy(X, Y, INF)
            csvname, minfname = d['files']
            header = {'INF': ['sulci', 'subject']}
            header['X'] = self._get_Xheader()
            self._writer.write(csvname, db, header, minfname)


class GravityCentersNodeDatabaseCreator(NodeDatabaseCreator):
    def __init__(self):
        NodeDatabaseCreator.__init__(self)

    def _toX(self, data):
        gc = []
        for subject, d in data['subjects'].items():
            gc += d['refgravity_center']
        X = numpy.vstack(gc)
        return X

    def _get_Xheader(self):
        return ['refgravity_x', 'refgravity_y', 'refgravity_z']

    def add_graph(self, g, subject):
        for v in g.vertices():
            if v.getSyntax() != 'fold':
                continue
            self._add_node(v, subject)

    def _add_node(self, v, subject):
        h = self._h
        if not 'refgravity_center' in v:
            return
        gc = numpy.asarray(v['refgravity_center'].list())
        label = v['name']
        if self._sulcus_filter is not None and \
                label != self._sulcus_filter:
                return
        if not h.has_key(label):
            csvname, minfname = \
                io.sulci2databasename(self._prefix, label)
            d = {'subjects': {}, 'files': (csvname, minfname)}
            d['subjects'][subject] = {'refgravity_center': [gc],
                                      'number': 1}
            h[label] = d
        else:
            subjects = h[label]['subjects']
            if not subjects.has_key(subject):
                subjects[subject] = {'refgravity_center':
                                     [gc], 'number': 1}
            else:
                s = subjects[subject]
                s['refgravity_center'].append(gc)
                s['number'] += 1

    def datatype(self):
        return 'refgravity_center'


################################################################################
class RelDatabaseCreator(AbstractDatabaseCreator):
    def __init__(self):
        AbstractDatabaseCreator.__init__(self)
        self._writer = datamind_io.WriterCsv()
        self._rel_done = set()

    def save_to_csv(self):
        print("save relation databases...")
        bar = ProgressionBarPct(len(self._h), '#', color='blue')
        for i, (labels, d) in enumerate(self._h.items()):
            bar.display(i)
            subjects = []
            for subject, data in d['subjects'].items():
                subjects += [subject] * data['number']
            X = self._toX(d)
            Y = None
            INF = numpy.zeros((X.shape[0], 3), dtype='S32')
            INF[:, 0] = labels[0]
            INF[:, 1] = labels[1]
            INF[:, 2] = subjects
            db = DbNumpy(X, Y, INF)
            csvname, minfname = d['files']
            header = {'INF': ['sulci1', 'sulci2', 'subject']}
            header['X'] = self._get_Xheader()
            self._writer.write(csvname, db, header, minfname)

    def add_graph(self, g, subject):
        for xi in g.vertices():
            if xi.getSyntax() != 'fold':
                continue
            for xj in xi.neighbours():
                if xj.getSyntax() != 'fold':
                    continue
                labels = xi['name'], xj['name']
                if self._sulcus_filter is not None and \
                    (labels[0] != self._sulcus_filter and
                     labels[1] != self._sulcus_filter):
                    continue
                # relations are symetrics
                if (xi, xj) in self._rel_done:
                    continue
                else:
                    self._rel_done.add((xi, xj))
                    self._rel_done.add((xj, xi))

                # uniq order of labels
                if labels[0] > labels[1]:
                    labels = labels[1], labels[0]
                    v1, v2 = xj, xi
                else:
                    v1, v2 = xi, xj
                loc = self._get_saving_location(subject, labels)
                self._add_relation(loc, v1, v2)
                # FIXME

    def _get_saving_location(self, subject, labels):
        h = self._h
        if not h.has_key(labels):
            csvname, minfname = \
                io.relation2databasename(self._prefix, labels)
            d = {'subjects': {},
                 'files': (csvname, minfname)}
            d['subjects'][subject] = {'number': 1}
            h[labels] = d
        else:
            subjects = h[labels]['subjects']
            if not subjects.has_key(subject):
                subjects[subject] = {'number': 1}
            else:
                s = subjects[subject]
                s['number'] += 1
        return h[labels]['subjects'][subject]

    def _saving_location_add(self, loc, name, data):
        if loc.has_key(name):
            loc[name].append(data)
        else:
            loc[name] = [data]

    def _find_xij(self, xi, xj):
        ri = xi['index']
        rj = xj['index']
        edges = {}
        for e in xi.edges():
            v1, v2 = e.vertices()
            r1, r2 = v1['index'], v2['index']
            if (ri == r1 and rj == r2) or (ri == r2 and rj == r1):
                edges[e.getSyntax()] = e
        return edges


class DeltaGravityCentersRelDatabaseCreator(RelDatabaseCreator):
    def __init__(self):
        RelDatabaseCreator.__init__(self)

    def _toX(self, data):
        delta_gc = []
        for subject, d in data['subjects'].items():
            delta_gc += d['delta_gravity_centers']
        X = numpy.vstack(delta_gc)
        return X

    def _get_Xheader(self):
        return ['delta_gravity_x', 'delta_gravity_y', 'delta_gravity_z']

    def _add_relation(self, loc, xi, xj):
        gi = numpy.asarray(xi['refgravity_center'].list())
        gj = numpy.asarray(xj['refgravity_center'].list())

        # vector from gc2 to gc1
        delta_gc = gi - gj
        self._saving_location_add(loc, 'delta_gravity_centers', delta_gc)

    def datatype(self):
        return 'delta_gravity_centers'


class MinDistanceRelDatabaseCreator(RelDatabaseCreator):
    def __init__(self):
        RelDatabaseCreator.__init__(self)

    def _toX(self, data):
        mind = []
        for subject, d in data['subjects'].items():
            mind += d['min_distance']
        X = numpy.vstack(mind)
        return X

    def _get_Xheader(self):
        return ['min_distance']

    def _add_relation(self, loc, xi, xj):
        edges = self._find_xij(xi, xj)
        if 'cortical' in edges.keys():
            xij = edges['cortical']
            pi = xij['refSS1nearest'].arraydata()
            pj = xij['refSS2nearest'].arraydata()
        else:
            pi = numpy.array([0, 0, 0])
            pj = numpy.array([0, 0, 0])
        dist = numpy.sqrt(((pi - pj) ** 2).sum())
        self._saving_location_add(loc, 'min_distance', dist)

    def datatype(self):
        return 'min_distance'


################################################################################
def dbcreatorFactory(node_model_type, rel_model_type):
    nodeh = {
        'refgravity_center': GravityCentersNodeDatabaseCreator,
    }
    relh = {
        'delta_gravity_centers': DeltaGravityCentersRelDatabaseCreator,
        'min_distance': MinDistanceRelDatabaseCreator,
    }
    if node_model_type and not nodeh.has_key(node_model_type):
        print("invalid node model type '%s'" % node_model_type)
        sys.exit(1)
    if rel_model_type and not relh.has_key(rel_model_type):
        print("invalid relation model type '%s'" % rel_model_type)
        sys.exit(1)

    NodeDbC = nodeh.get(node_model_type, None)
    RelDbC = relh.get(rel_model_type, None)
    return NodeDbC, RelDbC

################################################################################


def compute_database(graphs, dbdir, options):
    prefix = dbdir
    try:
        os.mkdir(prefix)
    except OSError as e:
        print("warning: directory '%s' already exists" % prefix)

    N, R = dbcreatorFactory(options.node_model_type, options.rel_model_type)

    if N:
        node_dbcreator = N()
    else:
        node_dbcreator = None
    if R:
        rel_dbcreator = R()
    else:
        rel_dbcreator = None

    # fill databases
    creator = DatabaseCreator(prefix, options.sulcus,
                              node_dbcreator, rel_dbcreator)

    print("create databases...")
    bar = ProgressionBarPct(len(graphs), '#', color='purple')
    for i, g in enumerate(graphs):
        bar.display(i)
        creator.add_graph(g)
    print("write csv...")
    summary_file = prefix + '.dat'
    creator.save_to_csv(summary_file)


################################################################################
def parseOpts(argv):
    description = 'Create databases of gravity centers for sulci segment ' \
        '(node) and gravity centers differencies for sulci segment ' \
        'relations.\n' \
        './create_segment_databases.py [OPTIONS] ' \
        'graph1.arg graph2.arg...'
    parser = OptionParser(description)
    add_translation_option_to_parser(parser)
    parser.add_option('-d', '--dbdir', dest='dbdir',
                      metavar='FILE', action='store',
                      default='bayesian_gaussian_databases',
                      help='output databases directory (default : %default).'
                      'A file named FILE.dat is created to store '
                      'labels/databases links.')
    parser.add_option('-s', '--sulcus', dest='sulcus',
                      metavar='NAME', action='store', default=None,
                      help='Compute gaussian only for nodes and relations related '
                      'to the specified sulcus (default : compute for all sulci)')
    parser.add_option('--node-model-type', dest='node_model_type',
                      metavar='TYPE', action='store', default=None,
                      help='default : nothing\n'
                      'refgravity_center : learn gravity centers of sulci nodes\n')
    parser.add_option('--relation-model-type', dest='rel_model_type',
                      metavar='TYPE', action='store', default=None,
                      help='default : nothing\n'
                      'delta_gravity_centers : learn gravity centers '
                      'differencies of 2 related sulci nodes\n'
                      'min_distance : learn minimum distance between 2 related '
                      'sulci nodes\n')
    parser.add_option('--mode', dest='mode',
                      metavar='FILE', action='store', default='normal',
                      help="'normal' : compute spams on given graphs, 'loo' : " +
                      "leave one out on graphs : create several models " +
                      "(default : %default), all given reference FILE options " +
                      "must be located in './all/' relative directory and similar " +
                      "data can be found in './cv_*/' directories relative to " +
                      "leave one out graphs folds.")

    return parser, parser.parse_args(argv)


def main():
    # handle options
    parser, (options, args) = parseOpts(sys.argv)
    graphnames = args[1:]
    if len(graphnames) == 0:
        parser.print_help()
        sys.exit(1)

    graphs = io.load_graphs(options.transfile, graphnames)

    if options.mode == 'normal':
        compute_database(graphs, options.dbdir, options)
    elif options.mode == 'loo':
        print("-- all --")
        dbdir = os.path.join('all', options.dbdir)
        compute_database(graphs, dbdir, options)
        for i in range(len(graphs)):
            subgraphs = graphs[:i] + graphs[i+1:]
            dir = 'cv_%d' % i
            print('-- %s --' % dir)
            dbdir = os.path.join(dir, options.dbdir)
            compute_database(subgraphs, dbdir, options)
    else:
        print("error : '%s' unknown mode" % options.mode)


if __name__ == '__main__':
    main()
