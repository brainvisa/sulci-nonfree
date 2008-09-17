import numpy
from soma import aims

################################################################################
# Main class
################################################################################
class Descriptor(object):
	def __init__(self): pass

	def name(self): return self._name


################################################################################
# Segments descriptors
################################################################################
class SegmentDescriptor(Descriptor):
	def __init__(self):
		Descriptor.__init__(self)

	def likelihood(self, distrib, vertex, motion):
		return distrib.likelihood(self.data(motion, vertex))

################################################################################
class VoxelsSegmentDescriptor(SegmentDescriptor):
	def __init__(self):
		SegmentDescriptor.__init__(self)
		self._depth_map = None

	def setDepthMap(self, depth_map):
		self._depth_map = depth_map

	def likelihood(self, distrib, motion, vertex):
		if distrib.name() == 'spam':
			return self.likelihood_spam(distrib, motion, vertex)
		elif distrib.name() == 'depth_weighted_spam':
			return self.likelihood_depth_weighted_spam(
					distrib, motion, vertex)
		return SegmentDescriptor.likelihood(self,
				distrib, motion, vertex)

	def likelihood_spam(self, distrib, motion, vertex):
		d = self.data(motion, vertex)
		if d is None: return 0., 1. # CTE : logli, li
		return distrib.prodlikelihoods(d)

	def likelihood_depth_weighted_spam(self, distrib, motion, vertex):
		X = self.data(motion, vertex)
		map = vertex[self._data_type].get()
		depths = numpy.array([self._depth_map.value(p[0], p[1], p[2]) \
						for p in map[0].keys()])
		logli, li = distrib.gaussian().likelihoods(depths)
		# li : weighting voxels along depthmap
		li *= len(X) / li.sum()
		return distrib.weighted_prodlikelihoods(X, li)

	def data(self, motion, vertex):
		map = vertex[self._data_type].get()
		s = numpy.array([map.sizeX(), map.sizeY(), map.sizeZ()])
		vox = [motion.transform(aims.Point3df(p * s)) \
					for p in map[0].keys()]
		if len(vox):
			return numpy.vstack(vox)
		else:	return None

class SurfaceSimpleSegmentDescriptor(VoxelsSegmentDescriptor):
	def __init__(self):
		VoxelsSegmentDescriptor.__init__(self)
		self._name = 'voxels_aims_ss'
		self._data_type = 'aims_ss'


class BottomSegmentDescriptor(VoxelsSegmentDescriptor):
	def __init__(self):
		VoxelsSegmentDescriptor.__init__(self)
		self._name = 'voxels_bottom'
		self._data_type = 'aims_bottom'


################################################################################
class GravityCenterSegmentDescriptor(SegmentDescriptor):
	def __init__(self):
		SegmentDescriptor.__init__(self)
		self._name = 'refgravity_center'

	def data(self, motion, vertex):
		#FIXME : motion
		return vertex['refgravity_center'].arraydata()


################################################################################
class OrientationSegmentDescriptor(SegmentDescriptor):
	def __init__(self):
		SegmentDescriptor.__init__(self)
		self._name = 'refhull_normal'

	def data(self, motion, vertex):
		#FIXME : motion
		return vertex['refhull_normal'].arraydata()


class CoordinateSystemSegmentDescriptor(SegmentDescriptor):
	def __init__(self):
		SegmentDescriptor.__init__(self)
		self._name = 'coordinate_system'

	def data(self, motion, vertex):
		#FIXME : motion
		h = vertex['refhull_normal'].arraydata()
		n = v['refnormal'].arraydata()
		o = numpy.vstack([h, n]).ravel()
		return o

################################################################################
class ComboSegmentDescriptor(SegmentDescriptor):
	def __init__(self, descriptors):
		SegmentDescriptor.__init__(self)
		self._descriptors = {}
		for descr in descriptors:
			self._descriptors[descr.name()] = descr

	def likelihood(self, distrib, vertex):
		logli_res, li_res = 0., 1.
		
		for datatype, descr in self._descriptors.items():
			subdistrib = distrib[datatype]
			logli, li = descr.likelihood(subdistrib, vertex)
			logli_res += logli
			li_res *= li
		return logli_res, li_res


################################################################################
# Relations descriptors
################################################################################
class RelationDescriptor(Descriptor):
	def __init__(self):
		Descriptor.__init__(self)

	def edges_from_graph(self, graph, selected_sulci=None):
		edges = {}
		for e in graph.edges():
			v1, v2 = e.vertices()
			# skip hull junctions
			if v1.getSyntax() != 'fold' or \
				v2.getSyntax() != 'fold':
				continue
			name1, name2 = v1['name'], v2['name']
			if selected_sulci != None and \
				(name1 not in selected_sulci) and \
				(name2 not in selected_sulci):
				continue
			r1, r2 = v1['index'], v2['index']
			if r1 > r2:
				v1, v2 = v2, v1
				r1, r2 = r2, r1
			key = (r1, r2)
			syntax = e.getSyntax()
			if edges.has_key(key):
				edges[key][1][syntax] = e
			else:	edges[key] = (v1, v2), {syntax : e}
		return edges


	def potential_matrix(self, distribs, edge_infos,
				avalaible_labels, potential=True):
		'''
    potential : True store -log(likelihood(data(edge))) for each label pair.
                False strore    likelihood(data(edge))       "     "
		'''
		(r1, r2), ((v1, v2), edges) = edge_infos
		data = self.data_edges(edges)
		labels_1, labels_2 = avalaible_labels[r1], avalaible_labels[r2]
		P = numpy.zeros((len(labels_1), len(labels_2)), numpy.float96)
		for s1, l_1 in enumerate(labels_1):
			for s2, l_2 in enumerate(labels_2):
				if l_1 > l_2:
					relation = l_2, l_1
				else:	relation = l_1, l_2
				if not distribs.has_key(relation):
					if l_1 == l_2:
						relation = 'default_intra'
					else:	relation = 'default_inter'
				distrib = distribs[relation]
				logli, li = distrib.likelihoods(data)
				if potential:
					P[s1, s2] = -logli
				else:	P[s1, s2] = li
		return P, (r1, r2)


