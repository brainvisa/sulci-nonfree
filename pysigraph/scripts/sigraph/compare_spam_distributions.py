#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import os, sys, numpy
import matplotlib
from six.moves import zip
matplotlib.use('Qt4Agg')
import pylab
from optparse import OptionParser
from sulci.common import io
from sulci.models import distribution, distribution_aims
from soma.qt_gui import qt_backend
qt_backend.init_matplotlib_backend()

thresholds = [0.2, 0.5, 0.75, 0.9, 0.95, 0.99]

################################################################################
def compute_volume(sulcus, spam):
	img_density = spam.img_density()
	array = img_density.volume().get().arraydata()
	array_sort = numpy.sort(array.flatten())[::-1]
	weights = array_sort.cumsum()
	delta = weights[1:] - weights[:-1]
	weights = weights[:-1]
	cumvol = numpy.arange(len(weights), dtype='float')
	# normalization factor = 1/2 = numpy.dot(1 - weights, delta)
	vol = 2 * numpy.dot((1 - weights) * delta, cumvol)
	array = array[array > 0]
	entropy = -(array * numpy.log(array)).sum()

	vol_th = numpy.array([numpy.sum(weights < th) for th in thresholds])
	return vol, entropy, weights, cumvol, vol_th

def plot(list):
	names = []
	for (name, weights, cumvol) in list:
		pylab.plot(weights, cumvol)
		names.append(name)
	pylab.xlabel('pct of distribution core')
	pylab.ylabel('volume')
	pylab.legend(names)
	pylab.show()

def plot_pct(sulcus, list):
	weights0 = list[0][1]
	weights1 = list[1][1]

	ths = numpy.linspace(0, 1, 100)[:-1]
	s0 = numpy.array([numpy.sum(weights0< th) for th in ths], dtype='float')
	s1 = numpy.array([numpy.sum(weights1< th) for th in ths], dtype='float')
	pct = s1 / s0 - 1.

	f = pylab.figure(1)
	pylab.plot(ths, pct)
	names = (list[0][0], list[1][0])
	pylab.xlabel('pct of highest probability region integral')
	pylab.ylabel("pct of gain/loss from %s to %s" % names)
	filename = 'pct_%s.png' % sulcus
	pylab.savefig(filename, dpi=300)
	f.clear()

	f = pylab.figure(1)
	pylab.plot(ths, pct, linewidth=6)
	f.subplots_adjust(0.25, 0.15, 0.9, 0.95)
	f.set_clip_on(False)
	pylab.box(0)
	pylab.xticks([])
	pylab.yticks([])
	xmi, ymi = numpy.min(ths), numpy.min(pct)
	xma, yma = numpy.max(ths), numpy.max(pct)
	sx = (xma - xmi) * 0.35
	sy = (yma - ymi) * 0.2
	xpi = xmi - sx
	ypi = ymi - sy
	pylab.text(xmi, ypi, '%2.2f' % xmi, size=40)
	pylab.text(xma - (xma - xmi) * 0.2, ypi, '%2.2f' % xma, size=40)
	pylab.text(xpi, ymi, '%2.2f' % ymi, size=40)
	pylab.text(xpi, yma, '%2.2f' % yma, size=40)
	r = 4/3.
	a = matplotlib.patches.FancyArrow(xmi-sx*0.1/r-sx*0.1/(2.*r),ymi-sy*0.1,
		xma - xmi, 0,
		length_includes_head=True, head_width=sy*0.4, zorder=0,
		head_length=(xma-xmi)*0.1, width=sy*0.1, facecolor='black')
	f.axes[0].add_artist(a)
	a = matplotlib.patches.FancyArrow(xmi-sx*0.1, ymi-sy*0.1-sy*0.1/2.,
		0, yma - ymi,
		length_includes_head=True, head_width=sx*0.4/r, zorder=0,
		head_length=(yma-ymi)*0.1*r, width=sx*0.1/r, facecolor='black')
	f.axes[0].add_artist(a)

	names = (list[0][0], list[1][0])
	filename = 'pct_light_%s.png' % sulcus
	pylab.savefig(filename, dpi=300)
	f.clear()



