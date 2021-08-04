from __future__ import absolute_import
from capsul.api import Pipeline
import traits.api as traits
import os
from six.moves import range


class SpamLearnPipelineCV(Pipeline):

    def pipeline_definition(self):
        # nodes
        self.add_iterative_process("spam_learn_cv", "sulci.capsul.spam_learn_pipeline_cv_step.SpamLearnPipelineCVStep", iterative_plugs=set([u'fold', u'cv_directory']))

        # links
        self.export_parameter("spam_learn_cv", "verbose")
        self.export_parameter("spam_learn_cv", "graphs")
        self.export_parameter("spam_learn_cv", "pipeline_steps")
        self.export_parameter("spam_learn_cv", "fold")
        self.export_parameter("spam_learn_cv", "subjects")
        self.export_parameter("spam_learn_cv", "threads")
        self.export_parameter("spam_learn_cv", "translation_file")
        self.export_parameter("spam_learn_cv", "nfolds")
        self.export_parameter("spam_learn_cv", "cv_directory",
                              "cv_directories")

        # pipeline steps
        self.add_pipeline_step("learning", [u'global', u'local', u'label_priors', u'talairach', u'transformation_priors'])
        self.add_pipeline_step("testing", [u'test_global', u'test_local', u'test'])
        self.add_pipeline_step("evaluation", [u'global_error', u'local_error'])

        # default and initial values
        self.graphs = ['/tmp/g1.arg', '/tmp/g2.arg', '/tmp/g3.arg']
        self.pipeline_steps = {'testing': True, 'evaluation': True, 'learning': True}
        self.subjects = [u's1', u's2', u's3']
        self.threads = 36

        # nodes positions
        self.node_position = {
            "inputs": (-522.224197343662, -453.4210899656953),
            "spam_learn_cv": (-233.2252158156881, -439.08881413344636),
            "outputs": (93.40736528520276, -338.7628833077034),
        }

        # nodes dimensions
        self.node_dimension = {
            "nnn": (133.0, 145.0),
            "label_priors_filename": (249.875, 145.0),
            "inputs": (141.98904525915623, 320.0),
            "direction_priors_filename": (259.75, 110.0),
            "global_model_filename": (217.875, 110.0),
            "angle_directory": (262.75, 145.0),
            "global": (241.75, 180.0),
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
            "output_local_referentials_dir": (427.25, 145.0),
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
            "spam_learn_cv": (203.75, 320.0),
            "local_referentials_directory": (339.5, 145.0),
            "global_posterior_probabilities_filename": (381.875, 145.0),
            "local_output_directory": (256.75, 145.0),
            "test_local": (375.75, 355.0),
            "local_error": (245.875, 180.0),
            "label_priors": (235.75, 110.0),
            "local_referentials_filename": (281.5, 110.0),
            "translation_priors_filename": (287.75, 110.0),
        }

        self.do_autoexport_nodes_parameters = False

        # manual modifs

        self.add_trait("output_directory", traits.Directory(output=True))

        # callback
        self.set_folds()
        self.on_trait_change(self.set_folds, ['nfolds', 'output_directory'])

    def set_folds(self):
        self.fold = list(range(self.nfolds))
        self.cv_directories = [os.path.join(self.output_directory,
                                            'cv_%d' % fold)
                               for fold in self.fold]


