from capsul.api import Pipeline
import traits.api as traits


class Spam_learn_pipeline_loo_step(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("global", "sulci.capsul.spam_learn_global_registration.SpamLearnGlobalRegistration")
        self.add_custom_node("loo_exclude", "capsul.pipeline.custom_nodes.exclude_node.ExcludeNode", {'param_type': u'File', 'is_output': True})
        self.add_custom_node("global_output_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_output_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["global_output_dir"].plugs["separator"].optional = True
        self.add_process("local", "sulci.capsul.spam_learn_local_registration.SpamLearnLocalRegistration")
        self.add_custom_node("local_output_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_output_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["local_output_dir"].plugs["separator"].optional = True
        self.add_custom_node("output_local_referentials_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_local_referentials_directory', 'outputs': [u'local_referentials_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'local_referentials_directory', u'loo_subject']})
        self.nodes["output_local_referentials_dir"].plugs["separator"].optional = True
        self.add_process("label_priors", "sulci.capsul.spam_learn_label_priors.SpamLearnLabelPriors")
        self.add_custom_node("label_priors_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_label_priors_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["label_priors_directory"].plugs["separator"].optional = True
        self.add_process("talairach", "sulci.capsul.spam_learn_talairach.SpamLearnTalairach")
        self.add_custom_node("talairach_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_talairach_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["talairach_dir"].plugs["separator"].optional = True
        self.add_process("transformation_priors", "sulci.capsul.spam_learn_transformation_priors.SpamLearnTransformationPrior")
        self.add_custom_node("angle_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_angle_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject', u'angle_dir']})
        self.nodes["angle_dir"].plugs["angle_dir"].optional = True
        self.nodes["angle_dir"].plugs["separator"].optional = True
        self.nodes["angle_dir"].angle_dir = u'vonmises_angle_trm_priors'
        self.add_custom_node("direction_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_direction_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject', u'direction_dir']})
        self.nodes["direction_dir"].plugs["direction_dir"].optional = True
        self.nodes["direction_dir"].plugs["separator"].optional = True
        self.nodes["direction_dir"].direction_dir = u'bingham_direction_trm_priors'
        self.add_custom_node("translation_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_translation_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject', u'translation_dir']})
        self.nodes["translation_directory"].plugs["translation_dir"].optional = True
        self.nodes["translation_directory"].plugs["separator"].optional = True
        self.nodes["translation_directory"].translation_dir = u'gaussian_translation_trm_priors'
        self.add_process("test_global", "sulci.capsul.spam_test_labeling_global.SpamTestLabelingGlobal")
        self.add_custom_node("label_priors_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'label_priors', 'outputs': [u'label_priors'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'', 'parameters': [u'labels_input_directory', u'ext']})
        self.nodes["label_priors_filename"].plugs["separator"].optional = True
        self.nodes["label_priors_filename"].plugs["ext"].optional = True
        self.nodes["label_priors_filename"].ext = u'.dat'
        self.add_custom_node("global_model_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_model', 'outputs': [u'global_model'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'input_directory', u'ext']})
        self.nodes["global_model_filename"].plugs["ext"].optional = True
        self.nodes["global_model_filename"].ext = u'.dat'
        self.add_custom_node("loo_global_test_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_loo_test_dir', 'outputs': [u'global_test_directory', u'global_loo_test_dir'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'global_test_directory', u'loo_subject']})
        self.nodes["loo_global_test_dir"].plugs["separator"].optional = True
        self.add_custom_node("global_test_graph_name", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_graph', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'global_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["global_test_graph_name"].plugs["global_test_dir"].optional = True
        self.nodes["global_test_graph_name"].plugs["sep"].optional = True
        self.nodes["global_test_graph_name"].plugs["suffix"].optional = True
        self.nodes["global_test_graph_name"].sep = u'/'
        self.nodes["global_test_graph_name"].suffix = u'_global.arg'
        self.add_custom_node("global_test_proba_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_global_posterior_probabilities', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'global_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["global_test_proba_filename"].plugs["sep"].optional = True
        self.nodes["global_test_proba_filename"].plugs["suffix"].optional = True
        self.nodes["global_test_proba_filename"].sep = u'/'
        self.nodes["global_test_proba_filename"].suffix = u'_global_proba.csv'
        self.add_custom_node("global_test_t1_transform_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_t1_to_global_transformation', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'global_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["global_test_t1_transform_filename"].plugs["sep"].optional = True
        self.nodes["global_test_t1_transform_filename"].plugs["suffix"].optional = True
        self.nodes["global_test_t1_transform_filename"].sep = u'/'
        self.nodes["global_test_t1_transform_filename"].suffix = u'_T1_TO_SPAM.trm'
        self.add_custom_node("global_test_transform_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_transformation', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'global_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["global_test_transform_filename"].plugs["sep"].optional = True
        self.nodes["global_test_transform_filename"].plugs["suffix"].optional = True
        self.nodes["global_test_transform_filename"].sep = u'/'
        self.nodes["global_test_transform_filename"].suffix = u'_Tal_TO_SPAM.trm'
        self.add_process("test_local", "sulci.capsul.spam_test_labeling_local.SpamTestLabelingLocal")
        self.nodes["test_local"].process.angle_priors = u'/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left//vonmises_angle_trm_priors.dat'
        self.add_custom_node("loo_local_test_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_loo_test_dir', 'outputs': [u'local_test_directory', u'local_loo_test_dir'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'local_test_directory', u'loo_subject']})
        self.nodes["loo_local_test_dir"].plugs["separator"].optional = True
        self.add_custom_node("angle_priors_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'angle_priors', 'outputs': [u'angle_priors'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'angle_priors_dir', u'ext']})
        self.nodes["angle_priors_filename"].plugs["ext"].optional = True
        self.nodes["angle_priors_filename"].ext = u'.dat'
        self.add_custom_node("direction_priors_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'direction_priors', 'outputs': [u'direction_priors'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'direction_priors_dir', u'ext']})
        self.nodes["direction_priors_filename"].plugs["ext"].optional = True
        self.nodes["direction_priors_filename"].ext = u'.dat'
        self.add_custom_node("translation_priors_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'translation_priors', 'outputs': [u'translation_priors'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'translation_priors_dir', u'ext']})
        self.nodes["translation_priors_filename"].plugs["ext"].optional = True
        self.nodes["translation_priors_filename"].ext = u'.dat'
        self.add_custom_node("local_model_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_model', 'outputs': [u'local_model'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'local_model_dir', u'ext']})
        self.nodes["local_model_filename"].plugs["ext"].optional = True
        self.nodes["local_model_filename"].ext = u'.dat'
        self.add_custom_node("local_test_graph_name", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_graph', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'local_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["local_test_graph_name"].plugs["sep"].optional = True
        self.nodes["local_test_graph_name"].plugs["suffix"].optional = True
        self.nodes["local_test_graph_name"].sep = u'/'
        self.nodes["local_test_graph_name"].suffix = u'_local.arg'
        self.add_custom_node("local_test_transform_dirname", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_local_transformations', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'local_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["local_test_transform_dirname"].plugs["sep"].optional = True
        self.nodes["local_test_transform_dirname"].plugs["suffix"].optional = True
        self.nodes["local_test_transform_dirname"].sep = u'/'
        self.nodes["local_test_transform_dirname"].suffix = u'_global_TO_local'
        self.add_custom_node("local_test_proba_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_posterior_probabilities', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'local_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["local_test_proba_filename"].plugs["sep"].optional = True
        self.nodes["local_test_proba_filename"].plugs["suffix"].optional = True
        self.nodes["local_test_proba_filename"].sep = u'/'
        self.nodes["local_test_proba_filename"].suffix = u'_local_proba.csv'
        self.add_custom_node("local_referentials_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_referentials', 'outputs': [u'local_referentials'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'local_referentials_dir', u'ext']})
        self.nodes["local_referentials_filename"].plugs["ext"].optional = True
        self.nodes["local_referentials_filename"].ext = u'.dat'
        self.add_process("global_error", "sulci.capsul.sulci_labeling_error.SulciLabelingError")
        self.add_process("local_error", "sulci.capsul.sulci_labeling_error.SulciLabelingError")
        self.add_custom_node("global_error_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_error_csv', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'global_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["global_error_filename"].plugs["sep"].optional = True
        self.nodes["global_error_filename"].plugs["suffix"].optional = True
        self.nodes["global_error_filename"].sep = u'/'
        self.nodes["global_error_filename"].suffix = u'_global_err.csv'
        self.add_custom_node("local_error_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_error_csv', 'outputs': [], 'param_types': [u'Any', u'Any', u'Any', u'Any', u'Any'], 'parameters': [u'local_test_dir', u'sep', u'loo_subject', u'suffix']})
        self.nodes["local_error_filename"].plugs["sep"].optional = True
        self.nodes["local_error_filename"].plugs["suffix"].optional = True
        self.nodes["local_error_filename"].sep = u'/'
        self.nodes["local_error_filename"].suffix = u'_local_err.csv'

        # links
        self.export_parameter("loo_exclude", "inputs", "graphs")
        self.export_parameter("loo_exclude", "exclude", "loo_graph")
        self.export_parameter("global_test_graph_name", "loo_subject")
        self.add_link("loo_subject->angle_dir.loo_subject")
        self.add_link("loo_subject->local_error.loo_subject")
        self.add_link("loo_subject->local_test_proba_filename.loo_subject")
        self.add_link("loo_subject->global_test_t1_transform_filename.loo_subject")
        self.add_link("loo_subject->global_test_proba_filename.loo_subject")
        self.add_link("loo_subject->local_error_filename.loo_subject")
        self.add_link("loo_subject->local_output_dir.loo_subject")
        self.add_link("loo_subject->local_test_graph_name.loo_subject")
        self.add_link("loo_subject->local_test_transform_dirname.loo_subject")
        self.add_link("loo_subject->global_error.loo_subject")
        self.add_link("loo_subject->loo_global_test_dir.loo_subject")
        self.add_link("loo_subject->global_test_transform_filename.loo_subject")
        self.add_link("loo_subject->output_local_referentials_dir.loo_subject")
        self.add_link("loo_subject->direction_dir.loo_subject")
        self.add_link("loo_subject->global_error_filename.loo_subject")
        self.add_link("loo_subject->talairach_dir.loo_subject")
        self.add_link("loo_subject->loo_local_test_dir.loo_subject")
        self.add_link("loo_subject->label_priors_directory.loo_subject")
        self.add_link("loo_subject->translation_directory.loo_subject")
        self.add_link("loo_subject->global_output_dir.loo_subject")
        self.export_parameter("transformation_priors", "threads")
        self.add_link("threads->global.threads")
        self.add_link("threads->local.threads")
        self.export_parameter("local_error", "labels_translation_map", "translation_file")
        self.add_link("translation_file->local.translation_file")
        self.add_link("translation_file->global.translation_file")
        self.add_link("translation_file->global_error.labels_translation_map")
        self.add_link("translation_file->test_global.labels_translation_map")
        self.add_link("translation_file->label_priors.translation_file")
        self.add_link("translation_file->test_local.labels_translation_map")
        self.add_link("translation_file->talairach.translation_file")
        self.export_parameter("local", "verbose")
        self.add_link("verbose->global.verbose")
        self.export_parameter("local_error", "reference_graph")
        self.add_link("reference_graph->global_error.reference_graph")
        self.add_link("reference_graph->test_global.data_graph")
        self.add_link("global.output_directory->global_model_filename.input_directory")
        self.add_link("global.output_directory->global_output_dir.global_output_directory")
        self.add_link("loo_exclude.filtered->label_priors.graphs")
        self.add_link("loo_exclude.filtered->local.graphs")
        self.add_link("loo_exclude.filtered->talairach.graphs")
        self.add_link("loo_exclude.filtered->global.graphs")
        self.export_parameter("global_output_dir", "output_directory", "output_global_directory")
        self.add_link("local.output_directory->local_output_dir.local_output_directory")
        self.add_link("local.output_directory->transformation_priors.spams_directory")
        self.add_link("local.output_directory->local_model_filename.local_model_dir")
        self.add_link("local.output_local_referentials_directory->output_local_referentials_dir.output_local_referentials_directory")
        self.add_link("local.output_local_referentials_directory->local_referentials_filename.local_referentials_dir")
        self.export_parameter("local_output_dir", "output_directory", "output_local_directory")
        self.export_parameter("output_local_referentials_dir", "local_referentials_directory", "output_local_referentials_directory")
        self.add_link("label_priors.output_directory->label_priors_filename.labels_input_directory")
        self.add_link("label_priors.output_directory->label_priors_directory.output_label_priors_directory")
        self.export_parameter("label_priors_directory", "output_directory", "output_label_priors_directory")
        self.add_link("talairach.output_directory->talairach_dir.output_talairach_directory")
        self.export_parameter("talairach_dir", "output_directory", "output_talairach_directory")
        self.add_link("transformation_priors.output_angle_directory->angle_priors_filename.angle_priors_dir")
        self.add_link("transformation_priors.output_angle_directory->angle_dir.output_angle_directory")
        self.add_link("transformation_priors.output_direction_directory->direction_priors_filename.direction_priors_dir")
        self.add_link("transformation_priors.output_direction_directory->direction_dir.output_direction_directory")
        self.add_link("transformation_priors.output_translation_directory->translation_directory.output_translation_directory")
        self.add_link("transformation_priors.output_translation_directory->translation_priors_filename.translation_priors_dir")
        self.export_parameter("angle_dir", "output_directory", "output_local_angle_directory")
        self.export_parameter("direction_dir", "output_directory", "output_local_direction_directory")
        self.export_parameter("translation_directory", "output_directory", "output_local_translation_directory")
        self.add_link("test_global.output_graph->global_error.test_graph")
        self.add_link("test_global.output_graph->global_test_graph_name.output_graph")
        self.add_link("test_global.output_graph->test_local.global_graph")
        self.add_link("test_global.output_posterior_probabilities->global_test_proba_filename.output_global_posterior_probabilities")
        self.add_link("test_global.output_t1_to_global_transformation->global_test_t1_transform_filename.output_t1_to_global_transformation")
        self.add_link("test_global.output_transformation->global_test_transform_filename.output_transformation")
        self.add_link("test_global.output_transformation->test_local.global_transformation")
        self.add_link("label_priors_filename.label_priors->test_local.label_priors")
        self.add_link("label_priors_filename.label_priors->test_global.label_priors")
        self.add_link("global_model_filename.global_model->test_global.model")
        self.export_parameter("loo_global_test_dir", "global_test_directory")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_t1_transform_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_transform_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_error_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_proba_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_graph_name.global_test_dir")
        self.add_link("test_local.output_graph->local_test_graph_name.output_graph")
        self.add_link("test_local.output_graph->local_error.test_graph")
        self.add_link("test_local.output_local_transformations->local_test_transform_dirname.output_local_transformations")
        self.add_link("test_local.output_posterior_probabilities->local_test_proba_filename.output_posterior_probabilities")
        self.export_parameter("loo_local_test_dir", "local_test_directory")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_error_filename.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_transform_dirname.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_graph_name.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_proba_filename.local_test_dir")
        self.add_link("angle_priors_filename.angle_priors->test_local.angle_priors")
        self.add_link("direction_priors_filename.direction_priors->test_local.direction_priors")
        self.add_link("translation_priors_filename.translation_priors->test_local.translation_priors")
        self.add_link("local_model_filename.local_model->test_local.model")
        self.add_link("local_referentials_filename.local_referentials->test_local.local_referentials")
        self.add_link("global_error.output_csv->global_error_filename.global_error_csv")
        self.add_link("local_error.output_csv->local_error_filename.local_error_csv")

        # pipeline steps
        self.add_pipeline_step("learning", [u'global', u'local', u'label_priors', u'talairach', u'transformation_priors'])
        self.add_pipeline_step("testing", [u'test_global', u'test_local'])
        self.add_pipeline_step("evaluation", [u'global_error', u'local_error'])

        # default and initial values
        self.graphs = ['']
        self.threads = 36
        self.translation_file = '/neurospin/lnao/Panabase/model_spam_archi_2018/sulci_model_2018.trl'
        self.output_global_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/global_registered_spam_left/spam_distribs'
        self.output_local_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left/spam_distribs'
        self.output_local_referentials_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left/local_referentials'
        self.output_label_priors_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/labels_priors/frequency_segments_priors_left'
        self.output_talairach_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/talairach_spam_left/spam_distribs'
        self.output_local_angle_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left'
        self.output_local_direction_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left'
        self.output_local_translation_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/segments/locally_from_global_registred_spam_left'
        self.global_test_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/loo_tests_results/global_left'
        self.local_test_directory = '/neurospin/lnao/Panabase/model_spam_archi_2018/loo_tests_results/local_left'

        # nodes positions
        self.node_position = {
            "label_priors_filename": (635.6999000000001, 902.6240000000003),
            "label_priors_directory": (613.1999000000001, 188.0020000000004),
            "loo_exclude": (178.86260000000001, 966.8120000000001),
            "global": (356.67510000000004, 931.3120000000001),
            "global_error_filename": (1453.6669000000002, 1836.0258000000003),
            "transformation_priors": (606.7624000000001, 1216.0620000000004),
            "global_test_graph_name": (1188.4179, 1071.1270000000002),
            "direction_dir": (896.6994, 281.9360000000004),
            "local_test_proba_filename": (1427.6669000000002, 2013.7567000000004),
            "local_error_filename": (1663.8568999999998, 2120.7758000000003),
            "loo_local_test_dir": (910.6369, 2038.8569000000002),
            "global_model_filename": (632.6374000000001, 1412.563),
            "angle_dir": (906.6369, 58.625),
            "local_model_filename": (912.5744, 728.6270000000002),
            "local_test_transform_dirname": (1427.6669000000002, 1461.938),
            "talairach": (356.67510000000004, 151.43600000000015),
            "loo_global_test_dir": (909.91815, 1650.813),
            "global_test_transform_filename": (1163.9179, 1604.313),
            "local_output_dir": (634.1999000000001, 563.0010000000004),
            "local": (297.30010000000004, 1127.6870000000001),
            "output_local_referentials_dir": (597.8249000000001, 750.9360000000004),
            "translation_directory": (890.6994, 505.3140000000003),
            "inputs": (0.0, 1124.1870000000001),
            "angle_priors_filename": (911.0744, 1178.0630000000003),
            "global_output_dir": (629.1999000000001, 411.31300000000033),
            "global_test_proba_filename": (1148.4179, 1248.8110000000001),
            "outputs": (1157.5429, 315.6250000000002),
            "global_error": (1185.9179, 1781.8569000000002),
            "local_test_graph_name": (1450.6669000000002, 792.8760000000004),
            "test_global": (859.1369, 1339.9390000000003),
            "global_test_t1_transform_filename": (1151.9179, 1426.5620000000004),
            "direction_priors_filename": (899.6369, 890.4990000000003),
            "talairach_dir": (622.2624000000001, 0.0),
            "local_error": (1443.6669000000002, 1639.4819000000002),
            "test_local": (1153.9179, 692.3760000000004),
            "label_priors": (356.67510000000004, 786.1270000000002),
            "local_referentials_filename": (618.7624000000001, 1090.5610000000004),
            "translation_priors_filename": (892.6369, 1016.1240000000003),
        }

        self.do_autoexport_nodes_parameters = False
