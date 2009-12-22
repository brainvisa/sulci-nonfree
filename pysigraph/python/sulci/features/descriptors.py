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
class HullJunctionDescriptor(Descriptor):
	def __init__(self):
		Descriptor.__init__(self)

	def hull_junction(self, vertex):
		edges = vertex.edges()
		edges = [e for e in edges if e.getSyntax() == 'hull_junction']
		if len(edges) == 0: return None
		return edges[0]

	def data(self, motion, vertex):
		e = self.hull_junction(vertex)
		if e is None: return None
		map = e["aims_junction"].get()
		s = numpy.array([map.sizeX(), map.sizeY(), map.sizeZ()])
		vox = [motion.transform(aims.Point3df(p * s)) \
					for p in map[0].keys()]
		if len(vox):
			return numpy.vstack(vox)
		else:	return None

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

	def selected_labels(self, selected_sulci, name1, name2):
		if selected_sulci != None and \
			((name1 not in selected_sulci) or \
			(name2 not in selected_sulci)): return False
		return True

	def euclidian_distance(self, X, Y=None): #from fff2
		if Y == None: Y = X
		if X.shape[1]!=Y.shape[1]:
			raise ValueError, "incompatible dimension " + \
						"for X and Y matrices"
		s1 = X.shape[0]
		s2 = Y.shape[0]
		NX = numpy.reshape(numpy.sum(X*X,1),(s1,1))
		NY = numpy.reshape(numpy.sum(Y*Y,1),(1,s2))
		ED = numpy.repeat(NX,s2,1)
		ED = ED + numpy.repeat(NY,s1,0)
		ED = ED-2*numpy.dot(X,numpy.transpose(Y))
		ED = numpy.maximum(ED,0)
		ED = numpy.sqrt(ED)
		return ED

	def euclidian_directions(self, X, Y):
		'''
    return {(y - x) for x in X for y in Y}
		'''
		sX, sY = X.shape[0], Y.shape[0]
		dim = X.shape[1]
		if sX < sY:
			D = numpy.repeat(Y[None], sX, 0).transpose(1, 0, 2) - X
		else:	D = numpy.repeat(-X[None], sY, 0).transpose(1, 0, 2) + Y
		D = D.reshape(sX * sY, dim)
		N = numpy.sqrt((D ** 2).sum(axis=1)) # norm
		nz = (N != 0)
		D, N = D[nz].T, N[nz]
		D /= N
		return D.T

	def edges_from_graph(self, graph, selected_sulci=None):
		edges = {}
		for e in graph.edges():
			v1, v2 = e.vertices()
			# skip hull junctions
			if v1.getSyntax() != 'fold' or \
				v2.getSyntax() != 'fold':
				continue
			name1, name2 = v1['name'], v2['name']
			if not self.selected_labels(selected_sulci,
					name1, name2): continue
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

	def local_energy(self, distrib, data):
		logli, li = distrib.likelihood(data)
		return -logli

	def data_labels(self, data, inverse):
		return data # default : symmetric

	def data_potential_edges(self, motion, edges):
		return self.data_edges(motion, edges)

	def potential_matrix(self, motion, distribs, edge_infos,
				avalaible_labels, potential=True):
		'''
    potential : True store -log(likelihood(data(edge))) for each label pair.
                False strore    likelihood(data(edge))       "     "
		'''
		(r1, r2), ((v1, v2), edges) = edge_infos
		data = self.data_potential_edges(motion, edges)
		labels_1, labels_2 = avalaible_labels[r1], avalaible_labels[r2]
		P = numpy.zeros((len(labels_1), len(labels_2)), numpy.longdouble)
		if data is None:
			if not potential: P += numpy.inf
			return P, (r1, r2)
		for s1, l_1 in enumerate(labels_1):
			for s2, l_2 in enumerate(labels_2):
				if l_1 > l_2:
					relation, inverse = (l_2, l_1), True
				else:	relation, inverse = (l_1, l_2), False
				if not distribs.has_key(relation):
					if l_1 == l_2:
						relation = 'default_intra'
					else:	relation = 'default_inter'
				distrib = distribs[relation]
				data2 = self.data_labels(data, inverse)
				en = self.local_energy(distrib, data2)
				if potential:
					P[s1, s2] = en
				else:	P[s1, s2] = numpy.exp(-en)
		return P, (r1, r2)

	def data_from_graphs(self, graphs, input_motions,
				no_tal, selected_sulci=None):
		data = {}
		count = {}
		for i, g in enumerate(graphs):
			if no_tal:
				motion = aims.Motion()
				motion.setToIdentity()
			else:	motion = aims.GraphManip.talairach(g)
			if input_motions:
				motion = input_motions[i] * motion
			graph_edges = self.edges_from_graph(g, selected_sulci)
			for (r1, r2), ((v1, v2), edges) in graph_edges.items():
				name1, name2 = v1['name'], v2['name']
				d = self.data_edges(motion, edges)
				if d is None: continue
				inverse = (name1 > name2)
				d2 = self.data_labels(d, inverse)
				# order names
				if name1 > name2: name1, name2 = name2, name1
				key = (name1, name2)
				if data.has_key(key):
					data[key].append(d2)
					count[key] += 1
				else:
					data[key] = [d2]
					count[key] = 1
		for rel, D in data.items():
			if len(D):
				data[rel] = numpy.vstack(D), count[rel]
			else:	del data[rel]
		return data


