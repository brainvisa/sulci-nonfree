
from capsul.api import Process

import traits.api as traits
import os
import sys

class SpamLearnGlobalRegistrationForLOO(Process):

    translation_file = traits.File(output=False, allowed_extensions=['.trl'],
                                   optional=True)
    output_directory = traits.Directory(output=True)
    graphs = traits.List(traits.File(output=False,
                                     allowed_extensions=['.arg']))
    loo_graph = traits.File(output=False, allowed_extensions=['.arg'])
    loo_subject = traits.Str()
    threads = traits.Int(0)
    verbose = traits.Int(0)

    @property
    def parallel_job_info(self):
        if self.threads > 1:
            return {'config_name': 'native',
                    'nodes_number': 1,
                    'cpu_per_node': self.threads}
        return {}

    def get_commandline(self):
        loo_dir = os.path.join(self.output_directory, self.loo_subject)
        graphs = list(self.graphs)
        graphs.remove(self.loo_graph)

        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m',
               'sulci.scripts.sulci_registration.'
               'learn_registred_spams_distributions']
        if self.translation_file not in (None, '', traits.Undefined):
            cmd += ['-t', self.translation_file]
        cmd += ['--threads', str(self.threads), '-d', loo_dir] + graphs
        if self.verbose != 0:
            cmd += ['--verbose', str(self.verbose)]
        return cmd

