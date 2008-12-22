#!/usr/bin/env python
import os, sys, numpy, pprint, copy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.registration.common import transformation_to_motion
from sulci.registration.spam import SpamMixtureRegistration, SpamRegistration
from sulci.registration.transformation import RigidTransformation, \
					SulcusWiseRigidTransformations


################################################################################
def compute_gravity_centers(sulci_set, input_motions=None,
				local_transformations=None):
	'''
    Compute sulcuswise gravity centers.
    sulci_set :     sulcuswise/subjectwise info (local gravity centers
                    in Talairach space)
    input_motions : subjectwise global motion (applied first)
    local_transformations : sulcuswise/subjectwise transformations (applied
                            in a second time)
	'''
	gravity_centers = {}
	for sulcus, data in sulci_set.items():
		db = []
		for i, h in data.items():
			g = h['gravity_center']
			if input_motions:
				motion = input_motions[i]
			else:
				motion = aims.Motion()
				motion.setToIdentity()
			if transformations:
				local_trans = local_transformations[i][sulcus]
				local_motion = local_trans.to_motion()
				motion = local_motion * motion
			db.append(motion.transform(g))
		distr = distribution.Gaussian()
		gravity_centers[sulcus] = distr.fit(db)
	return gravity_centers

################################################################################
class SpamLearner(object):
	def __init__(self, graphs, input_motions, ss, selected_sulci,
			sigmas, sigma_value, fromlog):
		self._graphs = graphs
		self._input_motions = input_motions
		self._ss = ss
		self._selected_sulci = selected_sulci
		self._sigmas = sigmas
		self._sigma_value = sigma_value
		self._fromlog = fromlog
		self._init_data()

	def get_labels(self): return self._labels

	def _init_data(self):
		subjects_data = []
		labels = set()
		sulci_set = {}
		motions = []

		if self._ss:
			bucket_name = 'aims_ss'
		else:	bucket_name = 'aims_bottom'
		gravity_centers = {}

		for i, g in enumerate(self._graphs):
			motion = aims.GraphManip.talairach(g)
			filename = g['aims_reader_filename']
			if self._input_motions:
				motion = self._input_motions[i] * motion
			motions.append(motion)
			data = {}
			for v in g.vertices():
				if v.getSyntax() != 'fold': continue
				name = v['name']
				if self._selected_sulci != None and \
					name not in self._selected_sulci:
					continue
				labels.add(name)
				c = v['refgravity_center']
				w = v['refsize']
				if sulci_set.has_key(name):
					h = sulci_set[name]
					if h.has_key(i):
						h[i]['vertices'] += [v]
						h[i]['gravity_center'][0].append(c)
						h[i]['gravity_center'][1].append(w)
					else:
						h[i] = {
							'vertices' : [v],
							'name' : filename,
							'gravity_center' : \
								[[c], [w]]
						}
				else:
					sulci_set[name] = {
						i : {'vertices' : [v],
						     'name' : filename,
						     'gravity_center' : \
								[[c], [w]] }
						}
				ss_map = v[bucket_name].get()
				size_in = numpy.array([ss_map.sizeX(),
					ss_map.sizeY(), ss_map.sizeZ()])
				X = []
				for p_in in ss_map[0].keys():
					p_in = aims.Point3df(p_in * size_in)
					p_out = motion.transform(p_in)
					X.append(p_out)
				if data.has_key(name):
					data[name].append(X)
				else:	data[name] = [X]
			for sulcus, data in sulci_set.items():
				d = data[i]
				C = d['gravity_center'][0]
				W = d['gravity_center'][1]
				d['gravity_center'] = numpy.dot(W, C)

			subjects_data.append(data)
		self._gravity_centers = compute_gravity_centers(sulci_set,
							self._input_motions)
		self._subjects_data = subjects_data
		self._sulci_set = sulci_set
		self._labels = labels
		self._motions = motions


