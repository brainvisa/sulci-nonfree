%Import aims.sip

%Module sigraph.sigraphsip

%ModuleCode
#include <pyaims/init.h>
%End

%ModuleHeaderCode
#include <pyaims/sipnewtypedefs.h>
%End

%PostInitialisationCode
carto::pyaims_init();
%End

%Include si.sip
%Include clique.sip
%Include rcptr_Clique.sip
%Include set_CliquePtr.sip
%Include learnparam.sip
%Include set_AdaptivePtr.sip
%Include set_rc_ptr_Clique.sip
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
