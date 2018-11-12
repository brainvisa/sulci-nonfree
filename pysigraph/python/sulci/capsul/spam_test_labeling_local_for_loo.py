
from capsul.api import Process

import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamTestLabelingLocalForLOO(Process):

    global_graph_directory = traits.Directory(output=False)
    local_model_directory = traits.Directory(output=False)
    loo_subject = traits.Str()
    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'],
                                         optional=False)
    labels_priors_directory = traits.Directory(output=False)
    local_referentials_directory = traits.Directory(output=False)
    transformation_priors_directory = traits.Directory(output=False)
    output_test_directory = traits.Directory(output=True)

    def _run_process(self):

        data_graph = os.path.join(self.global_graph_directory,
                                  self.loo_subject,
                                  self.loo_subject + '_global.arg')
        global_transformation = os.path.join(
            self.global_graph_directory, self.loo_subject,
            self.loo_subject + '_Tal_TO_SPAM.trm')
        model = os.path.join(self.local_model_directory,
                             self.loo_subject + '.dat')
        labels_priors = os.path.join(self.labels_priors_directory,
                                     self.loo_subject + '.dat')
        local_referentials = os.path.join(self.local_referentials_directory,
                                          self.loo_subject + '.dat')
        direction_priors = os.path.join(self.transformation_priors_directory,
                                        self.loo_subject,
                                        'bingham_direction_trm_priors.dat')
        angle_priors = os.path.join(self.transformation_priors_directory,
                                    self.loo_subject,
                                    'vonmises_angle_trm_priors.dat')
        translation_priors = os.path.join(
            self.transformation_priors_directory, self.loo_subject,
            'gaussian_translation_trm_priors.dat')

        odir = os.path.join(self.output_test_directory, self.loo_subject)
        output_graph = os.path.join(odir, self.loo_subject + '_local.arg')
        try:
            os.makedirs(odir)
        except:
            pass
        posterior_probabilities = os.path.join(
            odir,  self.loo_subject + '_local_proba.csv')
        output_local_transformations = os.path.join(
            odir, self.loo_subject + '_global_TO_local')

        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m', 'capsul',
               'morphologist.capsul.axon.sulcilabellingspamlocal.SulciLabellingSPAMLocal',
               'data_graph=%s' % data_graph,
               'global_transformation=%s' % global_transformation,
               'model=%s' % model,
               'labels_translation_map=%s' % self.labels_translation_map,
               'labels_priors=%s' % labels_priors,
               'local_referentials=%s' % local_referentials,
               'direction_priors=%s' % direction_priors,
               'angle_priors=%s' % angle_priors,
               'translation_priors=%s' % translation_priors,
               'output_graph=%s' % output_graph,
               'posterior_probabilities=%s' % posterior_probabilities,
               'output_local_transformations=%s'
                  % output_local_transformations,
        ]
        print(cmd)
        subprocess.check_call(cmd)

