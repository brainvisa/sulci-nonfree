#!/usr/bin/env python2
# -*- coding: utf-8 -*-
from __future__ import print_function
import sigraph
import anatomist.direct.api as anatomist
from soma import aims
import datamind.io.old_csvIO as io
import os, sys, exceptions, numpy
if sys.modules.has_key( 'PyQt4' ):
  USE_QT4=True
  import PyQt4.QtGui as qt
else:
  USE_QT4=False
  import qt
from optparse import OptionParser


def print_csv_format():
  format = '''
  4 formats are available for mapping data onto fold graphs.
  Each sulcus or node index can appear several times, resulting mapped
  information is the mean or the std of these sulcus' values.

  - The first 2 for sulci mapping.
  $ cat format1.csv
      sulci            ind1   ind2
      S.C._right       5.1      2.1
      F.C.L.p._right   9.2      3.2
      ...

  $ cat format2.csv
      subjects     sulci            ind1   ind2
      ammon        S.C._right       1.9      0.2
      zeus         F.C.L.p._right   9.2      3.1
      ...

  - The last ones for node mapping (sub-element composing sulci), based
  on node's indices :
  $ cat format3.csv
      nodes       ind1 ind2
      0           0.1  2.4
      1           1.5  6.5

  $ cat format4.csv
      sujbects    nodes   ind1 ind2
      ammon       0       0.1  2.4
      zeus        1       1.5  6.5
  '''
  print(format)

def read_csv(csvfilename, columns=[], operator='mean', filterCol=None,
  filterExpr=None):
  fd = open(csvfilename)
  line = fd.readline()
  delims = [ ',\t', ', ', '\t', ' ' ]
  for delim in delims:
    labels = line.rstrip('\n').rstrip('\r').strip().split( delim )
    if len( labels ) != 1:
      break
  fd.close()
  header_minf = { 'Y' : [], 'labels' : labels }
  # print('labels:', labels)
  labels2 = [ x.lower() for x in labels ]
  subjectcol = None
  labelcol = None
  sidecol = None
  infsidecol = None
  for sl in ( 'subjects', 'subject' ):
    if sl in labels2:
      subjectcol = labels2.index( sl )
      break
  for sl in ( 'sulci', 'nodes', 'label', 'name' ):
    if sl in labels2:
      labelcol = labels2.index( sl )
  if 'side' in labels2:
    sidecol = labels2.index( 'side' )
  if subjectcol is not None and labelcol is not None:
    header_minf['X'] = range(len(labels))
    header_minf['X'].remove(labelcol)
    header_minf['X'].remove(subjectcol)
    header_minf['INF'] = [subjectcol, labelcol]
    mode = labels2[labelcol]
    if sidecol:
      infsidecol = 1
      header_minf['INF'].insert( 1, sidecol )
      header_minf['X'].remove( sidecol )
  elif subjectcol is not None:
    # header_minf['X'] = range(labelcol+1, len(labels))
    header_minf['X'] = range(len(labels))
    header_minf['X'].remove(subjectcol)
    header_minf['INF'] = [subjectcol]
    mode = labels2[subjectcol]
    olabels = labels[subjectcol+1:]
    if sidecol:
      header_minf['INF'].insert( 0, sidecol )
      header_minf['X'].remove( sidecol )
      olabels.remove( labels[sidecol] )
      infsidecol = 0
  elif labelcol is not None:
    header_minf['X'] = range(len(labels))
    header_minf['X'].remove(labelcol)
    header_minf['INF'] = [labelcol]
    mode = labels2[labelcol]
    olabels = labels[labelcol+1:]
    if sidecol:
      header_minf['INF'].insert( 0, sidecol )
      header_minf['X'].remove( sidecol )
      olabels.remove( labels[sidecol] )
      infsidecol = 0
  else:
    print("bad csv format")
    sys.exit(1)
  if len( columns ) != 0:
    header_minf['X'] = columns
    header_minf['INF'] = \
      [ i for i in xrange( len( labels ) ) if i not in columns ]
    if labelcol is not None:
      # label column at end
      header_minf['INF'].remove( labelcol )
      header_minf['INF'].append( labelcol )
    olabels = [ labels[ i ] for i in columns ]
    if sidecol:
      header_minf['INF'].insert( 0, sidecol )
      infsidecol = 0
  olabels = [ labels[x] for x in header_minf['X'] ]
  db, header = io.ReaderHeaderCsv().read(csvfilename,header_minf,
    sep=delim)
  X = db.getX()
  inf = db.getINF()
  if filterCol is not None and filterExpr is not None:
    fcol = labels.index( filterCol )
    fcol = header_minf['INF'].index( fcol )
    filt = eval('numpy.where(inf[:,fcol]' + filterExpr + ')' )
    X = X[ filt ]
    inf = inf[ filt ]
  sulci = inf[:, -1]
  if sidecol:
    for i, s in enumerate( sulci ):
      side = inf[i, infsidecol]
      if side and side != 'both':
        s += '_' + side
        sulci[i] = s
  if hasattr( numpy, 'unique1d' ):
    uniq_sulci = numpy.unique1d(sulci)
  else:
    uniq_sulci = numpy.unique(sulci)
  sulci_data = {}
  for s in uniq_sulci:
    X2 = X[(sulci == s)]
    X2m = getattr(X2, operator)(axis=0)
    X2s = X2.std(axis=0)
    X2sum = X2.sum(axis=0)
    if s.startswith( "'" ) and s.endswith( "'" ):
      s = s[1:-1]
    sulci_data[s] = X2m, X2s, X2sum
  X3 = numpy.vstack([data[0] for data in sulci_data.values()])
  X4 = numpy.vstack([data[2] for data in sulci_data.values()])
  Xm = X3.mean(axis=0)
  Xs = X3.std(axis=0)
  Xsum = X4.sum(axis=0)
  print("global mean over sulci :", Xm)
  print( "global std over sulci : ", Xs)
  print("global sum over sulci :", Xsum)
  print(olabels)
  return olabels, sulci_data, mode

