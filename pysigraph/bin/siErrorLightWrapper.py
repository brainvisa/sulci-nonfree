#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
import sigraph.error, sigraph.nrj
from soma import aims
from sulci.common import lock, unlock
from soma.sorted_dictionary import SortedDictionary


def addInfoToCSV(csvfilename, graphname, errors, nrj):
	lock(csvfilename + '.lock')
	fd = open(csvfilename, 'a')
	if fd.tell() == 0:
		h = 'Subject\t%s\tEnergy\n' % '\t'.join(errors.keys())
		fd.write(h)
	errors_str = '\t'.join(str(x) for x in errors.values())
	fd.write('%s\t%s\t%s\n' % (graphname, errors_str, str(nrj)))
	fd.close()
	unlock(csvfilename + '.lock')



# Options parser
def parseOpts(argv):
	description = 'Call siError on graph inputs and store resulting ' \
			'error rate in a new entry of a CSV file.'
	parser = OptionParser(description)
	parser.add_option('-m', '--model', action='store',
		dest='model', metavar = 'FILE', default = None,
		help='graph model')
	parser.add_option('-l', '--labeled_graph', action='store',
		dest='labeled_graph', metavar = 'FILE', default = None,
		help='labeled graph file (evaluated one)')
	transfile = os.path.join(aims.carto.Paths.shfjShared(),
		'nomenclature', 'translation', 'sulci_model_noroots.trl')
	parser.add_option('-t', '--labels_translation', action='store',
		dest='labels_translation', metavar = 'FILE',
		default = transfile, help='labels translation file')
	parser.add_option('-b', '--base_graph', action='store',
		dest='base_graph', metavar = 'FILE', default = None,
		help='base graph file (reference) If not specified, '
		'labeled_graph is taken')
	parser.add_option('-c', '--csv', action='store',
		dest='csvfilename', metavar = 'CSV', default = None,
		help='file storing error rate')
	parser.add_option('-n', '--graph_name', action='store',
		dest='graphname', metavar = 'NAME', default = None,
		help='graph name to be stored in CSV')
	options, args = parser.parse_args(argv)
	if None in [options.labeled_graph,
			options.csvfilename, options.graphname]:
		parser.print_help()
		sys.exit(1)
	return options, args



def main():
	(options, args) =  parseOpts(sys.argv)
	filtred_labels = None # pas gere pour le moment
	if options.base_graph is None:
		options.base_graph = options.labeled_graph
	local_errors, global_errors = sigraph.error.computeErrorRates(
				options.base_graph, options.labeled_graph,
				options.labels_translation, filtred_labels)
	sigraph.error.print_global_errors(global_errors)
	# dict of dict -> dict, 0..1 -> 0..100%, round
	global_errors2 = SortedDictionary(*[(x + '_' + k,
					numpy.round(v * 10000) / 100) \
					for x in global_errors \
					for k, v in global_errors[x].items()])
	if options.model is not None:
		nrj = sigraph.nrj.computeNrj(options.model,
					options.labeled_graph,
					options.labels_translation)
		print "Energy = %f %%" % nrj
	else:	nrj = numpy.nan
	addInfoToCSV(options.csvfilename, options.graphname,
					global_errors2, nrj)
		


if __name__ == "__main__" : main()