################################################################################
class MinDistanceRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'min_distance'

	def data_edges(self, edges):
		if edges.has_key('plidepassage') or edges.has_key('junction'):
			pi = numpy.array([0, 0, 0])
			pj = numpy.array([0, 0, 0])
		elif edges.has_key('cortical'):
			edge = edges['cortical']
			pi = edge['refSS1nearest'].arraydata()
			pj = edge['refSS2nearest'].arraydata()
		else: # hull junction
			return None
		return numpy.sqrt(((pi - pj) ** 2).sum()) #distance

	def data_from_graphs(self, graphs, selected_sulci=None):
		data = {}
		for g in graphs:
			graph_edges = self.edges_from_graph(g, selected_sulci)
			for (r1, r2), ((v1, v2), edges) in graph_edges.items():
				name1, name2 = v1['name'], v2['name']
				d = numpy.array(self.data_edges(edges))
				# order names
				if name1 > name2: name1, name2 = name2, name1
				key = (name1, name2)
				if data.has_key(key):
					data[key].append(d)
				else:	data[key] = [d]
		for relation, D in data.items():
			data[relation] = numpy.vstack(D)
		return data

################################################################################
# Prior descriptors
################################################################################
# FIXME :
# l'API est pourrie, c'est a changer. Il s'agit juste d'un bricolage vite fait
class PriorDescriptor(Descriptor):
	def __init__(self):
		Descriptor.__init__(self)


class GlobalFrequencyDescriptor(PriorDescriptor):
	def __init__(self):
		PriorDescriptor.__init__(self)

	def get_data(self): return self._data

	def set_data(self, data): self._data = data

	def update_data(self, segment, label1, label2):
		s = self._sizes[segment]
		self._data[label1] -=  s # old
		self._data[label2] +=  s # new

	def likelihood(self, distrib, label):
		self.full_likelihood(self, distrib, None)

	def full_likelihood(self, distrib, taglabels):
		logli, li = distrib.likelihood(self._data)
		return logli / self._w
		

class LabelGlobalFrequencyDescriptor(GlobalFrequencyDescriptor):
	def __init__(self):
		GlobalFrequencyDescriptor.__init__(self)
		self._name = 'label_global_frequency'
		self._data = None
		self._w = None

	def update_data(self, segment, label1, label2):
		self._data[label1] -=  1 # old
		self._data[label2] +=  1 # new

	def compute_data(self, graph, taglabels, availablelabels,
						segments, labels):
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			ind = v['index']
		label_map = {}
		X = numpy.zeros(len(labels), numpy.float96)
		for (i, label) in enumerate(labels):
			label_map[label] = i
		for id in segments:
			label = availablelabels[id][taglabels[id]]
			X[label_map[label]] += 1
		self._data = X
		self._w = len(self._data)


class SizeGlobalFrequencyDescriptor(GlobalFrequencyDescriptor):
	def __init__(self):
		GlobalFrequencyDescriptor.__init__(self)
		self._name = 'size_global_frequency'
		self._data = None
		self._sizes = None
		self._w = None

	def compute_data(self, graph, taglabels, availablelabels,
						segments, labels):
		sizes = {}
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			size = v['refsize']
			ind = v['index']
			sizes[ind] = size
		label_map = {}
		X = numpy.zeros(len(labels), numpy.float96)
		for (i, label) in enumerate(labels):
			label_map[label] = i
		for id in segments:
			label = availablelabels[id][taglabels[id]]
			size = sizes[id]
			X[label_map[label]] += size
			sizes[id] = size
		self._sizes = sizes
		self._data = X
		self._w = len(self._data)


class LocalFrequencyDescriptor(PriorDescriptor):
	def __init__(self):
		PriorDescriptor.__init__(self)
		self._name = 'local_frequency'

	def get_data(self): return None
	def set_data(self, data): pass
	def compute_data(self, graph, taglabels, availablelabels,
					segments, labels): pass
	def update_data(self, segment, label1, label2): pass

	def likelihood(self, distrib, label):
		li = numpy.asarray(distrib.frequencies()).ravel()[label]
		return numpy.log(li)

	def full_likelihood(self, distrib, taglabels):
		s = 0.
		for id, label in taglabels.items():
			s += self.likelihood(distrib, label)
		return s


################################################################################
# Descriptor map/factory
################################################################################
descriptor_map = { \
	'voxels_aims_ss' : SurfaceSimpleSegmentDescriptor,
	'voxels_bottom' : BottomSegmentDescriptor,
	'refgravity_center' : GravityCenterSegmentDescriptor,
	'refhull_normal' : OrientationSegmentDescriptor,
	'coordinate_system' : CoordinateSystemSegmentDescriptor,
	'min_distance' : MinDistanceRelationDescriptor,
	'size_global_frequency' : SizeGlobalFrequencyDescriptor,
	'label_global_frequency' : LabelGlobalFrequencyDescriptor,
	'local_frequency' : LocalFrequencyDescriptor,
}

def descriptorFactory(datatypes):
	if isinstance(datatypes, tuple) and len(datatypes) != 1:
		descriptors = [descriptor_map[datatype]() \
					for datatype in datatypes]
		return ComboSegmentDescriptor(descriptors)
	else:	return descriptor_map[datatypes]()
