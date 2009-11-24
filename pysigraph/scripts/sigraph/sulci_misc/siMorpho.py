#!/usr/bin/env python

from optparse import OptionParser
import os, sys, numpy
import sigraph
from soma import aims
import datamind.io.old_csvIO as datamind_io
from sulci.common import io, add_translation_option_to_parser
from datamind.ml.database import DbNumpy

class SulcusInfo(object):
	def __init__(self, descr, names, subject=None):
		self._vectors = [descr]
		self._names = names
		self._subject = subject

	def append(self, descr):
		self._vectors.append(descr)

	def numpyzation(self):
		if len(self._vectors) > 1:
			self._vectors = numpy.vstack(self._vectors)
		else:	self._vectors = numpy.array(\
				self._vectors[0]).reshape(1, -1)

	def getVectors(self):
		return self._vectors

	def getNames(self):
		return self._names


def compute_descriptors(mf, graphs, sulcus):
	descriptors = {}
	print "compute descriptors..."
	size = len(graphs)
	for i, g in enumerate(graphs):
		sys.stdout.write("%d/%d\r" % (i + 1, size))
		sys.stdout.flush()
		for cl in g.cliques():
			model_type = cl['model_type']
			if sulcus is not None and \
				(model_type != 'random_vertex' or
				cl['label'] != sulcus):
				continue
			if model_type == 'fake_relation': continue
			if model_type == 'random_vertex':
				labels = cl['label']
				if labels == 'unknown': continue
			elif model_type == 'random_edge':
				labels = cl['label1'], cl['label2']
			mod = mf.selectModel(cl)['model']
			pot = mod.printDescription(cl, True)
			descr = cl['pot_vector']
			if descriptors.has_key(labels):
				descriptors[labels].append(descr)
			else:
				names = list(cl['descriptor_names'].get())
				descriptors[labels] = SulcusInfo(descr, names)
	for labels, vectors in descriptors.items():
		descriptors[labels].numpyzation()
	return descriptors

def save_to_csv(dbdir, descriptors):
	print "saving to csv..."
	h = { 'data' : 'sulci_features', 'files' : {}}
	w = datamind_io.WriterCsv()
	size = len(descriptors)
	for i, (labels, info) in enumerate(descriptors.items()):
		sys.stdout.write("%d/%d\r" % (i + 1, size))
		sys.stdout.flush()
		db = DbNumpy(info.getVectors())
		if isinstance(labels, tuple) or isinstance(labels, list):
			csvname, minfname = \
				io.relation2databasename(dbdir, labels)
		else:	csvname, minfname = io.sulci2databasename(dbdir, labels)
		header = { 'X' : info.getNames()}
		w.write(csvname, db, header, minfname)
		h['files'][labels] = minfname
	summary_file = dbdir + '.dat'
	io.write_databases(summary_file, h)


def parseOpts(argv):
	description = 'Create databases for gaussian model.\n' \
		'./siMorpho.py [OPTIONS] graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option( '-m', '--model_graph', action='store',
		dest='modelgraph', metavar = 'FILE', default = None,
		help='model graph file')
	parser.add_option('-d', '--dbdir', dest='dbdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_sulci_databases',
		help='output databases directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='it computes sulci descriptors only for specified sulcus.')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0 or None in [options.modelgraph]:
		parser.print_help()
		sys.exit(1)

	try:	os.mkdir(options.dbdir)
	except OSError, e:
		print "warning: directory '%s' allready exists" % dbdir

	graphs = io.load_graphs(options.transfile, graphnames)

	modelgraph = aims.Reader().read(options.modelgraph)
	modelgraph.removeWeights()
	mf = modelgraph.modelFinder()
	for graph in graphs:
		mf.initCliques(graph, False, False, True, False)
	descriptors = compute_descriptors(mf, graphs, options.sulcus)
	save_to_csv(options.dbdir, descriptors)

if __name__ == '__main__' : main()
