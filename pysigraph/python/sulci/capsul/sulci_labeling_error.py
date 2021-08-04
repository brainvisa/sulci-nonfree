
from __future__ import print_function

from __future__ import absolute_import
from capsul.api import Process
import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SulciLabelingError(Process):

    reference_graph = traits.File(output=False, allowed_extensions=['.arg'])
    test_graph = traits.File(output=False, allowed_extensions=['.arg'])
    output_csv = traits.File(output=True, allowed_extensions=['.csv'])
    loo_subject = traits.Str()
    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'],
                                         optional=False)

    def get_commandline(self):
        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m',
               'sulci.scripts.sulci_errors.siError',
               '-l', self.test_graph,
               '-t', self.labels_translation_map,
               '-b', self.reference_graph,
               '-s', self.loo_subject,
               '--csv', self.output_csv,
        ]
        print(cmd)
        return cmd