class GlobalSpamLearner(SpamLearner):
	def __init__(self, graphs, input_motions, ss, selected_sulci,
			sigmas, sigma_value, fromlog):
		SpamLearner.__init__(self, graphs, input_motions, ss,
			selected_sulci, sigmas, sigma_value, fromlog)

	def _init_data(self):
		SpamLearner._init_data(self)
			
		# compute weights, groups, priors (voxels/segments)
		weights = []
		groups = []
		for i, g in enumerate(self._graphs):
			data = self._subjects_data[i]
			subject_voxels = []
			subweights = [[] for k in range(len(self._labels))]
			subgroups = []
			for j, label in enumerate(self._labels):
				try:
					segments = data[label]
				except KeyError:
					segments = []
				size = len(segments)
				for k, w in enumerate(subweights):
					if k == j:
						w += [1.]
					else:	w += [0.]
				label_voxels = []
				for s in segments: label_voxels += s
				voxels_size = len(label_voxels)
				subject_voxels += label_voxels
				subgroups += [j] * voxels_size
			X = numpy.asmatrix(numpy.vstack(subject_voxels))
			self._subjects_data[i] = X
			weights.append(numpy.asmatrix(numpy.vstack(subweights)))
			groups.append(numpy.array(subgroups))

		# store data
		self._weights = weights
		self._groups = groups

	def learn_spams(self, motions, sulci_set):
		models = []
		for sulcus in self._labels:
			if self._selected_sulci != None and \
				sulcus not in self._selected_sulci:
				continue
			try: infos = sulci_set[sulcus]
			except KeyError:
				s = distribution.Dummy(1.)
			else:	
				sigma = self._sigmas['sulci'].get(sulcus,
							self._sigma_value)
				s = distribution_aims.Spam(sigma, self._fromlog)
				s.fit_graphs(infos, motions, self._ss)
			models.append(s)
		mixture = distribution_aims.SpamMixtureModel(models, None)
		return mixture

	def register(self, i, mixture, verbose):
		eps = 0.001
		X = self._subjects_data[i]
		reg = SpamMixtureRegistration(X.T, self._weights[i],
				mixture, self._groups[i], verbose - 1)
		old_R, old_t = self._old_params[i]
		reg.set_initialization(old_R, old_t)
		R, t = reg.optimize(eps=eps, mode='powell')
		self._old_params[i] = R, t
		energy = reg.energy()
		trans = RigidTransformation(R, t)
		return trans, energy

	def learn_onestep(self, motions, verbose=0):
		transformations = []
		total_energy = 0.
		# spams learning
		if verbose > 0: print "spams learning..."
		mixture = self.learn_spams(motions, self._sulci_set)
		# registration
		if verbose > 0: print "registration..."
		for i, g in enumerate(self._graphs):
			if verbose > 1:
				print "graph %d/%d" % (i + 1, len(self._graphs))
			trans, energy = self.register(i, mixture, verbose - 1)
			total_energy += energy
			transformations.append(trans)
		total_energy /= len(self._graphs)
		return transformations, total_energy

	def learn(self, miniter=0, maxiter=numpy.inf, verbose=0):
		energy_eps = 1.
		cur_motions = self._motions
		n = 0
		self._old_params = []
		id = numpy.asmatrix(numpy.identity(3))
		z = numpy.asmatrix(numpy.zeros((3, 1)))
		transformations = []
		for i in range(len(self._graphs)):
			R, t = id.copy(), z.copy()
			trans = RigidTransformation(R, t)
			self._old_params.append((R, t))
			transformations.append(trans)
		old_energy = numpy.inf
		while 1:
			if verbose > 0:
				print "*********"
				print "**  %d   " % n
				print "*********"
			n += 1
			transformations, energy = self.learn_onestep(\
						cur_motions, verbose)
			if verbose > 0: print "subjects mean energy :", energy
			if (n >= miniter) and \
				(old_energy - energy < energy_eps):
				break
			else:   old_energy = energy
			# compute transformation from subject space
			cur_motions = [(transformations[i].to_motion() * \
				self._motions[i]) \
				for i in range(len(self._graphs))]
			if n >= maxiter: break
		mixture = self.learn_spams(cur_motions, self._sulci_set)
		
		return transformations, mixture


