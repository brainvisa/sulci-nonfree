
from capsul.api import Process

import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamLearnTransformationPrior(Process):

    spams_directory = traits.Directory(output=False)
    output_translation_directory = traits.Directory(output=True)
    output_direction_directory = traits.Directory(output=True)
    output_angle_directory = traits.Directory(output=True)
    threads = traits.Int(0, optional=True)

    def _run_process(self):
        dat_files = glob.glob(os.path.join(self.spams_directory,
                                           '*_local.dat'))

        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m',
               'sulci.scripts.sulci_registration.learn_transformation_prior',
               '--translation-distribdir', self.output_translation_directory,
               '--direction-distribdir', self.output_direction_directory,
               '--angle-distribdir', self.output_angle_directory,
               '--threads', str(self.threads)] + dat_files
        print(cmd)
        subprocess.check_call(cmd)

