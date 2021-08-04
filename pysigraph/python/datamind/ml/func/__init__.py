from .loss import zeroOne_err, zeroOne_acc, ms_err, explained_var, weighted_classif_rate, sens_spec_rate

from .glm import OneWayAnovaFstat, SimpleLinRegFstat,\
    OneWayAnovaEffect, SimpleLinRegEffect,\
    TwoSamplesTtest,\
    Multivariate_glm, ManovaLDA, Manova_RSS
# from clf_eval import mse

from .bootstrap import Bootstrap, BootstrapOneWayAnovaFstat,\
    BootstrapOneWayAnovaEffect, BootstrapRocAuc

from .svmBasedFuncsAndBounds import SvmLinWeights

from .MutualInformation import *
from .permutations import *
from .testfunction import *
from .searchgrid import *
from .experimental import GrpScattering
from .roc import RocAuc
from .wilcoxon import Wilcoxon
from .activation import Activation, Activation_Anova
from .ProbabilityMisclassif import ProbabilityErrorSubset
from .misc import identity, Identity
from .friedman import Friedman
from .post_hoc import *
