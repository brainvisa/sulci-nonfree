#!/usr/bin/env python2

import sys, os, pprint
import numpy
from optparse import OptionParser
from soma import aims, aimsalgo
from sulci.common import io, add_translation_option_to_parser
from sulci.models.distribution import VonMisesFisher


################################################################################
def compute_depthmap(subject, graph, skel, write_data=False):
	fat = aims.FoldGraphAttributes(skel, graph)
	fat.prepareDepthMap()
	depth = fat.getDepth()
	depth_factor = fat.getDepthfactor()
	sX = depth.dimX()
	sY = depth.dimY()
	sZ = depth.dimZ()
	sT = depth.dimT()
	converter = aims.Converter_Volume_S16_Volume_FLOAT()
	depth2 = aims.AimsData_FLOAT(sX, sY, sZ, sT)
	converter.convert(depth, depth2)
	depth = depth2
	depth.volume().arraydata()[:] /= depth_factor
	if write_data: aims.Writer().write(depth, '%s_depth.ima' % subject)
	return depth



def compute_orientation(subject, splitmode, sulci, hie, graph,
			skel, depth, write_data=False):
	motion = aims.GraphManip.talairach(graph)
	motion2 = aims.Motion(motion)
	motion2.setTranslation(-motion2.translation())
	writer = aims.Writer()


	# compute gradient of depthmap
	grad = aimsalgo.AimsGradient_FLOAT()
	gX = grad.X(depth)
	gY = grad.Y(depth)
	gZ = grad.Z(depth)
	voxel_size = depth.header()['voxel_size']
	gX.header()['voxel_size'] = voxel_size
	gY.header()['voxel_size'] = voxel_size
	gZ.header()['voxel_size'] = voxel_size
	agX = gX.volume().arraydata()
	agY = gY.volume().arraydata()
	agZ = gZ.volume().arraydata()

	depth_array = depth.volume().arraydata()
	grads = {}
	positions = {}
	centers = {}
	depths = {}
	for sulcus in sulci:
		grads[sulcus] = []
		positions[sulcus] = []
		centers[sulcus] = []
		depths[sulcus] = []

	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		G = []
		P = []
		D = []
		label = v['name']
		ss_map = v['aims_ss'].get()
		size_in = numpy.array([ss_map.sizeX(), ss_map.sizeY(),
							ss_map.sizeZ()])
		c = numpy.array(motion.transform(v['gravity_center'] * size_in))
		for i, p in enumerate(ss_map[0].keys()):
			p2 = (0, p[2], p[1], p[0])
			depth = depth_array[p2]
			g = numpy.array((agX[p2], agY[p2], agZ[p2]))
			gnorm = numpy.linalg.norm(g)
			if gnorm == 0: continue
			# val gnorm should be < distance between two neighbours
			if gnorm > 10: continue
			g = numpy.array(motion2.transform(g * size_in))
			p = numpy.array(motion.transform(p * size_in))
			G.append(g)
			P.append(p)
			D.append(depth)
		if len(G) == 0: continue

		if splitmode == 'labels':
			if len(grads[label]) == 0:
				grads[label].append(G)
				positions[label].append(P)
				centers[label].append([c])
				depths[label].append(D)
			else:
				grads[label][0] += G
				positions[label][0] += P
				centers[label][0] += [c]
				depths[label][0] += D
		elif splitmode == 'nodes':
			grads[label].append(G)
			positions[label].append(P)
			centers[label].append([c])
			depths[label].append(D)

	# compute mean gradient (orientation)
	sigma2 = 9
	orientations = {}
	for sulcus in sulci:
		size = len(grads[sulcus])
		if size == 0: continue
		gs, weights = [], []
		if write_data:
			color = hie.find_color(sulcus)
			go =  {'diffuse' : color}
		lmesh = aims.AimsSurfaceTriangle()
		mmesh = aims.AimsSurfaceTriangle()
		for id in range(size):
			G = grads[sulcus][id]
			c = numpy.mean(centers[sulcus][id], axis=0)
			D = numpy.array(depths[sulcus][id])
			if write_data:
				for i, p in enumerate(positions[sulcus][id]):
					g = G[i]
					n = numpy.linalg.norm(g)
					lmesh += aims.SurfaceGenerator.arrow( \
							p - g, p, n * .1,
							n * .2, 10, 0.3)
			# mean_grad = <grads, exp(-depth/sigma^2)>
			# les valeurs en surface sont plus fiable est moins
			# sensibles aux effets de retrecissements des sillons
			# dans la profondeur
			G = numpy.vstack(G)
			D -= D.min()
			g = numpy.dot(numpy.exp(-D / sigma2), G)
			weight = numpy.linalg.norm(g)
			g /= numpy.linalg.norm(g)
			gs.append(g)
			weights.append(weight)
			if write_data:
				mmesh += aims.SurfaceGenerator.arrow( \
					c - g * 10, c, 1., 2., 10, 0.3)
		orientations[sulcus] = zip(gs, weights)
		if not write_data: continue
		mmesh.header()['material'] = go
		filename = '%s_grad_mean_%s.mesh' % (subject, sulcus)
		writer.write(mmesh, filename)
		filename = '%s_grad_%s.mesh' % (subject, sulcus)
		lmesh.header()['material'] = go
		writer.write(lmesh, filename)
	return orientations



