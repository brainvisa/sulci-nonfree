# Copyright CEA and IFR 49 (2000-2005)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ and IFR 49
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

from neuroProcesses import *
name = 'SPAM recognition'
userLevel = 0
import registration


signature = Signature(
    'data_graph', ReadDiskItem( 'Data graph', 'Graph' ), 
    'model', ReadDiskItem( 'Bayesian Model', 'Text Data Table' ),
    'output_graph',
    WriteDiskItem( 'Labelled Cortical folds graph', 'Graph', 
                   requiredAttributes = { 'labelled' : 'Yes',
                                          'automatically_labelled' \
                                          : 'Yes'} ),
    'posterior_probabilities',
        WriteDiskItem( 'Bayesian Recognition Posterior Probabilities',
            'CSV file' ),
    'labels_translation_map',
    ReadDiskItem( 'Label Translation',
                  [ 'Label Translation', 'DEF Label translation' ] ),
    'priors', ReadDiskItem( 'Bayesian Recognition Priors', 'Text Data Table' ),
    )

def initialization( self ):
    self.addLink( 'model', 'data_graph' )
    self.linkParameters( 'output_graph', 'data_graph' )
    self.linkParameters( 'posterior_probabilities', 'output_graph' )
    self.linkParameters( 'priors', 'model' )
    self.labels_translation_map = \
      self.signature[ 'labels_translation_map' ].findValue(
        { 'filename_variable' : 'sulci_model_2008' } )

def execution( self, context ):
    tmpfile = context.temporary( 'Text file' )
    # find script filename (since it is not in the PATH)
    progname = os.path.join( os.path.dirname( os.path.dirname( \
      registration.__file__ ) ), 'scripts', 'sigraph', 'sulci_registration',
      'independent_tag_with_registration.py' )
    # bidouille to retreive graphmodel filename as a hidden parameter
    modname = self.model.fullName()
    gm1 = len( os.path.dirname( modname ) ) + 1
    if os.path.basename( modname ).startswith( 'bayesian_' ):
      gm1 += 9
    side = self.data_graph.get( 'side', None )
    if side and modname.endswith( '_' + side ):
      gm2 = len( modname ) - len( side ) - 1
    else:
      gm2 = len( modname )
    graphmodel = modname[:gm1] + 'graphmodel' + modname[gm2:] + '.dat'
    # now run it
    context.system( sys.executable, progname, '-i', self.data_graph, '-o',
      self.output_graph, '-t', self.labels_translation_map, '-d', self.model,
      '-m', graphmodel, '--maxiter', 0, '--mode', 'global', '-c',
      self.posterior_probabilities, '-l', tmpfile, '-p', self.priors )
    trManager = registration.getTransformationManager()
    trManager.copyReferential( self.data_graph, self.output_graph )

