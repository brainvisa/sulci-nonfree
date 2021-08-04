
from __future__ import absolute_import
from capsul.api import Process

import traits.api as traits
import os
import sys

class SpamLearnLocalRegistration(Process):

    translation_file = traits.File(output=False, allowed_extensions=['.trl'],
                                   optional=True)
    output_directory = traits.Directory(output=True)
    graphs = traits.List(traits.File(output=False,
                                     allowed_extensions=['.arg']))
    global_spam_distribs = traits.Directory(output=False)
    threads = traits.Int(0)
    output_local_referentials_directory = traits.Directory(output=True)
    verbose = traits.Int(0)

    @property
    def parallel_job_info(self):
        if self.threads > 1:
            return {'config_name': 'native',
                    'nodes_number': 1,
                    'cpu_per_node': self.threads}
        return {}

    def get_commandline(self):
        python_cmd = os.path.basename(sys.executable)
        cmd = [python_cmd, '-m',
               'sulci.scripts.sulci_registration.'
               'learn_registred_spams_distributions', '--mode', 'local']
        if self.translation_file not in (None, '', traits.Undefined):
            cmd += ['-t', self.translation_file]
        cmd += ['--distrib-gaussians',
                self.output_local_referentials_directory]
        cmd += ['--threads', str(self.threads), '-d', self.output_directory]
        if self.verbose != 0:
            cmd += ['--verbose', str(self.verbose)]
        cmd += self.graphs
        if self.global_spam_distribs is not None:
            cmd.append('==')
            motions = [os.path.join(self.global_spam_distribs,
                                    os.path.basename(g)[:-4] + '_motion.trm')
                       for g in self.graphs]
            cmd += motions
        return cmd

