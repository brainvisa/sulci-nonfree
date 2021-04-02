#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sigraph
from soma import aims
from optparse import OptionParser
import sys


def potentials(mg, dg):
    mf = mg.modelFinder()
    mf.initCliques(dg, True, True)
    ann = sigraph.Anneal(dg, mg)
    ann.processAllPotentials()
    # cl = dg.cliques()

    for v in dg.vertices():
        vpot = 0.
        label = v['label']
        cl = sigraph.Clique.fromSetPtrObject(v['cliques'].get())
        for c in cl:
            try:
                l = c['label']
                if l == label:
                    vpot += c['potential']
            except:
                try:
                    l = c['label1']
                    if l == label:
                        vpot += c['potential']
                        continue
                except:
                    pass
                try:
                    l = c['label2']
                    if l == label:
                        vpot += c['potential']
                except:
                    pass
        #print('pot of node', label, ':', vpot)
        v['potential_summary'] = vpot
        #print(v[ 'potential_summary' ])
        # print(v.keys())


def main(argv):
    parser = OptionParser('Stores potential summary information into data '
                          'graph nodes')
    parser.add_option('-m', '--model', dest='mgraph', action='store',
                      help='model graph filename')
    parser.add_option('-d', '--data', dest='dgraph', action='store',
                      help='data graph filename')
    (options, args) = parser.parse_args(argv)
    mgraph = options.mgraph
    dgraph = options.dgraph
    r = aims.Reader()
    mg = r.read(mgraph)
    dg = r.read(dgraph)
    potentials(mg, dg)
    # now do something with dg...


if __name__ == '__main__':
    main(sys.argv)
