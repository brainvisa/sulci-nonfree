
from capsul.api import Process

import traits.api as traits
import os

class SpamLearnTalairach(Process):

    translation_file = traits.File(output=False, allowed_extensions=['.trl'],
                                   optional=True)
    output_directory = traits.Directory(output=True)
    graphs = traits.List(traits.File(output=False,
                                     allowed_extensions=['.arg']))

    def get_commandline(self):
        cmd = ['python', '-m', 'sulci.scripts.learn_spams_distributions']
        if self.translation_file not in (None, '', traits.Undefined):
            cmd += ['-t', self.translation_file]
        cmd += ['-d', self.output_directory] + self.graphs
        return cmd

