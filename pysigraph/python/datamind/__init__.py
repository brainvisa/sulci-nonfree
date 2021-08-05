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
__doc__ = '''
    This package is obsolete but is still used in sulci SPAM models tools.
    It was formerly the datamind project, which is obsolete and unmaintained.
    Thus we moved the used (? at least loaded) code here and left the rest
    in the former datamind project which should not be used anywhere.

    core :  ?
    data :  Data generator Module : small toysets usefull for tests, playing
            with machine learning methods and demo.
    gui :   ?
    image : ?
    io :    ?
    ml :    Machine Learning Module :
            - classifiers, regressors, feature selection, clustering ?
            - optimizers
            - databases, readers
            - ...and one day : some plot helpers ?
    tools : - Color messages and standard shaped messages (warning, error, info)
            - ASCII (text) Progression Bar to waste your time waiting your
            programs finishing or converging if its possible.


    datamind.Settings : Settings class
            - Main customizable information should be there
'''


class Settings(object):

    '''
plugins :    default list of datamind.ml plugins loaded when datamind.ml
             is imported.
debug :      boolean qualifying debug status.
    '''
    # List of plugins to be loaded
    plugins = ['numpy']
    # If debug value is True, it could help you to debug unloadable plugins.
    debug = False
