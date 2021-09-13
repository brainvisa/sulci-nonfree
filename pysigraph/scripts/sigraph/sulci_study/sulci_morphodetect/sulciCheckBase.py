#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sys
import os
import six

if sys.argv[1] in ('-h', '--help'):
    print('usage:')
    print(
        sys.argv[0], 'normalstats.txt threshold basesstatsfile.txt [basesstatsfile2.txt ...]')
    print('Checks the value for each sulcus / std deviation.')
    print('statsfiles must be computed using sulciRecordStats.py')
    print('The output is the ratio score for each subject/sulcus. Thresholding is applied: only values above the threshold are printed.')
    print('Output goes to the standard output and can be redirected to a file')
    sys.exit(0)

normalstats = sys.argv[1]
threshold = float(sys.argv[2])
basesstatsfiles = sys.argv[3:]

exec(compile(open( normalstats, "rb" ).read(), normalstats, 'exec'))
basestats = []

for base in basesstatsfiles:
    exec(compile(open( base, "rb" ).read(), base, 'exec'))
    basestats.append(subjectspotentials)

exec(compile(open( normalstats, "rb" ).read(), normalstats, 'exec'))

detected = []

for base in basestats:
    for subject, stats in six.iteritems(base):
        for label, pot in six.iteritems(stats):
            s = totalstats.get(label)
            if s and 'std' in s:
                stddev = s['std']
                if stddev != 0:
                    score = (pot - s['mean']) / stddev
                    #print(subject, label, ':', score)
                    if score >= threshold:
                        # print('****')
                        detected.append((subject, label, score))

for l in detected:
    print(os.path.basename(l[0]) + '\t' + l[1] + '\t' + str(l[2]))