class ComboRelationsDescriptor(RelationDescriptor):
	def __init__(self, descriptors):
		RelationDescriptor.__init__(self)
		self._descriptors = {}
		for descr in descriptors:
			self._descriptors[descr.name()] = descr

	def potential_matrix(self, motion, distribs, edge_infos,
				avalaible_labels, potential=True):
		P = numpy.longdouble(0.)
		for datatype in self._descriptors.keys():
			subdistribs = distribs[datatype]
			Pi, ind = self._descriptors[datatype].potential_matrix(\
					motion, subdistribs, edge_infos,
						avalaible_labels, potential)
			P += Pi
		return P, ind

################################################################################
class MinDistanceRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'min_distance'

	def data_edges(self, motion, edges):
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


class AllMinDistanceRelationDescriptor(MinDistanceRelationDescriptor):
	'''
    For each label pair (l1, l2), For each subject k :
    For each segment si1 with label l1
        dk_l1->l2 = min dist(si1, sj2) for sj2 in segments with label l2
    For each segment sj2 with label l2
        dk_l2->l1 = min dist(si1, sj2) for si1 in segments with label l1
    get the min of this distance for given (subject, labels pair).

    if l1 == l2 : only related (through relations edges) segments are considered
	'''
	def __init__(self):
		MinDistanceRelationDescriptor.__init__(self)
		self._name = 'all_min_distance'
		self._descr_hull = HullJunctionDescriptor()
		self._descr_ss = SurfaceSimpleSegmentDescriptor()

	def group_segments_per_labels(self, g):
		group = {}
		for v in g.vertices():
			if v.getSyntax() != 'fold': continue
			label = v['name']
			if group.has_key(label):
				group[label].append(v)
			else:	group[label] = [v]
		return group

	def cc_has_hull_junction(self, cc):
		b = True
		for v in cc:
			if self._descr_hull.hull_junction(v) is None:
				b = False
		return b

	def data_vertices(self, descr, motion, v1, v2):
		import time

		X = descr.data(motion, v1)
		Y = descr.data(motion, v2)
		C = self.euclidian_distance(X, Y)
		return numpy.sqrt(numpy.min(C))

	def compute(self, motion, g1, g2, connected):
		(name1, segments1), (name2, segments2) = g1, g2
		if not connected and self.cc_has_hull_junction(segments1) and\
			self.cc_has_hull_junction(segments2):
			descr = self._descr_hull
		else:	descr = self._descr_ss
		dist = []
		if name1 == name2 :
			pairs = []
			for s1 in segments1:
				r1 = s1['index']
				edges = s1.edges()
				for e in edges:
					si, sj = e.vertices()
					ri = si['index']
					rj = sj['index']
					if ri != r1:
						rj, ri = ri, rj
						sj, si = si, sj
					if ri > rj: continue
					if sj.getSyntax() != 'fold': continue
					if sj['name'] != name1: continue
					if ri < rj:
						pairs.append((si, sj))
					else:	pairs.append((sj, si))
			pairs = set(pairs)
			for (s1, s2) in pairs:
				d = self.data_vertices(descr, motion, s1, s2)
				dist.append(d)
		else:
			size1, size2 = len(segments1), len(segments2)
			C = numpy.zeros((size1, size2))
			for i, s1 in enumerate(segments1):
				dmin = numpy.inf
				for j, s2 in enumerate(segments2):
					d = self.data_vertices(descr,
							motion, s1, s2)
					C[i, j] = d
			# take min according to each sulci
			l = zip(numpy.arange(size1), C.argmin(axis=1))
			l += zip(C.argmin(axis=0), numpy.arange(size2))
			s = set(l)
			for ind in s: dist.append(C[ind])
		if name1 != name2 : return [numpy.min(dist)]
		return dist

	def label_gravity_center(self, segments):
		w = 0.
		g = numpy.zeros(3)
		for v in segments:
			s = v['refsize']
			w += s
			g += v['refgravity_center'].arraydata() * s
		g /= w
		return g

	def connected_cc(self, ci, cj):
		ri = [vi['index'] for vi in ci]
		for vj in cj:
			rj = vj['index']
			for e in vj.edges():
				v1, v2 = e.vertices()
				r1, r2 = v1['index'], v2['index']
				if v1 != vj:
					if r1 in ri: return True
				else:
					if r2 in ri: return True
		x = [vj['index'] for vj in cj]
		return False

	def data_from_graphs(self, graphs, input_motions,
				no_tal, selected_sulci=None):
		data = {}
		count = {}
		# If the squared distance between the gravity centers of
		# sulci is over this threshold the model is not computed
		dist2_th = 1000
		for i, g in enumerate(graphs):
			if no_tal:
				motion = aims.Motion()
				motion.setToIdentity()
			else:	motion = aims.GraphManip.talairach(g)
			if input_motions:
				motion = input_motions[i] * motion
			group = self.group_segments_per_labels(g)
			for g1 in group.items():
				(name1, segments1) = g1
				c1 = self.label_gravity_center(segments1)
				for g2 in group.items():
					(name2, segments2) = g2
					if not self.selected_labels(\
						selected_sulci, name1, name2):
						continue
					if name1 > name2: continue
					c2 = self.label_gravity_center(\
							segments2)
					dist = ((c1 - c2) ** 2).sum()
					connected = self.connected_cc(segments1,
								segments2)
					if not connected and dist > dist2_th:
						continue
					print name1, name2
					D = self.compute(motion, g1, g2,
								connected)
					if D is None: continue
					key = (name1, name2)
					if data.has_key(key):
						data[key] += D
						count[key] += 1
					else:
						data[key] = D
						count[key] = 1

		print "-- end data from graphs --"
		for rel, D in data.items():
			if len(D):
				data[rel] = numpy.vstack(D), count[rel]
			else:	del data[rel]
		return data


