def check_same_distribution(distribs):
	s = set()
	for sulcus, distrib in distribs.items():
		if isinstance(distrib, distribution.Gaussian): s.add('gaussian')
		elif isinstance(distrib, distribution_aims.DepthWeightedSpam):
			s.add('depth_weighted_spam')
		elif isinstance(distrib, distribution_aims.PySpam):s.add('spam')
		elif distrib.name() == 'gmm': s.add('gmm')
		elif distrib.name() == 'fixed_bgmm': s.add('fixed_bgmm')
		else: raise ValueError("unknown distrib type '%s'" % \
							distrib.name())
	return (len(s) == 1), s
