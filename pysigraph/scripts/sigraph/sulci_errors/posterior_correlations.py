#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import os, sys, numpy
from optparse import OptionParser
import datamind.io as datamind_io
from sulci.common import io as sulci_io, add_translation_option_to_parser
from six.moves import range


################################################################################
def read_local_posteriors(X, graph, sulci_data):
	'''
    for one subject

    X :     posteriors probabilities
    graph : sulci graph
	'''
	segments = {}
	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		segments[v['index']] = v

	# FIXME : had better check
	has_likelihood = X.colnames()[1] != 'proba'

	sulci_data_tmp = {}
	total_size = 0.
	for i, x in enumerate(X):
		if has_likelihood:
			index, likelihood, loglikelihood, label_proba, P = \
				x[0], x[1], x[2], x[3], x[4:]
		else:	index, label_proba, P = x[0], x[1], x[2:]
		v = segments[index]
		name, label, size = v['name'], v['label'], v['size']
		name_proba = X[i, 'proba_%s' % v['name']]
		total_size += size
		good_labeling = (name == label)
		P2 = P[P > 0]
		negentropy = (P2 * numpy.log(P2)).sum()
		data_label = [0, size, good_labeling, label_proba, negentropy]
		data_name = [1, size, good_labeling, name_proba,negentropy]
		try:	sulci_data_tmp[label].append(data_label)
		except KeyError:
			sulci_data_tmp[label] = [data_label]
		try:	sulci_data_tmp[name].append(data_name)
		except KeyError:
			sulci_data_tmp[name] = [data_name]

	# subectwise normalization of weights (size)
	for sulci, data in sulci_data_tmp.items():
		data2 = numpy.array(data)
		data2[:, 1] /= total_size
		data2 = data2.tolist()
		try:	sulci_data[sulci] += data2
		except KeyError:
			sulci_data[sulci] = data2


def summarize_sulci_data(sulci_data):
	# add 'all' item to compute and store global measures over all sulci
	sulci_data['all'] = []
	for sulci, data in sulci_data.items(): sulci_data['all'] += data

	# summarize
	sulci_data_summary = {}
	weighted_sulci_data_summary = {}
	for sulci, data in sulci_data.items():
		data = numpy.array(data)
		sulci_data_summary[sulci] = []
		weighted_sulci_data_summary[sulci] = []
		for i in range(2): # iter over name and label
			D = data[data[:, 0] == i] # select name or label
			W = D[:, 1] # size
			W /= W.sum()
			D = D[:, 2:] # without label/name, size
			M = numpy.mean(D, axis=0)
			WM = numpy.dot(W.T, D)

			# compute correlations
			X, Y = D[:, 0][None].T, D[:, 1:]
			EX, EY = M[0], M[1:]
			WEX, WEY = WM[0], WM[1:]
			EX2 = (X ** 2).mean()
			EY2 = (Y ** 2).mean(axis=0)
			WEX2 = numpy.dot(W.T, X ** 2)
			WEY2 = numpy.dot(W.T, Y ** 2)
			XY = (X * Y) # good_labeling * measures
			EXY = numpy.mean(XY, axis=0)
			WEXY = numpy.dot(W.T, XY)
			SX = numpy.sqrt(EX2 - EX ** 2)
			SY = numpy.sqrt(EY2 - EY ** 2)
			WSX = numpy.sqrt(WEX2 - WEX ** 2)
			WSY = numpy.sqrt(WEY2 - WEY ** 2)
			C = EXY - EX * EY      # cov
			WC = WEXY - WEX * WEY  # weighted cov
			cor = C / (SX * SY)
			Wcor = WC / (WSX * WSY)
			cor[numpy.isnan(cor)] = 10e-10
			Wcor[numpy.isnan(Wcor)] = 10e-10

			# store
			sulci_data_summary[sulci] += list(M) + list(cor)
			weighted_sulci_data_summary[sulci] += \
						list(WM) + list(Wcor)
	return sulci_data_summary, weighted_sulci_data_summary


def write_sulci_data_summary(filename, sulci_data_summary,
			weighted_sulci_data_summary):
	fd = open(filename, 'w')
	measures = ['proba', 'negentropy']
	measures = ['good_labeling'] + measures + \
		[('corr_' + m) for m in measures]
	measures = [('label_' + m) for m in measures] + \
		[('name_' + m) for m in measures]
	header = ['sulci'] + measures + [('weighted_' + m) for m in measures]
	header = '\t'.join(header) + '\n'
	fd.write(header)
	for sulci, data in sulci_data_summary.items():
		wdata = weighted_sulci_data_summary[sulci]
		s = ('%s' % sulci) + ('\t%f' * len(data)) % tuple(data) + \
			('\t%f' * len(wdata)) % tuple(wdata) + '\n'
		fd.write(s)


################################################################################
def parseOpts(argv):
	description = 'compute correlations between posterior mesures ' + \
		'(posterior local probabilities, posterior local ' + \
		'entropies...) from an automatic sulci labelization.\n' + \
	'./posterior_correlations.py [OPTIONS] graph1 graph2... == csv1 csv2...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('--summary-csv', dest='summarycsvfilename',
		metavar = 'FILE', action='store', default = None,
		help='summary csv file (one line per sulci)')

	return parser, parser.parse_args(argv)


def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	inputs = args[1:]
	if len(inputs) == 0:
		parser.print_help()
		sys.exit(1)
	ind = [i for i, input in enumerate(inputs) if input == '==']
	if len(ind) == 0:
		print("error : missing ==")
		sys.exit(1)
	ind = ind[0]
	graphnames = inputs[:ind]
	local_posteriors_names = inputs[ind + 1:]

	# read input data
	graphs = sulci_io.load_graphs(options.transfile, graphnames)
	datamind_reader = datamind_io.ReaderCsv()
	sulci_data = {}

	for i in range(len(graphs)):
		graph = graphs[i]
		local_posteriors_name = local_posteriors_names[i]
		X = datamind_reader.read(local_posteriors_name)
		read_local_posteriors(X, graph, sulci_data)

	sulci_data_summary, weighted_sulci_data_summary = \
			summarize_sulci_data(sulci_data)
	write_sulci_data_summary(options.summarycsvfilename,
		sulci_data_summary, weighted_sulci_data_summary)


if __name__ == '__main__' : main()
