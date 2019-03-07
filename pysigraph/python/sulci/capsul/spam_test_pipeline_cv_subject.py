from capsul.api import Pipeline
import traits.api as traits


class SpamTestSubject(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_process("test", "sulci.capsul.spam_test_pipeline.Spam_test_pipeline")
        self.nodes["test"].process.nodes_activation = {'global_error': True, 'test_global': True, 'test_local': True, 'local_error': True}
        self.nodes["test"].process.data_graph = '/tmp/g1.arg'
        self.nodes["test"].process.label_priors = u'/tmp/cv0_left/labels_priors/frequency_segments_priors.dat'
        self.nodes["test"].process.global_model = u'/tmp/cv0_left/segments/global_registered_spam/spam_distribs.dat'
        self.nodes["test"].process.local_model = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs.dat'
        self.nodes["test"].process.subject = u's1'
        self.nodes["test"].process.local_referentials = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/local_referentials.dat'
        self.nodes["test"].process.translation_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/gaussian_translation_trm_priors.dat'
        self.nodes["test"].process.angle_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/vonmises_angle_trm_priors.dat'
        self.nodes["test"].process.direction_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/bingham_direction_trm_priors.dat'
        self.nodes["test"].process.output_global_graph = u'/s1_global.arg'
        self.nodes["test"].process.output_global_posterior_probabilities = u'/s1_global_proba.csv'
        self.nodes["test"].process.output_t1_to_global_transformation = u'/s1_T1_TO_SPAM.trm'
        self.nodes["test"].process.output_global_transformation = u'/s1_t1_TO_global.trm'
        self.nodes["test"].process.output_global_csv = u'/s1_global.csv'
        self.nodes["test"].process.output_local_graph = u'/s1_local.arg'
        self.nodes["test"].process.output_local_transformations = u'/s1_global_TO_local'
        self.nodes["test"].process.output_local_posterior_probabilities = u'/s1local_proba.csv'
        self.nodes["test"].process.output_local_csv = u'/s1_local.csv'
        self.add_custom_node("subject_test_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'subject_test_directory', 'outputs': [u'cv_directory'], 'param_types': [u'Directory', u'Str', u'Str', u'Any', u'Str'], 'separator': u'/', 'parameters': [u'cv_directory', u'test_subdir', u'subject']})
        self.nodes["subject_test_directory"].plugs["test_subdir"].optional = True
        self.nodes["subject_test_directory"].plugs["separator"].optional = True
        self.nodes["subject_test_directory"].test_subdir = u'test'
        self.add_custom_node("global_csv_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_csv_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'ext']})
        self.nodes["global_csv_filename"].plugs["sep"].optional = True
        self.nodes["global_csv_filename"].plugs["ext"].optional = True
        self.nodes["global_csv_filename"].sep = u'/'
        self.nodes["global_csv_filename"].ext = u'_global.csv'
        self.add_custom_node("local_csv_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_csv_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'ext']})
        self.nodes["local_csv_filename"].plugs["sep"].optional = True
        self.nodes["local_csv_filename"].plugs["ext"].optional = True
        self.nodes["local_csv_filename"].sep = u'/'
        self.nodes["local_csv_filename"].ext = u'_local.csv'
        self.add_custom_node("local_transformations_directory", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_transformations_directory', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'Directory'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'trans_subdir']})
        self.nodes["local_transformations_directory"].plugs["sep"].optional = True
        self.nodes["local_transformations_directory"].plugs["trans_subdir"].optional = True
        self.nodes["local_transformations_directory"].sep = u'/'
        self.nodes["local_transformations_directory"].trans_subdir = u'_global_TO_local'
        self.add_custom_node("global_posterior_probabilities_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_posterior_probabilities_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Any', u'Str', u'Any'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["global_posterior_probabilities_filename"].plugs["sep"].optional = True
        self.nodes["global_posterior_probabilities_filename"].plugs["suffix"].optional = True
        self.nodes["global_posterior_probabilities_filename"].sep = u'/'
        self.nodes["global_posterior_probabilities_filename"].suffix = u'_global_proba.csv'
        self.add_custom_node("t1_to_global_transformation_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u't1_to_global_transrormation_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["t1_to_global_transformation_filename"].plugs["sep"].optional = True
        self.nodes["t1_to_global_transformation_filename"].plugs["suffix"].optional = True
        self.nodes["t1_to_global_transformation_filename"].sep = u'/'
        self.nodes["t1_to_global_transformation_filename"].suffix = u'_T1_TO_SPAM.trm'
        self.add_custom_node("local_posterior_probabilities_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_posterior_probabilities_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Any', u'Str', u'Any'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["local_posterior_probabilities_filename"].plugs["sep"].optional = True
        self.nodes["local_posterior_probabilities_filename"].plugs["suffix"].optional = True
        self.nodes["local_posterior_probabilities_filename"].sep = u'/'
        self.nodes["local_posterior_probabilities_filename"].suffix = u'local_proba.csv'
        self.add_custom_node("global_graph_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_graph_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File', u'Str'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["global_graph_filename"].plugs["sep"].optional = True
        self.nodes["global_graph_filename"].plugs["suffix"].optional = True
        self.nodes["global_graph_filename"].sep = u'/'
        self.nodes["global_graph_filename"].suffix = u'_global.arg'
        self.add_custom_node("local_graph_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'local_graph_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File', u'Str'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["local_graph_filename"].plugs["sep"].optional = True
        self.nodes["local_graph_filename"].plugs["suffix"].optional = True
        self.nodes["local_graph_filename"].sep = u'/'
        self.nodes["local_graph_filename"].suffix = u'_local.arg'
        self.add_custom_node("global_transformation_filename", "capsul.pipeline.custom_nodes.cat_node.CatNode", {'concat_plug': u'global_transformation_filename', 'outputs': [u'subject_test_dir'], 'param_types': [u'Directory', u'Str', u'Str', u'Str', u'File', u'Str'], 'parameters': [u'subject_test_dir', u'sep', u'subject', u'suffix']})
        self.nodes["global_transformation_filename"].plugs["sep"].optional = True
        self.nodes["global_transformation_filename"].plugs["suffix"].optional = True
        self.nodes["global_transformation_filename"].sep = u'/'
        self.nodes["global_transformation_filename"].suffix = u'_t1_TO_global.trm'

        # links
        self.export_parameter("test", "data_graph")
        self.export_parameter("test", "labels_translation_map")
        self.export_parameter("test", "label_priors")
        self.export_parameter("test", "global_model")
        self.export_parameter("test", "local_model")
        self.export_parameter("local_transformations_directory", "subject")
        self.add_link("subject->subject_test_directory.subject")
        self.add_link("subject->local_csv_filename.subject")
        self.add_link("subject->t1_to_global_transformation_filename.subject")
        self.add_link("subject->global_csv_filename.subject")
        self.add_link("subject->global_posterior_probabilities_filename.subject")
        self.add_link("subject->global_graph_filename.subject")
        self.add_link("subject->local_posterior_probabilities_filename.subject")
        self.add_link("subject->test.subject")
        self.add_link("subject->global_transformation_filename.subject")
        self.add_link("subject->local_graph_filename.subject")
        self.export_parameter("test", "local_referentials")
        self.export_parameter("test", "translation_priors")
        self.export_parameter("test", "angle_priors")
        self.export_parameter("test", "direction_priors")
        self.add_link("test.output_global_graph->global_graph_filename.global_graph_filename")
        self.add_link("test.output_global_posterior_probabilities->global_posterior_probabilities_filename.global_posterior_probabilities_filename")
        self.add_link("test.output_t1_to_global_transformation->t1_to_global_transformation_filename.t1_to_global_transrormation_filename")
        self.add_link("test.output_global_transformation->global_transformation_filename.global_transformation_filename")
        self.add_link("test.output_global_csv->global_csv_filename.global_csv_filename")
        self.add_link("test.output_local_graph->local_graph_filename.local_graph_filename")
        self.add_link("test.output_local_transformations->local_transformations_directory.local_transformations_directory")
        self.add_link("test.output_local_posterior_probabilities->local_posterior_probabilities_filename.local_posterior_probabilities_filename")
        self.add_link("test.output_local_csv->local_csv_filename.local_csv_filename")
        self.export_parameter("subject_test_directory", "cv_directory")
        self.add_link("global_csv_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("local_csv_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("local_transformations_directory.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("global_posterior_probabilities_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("t1_to_global_transformation_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("local_posterior_probabilities_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("global_graph_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("local_graph_filename.subject_test_dir->subject_test_directory.subject_test_directory")
        self.add_link("global_transformation_filename.subject_test_dir->subject_test_directory.subject_test_directory")

        # default and initial values
        self.data_graph = '/tmp/g1.arg'
        self.labels_translation_map = '/neurospin/lnao/Panabase/model_database_learnclean_2018/sulci_model_2018.trl'
        self.label_priors = u'/tmp/cv0_left/labels_priors/frequency_segments_priors.dat'
        self.global_model = u'/tmp/cv0_left/segments/global_registered_spam/spam_distribs.dat'
        self.local_model = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs.dat'
        self.subject = u's1'
        self.local_referentials = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/local_referentials.dat'
        self.translation_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/gaussian_translation_trm_priors.dat'
        self.angle_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/vonmises_angle_trm_priors.dat'
        self.direction_priors = u'/tmp/cv0_left/segments/locally_from_global_registered_spam/spam_distribs/bingham_direction_trm_priors.dat'
        self.cv_directory = u'/tmp/cv0_left'

        # nodes positions
        self.node_position = {
            "inputs": (-98.91920399039537, 575.1758246858972),
            "global_transformation_filename": (706.4607296743561, 1909.6745999999998),
            "local_graph_filename": (742.4607296743561, 1644.4685),
            "global_graph_filename": (737.9607296743561, 1379.2624),
            "outputs": (1416.657355826323, 857.4380000000001),
            "subject_test_directory": (1116.0542296743565, 804.9380000000001),
            "global_posterior_probabilities_filename": (680.4607296743561, 689.5639999999999),
            "global_csv_filename": (743.8982296743561, 0.0),
            "local_transformations_directory": (704.0232296743561, 459.6869999999999),
            "local_posterior_probabilities_filename": (684.9607296743561, 1149.251),
            "test": (209.3285296743561, 814.3739999999998),
            "t1_to_global_transformation_filename": (684.0232296743561, 919.3739999999998),
            "local_csv_filename": (748.3982296743561, 229.80999999999995),
        }

        # nodes dimensions
        self.node_dimension = {
            "inputs": (196.23904525915623, 390.0),
            "global_transformation_filename": (336.875, 215.0),
            "local_graph_filename": (264.875, 215.0),
            "global_graph_filename": (273.875, 215.0),
            "outputs": (254.875, 75.0),
            "subject_test_directory": (253.75, 180.0),
            "global_posterior_probabilities_filename": (388.875, 180.0),
            "global_csv_filename": (262.0, 180.0),
            "local_transformations_directory": (341.75, 180.0),
            "local_posterior_probabilities_filename": (379.875, 180.0),
            "test": (424.75, 390.0),
            "local_transformations_dirname": (290.75, 218.0),
            "t1_to_global_transformation_filename": (381.75, 180.0),
            "local_csv_filename": (253.0, 180.0),
        }

        self.do_autoexport_nodes_parameters = False
