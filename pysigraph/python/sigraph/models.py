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
from soma import aims
import sigraph, sigraph.cover


def to_key(labels):
        return '-'.join([l for l in labels if l != 'unknown'])


class DiffData(object):
	def __init__(self, keys, differences, data):
		self._keys = keys
		self._differences = differences
		self._data = data

	def keys(self): return self._keys
	def differences(self): return self._differences
	def data(self): return self._data

	def sort(self, col):
		ind = numpy.argsort(self._differences[:,col])
		return DiffData(self._keys[ind], self._differences[ind],
			(self._data[0][ind], self._data[1][ind]))

	def latest(self, col, n):
		return DiffData(self._keys[-n:], self._differences[-n:, col], \
			(self._data[0][-n:, col], self._data[1][-n:, col]))

	def first(self, col, n):
		return DiffData(self._keys[:n], self._differences[:n, col], \
			(self._data[0][:n, col], self._data[1][:n, col]))

	def biggest(self, col, n):
		di = self.sort(col)
		return di.latest(col, n)

	def smallest(self, col, n):
		di = self.sort()
		return di.first(col, n)
		
	def print_diff(self, name):
		from datamind.tools.message import msg
		blank = ' ' * 80
		max = numpy.array([len(k) for k in self._keys]).max()
		msg.write_list([('***', 'red'), ' ' + name + ' ' , \
				('***\n', 'red')])
		for k, v, s1, s2 in zip(self._keys, self._differences, \
			self._data[0], self._data[1]):
			b = blank[:max + 2 - len(k)]
			if s1 > s2: sym = '>'
			else:	sym = '<'
			msg.write_list([" - %s :%s" % (k, b) , \
				("%f" % v, "cyan"), '  (%f ' % s1,
				(sym, 'red'), ' %f)\n' % s2])



class ModelDifferentiator(object):
	def __init__(self, models, abs):
		self._models = models
		self._cmp_enum = {'raw' : 0, 'mean' : 1, 'good' : 2, 'bad' : 3}
		self._abs = abs
		self._infos_list = (None, None)
		self._diff = None

	def _dict_to_array(self, d):
		l = [x for x in d.items()]
		l.sort()
		return numpy.array([k[0] for k in l]), \
			numpy.array([k[1] for k in l])

	def _get_selected_diff(self, mode, diffdata, col, n):
		if mode == 'biggest':
			return diffdata.biggest(col, n)
		elif mode == 'smallest':
			return diffdata.smallest(col, n)

	def _compute(self, labels_filter=None, filter_mode=None):
		def get_local_info(al, infos):
			sa = al.workEl()
			re = sa.genErrorRate()
			me = sa.genMeanErrorRate()
			ge = sa.genGoodErrorRate()
			be = sa.genBadErrorRate()
			infos[to_key(al.topModel().significantLabels())] = \
							(re, me, ge, be)
		# cover model
		infos1, infos2 = {}, {}
		if len(self._models) == 2:
			model1, model2 = self._models
		else:	# len == 1
			model1, model2 = self._models[0], None
		fundict = {'adaptiveleaf' : get_local_info}
		sigraph.cover.cover(model1, fundict, infos1,
				labels_filter, filter_mode)
		if model2:
			sigraph.cover.cover(model2, fundict, infos2,
				labels_filter, filter_mode)
		self._infos_list = (infos1, infos2)

	def diff(self, labels_filter=None, filter_mode=None):
		self._compute(labels_filter, filter_mode)
		d1 = self._infos_list[0]
		d2 = self._infos_list[1]
		if len(d2) == 0: d2 = dict([(v, (0., 0., 0., 0.)) for v in d1])
		for r in [k for k in d1.keys() if not k in d2.keys()]: del d1[r]
		for r in [k for k in d2.keys() if not k in d1.keys()]: del d2[r]
		k1, a1 = self._dict_to_array(d1)
		k2, a2 = self._dict_to_array(d2)

		if (k1 != k2).any() : print "arrrrrrrg"
		else:	keys = k1
		di = a1 - a2
		if self._abs: di = numpy.absolute(di)
		self._diff = DiffData(keys, di, (a1, a2))

	def print_diff(self, mode, pct, cmp_modes):
		keys = self._diff.keys()
		n = int(pct * len(keys) / 100)
		for c in cmp_modes:
			di = self._get_selected_diff(mode,
				self._diff, self._cmp_enum[c], n)
			di.print_diff(c)
	
	def hist(self, histname):
		di = self._diff.differences()
		data = {'Raw' : di[:, 0], 'Mean' : di[:, 1],
			'Good' : di[:, 2], 'Bad': di[:, 3]}
		import sigraph.test_models as TM
		TM.make_errors_hist(histname, data,
			title = 'Differences between two models')

	def getValue(self, name, cmp_mode):
		bad_value = 1.
		if name == 'unknown' : return name, bad_value
		ind = numpy.argwhere(self._diff.keys() == name)
		if name == 'ventricle': print 'name = ', name, ind
		if numpy.shape(ind) == (1, 1):
			ind = ind[0, 0]
			val = self._diff.differences()[ind,
					self._cmp_enum[cmp_mode]]
			if name == 'ventricle':
				print "val = ", val
			return name, self._diff.differences()[ind, \
					self._cmp_enum[cmp_mode]]
		else:	return name, bad_value

	def getVertexValue(self, vertex, cmp_mode):
		name = vertex['label']
		return self.getValue(name, cmp_mode)

	def getEdgeValue(self, edge, cmp_mode):
		mod = sigraph.Model.fromObject(edge['model'].get())
		name = to_key(mod.significantLabels())
		return self.getValue(name, cmp_mode)

	def getMinMax(self, cmp_mode):
		m = self._diff.differences()[:, self._cmp_enum[cmp_mode]]
		return m.min(), m.max()
