#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import pprint
import numpy
from optparse import OptionParser
import datamind.io.old_csvIO as datamind_io
from datamind.tools import *
from sulci.models import classifier, classifier_all
from sulci.common import io


def parseOpts(argv):
    description = 'Learn supervised models for sulci'
    parser = OptionParser(description)
    parser.add_option('-m', '--graphmodel', dest='graphmodelname',
                      metavar='FILE', action='store',
                      default='bayesian_graphmodel.dat',
                      help='graphical model structure (default : %default)')
    parser.add_option('-b', '--database', dest='database',
                      metavar='FILE', action='store',
                      default='bayesian_sulci_databases.dat',
                      help='databases summary file (default : %default).')
    parser.add_option('-c', '--clfdir', dest='clfdir',
                      metavar='FILE', action='store',
                      default='bayesian_sulci_classifier',
                      help='output classifiers directory (default : %default).'
                      'A file named FILE.dat is created to store '
                      'labels/databases links.')
    parser.add_option('-s', '--sulcus', dest='sulcus',
                      metavar='NAME', action='store', default=None,
                      help='learn only specified sulcus.')
    return parser, parser.parse_args(argv)


def create_database(prefix, graphmodel, sulcidb, sulcus):
    sulci = [sulcus] + graphmodel['vertices'][sulcus]
    databases = []
    for neighbour in sulci:
        if neighbour is 'unknown':
            continue
        minfname = os.path.join(prefix, sulcidb[neighbour])
        db, header = datamind_io.ReaderMinfCsv().read(minfname)
        X = db.getX()
        X = X[X[:, 0] != 0]
        databases.append(X)

    X = numpy.vstack(databases)
    Y = []

    selected_dims = numpy.argwhere(X.std(axis=0) != 0).ravel()

    # Y = 0...01...1 (0 : sulci, 1 : neighbours)
    for i, db in enumerate(databases):
        Y += [i != 0] * len(db)
    Y = numpy.array(Y)
    return X, Y, selected_dims


def main():
    parser, (options, args) = parseOpts(sys.argv)
    sulcidb = io.read_databaselist(options.database)

    if sulcidb['data'] != 'sulci_features':
        print('database data type must be : sulci_features')
        sys.exit(1)

    dbdir = options.clfdir
    prefix = os.path.dirname(options.database)
    graphmodel = io.read_graphmodel(options.graphmodelname)

    # create output directory
    try:
        os.mkdir(dbdir)
    except OSError as e:
        print("warning: directory '%s' already exists" % dbdir)

    #clf_type = 'gaussian_crvm'
    clf_type = 'optimized_gaussian_crvm'

    # learn databases
    bar = ProgressionBarPct(len(sulcidb['files']), '#', color='yellow')
    h = {'model': 'filtred', 'files': {}}
    for i, labels in enumerate(sulcidb['files'].keys()):
        bar.display(i)
        if isinstance(labels, list) or isinstance(labels, tuple):
            continue
        else:
            if options.sulcus is not None and \
                    options.sulcus != labels:
                        continue
            sulcus = labels
        X, Y, dims = create_database(prefix, graphmodel,
                                     sulcidb['files'], sulcus)
        Filter = classifier.classifierFactory('filtred')
        Clf = classifier.classifierFactory(clf_type)
        #clf = Clf(1000.)
        clf = Clf(10.)
        filter = Filter(clf, dims)

        #size = len(X)
        #folds_n = 10
        #ind = range(size)
        #fold_size = size / folds_n
        #print("labels = ", labels)
        # for i in range(folds_n):
        #	s1 = (size - fold_size) / (folds_n - 1)
        #	test = ind[i * s1:i * s1 + fold_size + 1]
        #	train = ind[:i * s1] + ind[i * s1 + fold_size + 1:]
        #	filter.fit(X[train], Y[train])
        #	P, labels = filter.predict_db(X[test])
        #	print((labels ==Y[test]).sum() * 100. / len(labels), "%")
        #	print("P = ", (P * labels + (1-P) * (1-labels)).mean())
        # break #FIXME

        filter.fit(X, Y)
        filename = io.node2densityname(dbdir,
                                       'filtred_' + clf_type, labels)
        filter.write(filename)
        h['files'][labels] = ('filtred', filename)

    # write distribution summary file
    summary_file = options.clfdir + '.dat'
    fd = open(summary_file, 'w')
    fd.write('classifiers = \\\n')
    p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
    p.pprint(h)
    fd.close()


if __name__ == '__main__':
    main()
