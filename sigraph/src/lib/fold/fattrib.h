
#ifndef SI_FOLD_ATTRIBUTES
#define SI_FOLD_ATTRIBUTES


#include <si/graph/attrib.h>

/*	Attributs globaux (a priori, immuables, indépendants de la version).

	convention :
 
	Attributs : préfixe : SIA_ (Si Attrib)
		    no de version : quand qqchose est spécifique à une version
		     		    et pas du tout utilisé par les autres
		    pas de version : quand c'est utilisé en interne par 
		     		      toutes les versions, une fois converti

	Valeurs : préfixe : SIV_ (Si Val)
*/

//	Fichiers graphe

#define	SIA_GRAPH_SYNTAX		"CorticalFoldArg"
#define SIA_HULL_SYNTAX			"hull"
#define SIA_HULLJUNCTION_SYNTAX		"hull_junction"
#define SIA_CORTICAL_SYNTAX		"cortical"
#define	SIA_JUNCTION_SYNTAX		"junction"
#define SIA_PLI_DE_PASSAGE_SYNTAX	"plidepassage"
#define SIA_FAKEREL_SYNTAX		"fake_relation"
#define SIA_FOLD_SYNTAX			"fold"

#define	SIA_VERSION			"CorticalFoldArg_VERSION"

#define SIA_FILENAME_BASE		"filename_base"
#define SIA_VOXEL_SIZE			"voxel_size"
#define SIA_INDEX			"index"

#define SIA_TALAIRACH_ROTATION		"Talairach_rotation"
#define SIA_TALAIRACH_SCALE		"Talairach_scale"
#define SIA_TALAIRACH_TRANSLATION	"Talairach_translation"

#define	SIA_SS_FILENAME			"ss_filename"
#define SIA_SS_BUCKET			"aims_ss"
#define	SIA_BOTTOM_FILENAME		"bottom_filename"
#define SIA_BOTTOM_BUCKET		"aims_bottom"
#define SIA_OTHER_FILENAME		"other_filename"
#define SIA_OTHER_BUCKET		"aims_other"
#define SIA_OLDTRI			"oldtri"
#define SIA_OLDTRI_FILENAME		"oldtri_filename"
#define SIA_TMTKTRI_FILENAME		"Tmtktri_filename"
#define SIA_TYPETRI			"type.tri"
#define SIA_INDEX			"index"

#define SIA_GRAVITY_CENTER		"gravity_center"
#define SIA_NORMAL			"normal"
#define SIA_NAME			"name"
#define SIA_SIZE			"size"
#define SIA_DEPTH			"depth"
#define SIA_INERTIA			"talcovar"
#define SIA_POINT_NUMBER		"point_number"
#define SIA_BOTTOM_POINT_NUMBER		"bottom_point_number"

//	Attributs de relation

#define SIA_JUNCTION_FILENAME		"junction_filename"
#define SIA_JUNCTION_BUCKET		"aims_junction"
#define SIA_PLIDEPASSAGE_FILENAME	"plidepassage_filename"
#define SIA_PLIDEPASSAGE_BUCKET		"aims_plidepassage"
#define SIA_CORTICAL_FILENAME		"cortical_filename"
#define SIA_CORTICAL_BUCKET		"aims_cortical"

//	Modèles

#define SIA_FOLD_DESCRIPTOR		"fold_descriptor"
#define	SIA_INTER_FOLD_DESCRIPTOR	"inter_fold_descriptor"
#define	SIA_BRAIN_JUNCTION_DESCRIPTOR	"brain_junction_descriptor" // obsolète
#define	SIA_NSTATS_NORMAL		"nstats_normal"
#define SIA_LIMIT_SIZE			"limit_size"


///	Attributs de la version 0.8

#define	SIA_0_8_GRAPH_SYNTAX		"graphe_sillon"
#define SIA_0_8_TALAIRACH_M_ROTATION	"Talairach_m_rotation"

#define	SIV_0_8_BRAIN			"brain"


///	Attributs des versions suivantes

//	Fichiers graphes

#define SIA_0_9_BOUNDINGBOX_MIN		"boundingbox_min"
#define SIA_0_9_BOUNDINGBOX_MAX		"boundingbox_max"

