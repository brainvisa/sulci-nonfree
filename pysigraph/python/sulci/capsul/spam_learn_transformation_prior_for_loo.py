
from capsul.api import Process

import traits.api as traits
import os
import glob

class SpamLearnTransformationPriorForLOO(Process):

    spams_directory = traits.Directory(output=False)
    loo_subject = traits.Str()
    output_directory = traits.Directory(output=True)

    def _run_process(self):
        loo_dir = os.path.join(self.output_directory, self.loo_subject)
        translation_dir = os.path.join(loo_dir,
                                       'gaussian_translation_trm_priors')
        direction_dir = os.path.join(loo_dir, 'bingham_direction_trm_priors')
        angle_dir = os.path.join(loo_dir, 'vonmises_angle_trm_priors')

        dat_files = glob.glob(os.path.join(self.spams_directory,
                                           self.loo_subject,
                                           '*_local.dat'))

        cmd = ['python', '-m',
               'sulci.scripts.sulci_registration.learn_transformation_prior',
               '--translation-distribdir', translation_dir,
               '--direction-distribdir', direction_dir,
               '--angle-distribdir', angle_dir] + dat_files
        return cmd

