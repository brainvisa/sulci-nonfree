from capsul.api import Pipeline
import traits.api as traits


class SpamLearnPipelineLOO(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_iterative_process("learn_pipeline", "sulci.capsul.spam_learn_pipeline_for_loo.SpamLearnPipelineForLOO", iterative_plugs=set([u'loo_subject', u'loo_graph']))

        # links
        self.export_parameter("learn_pipeline", "graphs")
        self.export_parameter("learn_pipeline", "loo_graph")
        self.export_parameter("learn_pipeline", "loo_subject")
        self.export_parameter("learn_pipeline", "threads")
        self.export_parameter("learn_pipeline", "verbose")
        self.export_parameter("learn_pipeline", "translation_file")
        self.export_parameter("learn_pipeline", "output_label_priors_directory")
        self.export_parameter("learn_pipeline", "output_talairach_directory")
        self.export_parameter("learn_pipeline", "output_global_registration_directory")
        self.export_parameter("learn_pipeline", "output_local_registration_directory")
        self.export_parameter("learn_pipeline", "output_local_referentials_directory")
        self.export_parameter("learn_pipeline", "output_transformation_priors_directory")

        # nodes positions
        self.node_position = {
            "inputs": (0.0, 78.0),
            "outputs": (450.58268999999996, 78.0),
            "learn_pipeline": (154.53878999999995, 0.0),
        }

        self.do_autoexport_nodes_parameters = False
