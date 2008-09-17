TEMPLATE= subdirs
SUBDIRS   =     lib

#!include ../config-local

module(sisvm):SUBDIRS += libsigraphsvm

darwin-module(sisvm):PSUBDIRS	+= bundle_sisvm
         
PSUBDIRS = readFolds \
          sigraph-config \
          siChangeModels \
          siCopyNames \
          siDivNamelist \
          siDomTrain \
          siEnergy \
          siError \
          siErrorStats \
          siFlip \
          siGraph2Label \
          siLearn \
          siLyxErrors \
          siMakeModel \
          siMakeParcellationHierarchy \
          siMakeColoredHierarchy  \
          siMergeModels \
          siMeshSulciProjection \
          siMeshSulciOperture \
          siMkModelEdges \
          siMorpho \
          siMultilabelCompare \
          siParcellation\
          siPIDcommand \
          siPutTextureInModel \
          siRelax \
          siRevert \
	  siSulcalParcellation \
          siTestModels \
          siTriangModel \
          siTryGauss \
          siFunctionalGraphs
