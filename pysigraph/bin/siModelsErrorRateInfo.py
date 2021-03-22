#!/usr/bin/env python2

from __future__ import absolute_import
from optparse import OptionParser
import numpy
from soma import aims
from sigraph import *
from sigraph.cover import *
from datamind.tools import *
import sigraph.test_models as test_models
from six.moves import map
from six.moves import zip


# error related functions
def get_errors(adaptiveleaf, data):
    sa = adaptiveleaf.workEl()
    re = sa.genErrorRate()
    me = sa.genMeanErrorRate()
    ge = sa.genGoodErrorRate()
    be = sa.genBadErrorRate()
    for (k, v) in zip(['Raw', 'Mean', 'Good', 'Bad'], [re, me, ge, be]):
        if numpy.isnan(v):
            l = str(adaptiveleaf.topModel().significantLabels())
            msg.warning("found NaN value in (%s) '%s'" % (k, l))
        else:
            data[k].append(v)

# Options parser


def parseOpts(argv):
    description = 'Compute mean/std/min/max of model errors ' \
        '(good, bad, mean) and could make an histogram.'
    parser = OptionParser(description)
    add_filter_options(parser)
    parser.add_option('--filter-fifty-percent', dest='filter',
                      action='store_true', default=False,
                      help='filter values equal to 0.5')
    parser.add_option('-m', '--model', dest='modelfilename',
                      metavar='MODEL', action='store', default='model.arg',
                      help='model file name (default : %default)')
    parser.add_option('--hist', dest='hist',
                      action='store', default=None,
                      help='make histogram from classifiation/regression rates'
                      'of models. HIST should be a format supported by pylab')
    return parser, parser.parse_args(argv)


# main : load data and compute some error related stuff
def main():
    # read options
    import sys
    parser, (options, args) = parseOpts(sys.argv)
    if len(args) != 1:
        parser.print_help()
        sys.exit(1)

    # read model
    r = aims.Reader()
    model = r.read(options.modelfilename)

    # cover all models and print info
    data = {'Raw': [], 'Mean': [],
            'Good': [], 'Bad': []}
    fundict = {'adaptiveleaf': get_errors}
    cover(model, fundict, data, options.labels_filter, options.filter_mode)

    # fiter definition
    def nofilter(k_v): return (k_v[0], k_v[1])

    def filter_fifty_percent_rate(k_v1): return \
        (k_v1[0], [x for x in k_v1[1] if x != 0.5])

    error_filter = nofilter
    if options.filter:
        error_filter = filter_fifty_percent_rate
    data_filtered = {}
    for k, v in map(error_filter, list(data.items())):
        data_filtered[k] = v
    test_models.resume_errors_info(data_filtered)
    if options.hist:
        test_models.make_errors_hist(options.hist, data_filtered)


if __name__ == '__main__':
    main()
