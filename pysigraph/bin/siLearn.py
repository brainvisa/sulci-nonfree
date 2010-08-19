#!/usr/bin/env python

import os, sys, signal
from optparse import OptionParser
import sigraph
from soma import aims
from sigraph.cover import *
from datamind.tools import *

insave = 0

def select_cover(selectivetrainer, fundict, user_data):
	models = selectivetrainer.usedAdap()
	for m in models:
		ao = m.topModel().parentAO()
		if type(ao) == aims.Vertex:
			labels = [ao['label']]
		else :	 labels = [ao['label1'],
				ao['label2']]
		if not filtred(labels, user_data['labels_filter'],
				user_data['filter_mode']):
			m.cover(fundict, user_data)

def saveModel(labels_filter, filter_mode):
	global rg, par, tr, insave
	if insave:
		print '--- Recursive save ---\n(Not done again'
		return
	insave = 1
	if par.nosave == 0:
		print 'Saving model...'
		w = sigraph.FrgWriter(par.model)
		if labels_filter is None:
			if isinstance(tr, sigraph.SelectiveTrainer):
				print '(writing parts only)'
				tr.save(w)
			else:	w.write(rg)
		else:
			fundict = {
				'topadaptive_before' : save_adaptive_callback}
			data = {'labels_filter' : labels_filter,
				'filter_mode' : filter_mode,
				'frgwriter' : w,
                                'graph' : rg }
			if isinstance(tr, sigraph.SelectiveTrainer):
				select_cover(tr, fundict, data)
			else:
                          	cover(rg, fundict, data,
					labels_filter, filter_mode)
	insave = 0

class Param:
	def __init__( self ):
		self.model = ''
		self.trainscheme = ''
		self.graphs = []
		self.testGraphs = []
		self.dimreduction_mode = 'None'
		self.dimreduction_optim = 'None'
		self.predict_train = 0
		self.selected_dim = -1
		self.gaussian_mode = 'normalized'
		self.labelsMap = ''
		self.mode = sigraph.Trainer.GenerateAndTrain
		self.uninitflg = 0
		self.nosave = 0
		self.cycles = 0
		self.cycles_tst = 0
		self.atts = []
		self.pattern = ''
		self.verbose = 1
		self.stopDelay = 2000
		self.maxAppError = 0.25
		self.closeLearning = 0
		self.labelatt = None

par = Param()

def sig_break(sig, stack):
	print 'SIGINT : save ? '
	sys.stdout.flush()
	c = sys.stdin.readline()
	if c == 'o' or c == 'O' or c == 'y' or c == 'Y':
		saveModel(options.labels_filter, options.filter_mode)
	exit(0)

def sig_saveAndCont(sig, stack):
	signal(signal.SIGUSR1, sig_saveAndCont)
	print '--- Saving ... ---'
	saveModel(options.labels_filter, options.filter_mode)
	print '--- Continuing... ---'

def sig_term( sig, stack ):
	print '*** Reveived signal SIGTERM ***'
	print 'Saving before stoping...'
	saveModel(options.labels_filter, options.filter_mode)
	exit(2)

def sig_crash( sig, stack ):
  print '*** CRASH ***'
  sys.stdout.write( 'Received signal ' )
  if sig == signal.SIGSEGV:
    print 'SIGSEGV, segmentation fault'
  elif sig == signal.SIGBUS:
    print 'SIGBUS, bus error'
  elif sig == signal.SIGILL:
    print 'SIGILL, illegal instruction'
  elif sig == signal.SIGFPE:
    print 'SIGFPE, floating point exception'
  else:	print sig
  print 'python stack:'
  print stack
  saveModel(options.labels_filter, options.filter_mode)
  exit( 1 )

def initCliques(rg, par, learn, test):
  print 'Init cliques...'
  mf = rg.modelFinder()
  if par.labelsMap != '':
    tr = sigraph.FoldLabelsTranslator(rg, par.labelsMap)
  else:
    tr = sigraph.FoldLabelsTranslator(rg, '')
  for x in learn:
    if par.labelsMap != '':
      tr.translate( x, 'name', 'label' )
    mf.initCliques(x, par.verbose, True)
  for x in test:
    if par.labelsMap != '':
      tr.translate( x, 'name', 'label' )
    mf.initCliques(x, par.verbose, True)
 
