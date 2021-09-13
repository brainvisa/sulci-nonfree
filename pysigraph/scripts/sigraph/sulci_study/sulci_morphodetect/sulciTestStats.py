#!/usr/bin/env python

from __future__ import print_function
from soma import aims
import sigraph
import math
import sys

if sys.argv[1] == '-h' or sys.argv[1] == '--help':
    print(
        sys.argv[0], 'inputGraph.arg modelfile.arg statsfile.txt labelattribute [outgraphfile.arg]')
    sys.exit(0)

input = sys.argv[1]

modelfile = sys.argv[2]

statsfile = sys.argv[3]
labelatt = sys.argv[4]
if len(sys.argv) >= 6:
    outgraphfile = sys.argv[5]
else:
    outgraphfile = ''

model = aims.read(modelfile)
execfile(statsfile)

gr = aims.read(input)
mf = model.modelFinder()
mf.initCliques(gr, True, False, True)
del mf
an = sigraph.Anneal(gr, model)
p = an.processAllPotentials()
del an
cl = gr.cliques()

sulcires = {}

for c in cl:
    if c.has_key('label'):
        label = c['label']
        p = c['potential']
        if not sulcires.has_key(label):
            sulcires[label] = {'sulcus_pot': p,
                               'sulcus_std_ratio': 0., 'total_pot': p}
        else:
            sr = sulcires[label]
            sr['sulcus_pot'] += p
            sr['total_pot'] += p
        cs = nodecliquestats.get(label)
        if not cs or not cs.has_key('std'):
            print('no stats for sulcus', label)
        else:
            s = cs['std']
            if s != 0:
                sulcires[label]['sulcus_std_ratio'] = (p - cs['mean']) / s
            else:
                print('null std for', label)
    elif c.has_key('label1'):
        l = [c['label1'], c['label2']]
        l.sort()
        l = tuple(l)
        label1 = l[0]
        label2 = l[1]
        p = c['potential']
        cs = relcliquestats.get(l)
        if not cs:
            print('no stats for rel', l)
        else:
            if not sulcires.has_key(label1):
                sulcires[label1] = {'sulcus_pot': 0.,
                                    'sulcus_std_ratio': 0., 'total_pot': 0.}
            if not sulcires.has_key(label2):
                sulcires[label2] = {'sulcus_pot': 0.,
                                    'sulcus_std_ratio': 0., 'total_pot': 0.}
            sulcires[label1]['total_pot'] += p
            sulcires[label2]['total_pot'] += p
for label, c in sulcires.items():
    cs = totalstats[label]
    if cs.has_key('std'):
        s = cs['std']
        if s != 0:
            c['total_std_ratio'] = (p - cs['mean']) / s

for v in gr.vertices():
    if v.has_key(labelatt):
        label = v[labelatt]
        cs = sulcires.get(label)
        if cs and cs.has_key('total_std_ratio'):
            v['custom_num_val'] = cs['total_std_ratio']

print(sulcires)
if outgraphfile:
    w = aims.aimssip.Writer_Graph(outgraphfile)
    w.write(gr)