class AllConnectedDistanceRelationDescriptor(AllMinDistanceRelationDescriptor):
	'''
    For each pair of sulci (s1 and s2), connect with the Hungarian algorithm
    a set of n voxels of s1 with a set of n voxels of s2.
	'''
	def __init__(self):
		AllMinDistanceRelationDescriptor.__init__(self)
		self._name = 'all_connected_distance'
		self._synth = aims.set_STRING(["junction", "plidepassage"])

	def data_edges(self, motion, edges):
		if edges.has_key('cortical'):
			edge = edges['cortical']
			return edge['refmean_connected_dist']
		else:	return None

	def cc_size(self, has_hull, cc):
		s = 0.
		if has_hull:
			for v in cc: s += self._descr_hull.hull_junction(v)['refsize']
		else:
			for v in cc: s += v['refsurface_area']
		return s

	def cc_to_X(self, motion, descr, cc):
		X = []
		for v in cc: X.append(descr.data(motion, v))
		return numpy.vstack(X)

	def data_cc(self, motion, (ci, cj)):
		hull_junction_mode = self.cc_has_hull_junction(ci) and \
					self.cc_has_hull_junction(cj)
		if hull_junction_mode:
			descr = self._descr_hull
		else:	descr = self._descr_ss
		X = self.cc_to_X(motion, descr, ci)
		Y = self.cc_to_X(motion, descr, cj)
		sX, sY = len(X), len(Y)
		mi = min(sX, sY)
		ma = max(sX, sY)
		th_min = 10
		th_max = 40
		ma2 = (ma * th_min) / mi
		if ma2 > th_max:
			ma2 = th_max
			mi2 = (mi * ma2) / ma
			if mi2 <= th_min: mi2 = th_min
		else:	mi2 = th_min
		if sX < sY:
			nX, nY = mi2, ma2
		else:	nX, nY = ma2, mi2
		import scipy.cluster as C
		X, lX = C.vq.kmeans(X, nX)
		Y, lY = C.vq.kmeans(Y, nY)
		return self.data_cc_specific(X, Y)

	def data_intra(self, X, Y):
		return numpy.sqrt(((X - Y) ** 2).sum())

	def data_cc_specific(self, X, Y):
		nX, nY = len(X), len(Y)
		C = self.euclidian_distance(X, Y)
		if nX != nY:
			c_inf = numpy.max(C) * 10000
			if nX < nY:
				P = numpy.zeros((nY-nX, nY)) + c_inf
				C = numpy.vstack((C, P))
			else:
				P = numpy.zeros((nX, nX-nY)) + c_inf
				C = numpy.hstack((C, P))
		import munkres
		m = munkres.Munkres()
		indexes = m.compute(C.copy())
		dist = []
		#m3 = aims.AimsSurfaceTriangle()
		for (i,j) in indexes:
			if i >= nX or j >= nY: continue
			dist.append(C[i, j])
		#	print C[i,j]
		#	if C[i,j] > 60: continue
		#	m3 += aims.SurfaceGenerator.cylinder(X[i], Y[j], 0.3, 0.3, 10, 1, 1)
		#aims.Writer().write(m3, "plop3_0.mesh")

		#m1 = aims.AimsSurfaceTriangle()
		#for x in X:
		#	m1 += aims.SurfaceGenerator.sphere(x, 1, 32)
		#aims.Writer().write(m1, "plop1_0.mesh")
		#m2 = aims.AimsSurfaceTriangle()
		#for x in Y:
		#	m2 += aims.SurfaceGenerator.sphere(x, 1, 32)
		#aims.Writer().write(m2, "plop2_0.mesh")

		if len(dist):
			return numpy.vstack(dist)
		else:	return None

	def compute(self, motion, g1, g2, connected):
		import scipy.cluster as C
		import fff2.graph.graph as G
		(name1, segments1), (name2, segments2) = g1, g2
		dist = []
		n = 0.02 # density of centroids
		if name1 == name2 :
			import sigraph
			from soma import aims
			s = aims.set_VertexPtr()
			for seg in segments1: s.add(seg)
			cc = sigraph.VertexClique.connectivity(s, self._synth)
			for ci in cc:
				X = self.cc_to_X(motion, self._descr_ss, ci)
				size = self.cc_size(False, ci)
				nX = int(n * size)
				if nX < 2: continue
				X, lX = C.vq.kmeans(X, nX)
				nX = len(X)
				if nX < 2: continue
				g = G.WeightedGraph(nX)
				e = g.mst(X)
				for (id1, id2) in g.get_edges():
					d = self.data_intra(X[id1], X[id2])
					dist.append(d)
		else: 
			p = segments1, segments2
			dist.append(self.data_cc(motion, p))
		return dist