def setSignalHandlers(): 
	signal.signal(signal.SIGINT, sig_break)
	signal.signal(signal.SIGSEGV, sig_crash)
	signal.signal(signal.SIGILL, sig_crash)
	signal.signal(signal.SIGBUS, sig_crash)
	signal.signal(signal.SIGFPE, sig_crash)
	signal.signal(signal.SIGUSR1, sig_saveAndCont)
	signal.signal(signal.SIGTERM, sig_term)

def parseOpts(argv):
	usage = '%prog [Options] [silearn.cfg]'
	parser = OptionParser(usage = usage)
	add_filter_options(parser)
	parser.add_option('--label_attribute', dest='labelatt',
		metavar='ATTR', action='store',	default=None,
		help='''label attribute used to get labels from: usually
		"label" or "name", "auto" means try first label, and if no
		label is present, take name [default:auto]''')
	parser.add_option('--debug', dest='debug', action='store_true',
		default=None, help='''enable debug [default: disable]''')

	return parser, parser.parse_args(argv)

def checkfile(x):
	try:
		os.stat(x)
	except OSError, err:
		import sys
		msg.error(str(err))
		sys.exit(1)

def graph_select_names_or_labels(g):
	global par
	if par.labelatt == 'auto': return
	if par.labelatt == 'name':
		todel = 'label'
	else:	todel = 'name'
	#FIXME : right now, we use brutality (until a better system is
	#        done in LabelsTranslator).
	for v in g.vertices():
		if v.has_key(todel):
			del v[todel]

 
