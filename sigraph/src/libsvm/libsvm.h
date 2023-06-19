
#ifndef DATAMIND_LIBSVM_H
#define DATAMIND_LIBSVM_H

#include <svm.h>

#if LIBSVM_VERSION < 300
struct svm_model {
  struct svm_parameter param;	/* parameter */
  /** number of classes (n) , = 2 in regression/one class svm */
  int nr_class;
  int l;			/* total #SV */
  /** TOUS LES VECTEURS DE SUPPORTS (DE TOUTES LES CLASSES) :
    * Tableau de pointeurs sur les vecteurs contenus dans un grand
    * tableau contenant toutes les coordonnées de tous les vecteurs les
    * unes à la suite des autres. */
  struct svm_node **SV;		/* SVs (SV[l]) */
  double **sv_coef;	/* coefficients for SVs in decision functions (sv_coef[n-1][l]) */
  double *rho;		/* constants in decision functions (rho[n*(n-1)/2]) */
  double *probA;          /* pariwise probability information */
  double *probB;

  /* for classification only */

  /** Tableau de labels (numéros) de classes */
  int *label;		/* label of each class (label[n]) */
  int *nSV;		/* number of SVs for each class (nSV[n]) */
  /* nSV[0] + nSV[1] + ... + nSV[n-1] = l */
  /* XXX */
  int free_sv;		/* 1 if svm_model is created by svm_load_model */
                          /* 0 if svm_model is created by svm_train */
};
#endif

struct svm_node *svm_clean_model( const struct svm_model *model );
struct svm_model *svm_clone_model( const struct svm_model *model,
                                   const struct svm_node *data_old,
                                   struct svm_node **data_new_p );
struct svm_parameter *svm_clone_parameter( const struct svm_parameter *param );

double svm_predict_quality( const struct svm_model *model,
                            const struct svm_node *x,
                            const unsigned int dim, float qualityslope,
                            int shifted );

double svm_predict_decision( const struct svm_model *model,
                             const struct svm_node *x );

void svm_print_parameter( const struct svm_parameter *p );

  
#endif