def write_summary_csv(csvfilename, labels, sulci_data, mode):
  from datamind.ml.database import DbNumpy

  w = io.WriterCsv()
  X, Y, INF = [], None, []
  for s, data in sulci_data.items():
    INF.append(s)
    X.append(data[0])
  INF = numpy.array(INF, dtype='S32')[None].T
  X = numpy.vstack(X)
  db = DbNumpy(X, Y, INF)
  header = { 'X' : labels, 'Y' : [], 'INF' : ['sulci'] }
  w.write(csvfilename, db, header)

def parseOpts(argv):
  transfile = os.path.join(aims.carto.Paths.shfjShared(), 'nomenclature',
    'translation', 'sulci_model_2008.trl')

  description = 'Map csv values onto sulci.'
  parser = OptionParser(description)
  parser.add_option('-g', '--graph', dest='graphname',
    metavar = 'FILE', action='append', default = [],
    help='data graph')
  parser.add_option('--label-attribute', dest='label_attribute',
    metavar = 'STR', type='choice', choices=('name', 'label'),
    action='store', default='name',
    help="'name' or 'label' (default: %default)")
  parser.add_option('-m', '--mesh', dest='meshname',
    metavar = 'FILE', action='append', default = [],
    help='grey/white mesh in the same space of the input graph')
  parser.add_option('--csv', dest='csvfilename',
    metavar = 'FILE', action='store', default = None,
    help='csv file')
  parser.add_option('--summary-csv', dest='summarycsvfilename',
    metavar = 'FILE', action='store', default = None,
    help='summary csv file (one line per sulci)')
  parser.add_option('-f', '--format', dest='format',
    action='store_true', default = False,
    help='print csv format')
  parser.add_option('-t', '--translation', dest='transfile',
    metavar = 'FILE', action='store', default = transfile,
    help='translation file (.trl), or nomenclature file (.hie), ' \
    'or selection file (.sel) (default : %default)')
  parser.add_option('--log', dest='log',
    metavar = 'FILE', action='store_true', default=False,
    help='add log (neperian) of mean values read in the input csv')
  parser.add_option('--log10', dest='log10',
    metavar = 'FILE', action='store_true', default=False,
    help='add log10 of mean values read in the input csv')
  parser.add_option('-c', '--column', dest='columns', action='append',
    default=[], type='int',
    help='column number to be used in the csv file')
  parser.add_option('-o', '--operator', dest='operator', default='mean',
    type='string', action='store',
    help='operator to apply to summarize multiple values on the ' \
    'same sulcus. The default is "mean", but could be "min" or '
    '"max"')
  return parser, parser.parse_args(argv)

def getdata( sulci_data, label ):
  try:
    data = sulci_data[ label ]
    return data
  except:
    if label.endswith( '_left' ):
      label = label[:-5]
    elif label.endswith( '_right' ):
      label = label[:-6]
    else:
      raise
    data = sulci_data[ label ]
    return data

