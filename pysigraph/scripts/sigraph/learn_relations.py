#!/usr/bin/env python
import os, sys, numpy, pprint, re, glob
from optparse import OptionParser
import matplotlib
matplotlib.use('QtAgg')
import pylab
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.features.descriptors import descriptorFactory
from soma.gui.api import chooseMatplotlibBackend
chooseMatplotlibBackend()

################################################################################
def save_fig(relation, distr, X, filename):
	n = len(X) / 5
	if n < 5: n = 5
	if n > 100: n = 100
	xmin = X.min()
	xmax = X.max()
	if xmin == xmax: return
	f = pylab.gcf()
	if (X != 0).sum() != 0:
		pylab.hist(X, bins=n, normed=1)
	pylab.title("relation : " + str(relation))
	x = numpy.linspace(xmin, xmax, 1000)
	y = numpy.asarray(distr.likelihoods(x)[1]).ravel()
	pylab.plot(x, y, 'r-')
	pylab.savefig(filename, dpi=150)
	f.clear()


################################################################################
def compute_relations(graphs, distribdir, selected_sulci, options):
	min_db_size = options.size_threshold
	sulci_set = {}
	descriptor = descriptorFactory(options.data_type)
	data = descriptor.data_from_graphs(graphs, selected_sulci)

	# create output directory
	prefix = distribdir
	try:	os.mkdir(prefix)
	except OSError, e:
		print e
		sys.exit(1)

	if options.model_type == 'direction':
		mtype_inter = 'kent'
		mtype_intra = 'bingham'
		Distrib_inter = distribution.distributionFactory(mtype_inter)
		Distrib_intra = distribution.distributionFactory(mtype_intra)
	else:
		Distrib = distribution.distributionFactory(options.model_type)
		Distrib_inter = Distrib_intra = Distrib
		mtype_inter = mtype_intra = options.model_type

	# learn and write spams
	h = {'level' : 'relations', 'data_type' : options.data_type,
		'files' : {}}
	if options.savefig:
		s = [len(X) for relation, X in data.items()]
		print "number of relations= ", len(data)
		if len(data) and (numpy.min(s) != numpy.max(s)):
			f = pylab.gcf()
			pylab.hist(s, bins=150)
			filename = os.path.join(prefix, 'size_hist.png')
			pylab.savefig(filename, dpi=150)
			f.clear()
	Xd_intra = []
	Xd_inter = []
	for relation, X in data.items():
		sulcus1, sulcus2 = relation
		dim = X.shape[1]
		if len(X) < min_db_size:
			if sulcus1 == sulcus2:
				Xd_intra.append(X)
			else:	Xd_inter.append(X)
	if Xd_intra: data['default_intra'] = numpy.vstack(Xd_intra)
	else:	data['default_intra'] = None
	if Xd_inter: data['default_inter'] = numpy.vstack(Xd_inter)
	else:	data['default_inter'] = None

	for relation, X in data.items():
		if isinstance(relation, tuple):
			sulcus1, sulcus2 = relation
			if selected_sulci != None and \
				(sulcus1 not in selected_sulci) \
				and (sulcus2 not in selected_sulci): continue
		if X is not None and (len(X) < min_db_size): continue
		if relation == 'default_intra' or (sulcus1 == sulcus2):
			d = Distrib_intra()
			mtype = mtype_intra
		elif relation == 'default_inter' or (sulcus1 != sulcus2):
			d = Distrib_inter()
			mtype = mtype_inter
		if X is None:
			d.setUniform(dim)
		else:	d.fit(X)
		filename = io.node2densityname(prefix, mtype, relation)
		if options.savefig: save_fig(relation, d, X, filename + '.png')
		d.write(filename)
		relfilename = re.sub('^%s%s' % (os.path.dirname(prefix), \
						os.path.sep), '', filename)
		h['files'][relation] = (mtype, relfilename)

	# write distribution summary file
	summary_file = distribdir + '.dat'
	io.write_pp('distributions', summary_file, h)


################################################################################
def parseOpts(argv):
	description = 'Compute relations models from a list of graphs.\n' \
	'learn_relations.py [OPTIONS] graph1.arg graph2.arg...\n'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar='FILE', action='store',
		default='bayesian_relation_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar='LIST', action='store', default=None,
		help='compute models only for specified sulci. ex: ' + \
		'"S.C._right,S.C._right,S.Pe.C._right,S.F.inf._right" : ' + \
		'only pairs of sulci with its 2 labels in this list is ' + \
		'considered')
	parser.add_option('--data-type', dest='data_type',
		metavar='TYPE', action='store', default='min_distance',
		type='choice', choices=('min_distance', 'connexion_length',
		'all_directions_pair', 'all_distances_pair',
		'all_connected_distance', 'all_connected_mean_distance',
		'all_min_distance', 'gravity_centers_directions'),
		help="data type : kind of relation to be learned. " + \
		"'min_distance' : distance between the 2 " + \
		"nearest extremities (near from the minimal distance), " + \
		"'connexion_length' : 0 if disconnected segments, from " + \
		"0 to 1 for connected components (default : %default)")
	parser.add_option('--model-type', dest='model_type',
		metavar='TYPE', action='store', default='gamma', type='choice',
		choices=('gamma', 'beta', 'gamma_exponential_mixture',
		'direction'),
		help="distributions : gamma, beta, gamma_exponential_mixture,"+\
		" directions")
	parser.add_option('--mode', dest='mode', metavar = 'FILE',
		action='store', default='normal', type='choice',
		choices=('normal', 'loo'),
		help="'normal' : compute spams on given graphs, 'loo' : " + \
		"leave one out on graphs : create several models " + \
		"(default : %default), all given reference FILE options " + \
		"must be located in './all/' relative directory and similar " +\
		"data can be found in './cv_*/' directories relative to " + \
		"leave one out graphs folds.")
	parser.add_option('--save-fig', dest='savefig',
		action='store_true', default=False,
		help="save histograms of learned values")
	parser.add_option('--size-threshold', dest='size_threshold',
		metavar='INT', action='store', default=100, type='int',
		help="under this threshold the relations models are learned " +\
		" grouped in 2 big classes (inter-labels) and (intra-labels)")

	return parser, parser.parse_args(argv)


def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0:
		print "error : at least one graph is needed"
		parser.print_help()
		sys.exit(1)

	# read data
	graphs = io.load_graphs(options.transfile, graphnames)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')

	# computations
	if options.mode == 'normal' :
		compute_relations(graphs, options.distribdir,
					selected_sulci, options)
	elif options.mode == 'loo' :
		print "-- all --"
		distribdir = os.path.join('all', options.distribdir)
		if options.sigma_file is None:
			sigma_file = None
		else:	sigma_file = os.path.join('all', options.sigma_file)
		compute_relations(graphs, distribdir, selected_sulci, options)
		for i in range(len(graphs)):
			subgraphs = graphs[:i] + graphs[i+1:]
			directory = 'cv_%d' % i
			print '-- %s --' % directory
			distribdir = os.path.join(directory, options.distribdir)
			if options.sigma_file is None:
				sigma_file = None
			else:	sigma_file = os.path.join(directory,
					options.sigma_file)
			compute_relations(subgraphs, distribdir,
					selected_sulci, options)
	else:
		print "error : '%s' unknown mode" % options.mode



if __name__ == '__main__' : main()
