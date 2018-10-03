
from capsul.api import Process

import traits.api as traits
import os

class SpamLearnLocalRegistrationForLOO(Process):

    translation_file = traits.File(output=False, allowed_extensions=['.trl'],
                                   optional=True)
    output_directory = traits.Directory(output=True)
    graphs = traits.List(traits.File(output=False,
                                     allowed_extensions=['.arg']))
    loo_graph = traits.File(output=False, allowed_extensions=['.arg'])
    loo_subject = traits.Str()
    threads = traits.Int(0)
    output_local_referentials_directory = traits.Directory(output=True)
    verbose = traits.Int(0)

    @property
    def native_specification(self):
        if self.threads > 1:
            return '-l nodes=1:ppn=%d' % self.threads

    def get_commandline(self):
        loo_dir = os.path.join(self.output_directory, self.loo_subject)
        graphs = list(self.graphs)
        graphs.remove(self.loo_graph)
        local_referentials_dir = os.path.join(
            self.output_local_referentials_directory, self.loo_subject)

        cmd = ['python', '-m',
               'sulci.scripts.sulci_registration.'
               'learn_registred_spams_distributions', '--mode', 'local']
        if self.translation_file not in (None, '', traits.Undefined):
            cmd += ['-t', self.translation_file]
        cmd += ['--distrib-gaussians', local_referentials_dir]
        cmd += ['--threads', str(self.threads), '-d', loo_dir] + graphs
        if self.verbose != 0:
            cmd += ['--verbose', str(self.verbose)]
        return cmd

