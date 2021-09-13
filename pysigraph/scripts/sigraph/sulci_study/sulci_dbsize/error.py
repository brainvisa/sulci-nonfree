#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import glob
import sys
import os
import numpy
import collections
from optparse import OptionParser
from datamind.io import csvIO
from six.moves import map

# -- global --


def get_global_errors(filename):
    r = csvIO.ReaderCsv()
    a = r.read(filename)
    return a[:, ["Subject", "Weighted_Mean_Local_SI_Error"]]


def samples_global_mean_error(filename):
    a = get_global_errors(filename)
    return a[:, 1].mean()


def samples_global_error(dir):
    files = glob.glob('%s/*/test_global.csv' % dir)
    n = len(files)
    errors = numpy.zeros(n)
    for i, file in enumerate(files):
        e = samples_global_mean_error(file)
        errors[i] = e
    return errors.mean(), errors.std(), n


def subjects_global_error(dir):
    files = glob.glob('%s/*/test_global.csv' % dir)
    n = len(files)
    errors = []
    for i, file in enumerate(files):
        errors.append(get_global_errors(file))
    errors = numpy.vstack(errors)
    if hasattr(numpy, 'unique1d'):
        subjects = numpy.unique1d(errors[:, 0])
    else:
        subjects = numpy.unique(errors[:, 0])
    se = numpy.array([errors[errors[:, 0] == s, 1].mean()
                      for s in subjects])
    if numpy.isnan(se).any():
        print(("warning: not enough sample: some subjects "
               "have not been tested"))
        se = se[numpy.logical_not(numpy.isnan(se))]
    return se.mean(), se.std(), n


# -- local --
def get_local_errors(filename):
    r = csvIO.ReaderCsv()
    a = r.read(filename)
    return a[:, ['subjects', 'sulci', 'size_errors']]


def get_sulcuswise_local_errors(filename):
    r = csvIO.ReaderCsv()
    a = r.read(filename)
    # list of sulci may change from one database sample to another
    if hasattr(numpy, 'unique1d'):
        sulci = numpy.unique1d(a[:, 'sulci'].tolist()).tolist()
    else:
        sulci = numpy.unique(a[:, 'sulci'].tolist()).tolist()
    res = {}
    sulci_col = a[:, 'sulci']
    for sulcus in sulci:
        s = a[:, ['subjects', 'size_errors']]
        res[sulcus] = s[sulci_col == a.code(sulcus)]
    return res


def samples_local_mean_error(filename):
    res = get_sulcuswise_local_errors(filename)
    for sulcus, a in res.items():
        res[sulcus] = a[:, 1].mean() * 100  # convert to percentage
    return res


def samples_local_error(dir):
    files = glob.glob('%s/*/test_local.csv' % dir)
    n = len(files)
    res = collections.defaultdict(lambda: numpy.zeros(n))
    for i, file in enumerate(files):
        r = samples_local_mean_error(file)
        for sulcus, error in r.items():
            res[sulcus][i] = error
    for sulcus, errors in res.items():
        res[sulcus] = errors.mean(), errors.std()
    return res, n


def subjects_local_error(dir):
    files = glob.glob('%s/*/test_local.csv' % dir)
    n = len(files)
    n = len(files)
    res = collections.defaultdict(lambda: [])
    errors = []
    for i, file in enumerate(files):
        errors.append(get_local_errors(file))
    code = errors[0].code
    decode = errors[0].decode
    errors = numpy.vstack(errors)
    if hasattr(numpy, 'unique1d'):
        subjects = numpy.unique1d(errors[:, 0])
        sulci = list(map(decode, numpy.unique1d(errors[:, 1])))
    else:
        subjects = numpy.unique(errors[:, 0])
        sulci = list(map(decode, numpy.unique(errors[:, 1])))
    res = {}
    sulci_col = errors[:, 1]
    for subject in subjects:
        su = errors[errors[:, 0] == subject]
        if len(su) == 0:
            print(("warning: not enough sample: some subjects "
                   "have not been tested"))
    for sulcus in sulci:
        se = errors[sulci_col == code(sulcus)][:, [0, 2]]
        su = numpy.array([se[se[:, 0] == subject, 1].mean()
                          for subject in subjects]) * 100
        if numpy.isnan(su).any():
            su = su[numpy.logical_not(numpy.isnan(su))]
        res[sulcus] = su.mean(), su.std()
    return res, n


