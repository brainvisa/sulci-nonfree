#!/usr/bin/env python2

from soma import aims
from sigraph import *
from sigraph.cover import *
from sigraph.grid_io import *
from optparse import OptionParser
from datamind.tools import *


# Options parser
def parseOpts(argv):
	description = 'Generate diagrams based on grid optimization.'
	parser = OptionParser(description)
	add_filter_options(parser)
	parser.add_option('-m', '--model', dest='modelfilename',
		metavar = 'MODEL', action='store', default = 'model.arg',
		help='model file name (default : %default)')
	parser.add_option('--max', dest='max', action='store_true',
		default = False,
		help="best value is the maximum one (default : min)")
	parser.add_option('--svg', dest='svg', action='store_true',
		default = False, help='generate svg diagrams (default : eps)')
	parser.add_option('-a', '--all', dest='all', action='store_true',
		default = False, help='generate all optimization grids')
	parser.add_option('-v', '--verbose', dest='verbose',
		action='store', type = 'int', default = 0,
		help='level of verbosity')
	return parser.parse_args(argv)


class DataSynthesizer(object):
	gr = GridReader()
	gp = GridPlotter()

	def __init__(self, model, options, ext, best, prefix):
		self.mod_number = 0
		self.grid_number = 0
		self.options = options
		self.info = None
		self.ext = ext
		self.best_parameters = {}
		self.best = best
		self.array_mean = None
		self.n = 0
		self.prefix = prefix

	def _synth_average(self, a):
		names, ranges = self.info
		averagename = self.name + '-average_grid.eps'
		self.gp.plot_average(averagename, a, names, ranges)

	def _synth_best(self):
		names, ranges = self.info
		bestname = self.name + '-best_grid.eps'
		self.gp.plot_best(bestname, self.best_parameters, names, ranges)

	def synth(self):
		import numpy
		n = self.grid_number
		if n == 0:
			import sys
			sys.stderr.write(self.name + " : sorry, can't find " + \
				"any grid file\n")
			return
		else :
			print self.name + ' : synthesis based on ' + \
				'%d/%d .grid files over .mod files' % \
				(n, self.mod_number)
		a = self.array_mean
		a /= n
		if numpy.isnan(a).any():
			import sys
			error = self.name + " : sorry, nan array result"
			msg.error(error)
		else:
			self._synth_average(a)
		self._synth_best()

	#FIXME : ce truc n'est pas parfait, parce qu'il suppose qu'il y a
	# forcement une grille d'optim pour un modele (.mod) donne. Alors que
	# celui-ci devrait dependre de l'adaptiveleaf/subadaptive.
	def plot(self, el):
		import sys
		global gr

		self.mod_number += 1
		model_file = el['model_file'].get().getString()
		model_file = model_file[:model_file.rfind('.')]
		gridname = os.path.join(self.prefix, model_file +'_errors_grid')
		try :
			fd = open(gridname, 'r')
		except IOError:
			print "skip : '" + gridname + "' (unknown file)"
			return
		(filename, a, names, ranges) = self.gr.read(gridname)
		if self.info is not None:
			info = self.info
			if names != info[0] or (ranges[0] != info[1][0]).any() \
				or (ranges[1] != info[1][1]).any():
				error = "\t(skip)'" + gridname + \
					"' is incompatible (names or ranges) "+\
					"with other .grid file\n"
				msg.write(error, 'thin_gray')
				return
		else:	self.info = (names, ranges)
		best_values = self.best(a, ranges)
		if self.best_parameters.has_key(best_values):
			self.best_parameters[best_values] += 1
		else :	self.best_parameters[best_values] = 1
		if self.options.all:
			epsname = model_file + '_errors_grid.' + self.ext
			if (self.options.verbose > 0):
				print "write : '" + epsname + "'"
			else :
				self.progression_bar.display(self.n)
				self.n += 1
			self.gp.plot(os.path.join(self.prefix, epsname),
							a, names, ranges)
		if self.array_mean is None:
			self.array_mean = a
		else:
			try:
				import numpy
				if numpy.isnan(a).any():
					error =  gridname + " : sorry, nan grid"
					msg.error(error)
				else:	self.array_mean += a
			except ValueError:
				error = "'%s' is incompatible (array size) " + \
					"with other .grid file\n" % gridname
				msg.write(error, 'thin_gray')
				return
		self.grid_number += 1


class EdgesSynthesizer(DataSynthesizer):
	def __init__(self, model, options, ext, best, prefix):
		DataSynthesizer.__init__(self, model, options, ext, best,prefix)
		self.name = 'edges'
		self.progression_bar = ProgressionBarPct(len(model.edges()),
					msg = self.name + ' :')

class VerticesSynthesizer(DataSynthesizer):
	def __init__(self, model, options, ext, best, prefix):
		DataSynthesizer.__init__(self, model, options, ext, best,prefix)
		self.name = 'vertices'
		self.progression_bar = ProgressionBarPct(len(model.vertices()),
					msg = self.name + ' :')




def vertex_grid(el, ds):
	label = el['label'].get().getString()
	if label != 'unknown': ds.plot(el)


def edge_grid(el, ds):
	ds.plot(el)
	
# main function
def main():
	import sys

	# read options
	options, args = parseOpts(sys.argv)
	if options.svg:
		ext = 'svg'
	else:	ext = 'eps'
	prefix = os.path.dirname(options.modelfilename)

	# read model
	r = aims.Reader()
	model = r.read(options.modelfilename)

	# best function
	if options.max:
		def best(a, ranges) :
			ma = a.max(0).argmax()
			return (ranges[0][ma], ranges[1][a[:, ma].argmax()])
	else:
		def best(a, ranges) :
			mi = a.min(0).argmin()
			return (ranges[0][mi], ranges[1][a[:, mi].argmin()])

	# cover model
	edge_fundict = {'edge_before' : edge_grid}
	edge_ds = EdgesSynthesizer(model, options, ext, best, prefix)
	cover(model, edge_fundict, edge_ds, options.labels_filter,
						options.filter_mode)

	vertex_fundict = {'vertex_before' : vertex_grid}
	vertex_ds = VerticesSynthesizer(model, options, ext, best, prefix)
	cover(model, vertex_fundict, vertex_ds, options.labels_filter,
						options.filter_mode)

	# Synth
	edge_ds.synth()
	vertex_ds.synth()

if __name__ == '__main__' : main()
