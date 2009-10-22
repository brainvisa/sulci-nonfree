
from neuroProcesses import *

signature = Signature(
  'input_graphs', ListOf( ReadDiskItem( 'Cortical Folds Graph', 'Graph and data' ) ),
  'model_graph', ReadDiskItem( 'Model Graph', 'Model Graph' ),
  'output_stats', WriteDiskItem( 'Text File', 'Text File' ),
)

def initialization( self ):
  pass

def execution( self, context ):
  import sulciRecordStats
  sulciRecordStats( [ x.fullPath() for x in self.input_graphs ],
    self.model_graph.fullPath(), self.output_stats.fullPath() )

