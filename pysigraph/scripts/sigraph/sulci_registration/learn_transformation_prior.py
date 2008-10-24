#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from soma import aims
from sulci.registration.procrust import vector_from_rotation
from sulci.common import io
from sulci.models import distribution, distribution_aims

################################################################################


################################################################################
def parseOpts(argv):
	description = 'Learn sulcuswise transformation prior from output ' + \
	'data of command learn_transformation_prior.py in ' + \
	'local mode \n' \
	'learn_spams_distributions.py [OPTIONS] trans1.dat trans2.dat...'
	parser = OptionParser(description)
	for s in ['translation', 'direction', 'angle']:
		parser.add_option('--%s-distribdir' %s,
			dest='%s_distribdir' % s, metavar = 'FILE',
			action='store', default = 'bayesian_%s_distribs' %s,
			help='output distribution directory for %s ' % s + \
			'(default : %default). A file named FILE.dat is ' + \
			'created to store labels/databases links.')

	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	transformations = args[1:]
	if len(transformations) == 0:
		parser.print_help()
		sys.exit(1)

	data = {}
	for t in transformations:
		motions_local = io.read_from_exec(t, 'transformations')
		prefix = os.path.dirname(t)
		for sulcus, filename in motions_local.items():
			filename = os.path.join(prefix, filename)
			motion = aims.Reader().read(filename)
			if data.has_key(sulcus):
				data[sulcus].append(motion)
			else:	data[sulcus] = [motion]

	translation_priors = {}
	angle_priors = {}
	dir_priors = {}

	for distribdir in [options.translation_distribdir,
				options.direction_distribdir,
				options.angle_distribdir]:
		prefix = distribdir
		try:	os.mkdir(prefix)
		except OSError, e:
			print e
			sys.exit(1) #FIXME

	for sulcus, motions in data.items():
		translations = []
		directions = []
		thetas = []
		for m in motions:
			T = m.translation().arraydata().copy()
			R = m.rotation().volume().get().arraydata().copy()
			R = R.reshape(3, 3)
			w = vector_from_rotation(R)
			theta = numpy.linalg.norm(w)
			if theta:
				direction = w / theta
				directions.append(numpy.asarray(direction).ravel())
			translations.append(T)
			thetas.append(theta)
		translations = numpy.vstack(translations)
		directions = numpy.vstack(directions)
		thetas = numpy.array(thetas)
		dir_prior = distribution_aims.Bingham()
		dir_prior.fit(directions)
		dir_priors[sulcus] = dir_prior
		angle_prior = distribution.VonMises()
		angle_prior.fit(thetas[None].T)
		angle_priors[sulcus] = angle_prior
		translation_prior = distribution.Gaussian()
		translation_prior.fit(translations)
		translation_priors[sulcus] = translation_prior

	# save translation prior
	h = {'data_type' : 'translation_prior', 'files' : {}, 'level' : 'sulci'}
	prefix = options.translation_distribdir
	for sulcus, g in translation_priors.items():
		filename = io.node2densityname(prefix, 'gaussian', sulcus)
		g.write(filename)
		h['files'][sulcus] = (g.name(), filename)
	summary_file = options.translation_distribdir + '.dat'
	io.write_pp('distributions', summary_file, h)

	# save direction prior
	h = {'data_type' : 'direction_prior', 'files' : {}, 'level' : 'sulci'}
	prefix = options.direction_distribdir
	for sulcus, d in dir_priors.items():
		filename = io.node2densityname(prefix,
				'bingham', sulcus)
		d.write(filename)
		h['files'][sulcus] = (d.name(), filename)
	summary_file = options.direction_distribdir + '.dat'
	io.write_pp('distributions', summary_file, h)

	# save angle prior
	h = {'data_type' : 'angle_prior', 'files' : {}, 'level' : 'sulci'}
	prefix = options.angle_distribdir
	for sulcus, d in angle_priors.items():
		filename = io.node2densityname(prefix, 'von_mises', sulcus)
		d.write(filename)
		h['files'][sulcus] = (d.name(), filename)
	summary_file = options.angle_distribdir + '.dat'
	io.write_pp('distributions', summary_file, h)
	

if __name__ == '__main__' : main()
