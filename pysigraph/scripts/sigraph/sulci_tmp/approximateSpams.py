#!/usr/bin/env python

import numpy
from sulci.models import distribution, distribution_aims

spam_input = '/home/revilyo/devel/CEA/tmp/bayesian_2008/registration/bayesian_spam_distribs/bayesian_spam_density_S.C._left.ima'
spam_output = 'plop.ima'

# read spam
spam = distribution_aims.Spam()
spam.read(spam_input)

# fit rbf
rbf = distribution.Rbf(mode='normalized_gaussian',std=4., normalize_kernel=True)
# marchotte
#rbf.fit_from_Spam(spam, n=1000, pct=0.01, weighted=True,
#		interpolation=False, normalize_weights=True)
rbf.fit_from_Spam(spam, n=3000, pct=0.1, weighted=True,
		interpolation=False, normalize_weights=True)

# rbf to spam
img = spam._img_density.volume().arraydata()
t = numpy.array(spam._bb_talairach_offset)
s = numpy.array(spam._bb_talairach_size)
from numpy.lib import index_tricks
shape = tuple(numpy.array(img.shape)[1:])
X = numpy.array([x for x in index_tricks.ndindex(shape)])
X += t
d = 100
n = (X.shape[0] / d)
li = []
for i in range(d + 1):
	print i * n, (i + 1) * n, n, X.shape[0]
	Xi = X[i * n: (i + 1) * n]
	logli, li_i = rbf.likelihoods(Xi)
	li.append(li_i)
print "stack"
li = numpy.hstack(li)
print "stack"
print img.shape, li.shape
print img.min(), img.max(), img.mean()
print li.min(), li.max(), li.mean()
print "mse = ", (img.ravel() - li).mean()
img[:] = li.reshape(img.shape)

# write spam
spam.write(spam_output)
