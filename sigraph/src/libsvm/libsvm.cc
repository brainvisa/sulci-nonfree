
#include <cstdlib>
#include <stdlib.h>
#include <math.h>
#include <limits>
#include <iostream>
#include <si/sisvm/libsvm.h>

/** Our new predict function. */
double svm_predict_quality( const struct svm_model *model,
                            const struct svm_node *x, const unsigned int dim,
                            float qualityslope, int shifted )
{
  const double * const*  sv_coef =  model->sv_coef;
  const svm_node* const*  SV = model->SV;
  const svm_node*    xp;
  int      lmodel = model->l;
  double      decision;

  svm_predict_values(model, x, &decision);

  double    dmin = std::numeric_limits<double>::max();
  double    dis;
  double    s;
  int    i;

  for(int k = 0; k < lmodel; k++)
  {
    if(model->label[sv_coef[0][k] < 0 ] == 1)
      continue;
    const svm_node  *p = SV[k];
    dis = 0;
    xp = x;
    for(i = 0; p->index != -1; ++i, ++xp, ++p)
    {
      s = (xp->value - p->value);
      dis += s * s;
    }
    if (dis < dmin) dmin = dis;
  }
  /* normalize dist by dimensionality */
  dmin /= sqrt(dim);

  /* sigmoid */
  if(model->label[ decision < 0] == 1)
    if(shifted)
      decision = 1. / (1 + exp( -dmin * qualityslope ));
    else  decision = 2. / (1 + exp( -dmin * qualityslope )) - 1.;
  else  decision = 0;

  return decision;
}

/** Simple wrapper of svm_predict_values */
double svm_predict_decision( const struct svm_model *model,
                             const struct svm_node *x )
{
  double  decision;

  svm_predict_values(model, x, &decision);
  return decision;
}

/** Detach Support Vectors from training data. Pointer owned by training data
is not deleted. */
struct svm_node *svm_clean_model( const struct svm_model *model )
{
  int    i, j, elements;
  svm_node  *x_space = NULL, *p = NULL;

  for (i = 0, elements = 0; i < model->l; i++, elements++)
  {
    p = model->SV[i];
    for (j = 0; p->index != -1; p++, j++)
      elements++;
  }
  x_space = (svm_node *) malloc(sizeof(svm_node) * elements);
  for (i = 0, j = 0; i < model->l; i++, j++)
  {
    p = model->SV[i];
    model->SV[i] = &(x_space[j]);
    for(; p->index != -1; p++, j++)
    {
      x_space[j].index = p->index;
      x_space[j].value = p->value;
    }
    x_space[j].index = p->index;
    x_space[j].value = p->value;
  }

  return x_space;
}

