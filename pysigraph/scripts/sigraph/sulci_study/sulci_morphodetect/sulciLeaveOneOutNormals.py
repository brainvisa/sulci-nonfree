#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sys
import os
import math
import six


if sys.argv[1] in ('-h', '--help'):
    print('usage:')
    print(sys.argv[0], 'statsfile.txt')
    print('Checks the value for each sulcus / std deviation, in a leqve-one-out way.')
    print('statsfile.txt must be computed using sulciRecordStats.py')
    print('The output is the ratio score for each subject/sulcus. No filtering/thresholding is applied.')
    print('Output goes to the standard output and can be redirected to a file')
    sys.exit(0)

statsfile = sys.argv[1]

exec(compile(open( statsfile, "rb" ).read(), statsfile, 'exec'))

for s, stats in six.iteritems(subjectspotentials):
    for label, pot in six.iteritems(stats):
        st = totalstats.get(label)
        if st and 'std' in st and st[ 'number' ] >= 2:
            # remove this subject from stats
            sum = st['sum'] - pot
            sqsum = st['squaresum'] - pot*pot
            num = st['number'] - 1
            mean = sum / num
            std = math.sqrt(sqsum / (num - 1) - mean * mean)
            if std != 0:
                score = (pot - mean) / std
                print(os.path.basename(s) + '\t' + label + '\t' + str(score))