class AllConnectedMeanDistanceRelationDescriptor(\
		AllConnectedDistanceRelationDescriptor):
	'''
    For each pair of sulci (s1 and s2), connect with the Hungarian algorithm
    a set of n voxels of s1 with a set of n voxels of s2.
	'''
	def __init__(self):
		AllConnectedDistanceRelationDescriptor.__init__(self)
		self._name = 'all_connected_mean_distance'
		self._synth = aims.set_STRING(["junction", "plidepassage"])

	def compute(self, motion, g1, g2, connected):
		D = AllConnectedDistanceRelationDescriptor.compute(self,
						motion, g1, g2, connected)
		if D is None: return None
		return [numpy.mean(D)]


################################################################################
class ConnexionLengthRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'connexion_length'

	def data_edges(self, motion, edges):
		if edges.has_key('junction'):
			return edges['junction']['reflength']
		else:	return None

	def edges_from_graph(self, graph, selected_sulci=None):
		edges = {}
		for e in graph.edges():
			v1, v2 = e.vertices()
			# get only junction edges
			if e.getSyntax() != 'junction': continue
			name1, name2 = v1['name'], v2['name']
			if not self.selected_labels(selected_sulci,
					name1, name2): continue
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

################################################################################
class AllDirectionsPairRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'all_directions_pair'

	def data_labels(self, data, inverse):
		# antisymetric data
		if inverse:
			s = -1.
		else:	s = 1.
		return s * data

	def data_potential_edges(self, motion, edges):
		e = edges.values()[0]
		v1, v2 = e.vertices()
		name1, name2 = v1['name'], v2['name']
		g1 = v1['refgravity_center'].arraydata()
		g2 = v2['refgravity_center'].arraydata()
		dir = g2 - g1

		n = numpy.sqrt((dir ** 2).sum()) # norm
		if n: return dir / n
		return None

	def data_edges(self, motion, edges):
		e = edges.values()[0]
		v1, v2 = e.vertices()

		# get voxels
		map1 = v1['aims_ss'].get()
		s1 = numpy.array([map1.sizeX(), map1.sizeY(), map1.sizeZ()])
		X = numpy.array([motion.transform(aims.Point3df(p * s1)) \
					for p in map1[0].keys()])
		map2 = v2['aims_ss'].get()
		s2 = numpy.array([map2.sizeX(), map2.sizeY(), map2.sizeZ()])
		Y = numpy.array([motion.transform(aims.Point3df(p * s2)) \
					for p in map2[0].keys()])

		# kmeans to reduce the number of considered voxels
		sX, sY = len(X), len(Y)
		nX, nY = sX / 30, sY / 30
		if nX <= 10: nX = 10
		if nY <= 10: nY = 10

		import scipy.cluster as C
		X, lX = C.vq.kmeans(X, nX)
		Y, lY = C.vq.kmeans(Y, nY)
		data = self.euclidian_directions(X, Y)

		if data.shape[0]:
			return data
		else:	return None