class GlobalSpamLearnerLoo(GlobalSpamLearner):
	def __init__(self, graphs, input_motions, ss, selected_sulci,
			sigmas, sigma_value, fromlog):
		GlobalSpamLearner.__init__(self, graphs, input_motions, ss,
			selected_sulci, sigmas, sigma_value, fromlog)

	def learn_spams_loo(self, i, motions):
		sulci_set = copy.copy(self._sulci_set)
		for label, h in sulci_set.items():
			h2 = copy.copy(h)
			try: del h2[i]
			except KeyError: pass
			if len(h2) == 0:
				del sulci_set[label]
			else:	sulci_set[label] = h2
		mixture = self.learn_spams(motions, sulci_set)
		return mixture

	def learn_onestep(self, motions, verbose=0):
		eps = 1
		transformations = []
		total_energy = 0.
		for i, g in enumerate(self._graphs):
			if verbose > 1:
				print "graph %d/%d" % (i + 1, len(self._graphs))
			# learn spams
			mixture = self.learn_spams_loo(i, motions)
			# registration
			trans, energy = self.register(i, mixture, verbose)
			total_energy += energy
			transformations.append(trans)
		total_energy /= len(self._graphs)
		return transformations, total_energy


class LocalSpamLearner(SpamLearner):
	def __init__(self, graphs, input_motions, ss, selected_sulci,
			sigmas, sigma_value, fromlog, dir_priors):
		SpamLearner.__init__(self, graphs, input_motions, ss,
				selected_sulci, sigmas, sigma_value, fromlog)
		self._dir_priors = dir_priors

	def _init_data(self):
		SpamLearner._init_data(self)

		data_by_label = {}
		for sulcus in self._labels: data_by_label[sulcus] = []
		for i, g in enumerate(self._graphs):
			data = self._subjects_data[i]
			for label in self._labels:
				try:	segments = data[label]
				except KeyError:
					segments = None
				if segments is not None:
					segments = numpy.vstack(segments)
				data_by_label[label].append(segments)
		self._data_by_label = data_by_label
		del self._subjects_data

	def register(self, sulcus, i, X, spam, dir_prior, verbose=0):
		eps = 0.001
		if dir_prior:
			kappa, mu = dir_prior.kappa(), dir_prior.mu()
			dir_var = 10. / kappa
			angle_var = (numpy.pi / 4.) ** 2
			t_var = 100.
		else:	dir_var = mu = angle_var = t_var = None
		g = numpy.asarray(self._gravity_centers[sulcus].mean()).T

		reg = SpamRegistration(spam, g, X.T,
			R_angle_var=angle_var, R_dir_var=dir_var,
			R_dir_mean=mu, t_var=t_var, verbose=verbose-1)
		old_R, old_t = self._old_params[i]
		reg.set_initialization(old_R, old_t)
		R, t = reg.optimize(eps=eps, mode='powell')
		self._old_params[i] = R, t
		energy = reg.energy()
		# global expression of local sulcus transformation 
		# from t_g : local translation expressed in g referential
		# R.(X - g) + t_g + g = R.X + (t_g + g - R.g)
		# t0 : translation expressed in global referential : (g = 0)
		t0 = t + g - R * g
		global_trans = RigidTransformation(R, t0)
		return global_trans, energy

	def learn_spam(self, sulcus, motions, sulci_set):
		infos = sulci_set[sulcus]
		sigma = self._sigmas['sulci'].get(sulcus, self._sigma_value)
		spam = distribution_aims.Spam(sigma, self._fromlog)
		spam.fit_graphs(infos, motions, self._ss)
		return spam

	def learn_sulcus_onestep(self, sulcus, motions, verbose=0):
		transformations = []
		total_energy = 0.
		if self._dir_priors:
			dir_prior = self._dir_priors['vertices'][sulcus]
		else:	dir_prior = None
		X_subjects = self._data_by_label[sulcus]
		if verbose > 0: print "spams learning..."
		spam = self.learn_spam(sulcus, motions, self._sulci_set)
		for i, g in enumerate(self._graphs):
			if verbose > 1:
				print "graph %d/%d" % (i + 1, len(self._graphs))
			X = X_subjects[i]
			if X is None:
				transformations.append((None, None))
				if verbose > 1: print "(skip graph)"
				continue
			global_trans, energy = self.register(sulcus, i, X, spam,
					dir_prior, verbose - 1)
			if verbose > 1: print "graph energy :", energy
			total_energy += energy
			transformations.append(global_trans)
		total_energy /= len(self._graphs)
		return transformations, total_energy

	def learn_sulcus(self, sulcus, miniter=0, maxiter=numpy.inf, verbose=0):
		energy_eps = 0.1
		cur_motions = self._motions
		n = 0
		self._old_params = []
		id = numpy.asmatrix(numpy.identity(3))
		z = numpy.asmatrix(numpy.zeros((3, 1)))
		transformations = []
		for i in range(len(self._graphs)):
			R, t = id.copy(), z.copy()
			trans = RigidTransformation(R, t)
			self._old_params.append((R, t))
			transformations.append(trans) # global transformation
		old_energy = numpy.inf
		n = 0
		while 1:
			if verbose > 0:
				print "*********"
				print "**  %d   " % n
				print "*********"
			n += 1
			transformations, energy = \
				self.learn_sulcus_onestep(sulcus,
						cur_motions, verbose - 1)
			if verbose > 0: print "subjects mean energy :", energy
			if (n >= miniter) and \
				(old_energy - energy < energy_eps): break
			else:   old_energy = energy
			# compute transformation from subject space
			cur_motions = []
			for i in range(len(self._graphs)):
				global_trans = transformations[i]
				m = self._motions[i]
				if global_trans:
					m = global_trans.to_motion() * m
				cur_motions.append(m)
			if n >= maxiter: break
		spam = self.learn_spam(sulcus, cur_motions, self._sulci_set)
		return transformations, spam

	def learn(self, miniter=0., maxiter=numpy.inf, verbose=0):
		d = dict(zip(self._labels, [None] * len(self._labels)))
		local_graph_trans = [copy.copy(d) for i in self._graphs]
		global_graph_trans = [copy.copy(d) for i in self._graphs]
		spams = []
		for sulcus in self._labels:
			if self._selected_sulci != None and \
				sulcus not in self._selected_sulci:
				continue
			if verbose > 0: print "========== %s =========" % sulcus
			transformations, spam = self.learn_sulcus(sulcus,
						miniter, maxiter, verbose - 1)
			spams.append(spam)
			for i, global_trans in enumerate(transformations):
				global_graph_trans[i][sulcus] = transformations

		# gravity_centers in optimized referential space
		self._new_gravity_centers = compute_gravity_centers(sulci_set,
				self._input_motions, global_graph_trans)
		for sulcus in self._labels:
			for i, g in enumerate(self._graphs):
				trans = global_graph_trans[i][sulcus]
				R, t0 = trans._R, trans._t
				g = self._new_gravity_centers[sulcus]
				tg = t0 + R * g - g
				local_trans = RigidTransformation(R, tg)
				local_graph_trans[i][sulcus] = local_trans

		local_trans = [SulcusWiseRigidTransformations(t) \
					for t in local_graph_trans]
		global_trans = [SulcusWiseRigidTransformations(t) \
					for t in global_graph_trans]
		mixture = distribution_aims.SpamMixtureModel(spams, None)
		return zip(local_trans, global_trans), mixture