/** Copy all data (including sub-structures) in a new svm_model. */
struct svm_model *svm_clone_model( const struct svm_model *model,
                                   const svm_node *data_old,
                                   svm_node **data_new_p )
{
  int      n, i, j, elements;
  svm_node    *x_space = NULL;
  struct svm_model  *m = NULL;
  struct svm_parameter  *p = NULL;

  m = (struct svm_model *) malloc(sizeof(struct svm_model));
  p = svm_clone_parameter(&(model->param));
  m->param = *p;
  m->nr_class = model->nr_class;
  m->l = model->l;
  for (i = 0, elements = 0; i < m->l; i++, elements++)
    while(data_old[elements].index != -1)
      elements++;
  x_space = (svm_node *) malloc(sizeof(svm_node) * (elements+1));
  *data_new_p = x_space;
  for (i = 0; i < elements; i++)
  {
    x_space[i].value = data_old[i].value;
    x_space[i].index = data_old[i].index;
  }
  m->SV = (svm_node **) malloc(sizeof(svm_node *) * m->l);
  for (i = 0; i < m->l; i++)
    m->SV[i] = &x_space[(model->SV[i] - data_old)];
  m->sv_coef = (double **) malloc(sizeof(double *) *
    (m->nr_class - 1));
  for (i = 0; i < m->nr_class - 1; i++)
  {
    m->sv_coef[i] = (double *) malloc(sizeof(double) * m->l);
    for (j = 0; j < m->l; j++)
      m->sv_coef[i][j] = model->sv_coef[i][j];
  }
  n = m->nr_class * (m->nr_class - 1) / 2;
  m->rho = (double *) malloc(sizeof(double) * n);
  for (i = 0; i < n; i++)
    m->rho[i] = model->rho[i];
  if (model->probA)
  {
    m->probA = (double *) malloc(sizeof(double) * n);
    for (i = 0; i < n; i++)
      m->probA[i] = model->probA[i];
  }
  else
    m->probA = NULL;
  if (model->probB)
  {
    m->probB = (double *) malloc(sizeof(double) * n);
    for (i = 0; i < n; i++)
      m->probB[i] = model->probB[i];
  }
  else
    m->probB = NULL;

#if LIBSVM_VERSION >= 320
  if( model->sv_indices )
  {
    m->sv_indices = (int *) malloc( sizeof(int) * m->l );  // is it l ? (nSV)
    for( i=0; i<m->l; ++i )
      m->sv_indices[i] = model->sv_indices[i];
  }
  else
    m->sv_indices = NULL;
#endif

  m->label = (int *) malloc(sizeof(int) * m->nr_class);
  if (model->label)
  for (i = 0; i < m->nr_class; i++)
    m->label[i] = model->label[i];
  else  m->label = NULL;
  m->nSV = (int *) malloc(sizeof(int) * m->nr_class);
  if (model->nSV)
  for (i = 0; i < m->nr_class; i++)
    m->nSV[i] = model->nSV[i];
  else  m->nSV = NULL;
  m->free_sv = model->free_sv;

  return m;
}

/** should be static ? */
struct svm_parameter *svm_clone_parameter( const struct svm_parameter *param )
{
  struct svm_parameter  *p
    = (struct svm_parameter *) malloc(sizeof(struct svm_parameter));

  p->svm_type = param->svm_type;
  p->kernel_type = param->kernel_type;
  p->degree = param->degree;
  p->gamma = param->gamma;
  p->coef0 = param->coef0;
  p->cache_size = param->cache_size;
  p->eps = param->eps;
  p->C = param->C;
  //svm_load_model don't initialize p->*weight* parameters
  p->nr_weight = 0;
  p->weight_label = NULL;
  p->weight = NULL;
  /*
  p->nr_weight = param->nr_weight;
  p->weight_label = (int *) malloc(sizeof(int) * p->nr_weight);
  for (i = 0; i < p->nr_weight; i++)
    p->weight_label[i] = param->weight_label[i];
  p->weight = (double *) malloc(sizeof(double) * p->nr_weight);
  for (i = 0; i < p->nr_weight; i++)
    p->weight[i] = param->weight[i];
  */
  p->nu = param->nu;
  p->p = param->p;
  p->shrinking = param->shrinking;
  p->probability = param->probability;

  return p;
}

void svm_print_parameter( const struct svm_parameter *p )
{
  std::cout << "type = " << p->svm_type << std::endl;
  std::cout << "kernel_type = " << p->kernel_type << std::endl;
  std::cout << "degree = " << p->degree << std::endl;
  std::cout << "gamma = " << p->gamma << std::endl;
  std::cout << "coef0 = " << p->coef0 << std::endl;
  std::cout << "cache_size = " << p->cache_size << std::endl;
  std::cout << "eps = " << p->eps << std::endl;
  std::cout << "C = " << p->C << std::endl;
  std::cout << "nr_weight = " << p->nr_weight << std::endl;
  if (p->weight_label) for (int i = 0; i < p->nr_weight; ++i)
    std::cout << "weight_label[" << i << "] = "
      << p->weight_label[i] << std::endl;
  if (p->weight) for (int i = 0; i < p->nr_weight; ++i)
    std::cout << "weight[" << i << "] = "
      << p->weight[i] << std::endl;
}
