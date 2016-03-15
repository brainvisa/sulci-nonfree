#!/usr/bin/env python2
import sys, os, shutil, re

def read_minf(file):
	try:
		fd = open(file, 'r')
	except:
		print "skip '%s' (read minf)" % file
		return
	code = '\n'.join(fd.readlines())
	exec code
	size = attributes['size']
	split = attributes['sigraph']['split']
	data = os.path.join(os.path.dirname(file), attributes['data'])
	fd.close()
	return split, size, data

def convert_data(file, split, size):
	newfile = file + '.new'
	try:
		fdin = open(file, 'r')
	except:
		print "skip '%s' (read data)" % file
		return
	try:
		fdout = open(newfile, 'w')
	except:
		print "skip '%s' (write data)" % (newfile)
		fdin.close()
		return

	line = True
	line_number = 0
	removed_lines_before_split = 0
	removed_lines = 0
	for line in fdin.readlines():
		valid = line.split('\t')[0]
		if valid == '1' or line_number == 0: fdout.write(line)
		else:
			if line_number < split:
				removed_lines_before_split += 1
			removed_lines += 1
		line_number += 1
	fdin.close()
	fdout.close()
	oldfile = file + '.old'
	shutil.move(file, oldfile)
	shutil.move(newfile, file)
	newsplit = (split - removed_lines_before_split)
	newsize = (size - removed_lines)
	return newsplit, newsize

def convert_minf(file, oldsplit, newsplit, oldsize, newsize):
	newfile = file + '.new'
	try:
		fdin = open(file, 'r')
	except:
		print "skip '%s' (read minf : convert)" % file
		return
	try:
		fdout = open(newfile, 'w')
	except:
		print "skip '%s' (write data)" % newfile
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
	oldfile = file + '.old'
	shutil.move(file, oldfile)
	shutil.move(newfile, file)


def convert(file):
	oldsplit, oldsize, data = read_minf(file)
	newsplit, newsize = convert_data(data, oldsplit, oldsize)
	convert_minf(file, oldsplit, newsplit, oldsize, newsize)


def main():
	if len(sys.argv) == 1:
		print "Usage %s file1.minf file2.minf ..." % \
			os.path.basename(sys.argv[0])
		sys.exit(1)
	files = sys.argv[1:]
	for f in files: convert(f)

if __name__ == "__main__" : main()
