from capsul.api import Pipeline
import traits.api as traits


class Spam_test_pipeline(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("test_global", "sulci.capsul.spam_test_labeling_global.SpamTestLabelingGlobal")
        self.nodes["test_global"].process.data_graph = '/neurospin/lnao/PClean/database_learnclean/all/zeus/t1mri/t1/default_analysis/folds/3.3/base2008_manual/Lzeus_base2008_manual.arg'
        self.nodes["test_global"].process.label_priors = u'cv_9/labels_priors/frequency_segments_priors.dat'
        self.nodes["test_global"].process.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.nodes["test_global"].process.model = u'cv_9/segments/global_registered_spam/spam_distribs.dat'
        self.nodes["test_global"].process.output_posterior_probabilities = u'cv_9/test/zeus/zeus_global_proba.csv'
        self.nodes["test_global"].process.output_t1_to_global_transformation = u'cv_9/test/zeus/zeus_T1_TO_SPAM.trm'
        self.add_process("global_error", "sulci.capsul.sulci_labeling_error.SulciLabelingError")
        self.nodes["global_error"].process.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.nodes["global_error"].process.loo_subject = u'zeus'
        self.nodes["global_error"].process.output_csv = u'cv_9/test/zeus/zeus_global.csv'
        self.nodes["global_error"].process.reference_graph = '/neurospin/lnao/PClean/database_learnclean/all/zeus/t1mri/t1/default_analysis/folds/3.3/base2008_manual/Lzeus_base2008_manual.arg'
        self.add_process("test_local", "sulci.capsul.spam_test_labeling_local.SpamTestLabelingLocal")
        self.nodes["test_local"].process.angle_priors = u'cv_9/vonmises_angle_trm_priors.dat'
        self.nodes["test_local"].process.direction_priors = u'cv_9/bingham_direction_trm_priors.dat'
        self.nodes["test_local"].process.label_priors = u'cv_9/labels_priors/frequency_segments_priors.dat'
        self.nodes["test_local"].process.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.nodes["test_local"].process.local_referentials = u'cv_9/segments/locally_from_global_registered_spam/local_referentials.dat'
        self.nodes["test_local"].process.model = u'cv_9/segments/locally_from_global_registered_spam/spam_distribs.dat'
        self.nodes["test_local"].process.output_local_transformations = u'cv_9/test/zeus/zeus_global_TO_local'
        self.nodes["test_local"].process.output_posterior_probabilities = u'cv_9/test/zeus/zeuslocal_proba.csv'
        self.nodes["test_local"].process.translation_priors = u'cv_9/gaussian_translation_trm_priors.dat'
        self.add_process("local_error", "sulci.capsul.sulci_labeling_error.SulciLabelingError")
        self.nodes["local_error"].process.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.nodes["local_error"].process.loo_subject = u'zeus'
        self.nodes["local_error"].process.output_csv = u'cv_9/test/zeus/zeus_local.csv'
        self.nodes["local_error"].process.reference_graph = '/neurospin/lnao/PClean/database_learnclean/all/zeus/t1mri/t1/default_analysis/folds/3.3/base2008_manual/Lzeus_base2008_manual.arg'

        # links
        self.export_parameter("global_error", "reference_graph", "data_graph")
        self.add_link("data_graph->local_error.reference_graph")
        self.add_link("data_graph->test_global.data_graph")
        self.export_parameter("local_error", "labels_translation_map")
        self.add_link("labels_translation_map->test_local.labels_translation_map")
        self.add_link("labels_translation_map->test_global.labels_translation_map")
        self.add_link("labels_translation_map->global_error.labels_translation_map")
        self.export_parameter("test_global", "label_priors")
        self.add_link("label_priors->test_local.label_priors")
        self.export_parameter("test_global", "model", "global_model")
        self.export_parameter("test_local", "model", "local_model")
        self.export_parameter("local_error", "loo_subject", "subject")
        self.add_link("subject->global_error.loo_subject")
        self.export_parameter("test_local", "local_referentials")
        self.export_parameter("test_local", "translation_priors")
        self.export_parameter("test_local", "angle_priors")
        self.export_parameter("test_local", "direction_priors")
        self.export_parameter("test_global", "output_graph", "output_global_graph")
        self.add_link("test_global.output_graph->global_error.test_graph")
        self.add_link("test_global.output_graph->test_local.global_graph")
        self.export_parameter("test_global", "output_posterior_probabilities", "output_global_posterior_probabilities")
        self.export_parameter("test_global", "output_t1_to_global_transformation")
        self.export_parameter("test_global", "output_transformation", "output_global_transformation")
        self.add_link("test_global.output_transformation->test_local.global_transformation")
        self.export_parameter("global_error", "output_csv", "output_global_csv")
        self.export_parameter("test_local", "output_graph", "output_local_graph")
        self.add_link("test_local.output_graph->local_error.test_graph")
        self.export_parameter("test_local", "output_local_transformations")
        self.export_parameter("test_local", "output_posterior_probabilities", "output_local_posterior_probabilities")
        self.export_parameter("local_error", "output_csv", "output_local_csv")

        # default and initial values
        self.data_graph = '/neurospin/lnao/PClean/database_learnclean/all/zeus/t1mri/t1/default_analysis/folds/3.3/base2008_manual/Lzeus_base2008_manual.arg'
        self.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.label_priors = u'cv_9/labels_priors/frequency_segments_priors.dat'
        self.global_model = u'cv_9/segments/global_registered_spam/spam_distribs.dat'
        self.local_model = u'cv_9/segments/locally_from_global_registered_spam/spam_distribs.dat'
        self.subject = u'zeus'
        self.local_referentials = u'cv_9/segments/locally_from_global_registered_spam/local_referentials.dat'
        self.translation_priors = u'cv_9/gaussian_translation_trm_priors.dat'
        self.angle_priors = u'cv_9/vonmises_angle_trm_priors.dat'
        self.direction_priors = u'cv_9/bingham_direction_trm_priors.dat'
        self.output_global_posterior_probabilities = u'cv_9/test/zeus/zeus_global_proba.csv'
        self.output_t1_to_global_transformation = u'cv_9/test/zeus/zeus_T1_TO_SPAM.trm'
        self.output_global_csv = u'cv_9/test/zeus/zeus_global.csv'
        self.output_local_transformations = u'cv_9/test/zeus/zeus_global_TO_local'
        self.output_local_posterior_probabilities = u'cv_9/test/zeus/zeuslocal_proba.csv'
        self.output_local_csv = u'cv_9/test/zeus/zeus_local.csv'

        # nodes positions
        self.node_position = {
            "inputs": (-59.04, 132.80150000000003),
            "test_global": (247.24952967435598, 20.00390000000003),
            "global_error": (813.811929674356, -112.31999999999996),
            "outputs": (1414.847555826323, 63.981499999999926),
            "local_error": (1192.5189296743558, 436.20130000000006),
            "test_local": (732.2744296743562, 260.8612999999999),
        }

        # nodes dimensions
        self.node_dimension = {
            "inputs": (194.23904525915623, 390.0),
            "test_global": (416.75, 180.0),
            "global_error": (252.875, 180.0),
            "outputs": (268.909792955222, 355.0),
            "local_error": (252.875, 180.0),
            "test_local": (379.75, 355.0),
        }

        self.do_autoexport_nodes_parameters = False