#define SIA_TAL_BOUNDINGBOX_MIN		"Tal_boundingbox_min"
#define SIA_TAL_BOUNDINGBOX_MAX		"Tal_boundingbox_max"
#define SIA_REFSIZE			"refsize"
#define SIA_REFNORMAL			"refnormal"
#define SIA_REFGRAVITY_CENTER		"refgravity_center"
#define	SIA_REFEXTREMITY1		"refextremity1"
#define	SIA_REFEXTREMITY2		"refextremity2"
#define	SIA_REFDIRECTION		"refdirection"
#define SIA_MINDEPTH			"mindepth"
#define SIA_MAXDEPTH			"maxdepth"
#define SIA_REFSS1NEAREST		"refSS1nearest"
#define SIA_REFSS2NEAREST		"refSS2nearest"
#define	SIA_DIST			"dist"
#define SIA_FAKEREL_MODEL		"fakeRel_model"
#define SIA_FLIPPED_HEMISPHERES		"flipped_hemispheres"

#define SIA_SURFACE_AREA		"surface_area"
#define SIA_REFSURFACE_AREA		"refsurface_area"
#define SIA_REFDEPTH			"refdepth"
#define SIA_REFMINDEPTH			"refmindepth"
#define SIA_REFMAXDEPTH			"refmaxdepth"
#define SIA_MEANDEPTH			"mean_depth"
#define SIA_REFMEANDEPTH		"refmean_depth"
#define SIA_LENGTH			"length"
#define SIA_REFLENGTH			"reflength"
#define SIA_REFDIST			"refdist"
#define	SIA_EXTREMITY1			"extremity1"
#define	SIA_EXTREMITY2			"extremity2"
#define SIA_SS1NEAREST			"SS1nearest"
#define SIA_SS2NEAREST			"SS2nearest"
#define SIA_MOMENTS			"moments"



///	modèles

#define SIA_FOLD_DESCRIPTOR2		"fold_descriptor2"
#define SIA_FOLD_DESCRIPTOR3		"fold_descriptor3"
#define SIA_FOLD_DESCRIPTOR4		"fold_descriptor4"
#define SIA_FOLD_DESCRIPTOR5		"fold_descriptor5"
#define	SIA_INTER_FOLD_DESCRIPTOR2	"inter_fold_descriptor2"
#define	SIA_INTER_FOLD_DESCRIPTOR4	"inter_fold_descriptor4"
#define	SIA_INTER_FOLD_DESCRIPTOR5	"inter_fold_descriptor5"
#define SIA_NSTATS_E1E2			"nstats_E1E2"
#define SIA_E1E2			"e1e2"
#define	SIA_NSTATS_DIR			"nstats_dir"
#define	SIA_DIRECTION			"direction"
#define SIA_NORMALIZED_OUTPUT           "normalized_output"


//	Domaines

#define	SIA_DOMAIN_BOX			"domain_box"
#define	SIA_DOMAIN_BOX2			"domain_box2"
#define SIA_INERTIAL_DOMAIN_BOX		"inertial_domain_box"
#define	SIA_DOMAIN_RBF			"domain_rbf"

#define SIA_XMIN			"xmin"
#define SIA_XMAX			"xmax"
#define SIA_YMIN			"ymin"
#define SIA_YMAX			"ymax"
#define SIA_ZMIN			"zmin"
#define SIA_ZMAX			"zmax"
#define SIA_NDATA			"ndata"
#define SIA_EIGENVALUES			"eigenvalues"
#define SIA_ROTATION			"rotation"
#define SIA_DOM_INERTIA			"inertia"
#define SIA_TOLERENCE_MARGIN		"tolerence_margin"
#define SIA_GRAVITYCENTER_ATTRIBUTE	"gravity_center_attribute"

#define SIA_SIGMA			"sigma"
#define SIA_THRESHOLD			"threshold"
#define SIA_LEARNTHRESHOLD		"learn_threshold"
#define SIA_GAUSSCENTERS		"gauss_centers"


//	Cliques

#define SIA_FAKEREL			"fakeRel"


///	Valeurs d'attributs

#define SIV_OTHER			"other"
#define SIV_BRAIN_HULL			"brain_hull"
#define	SIV_FAKE_REL			"fake_relation"

#define SIV_VERSION_0_8			"1.0"
#define SIV_VERSION_0_9			"2.0"


#endif


