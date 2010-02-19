
from neuroProcesses import *
import neuroConfig
import sys, os

signature = Signature(
  'input_graphs', ListOf( ReadDiskItem( 'Cortical Folds Graph', 'Graph and data' ) ),
  'model_graph', ReadDiskItem( 'Model Graph', 'Graph' ),
  'output_stats', WriteDiskItem( 'Text File', 'Text File' ),
)

def initialization( self ):
  pass

def execution( self, context ):
  pth = os.path.join( neuroConfig.mainPath, 'scripts', 'sigraph',
    'sulci_morphodetect' )
  cleanpath = False
  if pth not in sys.path:
    sys.path.insert( 0, pth )
    cleanpath = True
  from sulciRecordStats import sulciRecordStats
  sulciRecordStats( [ x.fullPath() for x in self.input_graphs ],
    self.model_graph.fullPath(), self.output_stats.fullPath() )
  if cleanpath:
    del sys.path[0]
