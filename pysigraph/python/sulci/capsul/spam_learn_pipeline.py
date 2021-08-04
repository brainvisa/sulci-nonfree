from __future__ import absolute_import
from capsul.api import Pipeline
import traits.api as traits


class SpamLearnPipeline(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("label_priors", "sulci.capsul.spam_learn_label_priors.SpamLearnLabelPriors")
        self.add_process("global", "sulci.capsul.spam_learn_global_registration.SpamLearnGlobalRegistration")
        self.add_process("talairach", "sulci.capsul.spam_learn_talairach.SpamLearnTalairach")
        self.add_process("local", "sulci.capsul.spam_learn_local_registration.SpamLearnLocalRegistration")
        self.add_process("transformation_priors", "sulci.capsul.spam_learn_transformation_priors.SpamLearnTransformationPrior")

        # links
        self.export_parameter("talairach", "graphs")
        self.add_link("graphs->local.graphs")
        self.add_link("graphs->global.graphs")
        self.add_link("graphs->label_priors.graphs")
        self.export_parameter("local", "threads")
        self.add_link("threads->global.threads")
        self.add_link("threads->transformation_priors.threads")
        self.export_parameter("local", "translation_file")
        self.add_link("translation_file->label_priors.translation_file")
        self.add_link("translation_file->talairach.translation_file")
        self.add_link("translation_file->global.translation_file")
        self.export_parameter("global", "verbose")
        self.add_link("verbose->local.verbose")
        self.export_parameter("label_priors", "output_directory", "label_priors_output_directory")
        self.add_link("global.output_directory->local.global_spam_distribs")
        self.export_parameter("global", "output_directory", "global_output_directory")
        self.export_parameter("talairach", "output_directory", "talairach_output_directory")
        self.add_link("local.output_directory->transformation_priors.spams_directory")
        self.export_parameter("local", "output_directory", "local_output_directory")
        self.export_parameter("local", "output_local_referentials_directory")
        self.export_parameter("transformation_priors", "output_angle_directory")
        self.export_parameter("transformation_priors", "output_direction_directory")
        self.export_parameter("transformation_priors", "output_translation_directory")

        # nodes positions
        self.node_position = {
            "transformation_priors": (919.0329525657005, -109.26440796846478),
            "inputs": (-102.73422411832483, 297.9561),
            "outputs": (1315.0704450750022, 61.62590105778103),
            "global": (154.83373423006515, -52.61646184737708),
            "talairach": (712.0065613813068, 458.01151466850837),
            "label_priors": (320.6643032712702, 521.3215415366058),
            "local": (466.867762509301, 181.7481753878912),
        }

        # nodes dimensions
        self.node_dimension = {
            "nnn": (133.0, 145.0),
            "label_priors_filename": (249.875, 145.0),
            "inputs": (141.98904525915623, 180.0),
            "direction_priors_filename": (259.75, 110.0),
            "global_model_filename": (217.875, 110.0),
            "angle_directory": (262.75, 145.0),
            "global": (244.75, 180.0),
            "talairach_directory": (234.625, 145.0),
            "global_output_directory": (265.75, 145.0),
            "cv_subject_filter": (133.0, 145.0),
            "global_error_filename": (157.78125, 218.0),
            "loo_global_test_dir": (235.875, 113.0),
            "direction_directory": (285.625, 145.0),
            "global_error": (245.875, 180.0),
            "local_test_proba_filename": (226.90979295522197, 218.0),
            "local_error_filename": (148.78125, 218.0),
            "loo_local_test_dir": (225.875, 113.0),
            "label_priors_directory": (324.75, 145.0),
            "local_model_filename": (208.0, 110.0),
            "global_csv_filename": (255.0, 215.0),
            "local_test_transform_dirname": (223.90979295522197, 218.0),
            "talairach": (235.75, 110.0),
            "local_posterior_probabilities_filename": (360.875, 145.0),
            "test": (420.75, 390.0),
            "global_test_transform_filename": (219.5625, 218.0),
            "t1_to_global_transformation_filename": (360.75, 145.0),
            "local": (393.625, 215.0),
            "local_referentials_filename": (281.5, 110.0),
            "translation_directory": (323.625, 145.0),
            "transformation_priors": (326.625, 145.0),
            "angle_priors_filename": (214.0, 110.0),
            "global_test_proba_filename": (271.909792955222, 218.0),
            "outputs": (251.65979295522197, 320.0),
            "gogo": (189.75, 145.0),
            "subject_test_directory": (237.75, 180.0),
            "local_transformations_directory": (334.75, 145.0),
            "cv_filter": (133.0, 145.0),
            "local_transformations_dirname": (290.75, 218.0),
            "global_test_graph_name": (173.0, 218.0),
            "local_test_graph_name": (164.0, 218.0),
            "local_csv_filename": (246.0, 215.0),
            "global_test_t1_transform_filename": (263.909792955222, 218.0),
            "test_global": (409.75, 180.0),
            "spam_learn_cv": (209.75, 320.0),
            "local_referentials_directory": (339.5, 145.0),
            "global_posterior_probabilities_filename": (381.875, 145.0),
            "local_output_directory": (256.75, 145.0),
            "test_local": (375.75, 355.0),
            "local_error": (245.875, 180.0),
            "label_priors": (235.75, 110.0),
            "output_local_referentials_dir": (427.25, 145.0),
            "translation_priors_filename": (287.75, 110.0),
        }

        self.do_autoexport_nodes_parameters = False
