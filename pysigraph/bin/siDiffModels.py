#!/usr/bin/env python

import os, numpy, sigraph, sigraph.cover, sigraph.models
from soma import aims
from optparse import OptionParser
from datamind.tools import *

# Options parser
def parseOpts(argv):
	description = 'Compare two sigraph models.'
	parser = OptionParser(description)
	sigraph.cover.add_filter_options(parser)
	parser.add_option('-1', '--model1', dest='model1_filename',
		metavar = 'MODEL', action='store', default = 'model.arg',
		help='model file name (default : %default)')
	parser.add_option('-2', '--model2', dest='model2_filename',
		metavar = 'MODEL', action='store', default = None,
		help='model file name (default : %default)')
	parser.add_option('-g', '--graph', dest='graph',
		metavar = 'FILE', action='store', default = None,
		help='data graph file name')
	parser.add_option('--hist', dest='hist',
		action='store', default = None,
		help='make histogram from classifiation/regression rates' \
			'of models. HIST should be a format supported by pylab')
	parser.add_option('-p', '--pct', dest='pct',
		action='store', default = '10',
		help='percentage of the biggest differences between models ' \
			'ex : 5.6%, 5.6, 5 (default : %default)')
	parser.add_option('-c', '--cmp', dest='cmp', action='store',
		default='raw,mean,good,bad',
		help='compared data : a list of compared elements taken in ' \
			'raw, mean, good, bad (default : %default)')
	parser.add_option('-m', '--mode', dest='mode', action='store',
		default='biggest',
		help='return biggest or smallest values (default : %default)')
	parser.add_option('-a', '--abs', dest='abs', action='store_true',
		default=False,
		help='compare absolute differences (default : disabled)')
	parser.add_option('--anatomist', dest='anatomist', action='store_true',
		default=False,
		help='Display first compared value on a graph ' \
			'under anatomist software (default : disabled)')
	return parser.parse_args(argv)

def anatomist_plot(models, md, options, cmp_mode):
	import anatomist
	from soma import aims

	r = aims.Reader()
	a = anatomist.Anatomist()
	p = a.theProcessor()
	if not options.graph:
		graphfile = os.path.join('/', 'neurospin', 'lnao',
			'Panabase', 'data', 'diffusion', 'chaos',
			'graphe', 'RchaosBase.arg')
	else:	graphfile = options.graph
	g = r.read(graphfile)
	ag = anatomist.AObjectConverter.anatomist(g)
	
	# window
	cmd1 = anatomist.CreateWindowCommand('3D')
	p.execute(cmd1)
	win = cmd1.createdWindow()
	cmd2 = anatomist.AddObjectCommand([ag], [win])
	p.execute(cmd2)
	
	# load nomenclature
	nomenclature_filename = os.path.join(aims.carto.Paths.shfjShared(),
			'nomenclature', 'hierarchy',
			'sulcal_root_colors.hie')
	nomenclatureid = 0
	cmd3 = anatomist.LoadObjectCommand(nomenclature_filename,
						nomenclatureid)
	p.execute(cmd3)
	nomenclature = cmd3.loadedObject()
	
	# selection
	p.execute('SelectByNomenclature', {'nomenclature': nomenclatureid,
		'names' : 'brain', 'group' : 0})
	p.execute('SelectByNomenclature', {'nomenclature': nomenclatureid,
		'names' : 'brain', 'group' : 0, 'modifiers' : 'toggle'})

	am1 = anatomist.AObjectConverter.anatomist(models[0])
	am2 = anatomist.AObjectConverter.anatomist(models[1])
	cmd4 = anatomist.FusionObjectsCommand([ag, am1, am2],
				'modelsFusionMethod')
	p.execute(cmd4)

	ag.setColorMode(ag.PropertyMap)
	ag.setColorProperty('diff')
	ag.notifyObservers()
	import sys
        if sys.modules.has_key( 'PyQt4' ):
          import PyQt4.QtCore as qt
          qt.qApp.exec_()
        else:
          import qt
          qt.qApp.exec_loop()

	


	
# main function
def main():
	import sys

	# read options
	options, args = parseOpts(sys.argv)
	cmp_modes = options.cmp.split(',')
	if options.mode not in ['biggest', 'smallest']:
		msg.error("'%s' : invalid mode" % self._mode)
		sys.exit(1)

	# pct
	ind = options.pct.rfind('%')
	if ind == -1 :
		pct = float(options.pct)
	else:	pct = float(options.pct[:ind])

	# read models
	r = aims.Reader()
	model1 = r.read(options.model1_filename)
	if options.model2_filename:
		model2 = r.read(options.model2_filename)
	else:	model2 = None

	md = sigraph.models.ModelDifferentiator([model1, model2], options.abs)
	md.diff(options.labels_filter, options.filter_mode)
	md.print_diff(options.mode, pct, cmp_modes)
	if options.hist: md.hist(options.hist)
	if options.anatomist: anatomist_plot([model1, model2], md,
					options, cmp_modes[0])

if __name__ == '__main__' : main()
