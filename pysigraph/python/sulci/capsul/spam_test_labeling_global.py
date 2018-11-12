
from capsul.api import Process

import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamTestLabelingGlobal(Process):

    data_graph = traits.File(output=False, allowed_extensions=['.arg'])
    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'])
    model = traits.File(output=False, allowed_extensions=['.dat'])
    label_priors = traits.File(output=False, allowed_extensions=['.dat'])

    output_graph = traits.File(output=True, allowed_extensions=['.arg'])
    output_posterior_probabilities = traits.File(output=True,
                                                 allowed_extensions=['.csv'])
    output_transformation = traits.File(output=True,
                                        allowed_extensions=['.trm'])
    output_t1_to_global_transformation \
        = traits.File(output=True, allowed_extensions=['.trm'])

    def get_commandline(self):
        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m', 'capsul',
               'morphologist.capsul.axon.sulcilabellingspamglobal.SulciLabellingSPAMGlobal',
               'data_graph=%s' % self.data_graph,
               'model_type=Global registration',
               'model=%s' % self.model,
               'labels_translation_map=%s' % self.labels_translation_map,
               'labels_priors=%s' % self.label_priors,
               'output_graph=%s' % self.output_graph,
               'posterior_probabilities=%s'
                  % self.output_posterior_probabilities,
               'output_transformation=%s' % self.output_transformation,
               'output_t1_to_global_transformation=%s'
                  % self.output_t1_to_global_transformation,
        ]
        print(cmd)
        return cmd

