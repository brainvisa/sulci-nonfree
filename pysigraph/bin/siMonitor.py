#!/usr/bin/env python

from __future__ import print_function
import os
from soma import aims
from sigraph import *
from sigraph.cover import *
from optparse import OptionParser
from datamind.tools import *

# Options parser
def parseOpts(argv):
	description = 'Monitor sigraph learning process.'
	parser = OptionParser(description)
	add_filter_options(parser)
	parser.add_option('-m', '--model', dest='modelfilename',
		metavar='MODEL', action='store', default='model.arg',
		help='model file name (default : %default)')
	parser.add_option('-n', '--newer', dest='newer',
		metavar='FILE', action='store', default=None,
		help='generated files must be newer than this file')
	parser.add_option('-t', '--task', dest='task',
		metavar='TASK', action='store', default='models',
		help='monitoring task : models, log (default : %default)')
	parser.add_option('--mode', dest='mode',
		metavar='MODE', action='store', default='learning',
		help='monitoring mode : learning, generation, models '
			'(default : %default)')
	parser.add_option('-o', '--outputbatch', dest='outputbatch',
		metavar='FILE', action='store', default='',
		help='batch file')
	parser.add_option('-r', '--remote', dest='remote',
		action='store_true', default=False,
		help='enable remote control (default : disable)')
	parser.add_option('-l', '--log', dest='logfilename',
		action='store', default='', help='analyzed log')
	parser.add_option('-i', '--inputbatch', dest='inputbatch',
		action='store', default='', help='input batch file')
	return parser.parse_args(argv)


def add_subadaptive_files(al, user_data):
	ao = al.topModel().parentAO()
	f = al.workEl().fileNames()
	d = os.path.join(user_data['prefix'],
		os.path.dirname(ao['model_file'].get().getString()),
		os.path.dirname(f))

	try:
		user_data['dirs'][d] += [os.path.basename(f)]
	except:	user_data['dirs'][d] = [os.path.basename(f)]


def add_db_files(al, user_data):
	def set_file(f):
		d = os.path.dirname(f)
		try:
			user_data['dirs'][d] += [os.path.basename(f)]
		except:	user_data['dirs'][d] = [os.path.basename(f)]
		
	ao = al.topModel().parentAO()
	base = al.getDataBaseName(user_data['prefix'])
	set_file(base + '.data')
	set_file(base + '.minf')


def add_models_files(el, user_data):
	modelfile = el['model_file'].get().getString()
	d = os.path.join(user_data['prefix'], os.path.dirname(modelfile))

	try:
		user_data['dirs'][d] += [os.path.basename(modelfile)]
	except:	user_data['dirs'][d] = [os.path.basename(modelfile)]


def control(t, dirs, data, l, size, newer):
	import sys
	def stat(dir, f):
		st = os.stat(os.path.join(dir, f))
		if st[8] > newer:
			data['n'] += 1
			msg.write_list(['\r Generated models : ',
				('%s' % str(data['n']).zfill(l), 'red'), '/',
				('%s' % size, 'red')])
			sys.stdout.flush()

	for dir, files in dirs.items():
		remove_list = []
		for f in files:
			try:
				stat(dir, f)
				remove_list.append(f)
			except OSError: pass
		for r in remove_list:
			files.remove(r)
	
def monitor(t, dirs, newer):
	import numpy, time
	n = 0
	if newer:
		try:
			newer = os.stat(newer)[8]
		except OSError:
			msg.error("%s file can't be opened" % newer)
	else:	newer = 0
	size = str(numpy.array([len(f) for f in dirs.values()]).sum())
	l = len(size)
	data = {'n' : n}
	sys.stdout.write('...waiting')
	sys.stdout.flush()
	while 1:
		control(t, dirs, data, l, size, newer)
		if str(data['n']) == size: break
		time.sleep(t)
	print('\nMonitoring finished.')


def remote_working(hostname, files):
	import os, commands
	user = os.getlogin()
	cmd = "ssh %s@%s 'ps aux | grep %s | grep python'" % \
					(user, hostname, user)
	s, o = commands.getstatusoutput(cmd)
	o = [x for x in o.split('\n') if x.startswith(user)]
	if s == 0:
		if len(o) > 2:
			msg.write_list([' * ', (hostname, 'cyan'), ' : ',
				('running', 'green'), '...\n'])
		else:	msg.write_list([' * ', (hostname, 'cyan'), ' : ',
				('crashed', 'red'), '!\n'])
		print(files)
	elif s == '65280' :
		msg.error("ynknown %s : connection non available" % hostname)
	elif s == '256' :
		msg.error('%s : no route to host' % hostname)
	else:	msg.error('unknown error while connecting to %s' % hostname) 
	

