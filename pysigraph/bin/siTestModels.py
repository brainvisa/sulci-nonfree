#!/usr/bin/env python2

from __future__ import absolute_import
from __future__ import print_function
import os
from soma import aims
import datamind.ml
from sigraph import *
import sigraph.models
from sigraph.cover import *
import sigraph.learning as learning
import sigraph.test_models as test_models
from optparse import OptionParser
from datamind.tools import *
from six.moves import range

# Options parser
def parseOpts(argv):
	description = 'Test models on databases.'
	parser = OptionParser(description)
	add_filter_options(parser)
	parser.add_option('-m', '--model', dest='modelfilename',
		metavar='MODEL', action='store', default='model.arg',
		help='model file name (default : %default)')
	parser.add_option('-t', '--test', dest='test_modelfilename',
		metavar='TEST', action='store', default='model.arg',
		help='''model file name (default : %default). Only test
		part of models databases(vectors shaped) is used by default.''')
	parser.add_option('--train', dest='use_train', action='store_true',
		default=False, help='''use train part of models databases
		instead of test one.''')
	parser.add_option('--hist', dest='hist',
		action='store', default = None,
		help='make histogram from classifiation/regression rates '
			'of models. HIST should be a format supported by pylab')
	return parser.parse_args(argv)


def to_key(labels):
        return '-'.join([l for l in labels if l != 'unknown'])


# Hack to get bad/good mse values
def my_getResult_regressor(self, res) :
	return [res.mse, res.wmse, res.bad_mse, res.good_mse]

def my_getResult_classifer(self, res) :
	return [res.raw_classification_rate, res.weighted_classification_rate,
		res.bad_classification_rate, res.good_classification_rate]

# Covering of adaptive leafs
def adaptiveleaf_cover(al, user_data):
	# loading model
	clf = learning.clfFactory(al)
	if type(clf) in [sigraph.models.SvrSi, sigraph.models.MlpSi]:
		clf.__class__._getResult = my_getResult_regressor
	else:	clf.__class__._getResult = my_getResult_classifier
	user_data['models'][to_key(al.topModel().significantLabels())] = clf

def adaptiveleaf_cover_test(al, user_data):
	# loading of database
	try:
		al.readDataBase()
	except:
		name = al.getDataBaseName()
		msg.warning("reading of '" + name + "(.data, .minf)' failed.")
		return 0.
	database = al.getAdapDescr().getDBLearnable()
	db = datamind.ml.database.DbSi(database)
	split = database.getSplit()
	if user_data['use_train']:
		trainview = db.select(list(range(split)))
		view = learning.convertView(trainview)
	
	else:
		testview = db.select(list(range(split, database.size())))
		view = learning.convertView(testview)
	clf = user_data['models'][to_key(al.topModel().significantLabels())]	

	# test model
	r = clf.predict(view)
	user_data['Raw'] += [r[0]]
	user_data['Mean'] += [r[1]]
	user_data['Bad'] += [r[2]]
	user_data['Good'] += [r[3]]

	# display bar
	user_data['progression_bar'].display(user_data['n'])
	user_data['n'] += 1


def write_items(list):
	for l in list:
		msg.write_list([('   - ', 'red'), l])

# main function
def main():
	import sys

	# read options
	options, args = parseOpts(sys.argv)

	# read model
	r = aims.Reader()
	msg.write_list([(' * ', 'bold_yellow'), 'Read models :\n'])
	write_items(['learned model...\n'])
	model = r.read(options.modelfilename)
	write_items(['tested model...\n'])
	if options.test_modelfilename == options.modelfilename:
		test_model = model
	else:	test_model = r.read(options.test_modelfilename)

	# cover model
	fundict = {'adaptiveleaf' : adaptiveleaf_cover}
	data = {'models' : {}}
	test_fundict = {'adaptiveleaf' : adaptiveleaf_cover_test}
	test_data = {'use_train' : options.use_train, 'models' : data['models'],
		'Raw': [], 'Mean' : [], 'Good' : [], 'Bad' : [], 'n' : 0,
		'progression_bar' : \
		ProgressionBarPct(cover_adaptive_count(test_model))}
	cover(model, fundict, data, options.labels_filter, options.filter_mode)
	msg.write_list([(' * ', 'bold_yellow'), 'Test models :\n'])
	cover(test_model, test_fundict, test_data,
		options.labels_filter, options.filter_mode)
	print()

	# synthetize test
	test_models.resume_errors_info(test_data)
        if options.hist : test_models.make_errors_hist(options.hist, test_data)

if __name__ == '__main__' : main()
