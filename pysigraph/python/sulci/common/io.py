#!/usr/bin/env python2

from __future__ import print_function
import os, sys, pprint
from sulci.models import distribution, distribution_all, \
			classifier, classifier_all, sulci
import sigraph
from soma import aims
try:
	import datamind.io.old_csvIO as datamind_io
except ImportError as e:
	print("datamind disable: ", e)


################################################################################
# load data graph
def load_graphs(transfile, graphnames, label_mode='name'):
	'''
    transfile :  translation file
    graphnames : list of graph names
    label_mode : name (default), label or both
	'''
	# translation of labels
	ft = sigraph.FoldLabelsTranslator(transfile)
	sigraph.si().setLabelsTranslPath(transfile)

	reader = aims.Reader()
	graphs = []
	for graphname in graphnames:
		g = reader.read(graphname)
		graphs += [g]
		if label_mode == 'both':
			ft.translate(g, 'label', 'label')
			ft.translate(g, 'name', 'name')
		else:
			ft.translate(g, label_mode, label_mode)

	return graphs

def load_graph(transfile, graphname, label_mode='name'):
	return load_graphs(transfile, [graphname], label_mode)[0]

################################################################################
# read posterior segmentwise probabilities

def read_segments_weights(input_segments_weights):
	from datamind.io import ReaderCsv
	r = ReaderCsv()
	segments_weights = []
	for file in input_segments_weights:
		if file == 'None':
			X = None
		else:	X = r.read(file)
		segments_weights.append(X)
	return segments_weights

################################################################################
# generic readers
def read_database_sulci(prefix, sulci):
	minfname = os.path.join(prefix, 'bayesian_' + sulci + '.minf')
	try:
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		return db, header
	except IOError:
		return None, None


def read_from_exec(filename, var):
	try:
		execfile(filename)
		o = locals()[var]
		return o
	except Exception as e:
		print(e)
		sys.exit(1)

def numpy_read_from_exec(filename, var):
	import numpy
	for v in dir(numpy):
		locals()[v] = numpy.__getattribute__(v)
	try:
		execfile(filename)
		o = locals()[var]
		return o
	except Exception as e:
		print(e)
		sys.exit(1)


################################################################################
# specific datatype
def read_labels_prior_model(priorfilename):
	prefix = os.path.dirname(priorfilename)
	prior = read_distribution_models(priorfilename)
	model_type, distribfile = prior['model_file']
	Distribution = distribution.distributionFactory(model_type)
	distrib = Distribution()
	distrib.read(os.path.join(prefix, distribfile))
	prior['prior'] = distrib
	del prior['model_file']
	return prior

def read_segments_distrib(distribname, selected_sulci=None):
	prefix = os.path.dirname(distribname)
	distrib = read_distribution_models(distribname)
	distrib['vertices'] = {}
	datatypes = distrib['data_type']
	if isinstance(datatypes, tuple):
		return create_segments_distrib_several_datatypes(prefix,
				distrib, selected_sulci)
	else:	return create_segments_distrib_one_datatype(prefix,
				distrib, selected_sulci)

def create_segments_distrib_several_datatypes(prefix, distrib,
					selected_sulci=None):
	datatypes = distrib['data_type']
	subdistribs = {}
	sulci = set()
	for datatype in datatypes:
		distribname = distrib['files'][datatype]
		subdistrib = read_distribution_models(distribname)
		subdistribs[datatype] = subdistrib
		sulci.update(subdistrib['files'].keys())

	for sulcus in sulci:
		distribs = {}
		for datatype, subdistrib in subdistribs.items():
			if selected_sulci is not None and \
				not (sulcus in selected_sulci): continue
			(model_type, distribfile) = subdistrib['files'][sulcus]
			Distribution = distribution.distributionFactory(\
							model_type)
			density = Distribution()
			density.read(os.path.join(prefix,
				os.path.dirname(distrib['files'][datatype]) ,
				distribfile))
			distribs[datatype] = density
		distrib['vertices'][sulcus] = distribs
	del distrib['files']
	return distrib


def create_segments_distrib_one_datatype(prefix, distrib, selected_sulci=None):
	for sulcus, (model_type, distribfile) in distrib['files'].items():
		if selected_sulci is not None and \
			not (sulcus in selected_sulci): continue
		Distribution = distribution.distributionFactory(model_type)
		density = Distribution()
		density.read(os.path.join(prefix, distribfile))
		distrib['vertices'][sulcus] = density
	del distrib['files']
	return distrib


