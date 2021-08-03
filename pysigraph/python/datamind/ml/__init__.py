# Copyright CEA (2000-2006)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
#
# This software is governed by the CeCILL license version 2 under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.

from __future__ import print_function

from __future__ import absolute_import
__doc__ = '''
    Machine Learning Module

    wip :          work in progress helpers message
    plugins :      plugin directories and plugin manager
    database :     database, selectors, views, iterators
    reader :       reader for databases
    model :        generic object for classifiers, regressors, dimension
                   reduction techniques, clustering
    classifier :   classifiers/regressors, results, ofuncs, optimizers
    validation :   crossvalidation, fit(train)/predict(test) scheme...
    dimreduction : dimension reduction techniques (feature selection, tests,
                   embeddings...)
    scaling :      scale a database.
    explorer :     methods to cover hyper-parameters space.

    mlObject :     spurious object untill now.
    obsolete :     oldies : not used but could help designing our Api
                   ...wait and it will disappear.'''


# from . import wip
from .mlObject import *
from . import plugins
from . import database, classifier, dimreduction
from . import classif, resampling, dimred


def test_import(module):
    '''
Try to import a module and return True if it was a success or
False otherwise.

module : name of the module.'''
    try:
        __import__(module)
        del module
        return True
    except:
        return False


def test_all_imports(modules):
    '''
Try to import a list of modules. Program exit with code 1 when importing
errors occurs showing a reporting message with the list of concerned
modules.

modules : list of modules' name.'''
    errors_dict = {}
    error = False
    for m in modules:
        errors_dict[m] = test_import(m)
    error = sum(errors_dict.values()) != len(errors_dict)
    if error:
        print(" * error : can't load or find these modules :")
        print([m for m, e in errors_dict.items() if not e])
        import sys
        sys.exit(1)


def exit(exit_status=0):
    '''
Call this function to cleanly exit datamind.ml and its plugins.'''
    import datamind.ml.plugins
    for p in datamind.ml.plugins.plugin_manager.loaded().values():
        p.quit()
    import sys
    sys.exit(exit_status)

mandatory_modules = ['numpy']
test_all_imports(mandatory_modules)

# read plugins from plugins directory
plugins.init()
# load plugins according to settings
plugins.plugin_manager.load()
