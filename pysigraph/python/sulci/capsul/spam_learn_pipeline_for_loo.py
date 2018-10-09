from capsul.api import Pipeline
import traits.api as traits


class SpamLearnPipelineForLOO(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("label_priors", "sulci.capsul.spam_learn_label_priors_for_loo.SpamLearnLabelPriorsForLOO")
        self.add_process("learn_talairach", "sulci.capsul.spam_learn_talairach_for_loo.SpamLearnTalairachForLOO")
        self.add_process("learn_global_registration", "sulci.capsul.spam_learn_global_registration_for_loo.SpamLearnGlobalRegistrationForLOO")
        self.add_process("learn_local_registraton", "sulci.capsul.spam_learn_local_registration_for_loo.SpamLearnLocalRegistrationForLOO")
        self.add_process("learn_transformation_priors", "sulci.capsul.spam_learn_transformation_prior_for_loo.SpamLearnTransformationPriorForLOO")
        self.add_process("test_labeling_global", "sulci.capsul.spam_test_labeling_global_for_loo.SpamTestLabelingGlobalForLOO")
        self.add_process("test_labeling_local", "sulci.capsul.spam_test_labeling_local_for_loo.SpamTestLabelingLocalForLOO")
        self.add_process("global_error", "sulci.capsul.sulci_labeling_error_for_loo.SulciLabelingErrorForLOO")
        self.add_process("local_error", "sulci.capsul.sulci_labeling_error_for_loo.SulciLabelingErrorForLOO")

        # links
        self.export_parameter("learn_local_registraton", "graphs")
        self.add_link("graphs->learn_global_registration.graphs")
        self.add_link("graphs->label_priors.graphs")
        self.add_link("graphs->learn_talairach.graphs")
        self.export_parameter("test_labeling_global", "data_graph", "loo_graph")
        self.add_link("loo_graph->learn_global_registration.loo_graph")
        self.add_link("loo_graph->learn_local_registraton.loo_graph")
        self.add_link("loo_graph->learn_talairach.loo_graph")
        self.add_link("loo_graph->label_priors.loo_graph")
        self.export_parameter("learn_transformation_priors", "loo_subject")
        self.add_link("loo_subject->local_error.loo_subject")
        self.add_link("loo_subject->global_error.loo_subject")
        self.add_link("loo_subject->learn_global_registration.loo_subject")
        self.add_link("loo_subject->learn_local_registraton.loo_subject")
        self.add_link("loo_subject->test_labeling_local.loo_subject")
        self.add_link("loo_subject->learn_talairach.loo_subject")
        self.add_link("loo_subject->test_labeling_global.loo_subject")
        self.add_link("loo_subject->label_priors.loo_subject")
        self.export_parameter("label_priors", "translation_file")
        self.add_link("translation_file->learn_talairach.translation_file")
        self.add_link("translation_file->test_labeling_local.labels_translation_map")
        self.add_link("translation_file->global_error.labels_translation_map")
        self.add_link("translation_file->learn_global_registration.translation_file")
        self.add_link("translation_file->learn_local_registraton.translation_file")
        self.add_link("translation_file->local_error.labels_translation_map")
        self.add_link("translation_file->test_labeling_global.labels_translation_map")
        self.export_parameter("learn_global_registration", "threads")
        self.add_link("threads->learn_local_registraton.threads")
        self.export_parameter("learn_local_registraton", "verbose")
        self.add_link("verbose->learn_global_registration.verbose")
        self.export_parameter("local_error", "reference_graph")
        self.add_link("reference_graph->global_error.reference_graph")
        self.export_parameter("label_priors", "output_directory", "output_label_priors_directory")
        self.add_link("label_priors.output_directory->test_labeling_global.labels_priors_directory")
        self.add_link("label_priors.output_directory->test_labeling_local.labels_priors_directory")
        self.export_parameter("learn_talairach", "output_directory", "output_talairach_directory")
        self.export_parameter("learn_global_registration", "output_directory", "output_global_registration_directory")
        self.add_link("learn_global_registration.output_directory->test_labeling_global.global_model_directory")
        self.export_parameter("learn_local_registraton", "output_directory", "output_local_registration_directory")
        self.add_link("learn_local_registraton.output_directory->test_labeling_local.local_model_directory")
        self.add_link("learn_local_registraton.output_directory->learn_transformation_priors.spams_directory")
        self.export_parameter("learn_local_registraton", "output_local_referentials_directory")
        self.add_link("learn_local_registraton.output_local_referentials_directory->test_labeling_local.local_referentials_directory")
        self.export_parameter("learn_transformation_priors", "output_transform_directory", "output_transformation_priors_directory")
        self.add_link("learn_transformation_priors.output_transform_directory->test_labeling_local.transformation_priors_directory")
        self.add_link("test_labeling_global.output_test_directory->global_error.global_test_directory")
        self.add_link("test_labeling_global.output_test_directory->test_labeling_local.global_graph_directory")
        self.export_parameter("test_labeling_global", "output_test_directory", "output_test_global_directory")
        self.export_parameter("test_labeling_local", "output_test_directory", "output_test_local_directory")
        self.add_link("test_labeling_local.output_test_directory->local_error.global_test_directory")
        self.add_link("global_error.output_test_directory->output_test_global_directory")
        self.add_link("local_error.output_test_directory->output_test_local_directory")

        # pipeline steps
        self.add_pipeline_step("learning", ['label_priors', 'learn_talairach', 'learn_global_registration', 'learn_local_registraton'])
        self.add_pipeline_step("learning2", ['learn_transformation_priors'])
        self.add_pipeline_step("test", ['test_labeling_global', 'test_labeling_local'])
        self.add_pipeline_step("evaluation", ['global_error', 'local_error'])

        # values
        self.nodes['global_error'].process.graph_suffix = '_global'
        self.nodes['local_error'].process.graph_suffix = '_local'

        # nodes positions
        self.node_position = {
            "learn_local_registraton": (177.85131249999995, 768.2862),
            "inputs": (0.0, 486.47990000000004),
            "learn_transformation_priors": (476.6077625, 946.4675),
            "global_error": (757.1153250000001, 287.43499999999995),
            "learn_global_registration": (213.06224999999995, 176.56099999999992),
            "test_labeling_global": (497.959325, 301.3739999999999),
            "test_labeling_local": (729.5450125000001, 590.5486000000001),
            "local_error": (994.8983250000001, 837.4112),
            "outputs": (1205.0127625, 426.81769999999995),
            "label_priors": (236.95287499999995, 502.47990000000004),
            "learn_talairach": (624.8045750000001, -17.0),
        }

        self.do_autoexport_nodes_parameters = False
