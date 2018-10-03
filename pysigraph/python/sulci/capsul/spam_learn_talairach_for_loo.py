
from capsul.api import Process

import traits.api as traits
import os

class SpamLearnTalairachForLOO(Process):

    translation_file = traits.File(output=False, allowed_extensions=['.trl'],
                                   optional=True)
    output_directory = traits.Directory(output=True)
    graphs = traits.List(traits.File(output=False,
                                     allowed_extensions=['.arg']))
    loo_graph = traits.File(output=False, allowed_extensions=['.arg'])
    loo_subject = traits.Str()

    def get_commandline(self):
        loo_dir = os.path.join(self.output_directory, self.loo_subject)
        graphs = list(self.graphs)
        graphs.remove(self.loo_graph)

        cmd = ['python', '-m', 'sulci.scripts.learn_spams_distributions']
        if self.translation_file not in (None, '', traits.Undefined):
            cmd += ['-t', self.translation_file]
        cmd += ['-d', loo_dir] + graphs
        return cmd

