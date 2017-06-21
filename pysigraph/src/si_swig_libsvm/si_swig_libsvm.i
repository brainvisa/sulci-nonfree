%module _si_swig_libsvm
%{
#include <si/sisvm/libsvm.h>
%}

%import svmc.i

double  svm_predict_decision(const struct svm_model *model,
                             const struct svm_node *x);