# main function 
def main():
  global rg, par, tr, insave

  from soma import aims

  # options
  parser, (options, args) = parseOpts(sys.argv)
  if len(args) != 2:
    parser.print_help()
    exit(1)
  if options.debug:
    import sigraph
    sigraph.Settings.debug = True
  import sigraph.learning
  conffile = args[1]
  
  # read parameters file
  sr = aims.carto.SyntaxReader( os.path.join( sigraph.si().basePath(),
                                              'config/siLearn.stx' ) )
  synx = sr.read()
  # make graphFiles parameter optional
  synx.semantic( 'siLearn', 'graphFiles' ).needed = 0
  trr = aims.TreeReader( conffile, synx )
  try:
    conf = trr.read()
  except IOError, e:
	msg.error(str(e))
	sys.exit(1)
 
  ca = conf.keys()
  par.model = conf[ 'modelFile' ]
  par.trainscheme = conf[ 'trainschemeFile' ]
  if 'graphFiles' in ca:
    par.graphs = conf[ 'graphFiles' ].split()
  if 'testGraphFiles' in ca:
    par.testGraphs = conf[ 'testGraphFiles' ].split()
  if 'dimreduction_mode' in ca:
    par.dimreduction_mode = conf['dimreduction_mode']
  if 'dimreduction_optim' in ca:
    par.dimreduction_optim = conf['dimreduction_optim']
  if 'predict_train' in ca:
    par.predict_train = conf['predict_train']
  if 'gaussian_mode' in ca:
    par.gaussian_mode = conf['gaussian_mode']
  if 'labelsMapFile' in ca:
    par.labelsMap = conf[ 'labelsMapFile' ]
  if 'uninitflg' in ca:
    par.uninitflg = conf[ 'uninitflg' ]
  if 'nosave' in ca:
    par.nosave = conf[ 'nosave' ]
  if 'cycles' in ca:
    par.cycles = conf[ 'cycles' ]
  if 'cycles_tst' in ca:
    par.cycles_tst = conf[ 'cycles_tst' ]
  par.mode = sigraph.Trainer.GenerateAndTrain
  if 'mode' in ca:
    mode = conf[ 'mode' ]
    if mode == 'generateOnly':
      par.mode = sigraph.Trainer.GenerateOnly
    elif mode == 'generateAndTrain':
      par.mode = sigraph.Trainer.GenerateAndTrain
    elif mode == 'readAndTrain':
      par.mode = sigraph.Trainer.ReadAndTrain
    elif mode == 'trainDomain':
      par.mode = sigraph.Trainer.TrainDomain
    elif mode == 'trainStats':
      par.mode = sigraph.Trainer.TrainStats
    else:
      raise ValueError( mode + ' is not a valid mode' )
  if 'filter_attributes' in ca:
    par.atts = conf[ 'filter_attributes' ].split()
  if 'filter_pattern' in ca:
    par.pattern = conf[ 'filter_pattern' ]
  if 'verbose' in ca:
    par.verbose = conf[ 'verbose' ]
  if 'stop_delay' in ca:
    par.stopDelay = conf[ 'stop_delay' ]
  if 'max_app_error' in ca:
    par.maxAppError = conf[ 'max_app_error' ]
  if 'close_learning' in ca:
    par.closeLearning = conf[ 'close_learning' ]
  if 'label_attribute' in ca :
	par.labelatt = conf['label_attribute']
  if not par.labelatt or options.labelatt:
    if not options.labelatt:
      options.labelatt = 'auto'
    par.labelatt = options.labelatt

 
  
  print 'learn graphs:', par.graphs
  print 'test graphs:', par.testGraphs
  if par.mode != sigraph.Trainer.ReadAndTrain and len( par.graphs ) == 0:
    print 'No learning graphs - graphFiles parameter must be present in '
    'this mode'
    exit(1)
  
  # read model
  
  r = aims.Reader()
  
  print 'model:', par.model
  rg = r.read( par.model )
 
  print 'learner:', par.trainscheme
  if not (mode in ['readAndTrain', 'trainDomain']):
    flr = sigraph.FoldLearnReader( par.trainscheme )
    learner = flr.read()
  else:
    learner = None
  
  # read learning base
  learn = []
  print 'Reading learn base :'
  for x in par.graphs:
    checkfile(x)
    fg = r.read(x, 0, 0)
    learn.append(fg)
    vc = rg.checkCompatibility(fg)
    graph_select_names_or_labels(fg)
    if not vc.ok:
      print 'Warning: model / data graphs version mismatch'
      print vc.message
      print
      print 'I will continue but wrong or inaccurate results can be achieved'
  
  # read test base
  test = []
  print 'Reading test base :'
  for x in par.testGraphs:
    checkfile(x)
    fg = r.read(x)
    test.append(fg)
    vc = rg.checkCompatibility(fg)
    graph_select_names_or_labels(fg)
    if not vc.ok:
      print 'Warning: model / data graphs version mismatch'
      print vc.message
      print
      print 'I will continue but wrong or inaccurate results can be achieved'

 
  # labels translation
  if par.labelsMap != '': sigraph.si().setLabelsTranslPath(par.labelsMap)
  
  # cliques generation
  initCliques(rg, par, learn, test)
 
  if par.uninitflg == 0:
    rg.initAdap();

  if mode == 'trainStats':
    rg.initStats();
  
  # signal handlers
  setSignalHandlers()
 
  sc = sigraph.LearnStopCriterion.theCriterion
  sc.stopDelay = par.stopDelay
  sc.MaxAppError = par.maxAppError
  
  # SelectiveTrainer
  if len( par.atts ) != 0 and par.pattern != '':
    print 'SelectiveTrainer'
    tr = sigraph.SelectiveTrainer( rg, learner, par.pattern )
    tr.setFiltAttributes( par.atts )
  else:
    tr = sigraph.Trainer( rg, learner )
  tr.init(par.mode )
  tit = tr.trainIterator( learn, test, par.cycles, par.cycles_tst )
  while tit.isValid():
    ad = tit.adaptive()
    if not ad :
      tit.next()
      continue
    labels = [l for l in ad.significantLabels() if l != 'unknown']
    if not filtred(labels, options.labels_filter, options.filter_mode):
      tit.train(aims.Object(par))
    tit.next()
  
  # close learning models
  if par.closeLearning:
    rg.closeLearning()
  saveModel(options.labels_filter, options.filter_mode)
  os._exit(0)

def main_safe():
	try:	main()
	except KeyboardInterrupt:
		sig_break()
  	exit(0)

def exit(exit_status):
	import datamind.ml
	datamind.ml.exit(exit_status)

if __name__ == '__main__' : main_safe()