class GravityCentersDirections(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'gravity_centers_directions'

	def data_labels(self, data, inverse):
		# antisymetric data
		if inverse:
			s = -1.
		else:	s = 1.
		return s * data

	def data_edges(self, motion, edges):
		e = edges.values()[0]
		v1, v2 = e.vertices()
		name1, name2 = v1['name'], v2['name']
		g1 = v1['refgravity_center'].arraydata()
		g2 = v2['refgravity_center'].arraydata()
		dir = g2 - g1

		n = numpy.sqrt((dir ** 2).sum()) # norm
		if n: return dir / n
		return None


class AllConnectedDirectionRelationDescriptor(AllConnectedDistanceRelationDescriptor):
	'''
    For each pair of sulci (s1 and s2), connect with the Hungarian algorithm
    a set of n voxels of s1 with a set of n voxels of s2.
	'''
	def __init__(self):
		AllConnectedDistanceRelationDescriptor.__init__(self)
		self._name = 'all_connected_direction'

	def data_labels(self, data, inverse):
		# antisymetric data
		if inverse:
			s = -1.
		else:	s = 1.
		return s * data

	def data_potential_edges(self, motion, edges):
		e = edges.values()[0]
		v1, v2 = e.vertices()
		name1, name2 = v1['name'], v2['name']
		g1 = v1['refgravity_center'].arraydata()
		g2 = v2['refgravity_center'].arraydata()
		dir = g2 - g1

		n = numpy.sqrt((dir ** 2).sum()) # norm
		if n: return dir / n
		return None

	def data_cc_specific(self, X, Y):
		data = self.euclidian_directions(X, Y)
#		m1 = aims.AimsSurfaceTriangle()
#		for x in X:
#			m1 += aims.SurfaceGenerator.sphere(x, 1, 32)
#		aims.Writer().write(m1, "plop1_0.mesh")
#		m2 = aims.AimsSurfaceTriangle()
#		for x in Y:
#			m2 += aims.SurfaceGenerator.sphere(x, 1, 32)
#		aims.Writer().write(m2, "plop2_0.mesh")
#		m3 = aims.AimsSurfaceTriangle()
#		county = numpy.zeros(len(Y))
#		countx = numpy.zeros(len(X))
#		for j, x in enumerate(X):
#			dist = []
#			for y in Y:
#				d = numpy.sqrt(((x - y) ** 2).sum())
#				dist.append(d)
#
#			ind = numpy.argsort(dist)
#			for i in ind[:3]:
#				#if dist[i] > 50: continue
#				if county[i] > 3: continue
#				m3 += aims.SurfaceGenerator.cylinder(x, Y[i], 0.3, 0.3, 10, 1, 1)
#				county[i] += 1
#				countx[j] += 1
#		for y in Y:
#			dist = []
#			if county[i] > 3: continue
#			for x in X:
#				d = numpy.sqrt(((x - y) ** 2).sum())
#				dist.append(d)
#			ind = numpy.argsort(dist)
#			for i in ind[:3]:
#				if countx[i] > 3: continue
#				m3 += aims.SurfaceGenerator.cylinder(x, Y[i], 0.3, 0.3, 10, 1, 1)
#				county[i] += 1
#				countx[j] += 1
#		aims.Writer().write(m3, "plop3_0.mesh")

		if data.shape[0]:
			return data
		else:	return None

	def data_intra(self, X, Y):
		dir = Y - X
		n = numpy.sqrt((dir ** 2).sum())
		if n: return dir / n
		return None


################################################################################
class AllDistancesPairRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'all_distances_pair'

	def data_edges(self, motion, edges):
		e = edges.values()[0]
		v1, v2 = e.vertices()

		map1 = v1['aims_ss'].get()
		s1 = numpy.array([map1.sizeX(), map1.sizeY(), map1.sizeZ()])
		X = numpy.array([motion.transform(aims.Point3df(p * s1)) \
					for p in map1[0].keys()])
		map2 = v2['aims_ss'].get()
		s2 = numpy.array([map2.sizeX(), map2.sizeY(), map2.sizeZ()])
		Y = numpy.array([motion.transform(aims.Point3df(p * s2)) \
					for p in map2[0].keys()])

		sX, sY = len(X), len(Y)
		nX, nY = sX / 30, sY / 30
		if nX <= 10: nX = 10
		if nY <= 10: nY = 10

		import scipy.cluster as C
		X, lX = C.vq.kmeans(X, nX)
		Y, lY = C.vq.kmeans(Y, nY)

		C = self.euclidian_distance(X, Y).reshape(-1, 1)
		if len(C):
			return C
		else:	return None


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
		self._w = None

	def get_data(self): return self._data

	def set_data(self, data): self._data = data

	def update_data(self, segment, label1, label2):
		s = self._sizes[segment]
		self._data[label1] -=  s # old
		self._data[label2] +=  s # new

	def likelihood(self, distrib, label):
		return self.full_likelihood(distrib, None)

	def full_likelihood(self, distrib, taglabels):
		logli, li = distrib.likelihood(self._data)
		return logli / self._w
		

class LabelGlobalFrequencyDescriptor(GlobalFrequencyDescriptor):
	def __init__(self):
		GlobalFrequencyDescriptor.__init__(self)
		self._name = 'label_global_frequency'
		self._data = None

	def update_data(self, segment, label1, label2):
		self._data[label1] -=  1 # old
		self._data[label2] +=  1 # new

	def compute_data(self, graph, taglabels, availablelabels,
						segments, labels):
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			ind = v['index']
		label_map = {}
		X = numpy.zeros(len(labels), numpy.longdouble)
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

	def compute_data(self, graph, taglabels, availablelabels,
						segments, labels):
		sizes = {}
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			size = v['refsize']
			ind = v['index']
			sizes[ind] = size
		label_map = {}
		X = numpy.zeros(len(labels), numpy.longdouble)
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
# Prior descriptors
################################################################################
class SulciDescriptor(Descriptor):
	def __init__(self):
		Descriptor.__init__(self)


class RegistredSulciDescriptor(SulciDescriptor):
	def __init__(self):
		SulciDescriptor.__init__(self)
		self._name = 'registred_spam_ss'
		self._distribs = {}
		self._cache = {}
		self._descr = SurfaceSimpleSegmentDescriptor()
		self._data = {}

	def init(self, motion, distrib, labels, segments):
		# init cache
		for s in segments:
			id = s['index']
			X = self._descr.data(motion, s)
			n = len(X) / 50.
			if n < 2: n = 2
			self._data[id] = X
		for i, label in enumerate(labels):
			self._cache[i] = {}
			self._distribs[i] = distrib['vertices'][label]

	def likelihood(self, distrib, label, segments):
		if len(segments) == 0: return 40. #FIXME
		from sulci.registration.spam import SpamRegistration

		# in cache ?
		cache_key = tuple(sorted(segments.keys()))
		cache_label = self._cache[label]
		if cache_label.has_key(cache_key):
			return cache_label[cache_key]
		# not in cache
		X = []
		for id, s in segments.items(): X.append(self._data[id])
		X = numpy.vstack(X)
		g = X.mean(axis=0)
		spam = self._distribs[label]
		#FIXME : replace None by values
		spamreg = SpamRegistration(spam, X.T, g[None].T,None,None, None)
		R, t = spamreg.optimize_powell(eps=0.1)
		en = spamreg._energy
		cache_label[cache_key] = en # insert in cache
		return en

################################################################################
# Descriptor map/factory
################################################################################
descriptor_map = { \
	# segments
	'voxels_aims_ss' : SurfaceSimpleSegmentDescriptor,
	'voxels_bottom' : BottomSegmentDescriptor,
	'refgravity_center' : GravityCenterSegmentDescriptor,
	'refhull_normal' : OrientationSegmentDescriptor,
	'coordinate_system' : CoordinateSystemSegmentDescriptor,
	# relations
	'min_distance' : MinDistanceRelationDescriptor,
	'all_min_distance' : AllMinDistanceRelationDescriptor,
	'all_connected_distance' : AllConnectedDistanceRelationDescriptor,
	'all_connected_mean_distance' : \
				AllConnectedMeanDistanceRelationDescriptor,
	'all_distances_pair' : AllDistancesPairRelationDescriptor,
	'connexion_length' : ConnexionLengthRelationDescriptor,
	'gravity_centers_directions' : GravityCentersDirections,
	'all_directions_pair' : AllDirectionsPairRelationDescriptor,
	'all_connected_direction' : AllConnectedDirectionRelationDescriptor,
	# sulci
	'registred_spam_ss' : RegistredSulciDescriptor,
	# priors
	'size_global_frequency' : SizeGlobalFrequencyDescriptor,
	'label_global_frequency' : LabelGlobalFrequencyDescriptor,
	'local_frequency' : LocalFrequencyDescriptor,
}

def descriptorFactory(datatypes, level='segments'):
	if isinstance(datatypes, tuple) and len(datatypes) != 1:
		descriptors = [descriptor_map[datatype]() \
					for datatype in datatypes]
		if level == 'segments':
			return ComboSegmentDescriptor(descriptors)
		elif level == 'relations':
			return ComboRelationsDescriptor(descriptors)
	else:	return descriptor_map[datatypes]()