def kullback_leibler(spam1, spam2):
	off1, size1 = spam1.bb_talairach()
	off2, size2 = spam2.bb_talairach()
	off1 = numpy.array(off1)
	off2 = numpy.array(off2)
	size1 = numpy.array(size1)
	size2 = numpy.array(size2)
	off = numpy.max((off1, off2), axis=0)
	size = numpy.min((off1 + size1, off2 + size2), axis=0) - off
	size = size.astype('int')
	
	density1 = spam1.img_density()
	density2 = spam2.img_density()
	a1 = density1.volume().get().arraydata()
	a2 = density2.volume().get().arraydata()

	doff1 = (off - off1).astype('int')
	doff2 = (off - off2).astype('int')

	p = a1[:, doff1[2]:doff1[2] + size[2],
		doff1[1]:doff1[1] + size[1],
		doff1[0]:doff1[0] + size[0]]
	q = a2[:, doff2[2]:doff2[2] + size[2],
		doff2[1]:doff2[1] + size[1],
		doff2[0]:doff2[0] + size[0]]
	logp = numpy.log(p)
	logq = numpy.log(q)
	logp[numpy.isneginf(logp)] = -100.
	logq[numpy.isneginf(logq)] = -100.
	
	kl = (p * (logp - logq)).sum()
	return kl

################################################################################
def parseOpts(argv):
	description = 'compute an indice of sharpness of distribution for ' + \
		'each sulci of 2 distributions and compare them.\n' + \
		'./compute_volumes.py [OPTIONS] name1 distrib1.dat ' + \
		'name2 distrib2.dat'
	parser = OptionParser(description)
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='Compute mesh only for specified sulcus ' \
			'(default : compute all meshes)')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'NAME', action='store', default = None,
		help='output to map no sulci')
	parser.add_option('-p', '--plot', dest='plot',
		metavar = 'NAME', action='store_true', default = False,
		help='plot highest probability region integral vs pct ' + \
		'volume difference')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	inputs = args[1:]
	if len(inputs) != 4 or None in [options.output]:
		parser.print_help()
		sys.exit(1)

	models = []
	names = []
	for name, distribname in zip(inputs[::2], inputs[1::2]):
		model = io.read_segments_distrib(distribname, options.sulcus)
		models.append((name, model))
		names.append(name)

	infos = {}
	sulci = list(model['vertices'].keys())
	pcts = []
	for sulcus in sulci:
		print("*** %s ***" % sulcus)
		l, vols, entropies, distribs, vols_th = [], [], [], [], []
		for name, model in models:
			try:
				distrib = model['vertices'][sulcus]
			except KeyError: break
			vol, entropy, weights, cumvol, vol_th = \
					compute_volume(sulcus, distrib)
			l.append((name, weights, cumvol))
			vols.append(vol)
			entropies.append(entropy)
			distribs.append(distrib)
			vols_th.append(vol_th)
		if len(vols) != 2: continue
		#plot(l)
		if options.plot: plot_pct(sulcus, l)
		entropy_diff = entropies[1] - entropies[0]
		vols_diff = (vols_th[1] - vols_th[0])
		vols_pct = (vols_th[1].astype('float') / vols_th[0]) - 1
		vol_diff = (vols[1] - vols[0])
		vol_pct = (vols[1] / vols[0]) - 1
		kl = kullback_leibler(distribs[1], distribs[0])
		infos[sulcus] = [vols[0], vols[1], vol_diff, vol_pct] + \
			vols_diff.tolist() + vols_pct.tolist() + \
			[entropies[0], entropies[1], entropy_diff, kl]
		pcts.append(vol_pct)
	pct_mean = numpy.mean(pcts)
	print("pct mean = ", pct_mean)

	fd = open(options.output, 'w')
	l = ['sulci', "'volume_%s'" % names[0], "'volume_%s'" % names[1],
					'volume_diff', 'volume_diff_pct']
	l += [('volume_diff_%2.2f' % th) for th in thresholds]
	l += [('volume_pct_%2.2f' % th) for th in thresholds]
	l += ["'entropy_%s'" % names[0], "'entropy_%s'" % names[1], \
				'entropy_diff', 'kullback_leibler']
	s = '\t'.join(l) + '\n'
	for sulcus in sulci:
		s += "%s\t" % sulcus
		try:
			s += '\t'.join(str(f) for f in infos[sulcus]) + '\n'
		except KeyError: continue
	fd.write(s)
	fd.close()

if __name__ == '__main__' : main()