def log_control(files, logfilename):
	int = []
	for f in files:
		fd = open(f)
		r = fd.readlines()
		fd.close()
		if (len(r) == 0 or r[-1] != 'Saving model...\n'):
			int += [f]
	if len(int) == 0:
		msg.write_list([' * log : ', ('OK', 'green'), '\n'])
	else:
		print("Interrupted or unfinished processes :")
		msg.write(' * ', 'red')
		print(int)
	return int

def remote_log_control(logfilename, interrupted_tasks):
	fd = open(logfilename)
	lines = fd.readlines()
	fd.close()
	hosts = {}
	for l in lines:
		e = l[l.rfind('> ') + 2:]
		log = os.path.basename(e[:e.rfind('.log') + 4])
		if not log in interrupted_tasks: continue
		e = l[l.find('@') + 1:]
		hostname = e[:e.find(' ')]
		try:
			hosts[hostname] += [log]
		except KeyError:
			hosts[hostname] = [log]
	[remote_working(*item) for item in hosts.items()]


def batch_remaining_from_log(logfilename, batchfile, interrupted_tasks):
	fd = open(logfilename)
	lines = fd.readlines()
	fd.close()
	batchfd = open(batchfile, 'w')
	for l in lines:
		e = l[l.rfind('> ') + 2:]
		log = os.path.basename(e[:e.rfind('.log') + 4])
		if l.find('Execute: ssh') == -1: continue
		if not log in interrupted_tasks: continue
		cmd = l[l.find('nice "cd') + 5:-1]
		print(cmd, file=batchfd)
	batchfd.close()

def batch_remaining_from_batch(inputbatch, outputbatch, interrupted_tasks):
	fd = open(inputbatch)
	lines = fd.readlines()
	fd.close()
	batchfd = open(outputbatch, 'w')
	for l in lines:
		e = l[l.rfind('> ') + 2:]
		log = os.path.basename(e[:e.rfind('.log') + 4])
		if not log in interrupted_tasks:
			continue
		print(l[:-1], file=batchfd)
	batchfd.close()
		
def get_log_files(logfilename):
	files = os.listdir(os.path.dirname(os.path.join('.',
		logfilename)))
	return [f for f in files if f.endswith('.log') if f != logfilename]


# main function
def main():
	import sys

	# read options
	options, args = parseOpts(sys.argv)
	if options.mode == 'learning':
		fundict = {'adaptiveleaf' : add_subadaptive_files}
	elif options.mode == 'generation':
		fundict = {'adaptiveleaf' : add_db_files}
	elif options.mode == 'models':
		fundict = {'edge_before' : add_models_files,
			'vertex_before' : add_models_files}
	else:
		msg.error("'%s' is not a valid monitoring mode" % options.mode)
		sys.exit(1)

	# read model
	r = aims.Reader()
	try:
		os.stat(options.modelfilename)
	except OSError:
		msg.error("model file : '%s' does not exist" % \
			options.modelfilename)
		sys.exit(1)
	model = r.read(options.modelfilename)

	# set prefix
	prefix = os.path.dirname(options.modelfilename)
	if prefix == '' : prefix = '.'

	# cover model
	data = {'dirs' : {}, 'prefix' : prefix}
	cover(model, fundict, data, options.labels_filter, options.filter_mode)

	if options.task == 'models':
		try:
			monitor(5, data['dirs'], options.newer)
		except KeyboardInterrupt:
			print('keyboard interruption.')
			for dir, files in data['dirs'].items():
				for f in files:
					print(os.path.join(dir, f))
			logfiles = []
			for dir, files in data['dirs'].items():
				for f in files :
					if not f.endswith('.minf'): continue
					if f.startswith('edg'):
						f = f[3:]
					e = f.rfind('right')
					if e == -1:
						e = f.rfind('left') + 4
					else:	e += 5
					log = f[:e] + '.log'
					logfiles.append(log)
			if not '' in [options.inputbatch, options.outputbatch]:
				batch_remaining_from_batch(options.inputbatch,
					options.outputbatch, logfiles)
	elif options.task == 'log':
		files = get_log_files(options.logfilename)
		int = log_control(files, options.logfilename)
		if options.remote: remote_log_control(options.logfilename, int)
	elif options.task == 'batch':
		files = get_log_files(options.logfilename)
		int = log_control(files, options.logfilename)
		batch_remaining_from_log(options.logfilename,
					options.outputbatch, int)
	else :
		print("error : '%s' unknown task." % options.task, file=sys.stderr)
		sys.exit(1)
		

if __name__ == '__main__' : main()