def csvMapGraph( options, agraphs=None, window=None, displayProp=None,
  palette=None, propPrefix=None, csvfilename=None, filterCol=None,
  filterExpr=None ):
  '''csvMapGraph( options, agraphs=None, window=None, displayProp=None,
  palette=None, propPrefix=None, csvfilename=None )

  read a CSV-like file, extract numeric figures on sulci from it and map it
  on a sulci graph displayed in anatomist.

  Paramters:

  options: command-line parsing options of siCsvMapGraph (see the help of this
    command)
  agraphs: existing anatomist graph(s) on which to map data. If not specified, a
    graph is read from the options.graphname parameter.
  window: existing anatomist window in which the sulci graph will be displayed.
    If not specified, a new window will be opened.
  displayProp: when several data are read at the same time, specifies which
    one will be currently displayed. If not specified, the first data column
    will be used.
  palette: color palette to apply to the graph display. May be either an
    anatomist palette object, or a parameters dictionary, corresponding to the
    parameters of the SetObjectPalette command (see
    http://brainvisa.info/doc/anatomist-4.0/html/fr/programmation/commands.html#SetObjectPalette)
  propPrefix: data read from the CSV file are stored in the graph nodes as
    properties. The prefix is used in the properties names. If not specified,
    'csv' is used
  csvfilename: if specified, overrides options.csvfilename for convenience

  Return values: aobjects, awindows
  '''
  if csvfilename is None:
    csvfilename = options.csvfilename
  if csvfilename is None:
    parseOpts( [ sys.argv[0], '-h' ] )
    sys.exit(1)
  # read
  ft = sigraph.FoldLabelsTranslator(options.transfile)
  sigraph.si().setLabelsTranslPath(options.transfile)
  labels, sulci_data, mode = read_csv(csvfilename,
    options.columns, operator=options.operator, filterCol=filterCol,
    filterExpr=filterExpr)
  if options.summarycsvfilename:
    write_summary_csv(options.summarycsvfilename,
      labels, sulci_data, mode)

  # graph
  if not options.graphname: return
  from soma import aims
  graphs = []
  if agraphs:
    if isinstance( agraphs, anatomist.Anatomist.AGraph ):
      agraphs = [ agraphs ]
    graphs = [ x.graph() for x in agraphs ]
  else:
    r = aims.Reader(options = {'subobjectsfilter' : 1})
    graphs = [ r.read( f ) for f in options.graphname ]
    for g in graphs:
      ft.translate(g, options.label_attribute, options.label_attribute)

  # mesh
  for mfile in options.meshname:
    m = aims.read( mfile )
  else:	m = None
  if propPrefix is None:
    propPrefix='csv'

  for g in graphs:
    for v in g.vertices():
      if v.getSyntax() != 'fold': continue
      if mode in ( 'sulci', 'label', 'name' ):
        try:
          l = v[options.label_attribute]
          data = getdata( sulci_data,
            v[options.label_attribute] )
        except exceptions.KeyError:
          continue
      elif mode == 'nodes':
        try: data = getdata( sulci_data, str(int(v['index'])) )
        except exceptions.KeyError: continue
      for i, h in enumerate(labels):
        v[propPrefix + '_mean_' + h] = data[0][i]
        if options.log and data[0][i] != 0:
          v[ propPrefix + '_log_mean_' + h] = numpy.log(data[0][i])
        elif options.log10 and data[0][i] != 0:
          v[ propPrefix + '_log_mean_' + h] = numpy.log10(data[0][i])
        # add only no-null std
        if data[1][i]: v[ propPrefix + '_std_' + h] = data[1][i]
        v[ propPrefix + '_sum_' + h] = data[2][i]

  a = anatomist.Anatomist()
  aobjects = []
  if not agraphs:
    agraphs = [ a.toAObject( g ) for g in graphs ]
    aobjects = agraphs
  if displayProp is None:
    displayProp = propPrefix + '_mean_' + labels[0]
  for ag in agraphs:
    ag.setColorMode(ag.PropertyMap)
    ag.setColorProperty(displayProp)
    if palette is not None:
      if isinstance( palette, a.APalette ):
        ag.setPalette( palette )
      else: # dict
        a.execute( 'SetObjectPalette', objects=[ag], **palette )
    ag.notifyObservers()
    if m:
      am = a.toAObject(m)
      aobjects.append(am)
  if window is not None:
    win = window
    wins = []
  else:
    win = a.createWindow(wintype='3D')
    wins = [ win ]
  win.setHasCursor(0)
  win.addObjects(aobjects, add_graph_nodes=True)
  return aobjects, wins


def main():
  parser, (options, args) = parseOpts(sys.argv)
  if options.format is True:
    print_csv_format()
    sys.exit(1)

  start_qt_loop = False
  if USE_QT4:
    if not qt.QApplication.instance():
      app = qt.QApplication( sys.argv )
      start_qt_loop = True
  else:
    if qt.QApplication.startingUp():
      app = qt.QApplication( sys.argv )
      start_qt_loop = True

  aobjects, wins = csvMapGraph( options )

  # qt loop
  if start_qt_loop:
    if USE_QT4:
      qt.qApp.exec_()
    else:
      qt.qApp.exec_loop()

if __name__ == '__main__' : main()

