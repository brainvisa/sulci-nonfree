#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
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
exec(compile(open( statsfile, "rb" ).read(), statsfile, 'exec'))

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
    if 'label' in c:
        label = c['label']
        p = c['potential']
        if label not in sulcires:
            sulcires[label] = {'sulcus_pot': p,
                               'sulcus_std_ratio': 0., 'total_pot': p}
        else:
            sr = sulcires[label]
            sr['sulcus_pot'] += p
            sr['total_pot'] += p
        cs = nodecliquestats.get(label)
        if not cs or 'std' not in cs:
            print('no stats for sulcus', label)
        else:
            s = cs['std']
            if s != 0:
                sulcires[label]['sulcus_std_ratio'] = (p - cs['mean']) / s
            else:
                print('null std for', label)
    elif 'label1' in c:
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
            if label1 not in sulcires:
                sulcires[label1] = {'sulcus_pot': 0.,
                                    'sulcus_std_ratio': 0., 'total_pot': 0.}
            if label2 not in sulcires:
                sulcires[label2] = {'sulcus_pot': 0.,
                                    'sulcus_std_ratio': 0., 'total_pot': 0.}
            sulcires[label1]['total_pot'] += p
            sulcires[label2]['total_pot'] += p
for label, c in sulcires.items():
    cs = totalstats[label]
    if 'std' in cs:
        s = cs['std']
        if s != 0:
            c['total_std_ratio'] = (p - cs['mean']) / s

for v in gr.vertices():
    if labelatt in v:
        label = v[labelatt]
        cs = sulcires.get(label)
        if cs and 'total_std_ratio' in cs:
            v['custom_num_val'] = cs['total_std_ratio']

print(sulcires)
if outgraphfile:
    w = aims.aimssip.Writer_Graph(outgraphfile)
    w.write(gr)