class LocalSpamLearnerLoo(LocalSpamLearner):
	def __init__(self, graphs, input_motions, ss, selected_sulci,
			sigmas, sigma_value, fromlog, dir_priors):
		LocalSpamLearner.__init__(self, graphs, input_motions, ss,
					selected_sulci, sigmas, sigma_value,
					fromlog, dir_priors)

	def learn_spam_loo(self, sulcus, i, motions):
		sulci_set = copy.copy(self._sulci_set)
		for label, h in sulci_set.items():
			h2 = copy.copy(h)
			try: del h2[i]
			except KeyError: pass
			if len(h2) == 0:
				del sulci_set[label]
			else:	sulci_set[label] = h2
		spam = self.learn_spam(sulcus, motions, sulci_set)
		return spam

	def learn_sulcus_onestep(self, sulcus, motions, verbose=0):
		transformations = []
		total_energy = 0.
		if self._dir_priors:
			dir_prior = self._dir_priors['vertices'][sulcus]
		else:	dir_prior = None
		X_subjects = self._data_by_label[sulcus]
		if verbose > 0: print "spams learning..."
		for i, g in enumerate(self._graphs):
			if verbose > 1:
				print "graph %d/%d" % (i + 1, len(self._graphs))
			spam = self.learn_spam_loo(sulcus, i, motions)
			X = X_subjects[i]
			if X is None: continue
			global_trans, energy = self.register(i, X, spam,
					dir_prior, verbose - 1)
			if verbose > 1: print "graph energy :", energy
			total_energy += energy
			transformations.append(global_trans)
		total_energy /= len(self._graphs)
		return transformations, total_energy


