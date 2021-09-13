#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import pylab
from six.moves import range
import matplotlib
import numpy


def read_graph(filename):
    try:
        exec(compile(open(filename, "rb").read(), filename, 'exec'))
        graph2 = locals()['graph']
        return graph2
    except Exception as e:
        print(e)
        sys.exit(1)


def read_database_sulci(prefix, sulci):
    import datamind.io.old_csvIO as datamind_io
    minfname = os.path.join(prefix, 'bayesian_' + sulci + '.minf')
    try:
        db, header = datamind_io.ReaderMinfCsv().read(minfname)
        return db, header
    except IOError:
        return None, None


def plot_2d_sulci_vs_neighbours(colors, a, neighbours, dim1, dim2):
    '''
    a : axes.'''
    pylab.plot()
    xmin = ymin = numpy.inf
    xmax = ymax = -numpy.inf
    for i, (label, (db, header)) in enumerate(neighbours.items()):
        # skip neighbours with few instance (database is not generated)
        if db is None:
            continue
        X = db.getX()
        x = X[:, dim1]
        y = X[:, dim2]
        xmint, xmaxt = x.min(), x.max()
        ymint, ymaxt = y.min(), y.max()
        if xmint < xmin:
            xmin = xmint
        if ymint < ymin:
            ymin = ymint
        if xmaxt > xmax:
            xmax = xmaxt
        if ymaxt > ymax:
            ymax = ymaxt
        line = pylab.Line2D(x, y, color=colors[i], linestyle=' ',
                            marker='o', markersize=4, markeredgewidth=0.1)
        a.add_line(line)
    a.set_xlim(xmin, xmax)
    a.set_ylim(ymin, ymax)


def show_sulci_vs_neighbours(prefix, graph, sulci):
    neighbours = graph[sulci]
    neighbours.append(sulci)

    neighbours_db = {}
    for i, n in enumerate(neighbours):
        db, header = read_database_sulci(prefix, n)
        neighbours_db[n] = db, header

    colors = pylab.rand(3 * len(neighbours)).reshape(len(neighbours), 3)
    colors = [matplotlib.colors.rgb2hex(c) for c in colors]

    f = pylab.figure()
    k = 1
    pylab.title('sulci : ' + sulci)
    for i in range(3):
        for j in range(3):
            pylab.subplot('33%d' % k)
            if i == j:
                pylab.plot()
            else:
                plot_2d_sulci_vs_neighbours(colors,
                                            f.axes[k - 1],
                                            neighbours_db, j, i)
            k += 1

    f.axes[1].title.set_text('sulci : ' + sulci)
    pylab.show()


def main():
    graphname = 'data/bayesian_sulci_graph.dat'
    prefix = 'data'
    g = read_graph(graphname)
    show_sulci_vs_neighbours(prefix, g, 'S.C._right')


if __name__ == '__main__':
    main()
