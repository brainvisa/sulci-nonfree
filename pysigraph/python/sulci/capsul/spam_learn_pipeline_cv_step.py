from __future__ import absolute_import
from capsul.api import Pipeline
import traits.api as traits


class SpamLearnPipelineCVStep(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("global", "sulci.capsul.spam_learn_global_registration.SpamLearnGlobalRegistration")
        self.nodes["global"].process.graphs = ['/tmp/g2.arg', '/tmp/g3.arg']
        self.nodes["global"].process.output_directory = u'/tmp/cv0/segments/global_registered_spam/spam_distribs'
        self.nodes["global"].process.threads = 36
        self.add_process("local", "sulci.capsul.spam_learn_local_registration.SpamLearnLocalRegistration")
        self.nodes["local"].process.global_spam_distribs = u'/tmp/cv0/segments/global_registered_spam/spam_distribs'
        self.nodes["local"].process.graphs = ['/tmp/g2.arg', '/tmp/g3.arg']
        self.nodes["local"].process.output_directory = u'/tmp/cv0/segments/locally_from_global_registered_spam/spam_distribs'
        self.nodes["local"].process.output_local_referentials_directory = u'/tmp/cv0/segments/locally_from_global_registered_spam/local_referentials'
        self.nodes["local"].process.threads = 36
        self.add_process("label_priors", "sulci.capsul.spam_learn_label_priors.SpamLearnLabelPriors")
        self.nodes["label_priors"].process.graphs = ['/tmp/g2.arg', '/tmp/g3.arg']
        self.nodes["label_priors"].process.output_directory = u'/tmp/cv0/labels_priors/frequency_segments_priors'
        self.add_process("talairach", "sulci.capsul.spam_learn_talairach.SpamLearnTalairach")
        self.nodes["talairach"].process.graphs = ['/tmp/g2.arg', '/tmp/g3.arg']
        self.nodes["talairach"].process.output_directory = u'/tmp/cv0/segments/talairach_spam/spam_distribs'
        self.add_process("transformation_priors", "sulci.capsul.spam_learn_transformation_priors.SpamLearnTransformationPrior")
        self.nodes["transformation_priors"].process.output_angle_directory = u'/tmp/cv0/vonmises_angle_trm_priors'
        self.nodes["transformation_priors"].process.output_direction_directory = u'/tmp/cv0/bingham_direction_trm_priors'
        self.nodes["transformation_priors"].process.output_translation_directory = u'/tmp/cv0/gaussian_translation_trm_priors'
        self.nodes["transformation_priors"].process.spams_directory = u'/tmp/cv0/segments/locally_from_global_registered_spam/spam_distribs'
        self.nodes["transformation_priors"].process.threads = 36
        self.add_custom_node("label_priors_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'label_priors', 'outputs': [u'label_priors'], 'param_types': [u'Any', u'Any', u'Any', u'Str'], 'separator': u'', 'parameters': [u'labels_input_directory', u'ext']})
        self.nodes["label_priors_filename"].plugs["separator"].optional = True
        self.nodes["label_priors_filename"].plugs["ext"].optional = True
        self.nodes["label_priors_filename"].ext = u'.dat'
        self.add_custom_node("global_model_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_model', 'outputs': [u'global_model'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'input_directory', u'ext']})
        self.nodes["global_model_filename"].plugs["ext"].optional = True
        self.nodes["global_model_filename"].ext = u'.dat'
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
        self.add_custom_node("local_referentials_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_referentials', 'outputs': [u'local_referentials'], 'param_types': [u'Any', u'Any', u'Any'], 'parameters': [u'local_referentials_dir', u'ext']})
        self.nodes["local_referentials_filename"].plugs["ext"].optional = True
        self.nodes["local_referentials_filename"].ext = u'.dat'
        self.add_custom_node("cv_filter", "capsul.pipeline.custom_nodes.cvfilter_node.CVFilterNode", {'param_type': u'File', 'is_output': True})
        self.nodes["cv_filter"].plugs["inputs"].optional = True
        self.nodes["cv_filter"].plugs["fold"].optional = True
        self.nodes["cv_filter"].plugs["nfolds"].optional = True
        self.nodes["cv_filter"].plugs["learn_list"].optional = True
        self.nodes["cv_filter"].plugs["test_list"].optional = True
        self.add_custom_node("talairach_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'talairach_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'tal_subdir']})
        self.nodes["talairach_directory"].plugs["tal_subdir"].optional = True
        self.nodes["talairach_directory"].plugs["separator"].optional = True
        self.nodes["talairach_directory"].tal_subdir = u'segments/talairach_spam/spam_distribs'
        self.add_custom_node("global_output_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_output_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'global_subdir']})
        self.nodes["global_output_directory"].plugs["global_subdir"].optional = True
        self.nodes["global_output_directory"].plugs["separator"].optional = True
        self.nodes["global_output_directory"].global_subdir = u'segments/global_registered_spam/spam_distribs'
        self.add_custom_node("label_priors_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'label_priors_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'label_priors_subdir']})
        self.nodes["label_priors_directory"].plugs["label_priors_subdir"].optional = True
        self.nodes["label_priors_directory"].plugs["separator"].optional = True
        self.nodes["label_priors_directory"].label_priors_subdir = u'labels_priors/frequency_segments_priors'
        self.add_custom_node("local_output_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_output_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'local_subdir']})
        self.nodes["local_output_directory"].plugs["local_subdir"].optional = True
        self.nodes["local_output_directory"].plugs["separator"].optional = True
        self.nodes["local_output_directory"].local_subdir = u'segments/locally_from_global_registered_spam/spam_distribs'
        self.add_custom_node("angle_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_angle_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'angle_subdir']})
        self.nodes["angle_directory"].plugs["angle_subdir"].optional = True
        self.nodes["angle_directory"].plugs["separator"].optional = True
        self.nodes["angle_directory"].angle_subdir = u'vonmises_angle_trm_priors'
        self.add_custom_node("direction_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_direction_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'direction_subdir']})
        self.nodes["direction_directory"].plugs["direction_subdir"].optional = True
        self.nodes["direction_directory"].plugs["separator"].optional = True
        self.nodes["direction_directory"].direction_subdir = u'bingham_direction_trm_priors'
        self.add_custom_node("translation_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_translation_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'translation_subdir']})
        self.nodes["translation_directory"].plugs["translation_subdir"].optional = True
        self.nodes["translation_directory"].plugs["separator"].optional = True
        self.nodes["translation_directory"].translation_subdir = u'gaussian_translation_trm_priors'
        self.add_custom_node("cv_subject_filter", "capsul.pipeline.custom_nodes.cvfilter_node.CVFilterNode", {'param_type': u'Str', 'is_output': True})
        self.nodes["cv_subject_filter"].plugs["inputs"].optional = True
        self.nodes["cv_subject_filter"].plugs["fold"].optional = True
        self.nodes["cv_subject_filter"].plugs["nfolds"].optional = True
        self.nodes["cv_subject_filter"].plugs["learn_list"].optional = True
        self.nodes["cv_subject_filter"].plugs["test_list"].optional = True
        self.nodes["cv_subject_filter"].learn_list = [u's2', u's3']
        self.add_iterative_process("test", "sulci.capsul.spam_test_pipeline_cv_subject.SpamTestSubject", iterative_plugs=set([u'subject', u'data_graph']))
        self.add_custom_node("local_referentials_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'output_local_referentials_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'local_ref_subdir']})
        self.nodes["local_referentials_directory"].plugs["local_ref_subdir"].optional = True
        self.nodes["local_referentials_directory"].plugs["separator"].optional = True
        self.nodes["local_referentials_directory"].local_ref_subdir = u'segments/locally_from_global_registered_spam/local_referentials'

        # links
        self.export_parameter("cv_filter", "inputs", "graphs")
        self.export_parameter("local", "threads")
        self.add_link("threads->transformation_priors.threads")
        self.add_link("threads->global.threads")
        self.export_parameter("label_priors", "translation_file")
        self.add_link("translation_file->talairach.translation_file")
        self.add_link("translation_file->global.translation_file")
        self.add_link("translation_file->local.translation_file")
        self.add_link("translation_file->test.labels_translation_map")
        self.export_parameter("local", "verbose")
        self.add_link("verbose->global.verbose")
        self.export_parameter("cv_filter", "nfolds")
        self.add_link("nfolds->cv_subject_filter.nfolds")
        self.export_parameter("cv_subject_filter", "fold")
        self.add_link("fold->cv_filter.fold")
        self.export_parameter("cv_subject_filter", "inputs", "subjects")
        self.add_link("global.output_directory->local.global_spam_distribs")
        self.add_link("global.output_directory->global_model_filename.input_directory")
        self.add_link("global.output_directory->global_output_directory.global_output_directory")
        self.add_link("local.output_directory->local_model_filename.local_model_dir")
        self.add_link("local.output_directory->transformation_priors.spams_directory")
        self.add_link("local.output_directory->local_output_directory.local_output_directory")
        self.add_link("local.output_local_referentials_directory->local_referentials_filename.local_referentials_dir")
        self.add_link("local.output_local_referentials_directory->local_referentials_directory.output_local_referentials_directory")
        self.add_link("label_priors.output_directory->label_priors_filename.labels_input_directory")
        self.add_link("label_priors.output_directory->label_priors_directory.label_priors_directory")
        self.add_link("talairach.output_directory->talairach_directory.talairach_directory")
        self.add_link("transformation_priors.output_angle_directory->angle_priors_filename.angle_priors_dir")
        self.add_link("transformation_priors.output_angle_directory->angle_directory.output_angle_directory")
        self.add_link("transformation_priors.output_direction_directory->direction_directory.output_direction_directory")
        self.add_link("transformation_priors.output_direction_directory->direction_priors_filename.direction_priors_dir")
        self.add_link("transformation_priors.output_translation_directory->translation_priors_filename.translation_priors_dir")
        self.add_link("transformation_priors.output_translation_directory->translation_directory.output_translation_directory")
        self.add_link("label_priors_filename.label_priors->test.label_priors")
        self.add_link("global_model_filename.global_model->test.global_model")
        self.add_link("angle_priors_filename.angle_priors->test.angle_priors")
        self.add_link("direction_priors_filename.direction_priors->test.direction_priors")
        self.add_link("translation_priors_filename.translation_priors->test.translation_priors")
        self.add_link("local_model_filename.local_model->test.local_model")
        self.add_link("local_referentials_filename.local_referentials->test.local_referentials")
        self.add_link("cv_filter.learn_list->talairach.graphs")
        self.add_link("cv_filter.learn_list->label_priors.graphs")
        self.add_link("cv_filter.learn_list->local.graphs")
        self.add_link("cv_filter.learn_list->global.graphs")
        self.add_link("cv_filter.test_list->test.data_graph")
        self.export_parameter("label_priors_directory", "cv_directory")
        self.add_link("talairach_directory.cv_directory->cv_directory")
        self.add_link("global_output_directory.cv_directory->cv_directory")
        self.add_link("label_priors_directory.cv_directory->cv_directory")
        self.add_link("local_output_directory.cv_directory->cv_directory")
        self.add_link("angle_directory.cv_directory->cv_directory")
        self.add_link("direction_directory.cv_directory->cv_directory")
        self.add_link("translation_directory.cv_directory->cv_directory")
        self.add_link("cv_subject_filter.test_list->test.subject")
        self.add_link("test.cv_directory->cv_directory")
        self.add_link("local_referentials_directory.cv_directory->cv_directory")

        # pipeline steps
        self.add_pipeline_step("learning", [u'global', u'local', u'label_priors', u'talairach', u'transformation_priors'])
        self.add_pipeline_step("testing", [u'test_global', u'test_local', u'test'])
        self.add_pipeline_step("evaluation", [u'global_error', u'local_error'])

        # default and initial values
        self.graphs = ['/tmp/g1.arg', '/tmp/g2.arg', '/tmp/g3.arg']
        self.threads = 36
        self.subjects = [u's1', u's2', u's3']
        self.cv_directory = u'/tmp/cv0'

        # nodes positions
        self.node_position = {
            "nnn": (844.1764512745812, 170.52848084486524),
            "label_priors_filename": (688.8725996743563, 1349.2862),
            "inputs": (-25.39785884923525, 920.6481378004366),
            "label_priors_directory": (1108.3080996743563, 1384.6421),
            "angle_directory": (1992.266474474065, 473.89298485069173),
            "global": (328.62929967435616, 1005.1611999999999),
            "talairach_directory": (1996.6535996743562, 1479.561),
            "global_output_directory": (683.4350996743563, 370.30899999999997),
            "cv_subject_filter": (152.84659615196716, 256.8109999999999),
            "transformation_priors": (1107.3705996743563, 570.4379999999999),
            "direction_directory": (1971.1535996743562, 266.1239999999998),
            "direction_priors_filename": (1562.8410996743562, 692.1229999999999),
            "global_model_filename": (704.8725996743563, 1118.3234),
            "local_model_filename": (1588.7160996743564, 1047.6048999999998),
            "talairach": (331.62929967435616, 1517.5362),
            "test": (1901.0910996743562, 712.1859999999999),
            "cv_filter": (152.84659615196716, 1233.8921),
            "local": (616.9975996743563, 852.7499999999999),
            "local_referentials_filename": (1129.9330996743563, 916.372),
            "translation_directory": (1530.9035996743562, 301.5),
            "angle_priors_filename": (1585.7160996743564, 532.06),
            "outputs": (2368.206829348712, 479.37199999999984),
            "local_referentials_directory": (1118.469641443979, 201.59170518968568),
            "local_output_directory": (1984.0910996743562, 1152.0047),
            "label_priors": (331.62929967435616, 1357.4798999999998),
            "output_local_referentials_dir": (1057.0580996743563, 0.0),
            "translation_priors_filename": (1548.8410996743562, 852.1859999999999),
        }

        # nodes dimensions
        self.node_dimension = {
            "nnn": (133.0, 145.0),
            "label_priors_filename": (249.875, 145.0),
            "inputs": (141.98904525915623, 285.0),
            "direction_priors_filename": (259.75, 110.0),
            "global_model_filename": (217.875, 110.0),
            "angle_directory": (262.75, 145.0),
            "global": (241.75, 180.0),
            "talairach_directory": (229.625, 145.0),
            "global_output_directory": (265.75, 145.0),
            "cv_subject_filter": (133.0, 145.0),
            "global_error_filename": (157.78125, 218.0),
            "loo_global_test_dir": (235.875, 113.0),
            "local_referentials_directory": (339.5, 145.0),
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
            "outputs": (251.65979295522197, 75.0),
            "subject_test_directory": (237.75, 180.0),
            "local_transformations_directory": (334.75, 145.0),
            "cv_filter": (133.0, 145.0),
            "local_transformations_dirname": (290.75, 218.0),
            "global_test_graph_name": (173.0, 218.0),
            "local_test_graph_name": (164.0, 218.0),
            "local_csv_filename": (246.0, 215.0),
            "global_test_t1_transform_filename": (263.909792955222, 218.0),
            "test_global": (409.75, 180.0),
            "direction_directory": (285.625, 145.0),
            "global_posterior_probabilities_filename": (381.875, 145.0),
            "local_output_directory": (256.75, 145.0),
            "test_local": (375.75, 355.0),
            "local_error": (245.875, 180.0),
            "label_priors": (235.75, 110.0),
            "output_local_referentials_dir": (427.25, 145.0),
            "translation_priors_filename": (287.75, 110.0),
        }

        self.do_autoexport_nodes_parameters = False
