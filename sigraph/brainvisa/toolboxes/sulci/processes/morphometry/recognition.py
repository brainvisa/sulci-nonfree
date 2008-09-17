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
name = 'Automatic recognition'
userLevel = 0
import registration


signature = Signature(
    'data_graph', ReadDiskItem( 'Data graph', 'Graph' ), 
    'model', ReadDiskItem( 'Model graph', 'Graph' ),
    'output_graph',
    WriteDiskItem( 'Labelled Cortical folds graph', 'Graph', 
                   requiredAttributes = { 'labelled' : 'Yes',
                                          'automatically_labelled' \
                                          : 'Yes'} ),
    'energy_plot_file', WriteDiskItem( 'siRelax Fold Energy',
                                       'siRelax Fold Energy' ),
    'rate', Float(),
    'stopRate', Float(),
    'niterBelowStopProp', Integer(),
    )

def modelValue(self, data_graph):
  """
  the input graph which is linked to the model has not necessarily the same graph_version
  but this attribute is a key for Graph data, so it will be seen as a required attribute, so it is removed from the link information
  """
  val=None
  if data_graph:
    val=data_graph.attributes()
    del val['graph_version']
  return val
  
def initialization( self ):
    self.addLink( 'model', 'data_graph', self.modelValue )
    self.linkParameters( 'output_graph', 'data_graph' )
    self.linkParameters( 'energy_plot_file', 'output_graph' )
    for p in ['rate', 'stopRate', 'niterBelowStopProp']:
        self.signature[p].userLevel = 2
    self.rate = 0.98
    self.stopRate = 0.05
    self.niterBelowStopProp = 1
    self.parent = {}
    self.parent['manage_tasks'] = False

def getConfigFile(self, context, graphname):
    def exist(file):
        try : os.stat(file)
        except OSError : return False
        else: return True
    dir = self.parent['self'].parallel_config_directory.fullPath()
    cfgfile = os.path.join(dir, 'siRelax-' + graphname)
    if not exist(cfgfile + '.cfg'): return cfgfile + '.cfg'
    n = 0
    while exist(cfgfile + '-' + str(n) + '.cfg'): n += 1
    return cfgfile + '-' + str(n) + '.cfg'

def cmd_duch(self, batchfile, cfgfile):
    import os
    log = os.path.join(dir, batchfile + '.log')
    cmd = '"cd ' + dir + " && nice siRelax " + cfgfile + " > " + log + '"\n'
    return cmd


def execution( self, context ):
    context.write( "Automatic recognition" )
    graphname = self.data_graph.get('subject')
    if not self.parent['manage_tasks']:
        progname = 'siRelax'
        cfgfile = context.temporary( 'Config file' ).fullPath()
    else:
        package = self.parent['self'].package
        if package == 'default':
            progname = distutils.spawn.find_executable('siRelax')
        else:
            progname = os.path.join(self.parent['package_dir'], package,
                                    'bin', 'siRelax')
        cfgfile = self.getConfigFile(context, graphname)
    context.write( 'config : ', cfgfile)
    try:
        stream = open(cfgfile, 'w' )
    except IOError, (errno, strerror):
        error(strerror, maker.output)
    else:
        stream.write( '*BEGIN TREE 1.0 siRelax\n' )
        stream.write( 'modelFile ' + self.model.fullPath() + '\n' )
        stream.write( 'graphFile ' + self.data_graph.fullPath() + '\n' )
        stream.write( 'output ' + self.output_graph.fullPath() + '\n' )
        stream.write( 'plotfile ' + self.energy_plot_file.fullPath() \
                          + '\n' )
        stream.write( 'rate ' + str( self.rate ) + '\n' )
        stream.write( 'stopRate ' + str( self.stopRate ) + '\n' )
        stream.write( 'niterBelowStopProp ' + str( self.niterBelowStopProp ) \
                      + '\n' )
        stream.write( 'extensionMode CONNECT_VOID CONNECT\n' )
        stream.write( '*END\n' )
        stream.close()
        f = open(cfgfile)
        context.log( 'siRelax input file', html=f.read() )
        f.close()
    if self.parent['manage_tasks']:
        self.parent['tasks'].append(progname + ' ' + cfgfile)
        self.parent['file'] = cfgfile
    else:
        context.system(progname, cfgfile)
        trManager = registration.getTransformationManager()
        trManager.copyReferential( self.data_graph, self.output_graph )
