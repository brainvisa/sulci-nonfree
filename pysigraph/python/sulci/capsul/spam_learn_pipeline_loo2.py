from capsul.api import Pipeline
import traits.api as traits


class SpamLearnPipelineLOO(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_iterative_process("spam_loo", "sulci.capsul.spam_learn_pipeline_loo_step.Spam_learn_pipeline_loo_step", iterative_plugs=set([u'loo_subject', u'reference_graph', u'loo_graph']))

        # links
        self.export_parameter("spam_loo", "graphs")
        self.export_parameter("spam_loo", "loo_graph")
        self.export_parameter("spam_loo", "loo_subject")
        self.export_parameter("spam_loo", "threads")
        self.export_parameter("spam_loo", "translation_file")
        self.export_parameter("spam_loo", "verbose")
        self.export_parameter("spam_loo", "reference_graph")
        self.export_parameter("spam_loo", "pipeline_steps", "learn_steps")
        self.export_parameter("spam_loo",
                              "output_global_registration_directory")
        self.export_parameter("spam_loo",
                              "output_local_registration_directory")
        self.export_parameter("spam_loo",
                              "output_local_referentials_directory")
        self.export_parameter("spam_loo", "output_label_priors_directory")
        self.export_parameter("spam_loo", "output_talairach_directory")
        self.export_parameter("spam_loo", "output_local_angle_directory")
        self.export_parameter("spam_loo", "output_local_direction_directory")
        self.export_parameter("spam_loo", "output_local_translation_directory")
        self.export_parameter("spam_loo", "output_test_global_directory")
        self.export_parameter("spam_loo", "output_test_local_directory")

        # default and initial values
        self.loo_steps = {'testing': True, 'evaluation': True, 'learning': True}

        # nodes positions
        self.node_position = {
            "spam_loo": (168.16915000000003, 0.0),
            "inputs": (0.0, 130.0),
            "outputs": (433.50024999999994, 104.0),
        }

        self.do_autoexport_nodes_parameters = False
