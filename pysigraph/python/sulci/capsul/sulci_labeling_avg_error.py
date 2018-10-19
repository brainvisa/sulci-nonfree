
from __future__ import print_function

from capsul.api import Process
import traits.api as traits
import os
import glob

class SulciLabelingAvgError(Process):

    error_dir = traits.List(traits.Directory(output=False))
    output_file = traits.File(output=False, allowed_extensions=['.csv'])

    def _run_process(self):
        stats = []
        bylabel = []
        bysubject = []
        for d in self.error_dir:
            sum = 0.
            weight = 0.
            sbylabel = {}
            wbylabel = {}
            sbysubject = {}
            wbysubject = {}
            for sd in os.listdir(d):
                fd = os.path.join(d, sd)
                if os.path.isdir(fd):
                    for f in os.listdir(fd):
                        if f.endswith('_err.csv'):
                            ef = os.path.join(fd, f)
                            print('read:', ef)
                            with open(ef) as ief:
                                hdr = ief.readline().strip().split()
                                SI = hdr.index('SI_error')
                                TP = hdr.index('true_positive')
                                FP = hdr.index('false_positive')
                                FN = hdr.index('false_negative')
                                sj = hdr.index('subjects')
                                sc = hdr.index('sulci')
                                while ief:
                                    l = ief.readline().strip().split()
                                    if l:
                                        w = float(l[FN]) + float(l[TP])
                                        s = float(l[SI]) * w
                                        subject = l[sj]
                                        sulcus = l[sc]
                                        if sulcus != 'unknown' \
                                                and not sulcus.startswith(
                                                    'ventricle'):
                                            sum += s
                                            weight += w
                                            wbysubject[subject] \
                                                = wbysubject.setdefault(
                                                    subject, 0.) + w
                                            sbysubject[subject] \
                                                = sbysubject.setdefault(
                                                    subject, 0.) + s
                                        wbylabel[sulcus] \
                                            = wbylabel.setdefault(sulcus, 0.) \
                                                + w
                                        sbylabel[sulcus] \
                                            = sbylabel.setdefault(sulcus, 0.) \
                                                + s
                                    else:
                                        break
            if weight != 0.:
                avg = sum / weight
            else:
                avg = 0.
            stats.append((d, avg))
            for subject in sorted(sbysubject.keys()):
                s = sbysubject[subject]
                w = wbysubject[subject]
                err = 0.
                if w != 0.:
                    err = s / w
                bysubject.append((d, subject, err))
            for sulcus in sorted(sbylabel.keys()):
                s = sbylabel[sulcus]
                w = wbylabel[sulcus]
                err = 0.
                if w != 0.:
                    err = s / w
                bylabel.append((d, sulcus, err))

        with open(self.output_file, 'w') as wf:
            print('case,       subject,    sulcus,     Avg.SI', file=wf)
            for stat in stats:
                print('%s, , ,  %f' % stat, file=wf)
            for stat in bysubject:
                print('%s, %s, , %f' % stat, file=wf)
            for stat in bylabel:
                print('%s, , %s, %f' % stat, file=wf)

