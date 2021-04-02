#!/usr/bin/env python

from __future__ import absolute_import
from soma import aims
from sigraph import *
from sigraph.cover import *
from optparse import OptionParser


def to_key(labels):
    return '-'.join([l for l in labels if l != 'unknown'])

# Options parser


def parseOpts(argv):
    description = 'Get all descriptors from a model and set it into an ' + \
        ' other model.'
    parser = OptionParser(description)
    add_filter_options(parser)
    parser.add_option('-s', '--src', dest='src',
                      metavar='MODEL', action='store', default='model.arg',
                      help='model file name (default : %default)')
    parser.add_option('-d', '--dst', dest='dst',
                      metavar='MODEL', action='store', default='model.arg',
                      help='model file name (default : %default)')
    return parser, parser.parse_args(argv)


def get_descr(ad, user_data):
    descr = ad.getAdapDescr().clone()
    key = to_key(ad.topModel().significantLabels())
    user_data['descr'][key] = descr


def set_descr(ad, user_data):
    key = to_key(ad.topModel().significantLabels())
    ad.setCliqueDescr(user_data['descr'][key])

# main function


def main():
    import sys

    # read options
    parser, (options, args) = parseOpts(sys.argv)
    if options.src == options.dst or len(args) > 1:
        parser.print_help()
        sys.exit(1)

    # read model
    r = aims.Reader()
    model_src = r.read(options.src)
    model_dst = r.read(options.dst)

    # cover model
    fundict_src = {'adaptiveleaf': get_descr}
    fundict_dst = {'adaptiveleaf': set_descr}
    data = {'descr': {}}
    cover(model_src, fundict_src, data,
          options.labels_filter, options.filter_mode)
    cover(model_dst, fundict_dst, data,
          options.labels_filter, options.filter_mode)

    save(model_dst, options.dst, options.labels_filter, options.filter_mode)


if __name__ == '__main__':
    main()
