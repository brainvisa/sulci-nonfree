#!/usr/bin/env python

import sys, os, pprint, numpy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser

def create_graphmodel(graphs):
    hv, he = {}, set()
    i = 0
    size = len(graphs)
    print
    sys.stdout.write("\rvertices... %d/%d" % (i, size))
    sys.stdout.flush()
    for g in graphs:
        i += 1
        for v in g.vertices():
            if v.getSyntax() != 'fold': continue
            label = v['name']
            hv[label] = set()
        sys.stdout.write("\rvertices... %d/%d" % (i, size))
        sys.stdout.flush()
    i = 0
    print
    sys.stdout.write("\redges... %d/%d" % (i, size))
    sys.stdout.flush()
    for g in graphs:
        i += 1
        for e in g.edges():
            v1, v2 = e.vertices()
            if v1.getSyntax() != 'fold' or v2.getSyntax() != 'fold':
                continue
            labels = v1['name'], v2['name']
            if labels[0] == labels[1]: continue
            # uniq order of labels
            if labels[0] > labels[1]: labels = labels[1], labels[0]
            he.add(labels)
            hv[labels[0]].add(labels[1])
            hv[labels[1]].add(labels[0])
        sys.stdout.write("\redges... %d/%d" % (i, size))
        sys.stdout.flush()
    print
    # set -> list
    for k in hv.keys(): hv[k] = list(hv[k])
    return {'edges' : list(he), 'vertices' : hv}

def write_graphmodel(filename, graphmodel):
    if not os.path.exists(os.path.dirname(filename)):
        os.makedirs(os.path.dirname(filename))
    fd = open(filename, 'w')
    fd.write('graphmodel = \\\n')
    p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
    p.pprint(graphmodel)
    fd.close()

def parseOpts(argv):
    description = 'Create databases for gaussian model.\n' \
        './siSulciToSpam.py [OPTIONS] graph1.arg graph2.arg...'
    parser = OptionParser(description)
    add_translation_option_to_parser(parser)
    parser.add_option('-m', '--model', dest='modelname',
        metavar = 'FILE', action='store',
        default = 'bayesian_graphmodel.dat',
        help='output graphical model structure (default : %default)')
    return parser, parser.parse_args(argv)


def main():
    parser, (options, args) = parseOpts(sys.argv)
    graphnames = args[1:]
    if len(graphnames) == 0:
        parser.print_help()
        sys.exit(1)

    graphs = io.load_graphs(options.transfile, graphnames, nthread=0)
    graphmodel = create_graphmodel(graphs)
    write_graphmodel(options.modelname, graphmodel)

if __name__ == '__main__' : main()
