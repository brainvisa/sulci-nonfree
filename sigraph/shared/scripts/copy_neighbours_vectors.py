#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import sys, os, re

def read_minf(file):
	try:
		fd = open(file, 'r')
	except:
		print("skip '%s' (read minf)" % file)
		return
	code = '\n'.join(fd.readlines())
	exec(code)
	size = attributes['size']
	split = attributes['sigraph']['split']
	data = os.path.join(os.path.dirname(file), attributes['data'])
	fd.close()
	return split, size, data

def convert_minf(file, oldsplit, newsplit, oldsize, newsize):
	newfile = file + '.new'
	try:
		fdin = open(file, 'r')
	except:
		print("skip '%s' (read minf : convert)" % file)
		return
	try:
		fdout = open(newfile, 'w')
	except:
		print("skip '%s' (write data)" % newfile)
		fdin.close()
		return
	c1 = re.compile("\s*'split'\s+:\s+%d,\s*" % oldsplit)
	c2 = re.compile("\s*'size'\s+:\s+%d,\s*" % oldsize)
	line = True
	for line in fdin.readlines():
		if c1.match(line):
			fdout.write(re.sub('\d+,', '%d,' % newsplit, line))
		elif c2.match(line):
			fdout.write(re.sub('\d+,', '%d,' % newsize, line))
		else:	fdout.write(line)
		
	fdin.close()
	fdout.close()

def getVertexModel(model, label):
	for v in model.vertices():
		l = v['label'].get().getString()
		if label == l: return v

def getEdgeModel(model, labels):
	labels = set(labels)
	for e in model.edges():
		l = set([e['label1'].get().getString(),
                        e['label2'].get().getString()])
		if labels == l: return e
			

def fileToLabels(filename):
	filename = os.path.basename(filename)
	if filename.startswith('edg'):
		i = filename.rfind('_')
		filename = filename[3:i]
		i = filename.rfind('-')
		labels = filename[:i], filename[i + 1:]
		return labels, True
	else:
		i = filename.rfind('_')
		label = filename[:i]
		return label, False

def labelsToFile(labels):
	if len(labels) == 2:
		return "edg" + labels[0] + "-" + labels[1] + "_svm1.minf"
	else:	return labels[0] + "_svm1.minf"


def findNeigbhours(model, modelfilename, labels, is_edge):
	# find model of neighbours
	neighbours = []
	if is_edge:
		m = getEdgeModel(model, labels)
		s1 = set(labels)
		for v in m.vertices():
			for e in v.edges():
				s2 = set([e['label1'].get().getString(),
					e['label2'].get().getString()])
				if s1 != s2: neighbours.append(e)
	else:
		label = labels
		m = getVertexModel(model, label)
		for e in m.edges():
			l = [v for v in e.vertices() \
				if v['label'].get().getString() != label]
			if len(l) != 1:
				print('error : handle only relation between '\
					'2 sulci.')
				sys.exit(1)
			neighbours.append(l[0])
	# find full path of neighbours minf file
	modelpath = os.path.join(os.path.dirname(modelfilename), 'model')
	neighbours_file = []
	if is_edge:
		for n in neighbours:
			file = labelsToFile([n['label1'].get().getString(),
					n['label2'].get().getString()])
			file = os.path.join(modelpath, 'edges', file)
			neighbours_file.append(file)
	else:
		for n in neighbours:
			file = labelsToFile([n['label'].get().getString()])
			file = os.path.join(modelpath, 'adap', file)
			neighbours_file.append(file)
	return neighbours_file


def copy_data(fdout, filename, split, before_split, copy_header, class_n):
	try:
		fdin = open(filename, 'r')
	except:
		print("skip '%s' (read minf : convert)" % file)
		return

	lines = fdin.readlines()
	if copy_header: fdout.write(lines[0])
	lines = lines[1:]
	for i, l in enumerate(lines):
		if before_split:
			if i >= split - 1: continue
		elif i < split - 1 : continue
		line = l.rstrip('\n').rstrip('\t')
		values = line.split('\t')
		if class_n == 0:
			values[-3] = '-1' # potential
			values[-2] = '0'  # class
		else:
			values[-3] = '1'            # potential
			values[-2] = '%d' % class_n # class
		fdout.write('\t'.join(values) + '\n')
	fdin.close()


def convert_data(mode, filename, neighbours):
	# select mode
	if mode == '2classes': get_class = lambda x : 1
	elif mode == 'nclasses': get_class = lambda x : (x + 1)

	# read .minf
	oldsplit, oldsize, maindata = read_minf(filename)
	newsplit, newsize = oldsplit - 1, oldsize

	datas = {}
	for n in neighbours:
		split, size, data = read_minf(n)
		newsplit += (split - 1)
		newsize += size
		datas[data] = (split, size)
	newsplit += 1
	# New data file
	newfile = maindata + '.new'
	try:
		fdout = open(newfile, 'w')
	except:
		print("skip '%s' (write data)" % newfile)
		fdin.close()
		return

	# Write data
	copy_data(fdout, maindata, oldsplit, True, True, 0)
	for i, (d, (split, size)) in enumerate(datas.items()):
		copy_data(fdout, d, split, True, False, get_class(i))
	copy_data(fdout, maindata, oldsplit, False, False, 0)
	for i, (d, (split, size)) in enumerate(datas.items()):
		copy_data(fdout, d, split, False, False, get_class(i))

	# end
	fdout.close()
	return oldsplit, oldsize, newsplit, newsize


def convert(mode, modelfilename, model, filename):
	labels, is_edge = fileToLabels(filename)
	neighbours = findNeigbhours(model, modelfilename, labels, is_edge)
	oldsplit, oldsize, newsplit, newsize = convert_data(mode, filename, \
								neighbours)	
	convert_minf(filename, oldsplit, newsplit, oldsize, newsize)


def main():
	if len(sys.argv) <= 3:
		print("Usage %s MODE model.arg file1.minf file2.minf ..." % \
			os.path.basename(sys.argv[0]))
		print("    MODE : 2classes, nclasses")
		sys.exit(1)
	from soma import aims
	mode = sys.argv[1]
	if not mode in ['2classes', 'nclasses']:
		print("unknown mode '%s'" % mode)
		sys.exit(1)
	modelfilename = sys.argv[2]
	files = sys.argv[3:]
	
	# read model
	r = aims.Reader()
	model = r.read(modelfilename)
	for f in files: convert(mode, modelfilename, model, f)

if __name__ == "__main__" : main()
