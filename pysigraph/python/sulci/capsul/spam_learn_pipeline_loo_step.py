from capsul.api import Pipeline
import traits.api as traits


class Spam_learn_pipeline_loo_step(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("global", "sulci.capsul.spam_learn_global_registration.SpamLearnGlobalRegistration")
        self.nodes["global"].process.graphs = traits.Undefined
        self.nodes["global"].process.threads = 36
        self.add_custom_node("loo_exclude", "capsul.pipeline.custom_nodes.exclude_node.ExcludeNode", {'param_type': u'File', 'is_output': True})
        self.nodes["loo_exclude"].plugs["inputs"].optional = True
        self.nodes["loo_exclude"].plugs["exclude"].optional = True
        self.nodes["loo_exclude"].plugs["filtered"].optional = True
        self.add_custom_node("global_output_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_output_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["global_output_dir"].plugs["separator"].optional = True
        self.add_process("local", "sulci.capsul.spam_learn_local_registration.SpamLearnLocalRegistration")
        self.nodes["local"].process.graphs = traits.Undefined
        self.nodes["local"].process.threads = 36
        self.add_custom_node("local_output_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_output_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["local_output_dir"].plugs["separator"].optional = True
        self.add_custom_node("output_local_referentials_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_local_referentials_directory', 'outputs': [u'local_referentials_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'local_referentials_directory', u'loo_subject']})
        self.nodes["output_local_referentials_dir"].plugs["separator"].optional = True
        self.add_process("label_priors", "sulci.capsul.spam_learn_label_priors.SpamLearnLabelPriors")
        self.nodes["label_priors"].process.graphs = traits.Undefined
        self.add_custom_node("label_priors_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_label_priors_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["label_priors_directory"].plugs["separator"].optional = True
        self.add_process("talairach", "sulci.capsul.spam_learn_talairach.SpamLearnTalairach")
        self.nodes["talairach"].process.graphs = traits.Undefined
        self.add_custom_node("talairach_dir", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_talairach_directory', 'outputs': [u'output_directory'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'output_directory', u'loo_subject']})
        self.nodes["talairach_dir"].plugs["separator"].optional = True
        self.add_process("transformation_priors", "sulci.capsul.spam_learn_transformation_priors.SpamLearnTransformationPrior")
        self.nodes["transformation_priors"].process.threads = 36
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
        self.nodes["test_global"].process.label_priors = u'.dat'
        self.nodes["test_global"].process.model = u'.dat'
        self.nodes["test_global"].process.output_graph = u'None/<undefined>/<undefined>_global.arg'
        self.nodes["test_global"].process.output_posterior_probabilities = u'None/<undefined>/<undefined>_global_proba.csv'
        self.nodes["test_global"].process.output_t1_to_global_transformation = u'None/<undefined>/<undefined>_T1_TO_SPAM.trm'
        self.nodes["test_global"].process.output_transformation = u'None/<undefined>/<undefined>_Tal_TO_SPAM.trm'
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
        self.nodes["test_local"].process.angle_priors = u'.dat'
        self.nodes["test_local"].process.direction_priors = u'.dat'
        self.nodes["test_local"].process.global_graph = u'None/<undefined>/<undefined>_global.arg'
        self.nodes["test_local"].process.global_transformation = u'None/<undefined>/<undefined>_Tal_TO_SPAM.trm'
        self.nodes["test_local"].process.label_priors = u'.dat'
        self.nodes["test_local"].process.local_referentials = u'.dat'
        self.nodes["test_local"].process.model = u'.dat'
        self.nodes["test_local"].process.output_graph = u'None/<undefined>/<undefined>_local.arg'
        self.nodes["test_local"].process.output_local_transformations = u'None/<undefined>/<undefined>_global_TO_local'
        self.nodes["test_local"].process.output_posterior_probabilities = u'None/<undefined>/<undefined>_local_proba.csv'
        self.nodes["test_local"].process.translation_priors = u'.dat'
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
        self.nodes["global_error"].process.test_graph = u'None/<undefined>/<undefined>_global.arg'
        self.add_process("local_error", "sulci.capsul.sulci_labeling_error.SulciLabelingError")
        self.nodes["local_error"].process.test_graph = u'None/<undefined>/<undefined>_local.arg'
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
        self.add_link("loo_subject->loo_local_test_dir.loo_subject")
        self.add_link("loo_subject->local_error.loo_subject")
        self.add_link("loo_subject->direction_dir.loo_subject")
        self.add_link("loo_subject->global_test_transform_filename.loo_subject")
        self.add_link("loo_subject->angle_dir.loo_subject")
        self.add_link("loo_subject->local_error_filename.loo_subject")
        self.add_link("loo_subject->global_error.loo_subject")
        self.add_link("loo_subject->local_output_dir.loo_subject")
        self.add_link("loo_subject->local_test_graph_name.loo_subject")
        self.add_link("loo_subject->global_test_t1_transform_filename.loo_subject")
        self.add_link("loo_subject->local_test_proba_filename.loo_subject")
        self.add_link("loo_subject->output_local_referentials_dir.loo_subject")
        self.add_link("loo_subject->global_error_filename.loo_subject")
        self.add_link("loo_subject->talairach_dir.loo_subject")
        self.add_link("loo_subject->loo_global_test_dir.loo_subject")
        self.add_link("loo_subject->global_output_dir.loo_subject")
        self.add_link("loo_subject->translation_directory.loo_subject")
        self.add_link("loo_subject->label_priors_directory.loo_subject")
        self.add_link("loo_subject->global_test_proba_filename.loo_subject")
        self.add_link("loo_subject->local_test_transform_dirname.loo_subject")
        self.export_parameter("global", "threads")
        self.add_link("threads->transformation_priors.threads")
        self.add_link("threads->local.threads")
        self.export_parameter("local_error", "labels_translation_map", "translation_file")
        self.add_link("translation_file->global_error.labels_translation_map")
        self.add_link("translation_file->talairach.translation_file")
        self.add_link("translation_file->label_priors.translation_file")
        self.add_link("translation_file->local.translation_file")
        self.add_link("translation_file->test_global.labels_translation_map")
        self.add_link("translation_file->global.translation_file")
        self.add_link("translation_file->test_local.labels_translation_map")
        self.export_parameter("global", "verbose")
        self.add_link("verbose->local.verbose")
        self.export_parameter("local_error", "reference_graph")
        self.add_link("reference_graph->global_error.reference_graph")
        self.add_link("reference_graph->test_global.data_graph")
        self.add_link("global.output_directory->local.global_spam_distribs")
        self.add_link("global.output_directory->global_model_filename.input_directory")
        self.add_link("global.output_directory->global_output_dir.global_output_directory")
        self.add_link("loo_exclude.filtered->label_priors.graphs")
        self.add_link("loo_exclude.filtered->talairach.graphs")
        self.add_link("loo_exclude.filtered->global.graphs")
        self.add_link("loo_exclude.filtered->local.graphs")
        self.export_parameter("global_output_dir", "output_directory", "output_global_registration_directory")
        self.add_link("local.output_directory->transformation_priors.spams_directory")
        self.add_link("local.output_directory->local_output_dir.local_output_directory")
        self.add_link("local.output_directory->local_model_filename.local_model_dir")
        self.add_link("local.output_local_referentials_directory->local_referentials_filename.local_referentials_dir")
        self.add_link("local.output_local_referentials_directory->output_local_referentials_dir.output_local_referentials_directory")
        self.export_parameter("local_output_dir", "output_directory", "output_local_registration_directory")
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
        self.add_link("test_global.output_graph->test_local.global_graph")
        self.add_link("test_global.output_graph->global_error.test_graph")
        self.add_link("test_global.output_graph->global_test_graph_name.output_graph")
        self.add_link("test_global.output_posterior_probabilities->global_test_proba_filename.output_global_posterior_probabilities")
        self.add_link("test_global.output_t1_to_global_transformation->global_test_t1_transform_filename.output_t1_to_global_transformation")
        self.add_link("test_global.output_transformation->global_test_transform_filename.output_transformation")
        self.add_link("test_global.output_transformation->test_local.global_transformation")
        self.add_link("label_priors_filename.label_priors->test_local.label_priors")
        self.add_link("label_priors_filename.label_priors->test_global.label_priors")
        self.add_link("global_model_filename.global_model->test_global.model")
        self.export_parameter("loo_global_test_dir", "global_test_directory", "output_test_global_directory")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_t1_transform_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_transform_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_error_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_proba_filename.global_test_dir")
        self.add_link("loo_global_test_dir.global_loo_test_dir->global_test_graph_name.global_test_dir")
        self.add_link("test_local.output_graph->local_test_graph_name.output_graph")
        self.add_link("test_local.output_graph->local_error.test_graph")
        self.add_link("test_local.output_local_transformations->local_test_transform_dirname.output_local_transformations")
        self.add_link("test_local.output_posterior_probabilities->local_test_proba_filename.output_posterior_probabilities")
        self.export_parameter("loo_local_test_dir", "local_test_directory", "output_test_local_directory")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_error_filename.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_proba_filename.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_graph_name.local_test_dir")
        self.add_link("loo_local_test_dir.local_loo_test_dir->local_test_transform_dirname.local_test_dir")
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

        # nodes positions
        self.node_position = {
            "label_priors_filename": (490.1236600000001, 1218.062),
            "direction_priors_filename": (1037.80026, 1254.378),
            "label_priors_directory": (470.4674100000001, 372.1869999999999),
            "loo_exclude": (163.91755999999998, 1067.3139999999999),
            "global": (282.04231, 1150.0019999999997),
            "global_error_filename": (1320.07826, 2350.6079),
            "transformation_priors": (741.30051, 955.5009999999997),
            "direction_dir": (1039.92526, 433.6909999999998),
            "local_test_proba_filename": (1543.05026, 214.06500000000005),
            "local_error_filename": (1779.17326, 1976.5200999999997),
            "loo_local_test_dir": (1053.08151, 104.1869999999999),
            "global_model_filename": (484.6236600000001, 1369.7519999999997),
            "angle_dir": (1049.08151, 825.4399999999998),
            "local_model_filename": (766.45676, 1160.3769999999997),
            "local_test_transform_dirname": (1543.05026, 0.0),
            "talairach": (282.04231, 682.8149999999998),
            "loo_global_test_dir": (763.80051, 1926.2452999999998),
            "global_test_transform_filename": (1017.8002600000001, 1735.5009999999997),
            "local_output_dir": (767.80051, 362.9409999999998),
            "local": (447.6549100000001, 969.4999999999998),
            "output_local_referentials_dir": (733.48801, 754.6899999999998),
            "translation_directory": (1034.08151, 647.6889999999999),
            "inputs": (0.0, 1015.3139999999999),
            "angle_priors_filename": (1049.23776, 1128.753),
            "test_local": (1285.57826, 1208.8139999999999),
            "global_test_proba_filename": (1002.3002600000001, 1913.2452999999998),
            "outputs": (1281.70326, 358.44000000000005),
            "global_test_graph_name": (1042.30026, 2127.2700999999997),
            "local_test_graph_name": (1566.05026, 1309.3139999999999),
            "test_global": (713.01926, 1566.937),
            "global_test_t1_transform_filename": (1005.8002600000001, 1557.7499999999998),
            "global_error": (1049.73776, 2341.1079),
            "talairach_dir": (478.4674100000001, 523.875),
            "local_error": (1568.98776, 1967.0200999999997),
            "global_output_dir": (485.4674100000001, 675.5629999999999),
            "label_priors": (282.04231, 969.4409999999998),
            "local_referentials_filename": (748.08176, 1303.6899999999998),
            "translation_priors_filename": (1030.80026, 1003.1279999999999),
        }

        self.do_autoexport_nodes_parameters = False