################################################################################
def parseOpts(argv):
    usage = "Usage: %prog [OPTIONS] dir1 dir2...\n" + \
            "compute mean/std global or local error rate"
    parser = OptionParser(usage)
    parser.add_option('-o', '--output', dest='output',
                      metavar='FILE', action='store', default=None,
                      help='output csv filename')
    parser.add_option('-m', '--mode', dest='mode',
                      metavar='FILE', action='store', default='global',
                      type='choice', choices=('global', 'local'),
                      help="mode : 'global' or 'local' (default: %default)")
    parser.add_option('-g', '--group', dest='group',
                      metavar='FILE', action='store', default='subject',
                      type='choice', choices=('sample', 'subject'),
                      help="group first by 'sample' or 'subject' (default: %default),"
                      "then compute mean on each group. Finally compute mean and "
                      "standard deviation on the result")
    parser.add_option('-s', '--size', dest='size',
                      metavar='INT', action='store', default=None,
                      help='database size')  # size is stored as a string

    return parser, parser.parse_args(argv)


def main():
    # options
    parser, (options, args) = parseOpts(sys.argv)
    directories = args[1:]
    error = []
    if options.output is None:
        error.append("missing output")
    if options.size is None:
        error.append("missing database size")
    if len(directories) == 0:
        error.append("missing directory")
    if len(error):
        if len(error) == 1:
            print("error: %s" % error[0])
        else:
            print('error:\n    - ' + '\n    - '.join(error))
        print()
        parser.print_help()
        sys.exit(1)
    # compute errors
    if options.mode == 'global':
        main_global(options, directories)
    else:
        main_local(options, directories)


def main_global(options, directories):
    s = []
    for dir in directories:
        if options.group == 'subject':
            mean, std, n = subjects_global_error(dir)
        elif options.group == 'sample':
            mean, std, n = samples_global_error(dir)

        print("* " + dir)
        print("    mean (std) = %2.2f (%2.2f), n = %d" % (mean, std, n))
        s.append('%2.2f\t%2.2f\t%d' % (mean, std, n))

    # write result
    fd = open(options.output, 'a')
    add_header_if_needed(fd, directories)
    fd.write(options.size + '\t' + '\t'.join(s) + '\n')
    fd.close()


def main_local(options, directories):
    res = collections.defaultdict(lambda: numpy.zeros(len(directories) * 3))
    for i, dir in enumerate(directories):
        if options.group == 'subject':
            r, n = subjects_local_error(dir)
        elif options.group == 'sample':
            r, n = samples_local_error(dir)
        for sulcus, (error, std) in r.items():
            a = res[sulcus]
            a[3 * i:3 * i + 3] = error, std, n

    # write sulcuswise results
    format = ['%2.2f', '%2.2f', '%d']
    line_format = format * len(directories)
    for sulcus, r in res.items():
        base, ext = os.path.splitext(options.output)
        fd = open(base + '_' + sulcus + ext, 'a')
        add_header_if_needed(fd, directories)
        fd.write(options.size + '\t' + '\t'.join(
            map((lambda n, f: f % n), r, line_format)) + '\n')
        fd.close()

    # write modelwise results
    for i, dir in enumerate(directories):
        fd = open(os.path.join(dir, 'test_local.csv'), 'a')
        s = ''
        if fd.tell() == 0:
            s += 'sulci\tmean\tstd\tnumber\n'
        for sulcus, r in res.items():
            s += sulcus + '\t' + '\t'.join(format) % \
                tuple(r[3 * i: 3 * i + 3]) + '\n'
        fd.write(s)
        fd.close()


def add_header_if_needed(fd, directories):
    if fd.tell() == 0:
        fd.write('size\t' + '\t'.join(d.strip('/') + '_' + s
                                      for d in directories
                                      for s in ['mean', 'std', 'number']) + '\n')


if __name__ == '__main__':
    main()
