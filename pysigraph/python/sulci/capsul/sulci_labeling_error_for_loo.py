
from __future__ import print_function

from capsul.api import Process
import traits.api as traits
import os
import glob
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SulciLabelingErrorForLOO(Process):

    reference_graph = traits.File(output=False, allowed_extensions=['.arg'])
    global_test_directory = traits.Directory(output=False)
    loo_subject = traits.Str()
    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'],
                                         optional=False)
    graph_suffix = traits.Str('_global', output=False, optional=True)
    output_test_directory = traits.Directory(output=True)

    def _run_process(self):

        odir = os.path.join(self.output_test_directory, self.loo_subject)
        test_graph = os.path.join(
            odir, self.loo_subject + self.graph_suffix + '.arg')
        output_csv = os.path.join(
            odir, self.loo_subject + self.graph_suffix + '_err.csv')
        try:
            os.makedirs(odir)
        except:
            pass

        cmd = ['python', '-m',
               'sulci.scripts.sulci_errors.siError',
               '-l', test_graph,
               '-t', self.labels_translation_map,
               '-b', self.reference_graph,
               '-s', self.loo_subject,
               '--csv', output_csv,
        ]
        print(cmd)
        subprocess.check_call(cmd)

