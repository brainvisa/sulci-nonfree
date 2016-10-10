#!/usr/bin/env python2


from __future__ import print_function
import numpy, sys

import sigraph
from soma import aims

import sulci.models.distribution_aims
from sulci.models.distribution_aims import Spam
import sulci.registration.spam
import sulci.registration.procrust
from sulci.registration.common import save_transformation
from sulci.registration.spam import SpamRegistration

def main():
	spamname = sys.argv[3]
	#spamname = '/volatile/perrot/These/localization_models/independent/spam_model_Right/all/bayesian_spam_distribs/bayesian_spam_density_S.C._right.ima'
	#spamname = '/volatile/perrot/These/localization_models/independent/spam_model_Right/all/bayesian_spam_distribs/bayesian_spam_density_S.T.s._right.ima'
	#graphname = '/neurospin/lnao/Panabase/data_sulci/base2008/graphs/Rsujet10_man.arg'
	#graphname = '/neurospin/lnao/Panabase/data_sulci/base2008/graphs/Rammon_default_session_manual.arg'
	#graphname = '/volatile/perrot/These/localization_models/independent/spam_model_Right/cv_1/Ranubis_default_session_manual.arg'
	#graphname = '/volatile/perrot/These/localization_models/independent/spam_model_Right/cv_2/Rathena_default_session_manual.arg'
	#graphname = '/volatile/perrot/These/localization_models/independent/spam_model_Right/cv_3/Ratlas_default_session_manual.arg'
	graphname = sys.argv[1]
	motionname = sys.argv[2]
	transfile = '/volatile/perrot/p4/shared-stable/nomenclature/translation/sulci_model_2008.trl'
	#spamname = '/home/revilyo/devel/CEA/tmp/bayesian/spam_model_Right/cv_0/bayesian_spam_distribs/bayesian_spam_density_S.C._right.ima'
	#graphname = '/home/revilyo/devel/CEA/data/base2000/graphs/RmorpheeBase.arg'
	#transfile = '/home/revilyo/p4/shared-stable/nomenclature/translation/sulci_model_2008.trl'
	g = aims.Reader().read(graphname)
	ft = sigraph.FoldLabelsTranslator(transfile)
	sigraph.si().setLabelsTranslPath(transfile)
	ft.translate(g)
	motion = aims.GraphManip.talairach(g)
	p_list = []
	for v in g.vertices():
		if v.getSyntax() != 'fold': continue
		if v['name'] != 'S.T.s.ter.asc.ant._right': continue
		bucket_name = 'aims_ss'
		ss_map = v[bucket_name].get()
		size_in = numpy.array([ss_map.sizeX(), ss_map.sizeY(),
							ss_map.sizeZ()])
		for p_in in ss_map[0].keys():
			p_in = aims.Point3df(p_in * size_in)
			p_out = motion.transform(p_in)
			p_list.append(p_out)
	X = numpy.vstack(p_list)
	spam = Spam()
	spam.read(spamname)
	print("sulci registration...")
	spamreg = SpamRegistration(spam, X, None, None, None, None)
	R, t = spamreg.optimize_powell()
	#motionname = 'motion.trm'
	save_transformation(motionname, R, t)

if __name__ == '__main__' : main()
