
#ifndef SI_GRAPH_ATTRIB_H
#define SI_GRAPH_ATTRIB_H

/*	Attributs internes des graphes.
	Attributs non spécialisés
 */


#define SIA_MODEL_GRAPH_SYNTAX		"model_graph"

#define SIA_NBASEGRAPHS			"nbase_graphs"

#define SIA_POINTER			"pointer"
#define SIA_POSSIBLE_LABELS		"possible_labels"
#define SIA_GRAPH			"graph"
#define SIA_ORIGINAL_CACHE		"original_cache"
#define SIA_CACHE			"cache"
#define SIA_LABEL			"label"
#define	SIA_LABEL1			"label1"
#define	SIA_LABEL2			"label2"
#define SIA_CLIQUES			"cliques"
#define	SIA_IS_COPY			"is_copy"
#define SIA_POTENTIAL			"potential"
#define SIA_NAME			"name"
#define SIA_VOID_LABEL			"void_label"

#define SIA_MODEL			"model"
#define SIA_MODEL_FILE			"model_file"
#define SIA_DEFAULT_MODEL		"default_model"
#define SIA_DEFAULT_MODEL_FILE		"default_model_file"
#define SIA_MODEL_NODE			"model_node"
#define SIA_MODEL_EDGE			"model_edge"
#define SIA_MODEL_TYPE			"model_type"
#define SIA_OCCURENCE_COUNT		"occurence_count"
#define SIA_NOINSTANCE_COUNT		"noinstance_count"

#define SIA_DOMAIN			"domain"
#define SIA_DOMAIN_FILE			"domain_file"
#define SIA_NULL_DOMAIN			"null_domain"

#define SIA_POT_VECTOR			"pot_vector"
#define SIA_POT_VECTOR_NORM		"pot_vector_norm"

#define SIA_MANDATORY_EMPTY_OUTPUT	"mandatory_empty_output"
#define SIA_EMPTY_PENALTY_THRESHOLD	"empty_penalty_threshold"


//	Data graphs

#define SIA_DATAGRAPH_VERSION		"datagraph_VERSION"
/// minimum model graph version able to deal with these data
#define SIA_DATAGRAPH_COMPAT_MODEL_VERSION \
  "datagraph_compatibility_model_VERSION"

//	Models

#define SIA_AD_LEAF_SYNTAX		"ad_leaf"
#define	SIA_WORK			"work"
#define SIA_EVAL			"eval"
#define	SIA_WORKMEMO			"workmemo"
#define SIA_EVALMEMO			"evalmemo"
#define SIA_NB_LEARN_DATA		"nb_learn_data"
#define	SIA_LEARN_STATE			"learn_state"
#define	SIA_NB_LEARN_DATA_MEMO		"nb_learn_data_memo"
#define	SIA_WORK_MEMO			"work_memo"
#define SIA_EVAL_MEMO			"eval_memo"
#define SIA_MODEL_VERSION		"model_VERSION"
/// minimum data graph version this model can deal with
#define SIA_MODEL_COMPAT_DATA_VERSION	"model_compatibility_data_VERSION"


//	Sub-Models

#define	SIA_USEDINPUTS			"usedinputs"
#define SIA_NSTATS			"nstats"
#define SIA_MEAN			"mean"
#define SIA_SIGMA			"sigma"
#define SIA_ERROR_RATE			"error_rate"
#define SIA_GEN_ERROR_RATE		"gen_error_rate"
#define SIA_GLOBAL_MIN_GERROR		"global_min_gen_error"
#define SIA_GLOBAL_MAX_GERROR		"global_max_gen_error"
#define SIA_LOCAL_MIN_GERROR		"local_min_gen_error"
#define SIA_LOCAL_MAX_GERROR		"local_max_gen_error"
#define SIA_GEN_GOOD_ERROR_RATE		"gen_good_error_rate"
#define SIA_GEN_BAD_ERROR_RATE		"gen_bad_error_rate"
#define SIA_GLOBAL_GOOD_MIN_GERROR	"global_good_min_gen_error"
#define SIA_GLOBAL_GOOD_MAX_GERROR	"global_good_max_gen_error"
#define SIA_LOCAL_GOOD_MIN_GERROR	"local_good_min_gen_error"
#define SIA_LOCAL_GOOD_MAX_GERROR	"local_good_max_gen_error"
#define SIA_APP_GOOD_ERROR_RATE		"app_good_error_rate"
#define SIA_STEPS_SINCE_GEN_MIN		"steps_since_gen_min"
#define SIA_GLOBAL_GEN_MIN_ERROR	"global_gen_min_error"
#define SIA_MISCLASS_GOOD_RATE          "misclass_good_rate"
#define SIA_MISCLASS_BAD_RATE           "misclass_bad_rate"
#define SIA_RELIANCE_WEIGHT_METHOD      "reliance_weight_method"

//	Clique attributes

#define SIA_DESCRIPTOR			"descriptor"
#define SIA_DESCRIPTOR_NAMES		"descriptor_names"

// other

#define SIA_SUBJECT "subject"


//	Values

#define SIV_RANDOM_VERTEX		"random_vertex"
#define SIV_RANDOM_EDGE			"random_edge"
#define SIV_VOID_LABEL			"unknown"

#endif



