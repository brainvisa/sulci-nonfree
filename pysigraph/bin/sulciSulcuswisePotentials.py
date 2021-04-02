#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import absolute_import
import sigraph
from soma import aims
from optparse import OptionParser
from six.moves import range

parser = OptionParser(
    description='Records sulcuswise potential (including relations) in a CSV file')
parser.add_option('-o', '--output', dest='output', help='output CSV file')
parser.add_option('-m', '--model', dest='model',
                  help='model graph')
parser.add_option("-i", "--input", dest="graphs", action='append',
                  help="input graph(s). Several graphs may be specified")

(options, args) = parser.parse_args()

if args and not options.output:
    options.output = args[0]
    del args[0]
if args and not options.model:
    options.model = args[0]
    del args[0]
if options.graphs is None:
    options.graphs = []
options.graphs += args

if not options.graphs or not options.output or not options.model:
    parser.parse_args(['-h'])

mg = aims.read(options.model)
# match standard annealing conditions
mg.removeWeights()

sulci = {}
ng = len(options.graphs)

for ig, gn in enumerate(options.graphs):
    print(gn)
    graph = aims.read(gn)
    mg.modelFinder().initCliques(graph)
    an = sigraph.Anneal(graph, mg)
    energy = an.processAllPotentials()
    del an
    print('cliques:', graph.cliques().size(), ', energy:', energy)
    cliques = graph.cliques()
    for c in cliques:
        if 'potential' not in c:
            print('no pot for clique', c)
        pot = c['potential']
        if 'label' in c:
            label = c['label']
            if label not in sulci:
                sulci[label] = [0] * ng
            sulci[label][ig] += pot
        elif 'label1' in c and 'label2' in c:
            label = c['label1']
            if label not in sulci:
                sulci[label] = [0] * ng
            sulci[label][ig] += pot
            label = c['label2']
            if label not in sulci:
                sulci[label] = [0] * ng
            sulci[label][ig] += pot

print('labels:', len(sulci))

snames = list(sulci.keys())
snames.sort()
wf = open(options.output, 'w')
print('subject,', ', '.join(snames), file=wf)
for i in range(ng):
    print(options.graphs[i] + ',', ', '.join([str(sulci[n][i]) for n in snames]), file=wf)
