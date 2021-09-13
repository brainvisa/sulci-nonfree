#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import numpy
from optparse import OptionParser
from sulci.common import io, add_translation_option_to_parser

################################################################################


def compute_mixup_matrix(sulci, graph):
    sulci_n = len(sulci)
    sulci_id = {}
    for i, sulcus in enumerate(sulci):
        sulci_id[sulcus] = i
    M = numpy.zeros((sulci_n, sulci_n))
    for v in graph.vertices():
        if v.getSyntax() != 'fold':
            continue
        M[sulci_id[v['label']],	sulci_id[v['name']]] += 1
    return M


def save_matrix(filename, sulci, M):
    fd = open(filename, 'w')
    print('\t'.join(['sulci'] + sulci), file=fd)
    for i, m in enumerate(M):
        print('\t'.join([sulci[i]] + [str(x) for x in m]), file=fd)
    fd.close()


################################################################################
def parseOpts(argv):
    description = 'compute mix-up matrix of labeling'
    parser = OptionParser(description)
    add_translation_option_to_parser(parser)
    parser.add_option('-g', '--graph', dest='graphname',
                      metavar='NAME', action='store', default=None,
                      help='input graph name')
    parser.add_option('-m', '--graphmodel', dest='graphmodelname',
                      metavar='FILE', action='store',
                      default='bayesian_graphmodel.dat', help='bayesian model : '
                      'graphical model structure (default : %default)')
    parser.add_option('-o', '--output', dest='output',
                      metavar='FILE', action='store',
                      default=None, help='mixup matrix')
    return parser, parser.parse_args(argv)


def main():
    # options
    parser, (options, args) = parseOpts(sys.argv)
    inputs = args[1:]
    if None in [options.graphname, options.graphmodelname, options.output]:
        print("error: missing options")
        parser.print_help()
        sys.exit(1)

    # reading
    graph = io.load_graph(options.transfile, options.graphname)
    graphmodel = io.read_graphmodel(options.graphmodelname)
    sulci = list(graphmodel['vertices'].keys())

    # compute and save
    M = compute_mixup_matrix(sulci, graph)
    save_matrix(options.output, sulci, M)


if __name__ == '__main__':
    main()
