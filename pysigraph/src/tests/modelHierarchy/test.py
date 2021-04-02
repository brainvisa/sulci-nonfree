#!/usr/bin/env python

from __future__ import print_function
from soma import aims
from sigraph import *

def example():
	'''
    Open a modelFile and create a hierarchy of models.'''

	def cover(model, fun):
		'''
    Cover all vertices/edges of graph calling fun function for each.

    model :  graph model.
    fun :    function with prototype f(el, is_vertex) with el the current
             element and is_vertex a boolean stressing element status (vertex
             or edge).'''
		# get vertices / edges of model graph
		vertices = model.vertices()
		edges = model.edges()
		# cover all models
		for v in vertices:
			fun(v, True)
		for e in edges:
			fun(e, False)

	def display(el, is_vertex):
		'''
    Function called for each elements of model graph.

    el :         current graph element.
    is_vertex :  boolean, True if element is a vertex, False if element
                 is an edge.'''
		def get_string(el, name):
			'''Get string from element.'''
			return el[name].get().getString()
		print("----")
		print("  model file :  '%s'" % get_string(el, 'model_file'))
		if is_vertex:
			print("  domain file : '%s'" % \
				get_string(el, 'domain_file'))
			print("  label :       '%s'" % get_string(el, 'label'))
		else:
			print("  label1 :      '%s'" % get_string(el,'label1'))
			print( "  label2 :      '%s'" % get_string(el,'label2'))
		print("  occurence :   '%s'" % \
			get_string(el, 'occurence_count'))
		# get model/adaptive instance
		mod = Model.fromObject(el['model'].get())

	# read
	modelfile = 'model.arg'
	r = aims.Reader()
	model = r.read(modelfile)

	# cover all models and print info
	cover(model, display)

def main():
	example()

if __name__ == '__main__' : main()
