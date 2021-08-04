#!/usr/bin/env python
from __future__ import absolute_import
import os, sys, numpy
from optparse import OptionParser
import sigraph
import sigraph.error
from sulci.common import lock, unlock
from soma import aims


def addInfoToCSV(csvfilename, subject, sulci_errors, append=False):
    '''
    List of errors :
    - size errors : errors are weighted by their sizes.
    - nodes errors : only count the number of nodes bad tagged.
    - binary errors : perfect tag : 0, one or more error : 1.
    '''
    lock(csvfilename + '.lock')
    if append:
        mode = 'a'
    else:
        mode = 'w'
    fd = open(csvfilename, mode)
    if fd.tell() == 0:
        header = ['subjects', 'sulci', 'size_errors', 'binary_errors',
            'node_errors', 'SI_error', 'false_positive',
            'false_negative', 'true_positive']
        h = '\t'.join(header) + '\n'
        fd.write(h)
    for sulcus, error_rate in sulci_errors.items():
        se = error_rate.compute_size_error()
        ne = error_rate.compute_nodes_error()
        sie = error_rate.compute_SI_error()
        fp = error_rate.false_positive()
        fn = error_rate.false_negative()
        tp = error_rate.true_positive()
        be = (se != 0.)
        fd.write(('%s\t%s' + '\t%f' * 7 + '\n') % (subject, sulcus, \
            se, be, ne, sie, fp, fn, tp))
    fd.close()
    unlock(csvfilename + '.lock')

def parseOpts(argv):
    description = 'Count labelling differences on auto/manual ' \
                  'recognitions on a cortical folds graph..'
    parser = OptionParser(description)
    parser.add_option('-l', '--labeled_graph', action='store',
        dest='labeled_graph', metavar = 'FILE', default = None,
        help='labeled graph file (evaluated one)')
    transfile = os.path.join(aims.carto.Paths.shfjShared(),
        'nomenclature', 'translation', 'sulci_model_noroots.trl')
    parser.add_option('-t', '--labels_translation', action='store',
        dest='labels_translation', metavar = 'FILE',
        default = transfile, help='labels translation file')
    parser.add_option('-b', '--base_graph', action='store',
        dest='base_graph', metavar = 'FILE', default = None,
        help='base graph file (reference). If not specified, '
            'labeled_graph is taken')
    parser.add_option('-s', '--subject', action='store',
        dest='subject', metavar = 'NAME', default = None,
        help='subject name (ex : amon, zeus, subject01, ...)')
    parser.add_option('--csv', action='store',
        dest='csvfilename', metavar = 'FILE', default = None,
        help='save sulcuswise errors. If FILE already exists, ' \
            'results are appened at the end. Multiple ' \
            'writting by several instances of this scrip is safe.')
    parser.add_option('-f', '--labels-filter', dest='labels_filter',
        metavar = 'FILE', action='store', default = None,
        help='list of labels (one per line) fixed during recognition '\
        'based on true labels \'name\' field.')
    parser.add_option('-a', '--append', action='store_true',
        dest='append', default = False,
        help='append errors at the end of an existing files instead of ' \
            'erasing it')
    return parser, parser.parse_args(argv)


def main():
    parser, (options, args) = parseOpts(sys.argv)
    if None in [options.labeled_graph, \
        options.labels_translation]:
        parser.print_help()
        sys.exit(1)
    if options.base_graph is None:
        options.base_graph = options.labeled_graph
    # options
    if options.labels_filter:
        fd = open(options.labels_filter)
        filtred_labels = [l[:-1] for l in fd.readlines()]
        fd.close()
    else:	filtred_labels = None

    local_errors, global_errors = sigraph.error.computeErrorRates(
        options.base_graph, options.labeled_graph,
        options.labels_translation, filtred_labels)
    subject = "unknown"
    if options.subject is not None: subject = options.subject
    if options.csvfilename is not None:
        addInfoToCSV(options.csvfilename, subject, local_errors,
            append=options.append)
    sigraph.error.print_global_errors(global_errors)

if __name__ == '__main__' : main()
