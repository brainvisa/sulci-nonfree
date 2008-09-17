#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from soma import aims
from sulci.registration.procrust import vector_from_rotation
from sulci.common import io
from sulci.models import distribution

################################################################################


################################################################################
def parseOpts(argv):
	description = 'Learn sulcuswise transformation prior from output ' + \
	'data of command learn_registred_spams_distributions.py in ' + \
	'local mode \n' \
	'learn_spams_distributions.py [OPTIONS] trans1.dat trans2.dat...'
	parser = OptionParser(description)
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_transformations_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')

	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	transformations = args[1:]
	if len(transformations) == 0:
		parser.print_help()
		sys.exit(1)

	sulci
	data = {}
	for t in transformations:
		motions_local = io.read_from_exec(t, 'transformations')
		for sulcus, filename in motions_local.items():
			if data.has_key(sulcus):
				motion = aims.Reader().read(filename)
				data[sulcus] += motion
			else:	data[sulcus = [motion]

	for sulcus, motions in data.items():
		translations = []
		directions = []
		thetas = []
		for m in motions:
			T = m.translation().arraydata().copy()
			R = m.rotation().volume().get().arraydata().copy()
			w = vector_from_rotation(R)
			theta = numpy.linalg.norm(w)
			dir = w / theta
			translations.append(T)
			directions.append(dir)
			thetas.append(theta)
		translations = numpy.vstack(translations)
		directions = numpy.vstack(directions)
		thetas = numpy.array(thetas)
		dir_prior = distribution.VonMisesFisher(p=3)
		dir_prior.fit(directions)
		angle_prior = distribution.VonMises()
		angle_prior.fit(thetas)
		translation_prior = distribution.gaussian()
		translation_prior.fit(translations)

	#FIXME : save distributions
	# display distributions
	

if __name__ == '__main__' : main()
