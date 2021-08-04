
from __future__ import absolute_import
from __future__ import print_function
from capsul.api import Process

import traits.api as traits
import os
import glob
import sys
try:
    import subprocess32 as subprocess
except:
    import subprocess

class SpamTestLabelingLocal(Process):

    global_graph = traits.File(output=False, allowed_extensions=['.arg'])
    global_transformation = traits.File(output=False,
                                        allowed_extensions=['.trm'])
    model = traits.File(output=False, allowed_extensions=['.dat'])
    label_priors = traits.File(output=False, allowed_extensions=['.dat'])
    local_referentials = traits.File(output=False, allowed_extensions=['.dat'])
    translation_priors = traits.File(output=False,
                                     allowed_extensions=['.dat'])
    direction_priors = traits.File(output=False, allowed_extensions=['.dat'])
    angle_priors = traits.File(output=False, allowed_extensions=['.dat'])
    output_graph = traits.File(output=True, allowed_extensions=['.arg'])
    output_posterior_probabilities = traits.File(output=True,
                                          allowed_extensions=['.csv'])
    output_local_transformations = traits.Directory(output=True)

    labels_translation_map = traits.File(output=False,
                                         allowed_extensions=['.trl'],
                                         optional=False)

    def get_commandline(self):

        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m', 'capsul',
               'morphologist.capsul.axon.sulcilabellingspamlocal.SulciLabellingSPAMLocal',
               'data_graph=%s' % self.global_graph,
               'global_transformation=%s' % self.global_transformation,
               'model=%s' % self.model,
               'labels_translation_map=%s' % self.labels_translation_map,
               'labels_priors=%s' % self.label_priors,
               'local_referentials=%s' % self.local_referentials,
               'direction_priors=%s' % self.direction_priors,
               'angle_priors=%s' % self.angle_priors,
               'translation_priors=%s' % self.translation_priors,
               'output_graph=%s' % self.output_graph,
               'posterior_probabilities=%s'
                  % self.output_posterior_probabilities,
               'output_local_transformations=%s'
                  % self.output_local_transformations,
        ]
        print(cmd)
        return cmd

