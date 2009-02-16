#!/usr/bin/env python
import sigraph
import anatomist.direct.api as anatomist
import anatomist.cpp as cpp
from soma import aims
import datamind.io.old_csvIO as io
import os, sys, exceptions, numpy
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
	print format

def read_csv(csvfilename):
	fd = open(csvfilename)
	lines = fd.readlines()
	labels = lines[0].rstrip('\n').split('\t')
	fd.close()
	header_minf = { 'Y' : [], 'labels' : labels }
	if labels[0] == 'subjects' and labels[1] in ['sulci', 'nodes']:
		header_minf['X'] = range(2, len(labels))
		header_minf['INF'] = [0, 1]
		mode = labels[1]
		labels = labels[2:]
	elif labels[0] in ['sulci', 'nodes']:
		header_minf['X'] = range(1, len(labels))
		header_minf['INF'] = [0]
		mode = labels[0]
		labels = labels[1:]
	else:
		print "bad csv format"
		sys.exit(1)
	db, header = io.ReaderHeaderCsv().read(csvfilename,header_minf)
	X = db.getX()
	sulci = db.getINF()[:, -1]
	uniq_sulci = numpy.unique1d(sulci)
	sulci_data = {}
	for s in uniq_sulci:
		X2 = X[(sulci == s)]
		X2m = X2.mean(axis=0)
		X2s = X2.std(axis=0)
		X2sum = X2.sum(axis=0)
		sulci_data[s] = X2m, X2s, X2sum
	X3 = numpy.vstack([data[0] for data in sulci_data.values()])
	X4 = numpy.vstack([data[2] for data in sulci_data.values()])
	Xm = X3.mean(axis=0)
	Xs = X3.std(axis=0)
	Xsum = X4.sum(axis=0)
	print "global mean over sulci :", Xm
	print "global std over sulci : ", Xs
	print "global sum over sulci :", Xsum
	return labels, sulci_data, mode

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
		'translation', 'sulci_model_noroots.trl')

	description = 'Map csv values onto sulci.'
	parser = OptionParser(description)
	parser.add_option('-g', '--graph', dest='graphname',
		metavar = 'FILE', action='store', default = None,
		help='data graph')
	parser.add_option('--label-attribute', dest='label_attribute',
		metavar = 'STR', type='choice', choices=('name', 'label'),
		action='store', default='name',
		help="'name' or 'label' (default: %default)")
	parser.add_option('-m', '--mesh', dest='meshname',
		metavar = 'FILE', action='store', default = None,
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
		help='translation file (default : %default)')
	parser.add_option('--log', dest='log',
		metavar = 'FILE', action='store_true', default=False,
		help='add log of mean values read in the input csv')
	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if options.format is True:
		print_csv_format()
		sys.exit(1)
	if options.csvfilename is None:
		parser.print_help()
		sys.exit(1)
	# read
	ft = sigraph.FoldLabelsTranslator(options.transfile)
	sigraph.si().setLabelsTranslPath(options.transfile)
	labels, sulci_data, mode = read_csv(options.csvfilename)
	if options.summarycsvfilename:
		write_summary_csv(options.summarycsvfilename,
					labels, sulci_data, mode)

	# graph
	if not options.graphname: return
	from soma import aims
	r = aims.Reader(options = {'subobjectsfilter' : 1})
	g = r.read(options.graphname)
	ft.translate(g)

	# mesh
	if options.meshname:
		m = aims.Reader().read(options.meshname)
	else:	m = None

	for v in g.vertices():
		if v.getSyntax() != 'fold': continue
		if mode == 'sulci':
			try: data = sulci_data[v[options.label_attribute]]
			except exceptions.KeyError: continue
		elif mode == 'nodes':
			try: data = sulci_data[str(int(v['index']))]
			except exceptions.KeyError: continue
		for i, h in enumerate(labels):
			v['csv_mean_' + h] = data[0][i]
			if options.log:
				v['csv_log_mean_' + h] = numpy.log(data[0][i])
			# add only no-null std
			if data[1][i]: v['csv_std_' + h] = data[1][i]
			v['csv_sum_' + h] = data[2][i]

	a = anatomist.Anatomist()
	ag = a.toAObject(g)
	ag.setColorMode(ag.PropertyMap)
	ag.setColorProperty('csv_mean_' + labels[0])
	ag.notifyObservers()
	aobjects = [ag]
	if m:
		am = a.toAObject(m)
		aobjects.append(am)
	win = a.createWindow(wintype='3D')
	win.setHasCursor(0)
	a.addObjects(aobjects, [win])
	c = a.getControlWindow()
	c.SelectWindow(win.internalRep)
	cpp.ObjectActions.displayGraphChildrenMenuCallback().doit(\
					[ag.internalRep])
	c.UnselectAllWindows()

	# qt loop
	qt.qApp.exec_loop()

if __name__ == '__main__' : main()
