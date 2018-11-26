
from capsul.api import Process

import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamLearnTransformationPriorForLOO(Process):

    spams_directory = traits.Directory(output=False)
    loo_subject = traits.Str()
    output_transform_directory = traits.Directory(output=True)
    threads = traits.Int(0, optional=True)

    def _run_process(self):
        loo_dir = os.path.join(self.output_transform_directory, self.loo_subject)
        translation_dir = os.path.join(loo_dir,
                                       'gaussian_translation_trm_priors')
        direction_dir = os.path.join(loo_dir, 'bingham_direction_trm_priors')
        angle_dir = os.path.join(loo_dir, 'vonmises_angle_trm_priors')

        dat_files = glob.glob(os.path.join(self.spams_directory,
                                           self.loo_subject,
                                           '*_local.dat'))

        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m',
               'sulci.scripts.sulci_registration.learn_transformation_prior',
               '--translation-distribdir', translation_dir,
               '--direction-distribdir', direction_dir,
               '--angle-distribdir', angle_dir,
               '--threads', str(self.threads)] + dat_files
        #print(cmd)
        subprocess.check_call(cmd)