################################################################################
# main + options
def parseOpts(argv):
	description = 'compute label priors\n' \
		'./learn_priors.py --mode subject --graph input_graph ' + \
		'--skeleton input_skeleton -o output_orientations [-w]\n' + \
		'./learn_priors.py --mode mean -o output_mean_orientations ' + \
		'input_orientation1 input_orientation2 ...\n'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-g', '--graph', dest='graphname', metavar='FILE',
		action='store', default = None, help='input sulci graph')
	parser.add_option('-m', '--graphmodel', dest='graphmodelname',
		metavar = 'FILE', action='store',
		default = 'bayesian_graphmodel.dat', help='bayesian model : '\
			'graphical model structure (default : %default)')
	parser.add_option('-s', '--skeleton', dest='skelname', metavar='FILE',
		action='store', default = None, help='input skeleton file')
	parser.add_option('-w', '--write_data', dest='write_data',
		action='store_true', default = False,
		help='write ima/meshes data (default disable)')
	parser.add_option('-o', '--orientations', dest='orientations',
		metavar='FILE', action='store', default = None,
		help='output orientations file')
	parser.add_option('--splitmode', dest='splitmode',
		metavar='FILE', action='store', default = 'labels',
		help="splitmode : 'labels', 'nodes'")
	parser.add_option('--mode', dest='mode',
		metavar='FILE', action='store', default = 'subject',
		help="mode : one among ['subject', 'mean'].\n" + \
		'subject : compute for one subject' + \
		'mean : compute mean orientations of several subjects')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)

	if options.mode == 'subject': main_subject(parser, options, args)
	elif options.mode == 'mean': main_mean(parser, options, args)

def main_subject(parser, options, args):
	if None in [options.graphname, options.skelname,
		options.orientations, options.graphmodelname]:
		print "error: missing option(s)"
		parser.print_help()
		sys.exit(1)

	if not (options.splitmode in ['labels', 'nodes']):
		print "error: unknown split mode '%s'" % options.splitmode
		parser.print_help()
		sys.exit(1)

	# read
	graph = io.load_graphs(options.transfile, [options.graphname])[0]
	graphmodel = io.read_graphmodel(options.graphmodelname)
	skel = aims.Reader().read(options.skelname)
	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	hie = aims.Reader().read(hie_filename)

	# init
	sulci = graphmodel['vertices'].keys()
	subject = os.path.splitext(os.path.basename(\
			graph['aims_reader_filename']))[0]
	depthmapname = '%s_depth.ima' % subject
	if os.path.exists(depthmapname):
		print "find depthmap '%s'" % depthmapname
		depth = aims.Reader().read(depthmapname)
		depth = aims.AimsData_FLOAT(depth)
	else:	depth = compute_depthmap(subject, graph,
				skel, options.write_data)

	# compute
	orientations = compute_orientation(subject, options.splitmode,
			sulci, hie, graph, skel, depth, options.write_data)

	# write
	io.write_pp('orientations', options.orientations, orientations)

def main_mean(parser, options, args):
	if None in [options.orientations]:
		print "error: missing option(s)"
		parser.print_help()
		sys.exit(1)
	orientations_files = args[1:]
	if len(orientations_files) == 0:
		print "error: missing orientations files"
		parser.print_help()
		sys.exit(1)
	res = {}
	for f in orientations_files:
		orientations = io.numpy_read_from_exec(f, 'orientations')
		for label, list in orientations.items():
			for (grad, weight) in list:
				if res.has_key(label):
					res[label][0].append(grad)
					res[label][1].append(weight)
				else:	res[label] = ([grad], [weight])
	mean_orientations = {}
	for label, (grads, weights) in res.items():
		grads = numpy.vstack(grads)
		weights = numpy.array(weights)
		mean_grad = numpy.dot(weights, grads)
		mean_grad /= numpy.linalg.norm(mean_grad)
		distr = VonMisesFisher()
		distr.fit(grads, weights)
		kappa = distr.kappa()
		mu = distr.mu()
		distr.fit(grads)
		kappa = distr.kappa()
		mu = distr.mu()
		mean_orientations[label] = mu, kappa
	io.write_pp('orientations_model', options.orientations,
						mean_orientations)

if __name__ == '__main__' : main()