def read_relations_distrib(distribname, selected_sulci=None):
	prefix = os.path.dirname(distribname)
	distrib = read_distribution_models(distribname)
	distrib['edges'] = {}
	datatypes = distrib['data_type']
	if isinstance(datatypes, tuple):
		return create_relations_distrib_several_datatypes(prefix,
				distrib, selected_sulci)
	else:	return create_relations_distrib_one_datatype(prefix,
				distrib, selected_sulci)


def create_relations_distrib_one_datatype(prefix, distrib, selected_sulci=None):
	for relation, (model_type, distribfile) in distrib['files'].items():
		# test to handle 'default' key which must be load
		if isinstance(relation, list):
			sulcus1, sulcus2 = relation
			if selected_sulci is not None and \
				(sulcus1 not in selected_sulci) and \
				(sulcus2 not in selected_sulci): continue
		Distribution = distribution.distributionFactory(model_type)
		density = Distribution()
		density.read(os.path.join(prefix, distribfile))
		distrib['edges'][relation] = density
	del distrib['files']
	return distrib


def create_relations_distrib_several_datatypes(prefix, distrib,
					selected_sulci=None):
	datatypes = distrib['data_type']
	subdistribs = {}
	relations = set()
	for datatype in datatypes:
		distribname = os.path.join(prefix, distrib['files'][datatype])
		subdistrib = read_distribution_models(distribname)
		subdistribs[datatype] = subdistrib
		relations.update(subdistrib['files'].keys())
	for datatype in datatypes: distrib['edges'][datatype] = {}
	for rel in relations:
		if isinstance(rel, tuple):
			sulcus1, sulcus2 = rel
			if selected_sulci is not None and \
				(sulcus1 not in selected_sulci) and \
				(sulcus2 not in selected_sulci): continue
		for datatype, subdistrib in subdistribs.items():
			if not subdistrib['files'].has_key(rel): continue
			(model_type, distribfile) = subdistrib['files'][rel]
			Distribution = distribution.distributionFactory(\
							model_type)
			density = Distribution()
			density.read(os.path.join(prefix,
				os.path.dirname(distrib['files'][datatype]) ,
				distribfile))
			distrib['edges'][datatype][rel] = density
	del distrib['files']
	return distrib



def read_sulci_distrib(distribname, selected_sulci=None):
	return read_segments_distrib(distribname, selected_sulci)

def read_full_model(graphmodelname=None, 
	segmentsdistribname=None, reldistribname=None, sulcidistribname=None,
	labelspriorname=None, globalrotationpriorname=None,
	localrotationspriorname=None, selected_sulci=None):
	if graphmodelname:
		graphmodel = read_graphmodel(graphmodelname, selected_sulci)
	else:	graphmodel = None
	if segmentsdistribname:
		segments_distrib = read_segments_distrib(segmentsdistribname,
							selected_sulci)
	else:	segments_distrib = None
	if reldistribname:
		relations_distrib = read_relations_distrib(reldistribname,
							selected_sulci)
	else:	relations_distrib = None
	if sulcidistribname:
		sulci_distrib = read_sulci_distrib(sulcidistribname,
							selected_sulci)
	else:	sulci_distrib = None
	if labelspriorname:
		labels_prior = read_labels_prior_model(labelspriorname)
	else:	labels_prior = None
	#FIXME add other reader
	if globalrotationpriorname:
		global_rotations_prior = None #FIXME
	else:	global_rotations_prior = None
	if localrotationspriorname:
		local_rotations_prior = None #FIXME
	else:	local_rotations_prior = None
	return sulci.SulciModel(graphmodel, segments_distrib, relations_distrib,
			sulci_distrib, labels_prior, global_rotations_prior,
			local_rotations_prior)
	
	
def read_distribution_models(filename):
	return read_from_exec(filename, 'distributions')

def read_classifiers_models(filename):
	return read_from_exec(filename, 'classifiers')

def read_graphmodel(filename, selected_sulci=None):
	graphmodel = read_from_exec(filename, 'graphmodel')
	if selected_sulci is None: return graphmodel
	todel = []
	for sulcus in graphmodel['vertices'].keys():
		if not (sulcus in selected_sulci): todel.append(sulcus)
	for sulcus in todel: del graphmodel['vertices'][sulcus]
	todel = []
	for sulcus1, sulcus2 in graphmodel['edges']:
		if (not (sulcus1 in selected_sulci)) and \
			(not (sulcus2 in selected_sulci)):
			todel.append((sulcus1, sulcus2))
	for sulci in todel: graphmodel['edges'].remove(sulci)
	return graphmodel

def read_databaselist(filename):
	return read_from_exec(filename, 'databases')

def read_availablelabels(filename):
	return read_from_exec(filename, 'labels')


