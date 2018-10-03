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

        # links
        self.export_parameter("label_priors", "graphs")
        self.add_link("graphs->learn_talairach.graphs")
        self.add_link("graphs->learn_local_registraton.graphs")
        self.add_link("graphs->learn_global_registration.graphs")
        self.export_parameter("learn_global_registration", "loo_graph")
        self.add_link("loo_graph->learn_local_registraton.loo_graph")
        self.add_link("loo_graph->learn_talairach.loo_graph")
        self.add_link("loo_graph->label_priors.loo_graph")
        self.export_parameter("learn_global_registration", "loo_subject")
        self.add_link("loo_subject->label_priors.loo_subject")
        self.add_link("loo_subject->learn_transformation_priors.loo_subject")
        self.add_link("loo_subject->learn_talairach.loo_subject")
        self.add_link("loo_subject->learn_local_registraton.loo_subject")
        self.export_parameter("learn_talairach", "translation_file")
        self.add_link("translation_file->label_priors.translation_file")
        self.add_link("translation_file->learn_global_registration.translation_file")
        self.add_link("translation_file->learn_local_registraton.translation_file")
        self.export_parameter("learn_local_registraton", "threads")
        self.add_link("threads->learn_global_registration.threads")
        self.export_parameter("learn_local_registraton", "verbose")
        self.add_link("verbose->learn_global_registration.verbose")
        self.export_parameter("label_priors", "output_directory", "output_label_priors_directory")
        self.export_parameter("learn_talairach", "output_directory", "output_talairach_directory")
        self.export_parameter("learn_global_registration", "output_directory", "output_global_registration_directory")
        self.add_link("learn_local_registraton.output_directory->learn_transformation_priors.spams_directory")
        self.export_parameter("learn_local_registraton", "output_directory", "output_local_registration_directory")
        self.export_parameter("learn_local_registraton", "output_local_referentials_directory")
        self.export_parameter("learn_transformation_priors", "output_directory", "output_transformation_priors_directory")

        # nodes positions
        self.node_position = {
            "learn_local_registraton": (154.36989, 456.0563),
            "inputs": (0.0, 202.88740000000007),
            "learn_transformation_priors": (419.57003999999995, 391.7061),
            "learn_global_registration": (184.46364, 189.88740000000007),
            "outputs": (645.98149, 271.75),
            "label_priors": (449.85128999999995, 0.0),
            "learn_talairach": (449.85128999999995, 177.73759999999993),
        }

        self.do_autoexport_nodes_parameters = False
