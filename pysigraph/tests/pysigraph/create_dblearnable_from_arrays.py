#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import numpy, os, pylab, sys, svm
from soma import aims
import sigraph.datamind_backend

from  datamind.ml import plugins
from  datamind.ml import reader
from six.moves import range

plugins.plugin_manager.load_plugin('Sigraph')


# read data
def read():
	#modelpath = '/home/Panabase/data_sulci_ccrt/base2000/models/R_fd2_svr_cliqueMemorizer_28_08_06/model'
	modelpath = '/home/revilyo/cea_data_backup/base2000/models/R_fd5_svm_cliqueMemorizer_28_07_06/model'
	filename = os.path.join(modelpath, 'adap', 'S.T.s._right_svm1.minf')
	db = reader.Reader('Sigraph').read(filename)
	train, test = db.filter('no-class0-duplication')
	return train

def fun(db):
	global glob
	X, Y, I = db.getX(), db.getY(), None
	db2 = sigraph.DBLearnable(X, Y, None)
	v = aims.vector_U32(list(range(db2.size())))
	view = sigraph.DBLearnableView(db2, v, None)

	return db2, view

def main():
	db = read()
	db2, view = fun(db)
	print("Y = ", view.getDB().arraydataY()[[0, 5], :])
	print("X = ", view.getDB().arraydataX()[[0,5], :])


if __name__ == "__main__": main()
