# Copyright CEA (2000-2006)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
#
# This software is governed by the CeCILL license version 2 under 
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the 
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.

import numpy
import sigraph
from soma import aims

autolabel, manuallabel = 'label', 'name'

# old method, may be removed in future
def computeErrorRate_with_siError(labeled_graph,
		labels_translation, base_graph):
	'''
    Used by siErrorLightWrapper.py and recognition_error.py brainvisa process.
	'''
	import distutils.spawn, sys, popen2
	pgm = distutils.spawn.find_executable('siError')
	cmd = [pgm, labeled_graph, labels_translation, base_graph]
	p = popen2.Popen3(cmd)
	p.wait()
	if p.poll() == 256: sys.exit(1)
	lines = p.fromchild.readlines()
	r = [l for l in lines if l.startswith('En masse')][0]
	error_rate = r[r.find(':') + 2:r.find('%')]
	return float(error_rate)


def translate(lg, bg, labels_translation):
	ft = sigraph.FoldLabelsTranslator(labels_translation)
	ft.translate(lg)
	ft.translate(lg, autolabel, autolabel)
	ft.translate(bg)
	ft.translate(lg, manuallabel, manuallabel)


class SulciError(object):
	def __init__(self, error=0, sulci_size_for_error=0):
		''' For a given sulcus of interest S.  '''
		self._error = float(error)
		self._sulci_size_for_error = float(sulci_size_for_error)
		self._bad_tagged_nodes = 0
		self._total_nodes = 0
		self._sulci_size = 0
		self._fp = self._fn = self._tp = 0

	def update_name(self, tp, fn):
		self._sulci_size += float(tp) + float(fn)
		self._tp += tp
		self._fn += fn
		self._error += float(fn)
		self._sulci_size_for_error += float(tp) + float(fn)
		if fn: self._bad_tagged_nodes += 1
		self._total_nodes += 1

	def update_label(self, fp):
		self._fp = fp
		self._error += float(fp)
		self._sulci_size_for_error += float(fp)
		self._bad_tagged_nodes += 1
		self._total_nodes += 1

	def compute_nodes_error(self):
		return self._bad_tagged_nodes / float(self._total_nodes)

	def compute_size_error(self):
		if self._sulci_size_for_error:
			return self._error / self._sulci_size_for_error
		else:	return 0.

        def compute_SI_error(self):
                # SI = 2*TP / ( 2*TP + FP + FN )
                # TP: true positive, FP: false positive, FN: false negative
                # here we return 1-SI = ( FP + FN ) / (...)
		if self._sulci_size_for_error:
			return self._error / \
				(self._sulci_size_for_error * 2 - self._error)
                else:
                        return 0
	
	def sulci_size(self): return self._sulci_size
	def false_positive(self): return self._fp
	def false_negative(self): return self._fn
	def true_positive(self): return self._tp


def computeLocalErrorRates(lg, bg, filtred_labels):
	corresp = {}
	for v in bg.vertices():
		ind = v['index']
		corresp[ind] = v
	sulci_errors = {}
	for v in lg.vertices():
		if filtred_labels and v['name'] in filtred_labels: continue
		ind = v['index']
		v2 = corresp[ind]
		name = v2[manuallabel]
		label = v[autolabel]
		try: size = v['size']
		except KeyError: size = 0
		if label != name:
			print "%s -> %s (size : %f)" % (name, label, size)
			tp, fn = 0, size
		else:	tp, fn = size, 0
		if not sulci_errors.has_key(name):
			sulci_errors[name] = SulciError()
		sulci_errors[name].update_name(tp, fn)
		if label != name:
			if not sulci_errors.has_key(label):
				sulci_errors[label] = SulciError()
			sulci_errors[label].update_label(fn)
	# at the end : labels not in sulci_errors, does not appear
	# in manual labelling or has been filtred
	return sulci_errors


def computeGlobalErrorRates(local_errors):
	errors, sizes, fp = [], [], []
	for sulcus, error_rate in local_errors.items():
		se = error_rate.compute_size_error()
		ne = error_rate.compute_nodes_error()
		sie = error_rate.compute_SI_error()
		be = (se != 0.)
		errors.append([se, be, ne, sie])
		sizes.append(error_rate.sulci_size())
		fp.append(error_rate.false_negative())
	errors = numpy.vstack(errors)
	sizes = numpy.array(sizes)
	total_size = sizes.sum()

	errors_mean = errors.mean(axis=0)
	errors_std = errors.std(axis=0)
	errors_weighted = (errors.T * sizes).T
	errors_weighted_mean = errors_weighted.sum(axis=0) / total_size
	mass_error = numpy.sum(fp) / total_size
	return {'Balanced' : {
			'Mean_Local_Size_Error' : errors_mean[0],
			'Mean_Local_Binary_Error' : errors_mean[1],
			'Mean_Local_Nodes_Error' : errors_mean[2],
			'Mean_Local_SI_Error' : errors_mean[3]
		},
		'Weighted' : {
			'Mean_Local_Size_Error' : errors_weighted_mean[0],
			'Mean_Local_Binary_Error' : errors_weighted_mean[1],
			'Mean_Local_Nodes_Error' : errors_weighted_mean[2],
			'Mean_Local_SI_Error' : errors_weighted_mean[3],
			'Global_Mass_Error' : mass_error} # sum F_N /total_size
		}


def print_global_errors(dict):
	'''
    dict : output of computeGlobalErrorRates function.
	'''
	for type in dict.keys():
		print "\n%s Errors (%%):" % type
		for name, val in dict[type].items():
			print "- %s : %f" % (name, val * 100)


def computeErrorRates(base_graph, labeled_graph,
	labels_translation, filtred_labels=None):
	r = aims.Reader()
	lg = r.read(labeled_graph)
	if labeled_graph == base_graph:
		bg = lg
	else:	bg = r.read(base_graph)

	translate(lg, bg, labels_translation)
	local_errors = computeLocalErrorRates(lg, bg, filtred_labels)
	global_errors = computeGlobalErrorRates(local_errors)
	return local_errors, global_errors

