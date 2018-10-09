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

        # links
        self.export_parameter("learn_talairach", "graphs")
        self.add_link("graphs->learn_global_registration.graphs")
        self.add_link("graphs->label_priors.graphs")
        self.add_link("graphs->learn_local_registraton.graphs")
        self.export_parameter("test_labeling_global", "data_graph", "loo_graph")
        self.add_link("loo_graph->learn_global_registration.loo_graph")
        self.add_link("loo_graph->learn_local_registraton.loo_graph")
        self.add_link("loo_graph->learn_talairach.loo_graph")
        self.add_link("loo_graph->label_priors.loo_graph")
        self.export_parameter("learn_local_registraton", "loo_subject")
        self.add_link("loo_subject->learn_global_registration.loo_subject")
        self.add_link("loo_subject->test_labeling_local.loo_subject")
        self.add_link("loo_subject->learn_transformation_priors.loo_subject")
        self.add_link("loo_subject->test_labeling_global.loo_subject")
        self.add_link("loo_subject->label_priors.loo_subject")
        self.add_link("loo_subject->learn_talairach.loo_subject")
        self.export_parameter("label_priors", "translation_file")
        self.add_link("translation_file->learn_talairach.translation_file")
        self.add_link("translation_file->learn_local_registraton.translation_file")
        self.add_link("translation_file->learn_global_registration.translation_file")
        self.add_link("translation_file->test_labeling_global.labels_translation_map")
        self.add_link("translation_file->test_labeling_local.labels_translation_map")
        self.export_parameter("learn_global_registration", "threads")
        self.add_link("threads->learn_local_registraton.threads")
        self.export_parameter("learn_local_registraton", "verbose")
        self.add_link("verbose->learn_global_registration.verbose")
        self.export_parameter("label_priors", "output_directory", "output_label_priors_directory")
        self.add_link("label_priors.output_directory->test_labeling_global.labels_priors_directory")
        self.add_link("label_priors.output_directory->test_labeling_local.labels_priors_directory")
        self.export_parameter("learn_talairach", "output_directory", "output_talairach_directory")
        self.export_parameter("learn_global_registration", "output_directory", "output_global_registration_directory")
        self.add_link("learn_global_registration.output_directory->test_labeling_global.global_model_directory")
        self.add_link("learn_local_registraton.output_directory->learn_transformation_priors.spams_directory")
        self.add_link("learn_local_registraton.output_directory->test_labeling_local.local_model_directory")
        self.export_parameter("learn_local_registraton", "output_directory", "output_local_registration_directory")
        self.export_parameter("learn_local_registraton", "output_local_referentials_directory")
        self.add_link("learn_local_registraton.output_local_referentials_directory->test_labeling_local.local_referentials_directory")
        self.add_link("learn_transformation_priors.output_transform_directory->test_labeling_local.transformation_priors_directory")
        self.export_parameter("learn_transformation_priors", "output_transform_directory", "output_transformation_priors_directory")
        self.add_link("test_labeling_global.output_test_directory->test_labeling_local.global_graph_directory")
        self.export_parameter("test_labeling_global", "output_test_directory", "output_test_global_directory")
        self.export_parameter("test_labeling_local", "output_test_directory", "output_test_local_directory")

        # nodes positions
        self.node_position = {
            "learn_local_registraton": (154.36989, 574.2040999999999),
            "inputs": (0.0, 347.0982),
            "learn_transformation_priors": (419.57003999999995, 713.6478),
            "learn_global_registration": (184.46364, 94.0104),
            "test_labeling_global": (439.85128999999995, 227.0543),
            "test_labeling_local": (645.67049, 447.623),
            "outputs": (880.48149, 301.5542999999999),
            "label_priors": (204.18239, 360.0982),
            "learn_talairach": (680.17049, 0.0),
        }

        self.do_autoexport_nodes_parameters = False

        # pipeline steps
        self.add_pipeline_step('learning',
                               ['label_priors', 'learn_talairach',
                                'learn_global_registration',
                                'learn_local_registraton',
                                ])
        self.add_pipeline_step('learning2',
                               ['learn_transformation_priors'])
        self.add_pipeline_step('test', ['test_labeling_global',
                                        'test_labeling_local'])
