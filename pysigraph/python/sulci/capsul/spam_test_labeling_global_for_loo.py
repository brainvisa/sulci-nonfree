
from capsul.api import Process

import traits.api as traits
import os
import glob
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamTestLabelingGlobalForLOO(Process):

    data_graph = traits.File(output=False, allowed_extensions=['.arg'])
    global_model_directory = traits.Directory(output=False)
    loo_subject = traits.Str()
    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'],
                                         optional=False)
    labels_priors_directory = traits.Directory(output=False)
    output_directory = traits.Directory(output=True)

    def _run_process(self):

        model = os.path.join(self.global_model_directory,
                             self.loo_subject + '.dat')
        labels_priors = os.path.join(self.labels_priors_directory,
                                     self.loo_subject + '.dat')
        odir = os.path.join(self.output_directory, self.loo_subject)
        output_graph = os.path.join(odir, self.loo_subject + '_global.arg')
        os.makedirs(odir)
        posterior_probabilities = os.path.join(
            odir,  self.loo_subject + '_global_proba.csv')
        output_transformation = os.path.join(
            odir, self.loo_subject + '_Tal_TO_SPAM.trm')
        output_t1_to_global_transformation = os.path.join(
            odir, self.loo_subject + '_T1_TO_SPAM.trm')

        cmd = ['python', '-m', 'capsul',
               'morphologist.capsul.axon.sulcilabellingspamglobal.SulciLabellingSPAMGlobal',
               'data_graph=%s' % self.data_graph,
               'model_type=Global registration',
               'model=%s' % model,
               'labels_translation_map=%s' % self.labels_translation_map,
               'labels_priors=%s' % labels_priors,
               'output_graph=%s' % output_graph,
               'posterior_probabilities=%s' % posterior_probabilities,
               'output_transformation=%s' % output_transformation,
               'output_t1_to_global_transformation=%s'
                  % output_t1_to_global_transformation,
        ]
        print(cmd)
        subprocess.check_call(cmd)