################################################################################
def parseOpts(argv):
	description = 'Compute Spam from a list of graphs.\n' \
	'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg...\n' \
	'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg... -- ' \
	'motion1.trm motion2.trm...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_spam_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('--distrib-gaussians', dest='distrib_gaussians',
		metavar = 'DIR', action='store', default = None,
		help='output distrib model of gravity centers')
	parser.add_option('--dir-priors', dest='dir_priorsname',
		metavar = 'FILE', action='store', default = None,
		help='von mises Fisher prior on sulcuswise rotation directions')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('--data-type', dest='data_type',
		metavar = 'TYPE', action='store', default = 'simple_surface',
		help="data type on which spam are learned. Choose one among " \
		"'simple_surface', 'bottom' (default : %default)")
	parser.add_option('--sigma-value', dest='sigma_value',
		metavar = 'FILE', action='store', default = 2.,
		help='fixed smoothing parameter for all sulci ' \
		'(default : %default)')
	parser.add_option('--sigma-file', dest='sigma_file',
		metavar = 'FILE', action='store', default = None,
		help='dictionnary storing sigma values (smoothing parameters)'\
		'for each sulci (file produced by learn_spam_sigma.py)')
	parser.add_option('--verbose', dest='verbose',
		metavar = 'INT', action='store', default = 0,
		help='verbosity level (default : disable)')
	parser.add_option('--fromlog', dest='fromlog',
		action='store_true', default = False,
		help='loglikelihoods are stored in spams rather than ' + \
		'likelihoods')
	parser.add_option('--mode', dest='mode', metavar = 'MODE',
		action='store', default = 'global',
		help="use 'global' or 'local' (sulcuswise) registration")
	parser.add_option('--maxiter', dest='maxiter', metavar = 'INT',
		action='store', default = numpy.inf,
		help="max iterations number of optimization process")
	parser.add_option('--miniter', dest='miniter', metavar = 'INT',
		action='store', default = 0,
		help="min iterations number of optimization process")
	parser.add_option('--loo', dest='loo', action='store_true',
		default = False,
		help="leave one out in registration estimation (each subject" +\
		"is registred on SPAMs computed without this subject)")

	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)

	# read inputs
	inputs = args[1:]
	if len(inputs) == 0:
		parser.print_help()
		sys.exit(1)
	ind = [i for i, input in enumerate(inputs) if input == '==']
	if len(ind) == 0:
		graphnames = inputs
		input_motions_names = None
	else:
		ind = ind[0]
		graphnames = inputs[:ind]
		input_motions_names = inputs[ind + 1:]
	graphs = io.load_graphs(options.transfile, graphnames)
	if input_motions_names:
		reader = aims.Reader()
		input_motions = [reader.read(f) for f in input_motions_names]
	else:	input_motions = None

	# options
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	if options.data_type == 'simple_surface':
		ss = True
		data_type = 'voxels_aims_ss'
	elif options.data_type == 'bottom' :
		ss = False
		data_type = 'voxels_bottom'
	else:
		print "error : '%s' is not a valid data type" % \
						options.data_type
		sys.exit(1)
	if options.sigma_file is not None:
		sigmas = io.read_from_exec(options.sigma_file, 'sigma')
	else:	sigmas = {'sulci' : {}}
	sigma_value = int(options.sigma_value)

	# create output directory
	prefix = options.distribdir
	try:	os.mkdir(prefix)
	except OSError, e:
		print e
		sys.exit(1)

	# learn
	if options.mode == 'global':
		opt = [graphs, input_motions, ss, selected_sulci, sigmas,
				sigma_value, options.fromlog]
		if options.loo:
			Learner = GlobalSpamLearnerLoo
		else:	Learner = GlobalSpamLearner
	elif options.mode == 'local':
		if options.dir_priorsname:
			dir_priors = io.read_segments_distrib(\
					options.dir_priorsname,
					selected_sulci=selected_sulci)
		else:	dir_priors = None
		opt = [graphs, input_motions, ss, selected_sulci, sigmas,
				sigma_value, options.fromlog, dir_priors]
		if options.loo:
			Learner = LocalSpamLearnerLoo
		else:	Learner = LocalSpamLearner
	learner = Learner(*opt)
	transformations, mixture = learner.learn(float(options.miniter),
			float(options.maxiter), verbose=int(options.verbose))
	spams = mixture.get_models()
	labels = learner.get_labels()

	# write motions
	for i, g in enumerate(graphs):
		transformation = transformations[i]
		subject = os.path.splitext(os.path.basename(\
				g['aims_reader_filename']))[0]
		base = subject + '_motion'
		if options.mode == 'global':
			filename = os.path.join(prefix, base + '.trm')
			transformation.write(filename)
		elif options.mode == 'local':
			local_trans, global_trans = transformation
			dir = os.path.join(base)
			local_trans.write(dir + '_local', dir + '_local.dat')
			global_trans.write(dir + '_global', dir + '_global.dat')
	if options.mode == 'local':
		gravity_centers = learner._new_gravity_centers
		prefix = options.distrib_gaussians
		try:    os.mkdir(prefix)
		except OSError, e:
			print e
			sys.exit(1)
		h = {'data_type' : 'refgravity_center', 'files' : {},
			'level' : 'segments'}
		for sulcus, distr in gravity_centers.items():
			filename = io.node2densityname(prefix,'gaussian',sulcus)
			distr.write(filename)
			h['files'][sulcus] = (distr.name(), filename)
		summary_file = options.distrib_gaussians + '.dat'
		fd = open(summary_file, 'w')
		fd.write('distributions = \\\n')
		p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
		p.pprint(h)
		fd.close()

	# write distributions
	h = {'model' : 'spam', 'data_type' : data_type, 'files' : {}}
	for j, sulcus in enumerate(labels):
		if selected_sulci != None and sulcus not in selected_sulci:
			continue
		filename = io.node2densityname(prefix, 'spam', sulcus)
		s = spams[j]
		s.write(filename)
		h['files'][sulcus] = ('spam', filename)

	# write distribution summary file
	summary_file = options.distribdir + '.dat'
	fd = open(summary_file, 'w')
	fd.write('distributions = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()


if __name__ == '__main__' : main()
