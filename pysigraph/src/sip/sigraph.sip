%Import aims.sip

%Module sigraph.sigraphsip
%ModuleCode
#include <pyaims/init.h>
%End

%PostInitialisationCode
carto::pyaims_init();
%End

%Include si.sip
%Include clique.sip
%Include learnparam.sip
%Include set_AdaptivePtr.sip
%Include set_CliquePtr.sip
%Include model.sip
%Include subadaptive.sip
%Include learner.sip
%Include finder.sip
%Include learnable.sip
%Include cliquedescr.sip
%Include cgraph.sip
%Include mgraph.sip
%Include trainer.sip
%Include anneal.sip
%Include optimizer.sip
%Include dimreductor.sip
%Include translator.sip
%Include fold.sip
