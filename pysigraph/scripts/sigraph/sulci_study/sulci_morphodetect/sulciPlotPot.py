#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sys
import os
import types
import pylab


def plotSulcus(label, subjectspotentials):
    points = ['bo', 'ro', 'go', 'yo']
    if type(subjectspotentials) not in (tuple, list):
        subjectspotentials = [subjectspotentials]
    i = 0
    pylab.figure()
    pylab.title(label)
    for group in subjectspotentials:
        table = []
        for subj, st in group.items():
            if label in st:
                table.append(st[label])
        pylab.plot(table, points[i % len(points)])
        i += 1


def loadSubjectsPotentials(files):
    sp = []
    for statsfile in files:
        gl = {}
        exec(compile(open(statsfile, "rb").read(), statsfile, 'exec'), gl, gl)
        sp.append(gl['subjectspotentials'])
    return sp


if __name__ == '__main__':
    if sys.argv[1] in ('-h', '--help'):
        print('usage:')
        print(sys.argv[0], 'label statsfile.txt [statsfile2.txt ...]')
        print(
            'Plots the potentials of all subjects of one or several groups for one sulcus.')
        print('This python script should rather be used interactively in an IPython shell to avoid GUI event loop problems:')
        print('run:\nipython -pylab')
        print('then within ipython:')
        print('import sulciPlotPot')
        print('files = [ "statsfile.txt", "statsfile2.txt" ]')
        print('sp = sulciPlotPot.loadSubjectsPotentials( files )')
        print('sulciPlotPot.plotSulcus( "S.T.s._left", sp )')
        print('sulciPlotPot.plotSulcus( "F.C.L.a._left", sp )')
        sys.exit(0)
    label = sys.argv[1]
    files = sys.argv[2:]
    plotSulcus(label, loadSubjectsPotentials(files))