################################################################################
# writers
def write_pp(parameter, filename, h):
	fd = open(filename, 'w')
	fd.write(parameter + ' = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()

def write_databases(filename, h):
	write_pp('databases', filename, h)



################################################################################
# get filenames
def sulci2databasename(prefix, label):
	csvname = 'bayesian_' + label + '.data'
	minfname = os.path.join(prefix, 'bayesian_' + label + '.minf')
	return csvname, minfname

def relation2databasename(prefix, labels):
	str_labels = ','.join(labels)
	csvname = 'bayesian_' + str_labels + '.data'
	minfname = os.path.join(prefix, 'bayesian_' + str_labels + '.minf')
	return csvname, minfname

def node2densityname(prefix, mode, labels):
	if isinstance(labels, list) or isinstance(labels, tuple):
		str_labels = ','.join(labels)
	else:	str_labels = labels
	if mode == 'spam':
		ext = '.ima'
	else:	ext = '.dat'
	file = 'bayesian_' + mode + '_density_' + str_labels + ext
	return os.path.join(prefix, file)

def minfdatabasename2labels(minfname):
	return minfname[len("bayesian_"):minfname.rfind('.minf')]


################################################################################
# old things : remove when it's possible
def distrib_to_model(bayesian_model, distribname, filter_label, read_node):
	prefix = os.path.dirname(distribname)
	distrib = read_distribution_models(distribname)
	gv = bayesian_model['vertices']
	ge = bayesian_model['edges']

	for labels, local_model_infos in distrib['files'].items():
		if filter_label is not None and filter_label != labels: continue
		model_type, distribfile, weights = local_model_infos
		Distribution = distribution.distributionFactory(model_type)
		density = Distribution()
		density.read(os.path.join(prefix, distribfile))
		if isinstance(labels, list) or isinstance(labels, tuple):
			total_weights = distrib['priors_relations_total']
			priors = [float(w) / t for (w, t) in \
					zip(weights, total_weights)]
			ge[labels] = {'density' : density, 'priors' : priors}
		else:
			if not read_node: continue
			total_weights = distrib['priors_nodes_total']
			priors = [float(w) / t for (w, t) in \
					zip(weights, total_weights)]
			gv[labels] = {'density' : density, 'priors' : priors}
	return distrib



def read_bayesian_model(graphmodelname=None, node_distribname=None,
		rel_distribname=None, filter_label=None):
	if graphmodelname:
		graph_model = read_graphmodel(graphmodelname)
	else:	graph_model = None
	bayesian_model = {'type' : 'distributions',
			'vertices' : {}, 'edges' : {}, 
			'priors_relations_hash' : {},
			'priors_nodes_hash' : {}}
	pn = bayesian_model['priors_nodes_hash']
	pr = bayesian_model['priors_relations_hash']

	if node_distribname:
		node_distrib = distrib_to_model(bayesian_model, \
				node_distribname, filter_label, True)
	else:	node_distrib = None
	if rel_distribname:
		rel_distrib = distrib_to_model(bayesian_model, \
				rel_distribname, filter_label, False)
	else:	rel_distrib = None

	if node_distrib:
		priors_nodes_names = node_distrib['priors_nodes_names']
		try:
			priors_relations_names = \
					node_distrib['priors_relations_names']
		except KeyError:
			if rel_distrib:
				priors_relations_names = \
					rel_distrib['priors_relations_names']

	if len(bayesian_model['vertices']):
		for i, p in enumerate(priors_nodes_names): pn[p] = i
	if len(bayesian_model['edges']):
		for i, p in enumerate(priors_relations_names): pr[p] = i
	bayesian_model['graph_model'] = graph_model
	bayesian_model['node_distrib'] = node_distrib
	bayesian_model['rel_distrib'] = rel_distrib
	return bayesian_model


#FIXME : changer comme pour read_bayesian_model :
# separer node/rel
def read_clf_bayesian_model(graphmodelname, clfname, filter_label=None):
	clfprefix = os.path.dirname(clfname)
	graphmodel = read_graphmodel(graphmodelname)
	classifiers = read_classifiers_models(clfname)
	
	bayesian_model = {'type' : 'classifiers', 'vertices' : {}, 'edges' : {}}
	gv = bayesian_model['vertices']
	ge = bayesian_model['edges']

	for labels, local_model_infos in classifiers['files'].items():
		if filter_label is not None and filter_label != labels: continue
		model_type, clffile = local_model_infos
		Classifier = classifier.classifierFactory(model_type)
		clf = Classifier()
		clf.read(os.path.join(clfprefix, clffile))
		if isinstance(labels, list) or isinstance(labels, tuple):
			ge[labels] = {'clf' : clf}
		else:	gv[labels] = {'clf' : clf}

	return bayesian_model, graphmodel, classifiers


