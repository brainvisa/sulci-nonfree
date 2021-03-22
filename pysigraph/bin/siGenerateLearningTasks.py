#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import absolute_import
from soma import aims
from sigraph import *
from sigraph.cover import *
import distutils.spawn
from optparse import OptionParser


def get_logfile(dir, labels):
    import os
    return os.path.join(dir, '-'.join(labels) + '.log')

# Generate commands


def get_cmd(labels, options, user_data):
    cmd = options.bin + " --filtermode 'strict' --labelsfilter '" + \
        ','.join(labels) + "' '" + options.cfg + "'"
    return 'bash -c \\"%s\\"' % cmd


def get_cmd2(labels, options, user_data):
    cmd = options.bin + " --filtermode 'strict' --labelsfilter '" + \
        ','.join(labels) + "' '" + options.cfg + "'"
    return 'bash -c "%s"' % cmd


def get_cmd_raw(labels, options, user_data):
    return [options.bin, "--filtermode", 'strict', "--labelsfilter",
            ','.join(labels), options.cfg]


def cmd_grid(labels, options, user_data):
    dir = user_data['output_dir']
    log = get_logfile(dir, labels)
    cmd = 'cd ' + dir + " && nice " + get_cmd2(labels, options, user_data) +\
        " > " + log + '\n'
    user_data['output_fd'].write(cmd)


def cmd_duch(labels, options, user_data):
    dir = user_data['output_dir']
    log = get_logfile(dir, labels)
    cmd = '"cd ' + dir + " && nice " + get_cmd(labels, options, user_data) +\
        " > " + log + '"\n'
    user_data['output_fd'].write(cmd)


def cmd_cath(labels, options, user_data):
    pass


def cmd_LSF(labels, options, user_data):
    import os
    n = repr(user_data['n']).zfill(user_data['zeros'])
    filename = options.output + n
    dir = user_data['output_dir']
    log = get_logfile(dir, labels)
    fd = open(filename, 'w')
    print('#!/bin/bash', file=fd)
    print('#BSUB -n 1', file=fd)
    print('#BSUB -J ' + os.path.basename(options.bin) + '-' + n, file=fd)
    print('#BSUB -c ' + options.time, file=fd)
    print('#BSUB -o ' + log, file=fd)
    print('#BSUB -e ' + log, file=fd)
    print('#BSUB -u ' + user_data['email'], file=fd)
    print('#BSUB -N ', file=fd)
    print('cd ' + dir, file=fd)
    print(get_cmd(labels, options, user_data), file=fd)
    fd.close()
    user_data['n'] += 1


def cmd_somaworkflow(labels, options, user_data):
    cmd = get_cmd_raw(labels, options, user_data)
    print(cmd)
    # user_data['output_fd'].write(cmd)
    from soma.workflow import client as swf
    job = swf.Job(cmd, name=str(labels))
    jobs = user_data.setdefault('jobs', [])
    jobs.append(job)


commands = {'grid': cmd_grid, 'duch': cmd_duch,
            'cath': cmd_cath, 'LSF': cmd_LSF,
            'somaworkflow': cmd_somaworkflow}


# Find user email thanks to his login and then his name / family name.
def autofind_email():
    import os
    import pwd
    try:
        names = pwd.getpwnam(os.getlogin())[4]
        return '.'.join(names.split(' ')[:2]) + '@cea.fr'
    except OSError:
        return ''


# Options parser
def parseOpts(argv):
    description = 'Generate a file of learning tasks. Each line ' + \
        'represents a call to siLearn command on a particular model.'
    parser = OptionParser(description)
    add_filter_options(parser)
    parser.add_option('-m', '--model', dest='modelfilename',
                      metavar='MODEL', action='store', default='model.arg',
                      help='model file name (default : %default)')
    parser.add_option('-e', '--email', dest='email',
                      metavar='MAIL', action='store', default=None,
                      help='(default : ' + autofind_email() + ')')
    parser.add_option('-p', '--parallelism-mode', dest='parallelmode',
                      metavar='MODE', action='store', default='duch',
                      help='%s' % repr(list(commands.keys())) + ' (default : %default)')
    parser.add_option('-o', '--output', dest='output',
                      metavar='FILE', action='store', default='learningtasks',
                      help='''output file name storing tasks (default : %default) or
pattern when several files are generated (LSF, cath) : output files are named
$pattern1, $pattern2...''')
    parser.add_option('-c', '--config', dest='cfg',
                      metavar='FILE', action='store', default='siLearn-read.cfg',
                      help='silearn config file (default : %default)')
    parser.add_option('-t', '--time', dest='time',
                      metavar='TIME', action='store', default='00:30',
                      help='format : hh:mm (default : %default (30 min))')
    parser.add_option('-b', '--bin', dest='bin',
                      metavar='FILE', action='store', default='siLearn.py',
                      help='siLearn binary (default : %default)')
    return parser.parse_args(argv)


# Cover
def get_edge_labels(el, user_data):
    label1 = el['label1']
    label2 = el['label2']
    user_data['cmd']([label1, label2], user_data['options'], user_data)


def get_vertex_labels(el, user_data):
    label = el['label']
    user_data['cmd']([label], user_data['options'], user_data)


# main function
def main():
    import sys
    import os

    # read options
    options, args = parseOpts(sys.argv)
    if not options.parallelmode in list(commands.keys()):
        msg.error('parallel mode must be one of ' +
                  repr(list(commands.keys())))
        sys.exit(1)
    if options.parallelmode == 'cath':
        msg.error('mode cath : not implemented yet')
        sys.exit(1)
    options.bin = distutils.spawn.find_executable(options.bin)

    # read model
    r = aims.Reader()
    model = r.read(options.modelfilename)
    if model is None:
        msg.error("can't open '%s'" % options.modelfilename)
        sys.exit(1)

    # cover model
    fundict = {'edge_before': get_edge_labels,
               'vertex_before': get_vertex_labels}
    data = {'options': options,
            'cmd': commands[options.parallelmode]}

    if options.parallelmode in ['grid', 'duch', 'somaworkflow']:
        fd = open(options.output, 'w')
        data['output_fd'] = fd
    elif options.parallelmode in ['cath', 'LSF']:
        import math
        data['n'] = 0
        size = cover_count(model)
        data['zeros'] = int(math.log(size) / math.log(10)) + 1
    if options.parallelmode == 'LSF':
        if options.email:
            data['email'] = options.email
        else:
            data['email'] = autofind_email()

    dir = os.path.dirname(options.output)
    if dir == '':
        dir = os.getcwd()
    data['output_dir'] = dir

    cover(model, fundict, data, options.labels_filter, options.filter_mode)

    if options.parallelmode == 'somaworkflow':
        from soma.workflow import client as swf
        wf = swf.Workflow(data['jobs'])
        import pickle
        pickle.dump(wf, data['output_fd'])

    if options.parallelmode in ['duch', 'grid', 'somaworkflow']:
        data['output_fd'].close()


if __name__ == '__main__':
    main()
