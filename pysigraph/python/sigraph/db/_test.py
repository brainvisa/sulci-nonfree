#!/usr/bin/env python

from __future__ import print_function
from sigraph import *
from numpy import *

def test_optimizer():
	db = None
	m = model.Model()
	m.centers = array([0., 5., 10.])
	m.nvalues = array([3, 3, 3])
	m.step = array([1, 20, 100])
	mo = model.optimizers.grid(m)
	mo.optimize(db)

def test_view():
	db = dbLearnable()
	db._X = array([[0], [1], [2], [3], [4], [5], [6], [7]])
	db._Y = array([[0], [10], [20], [30], [40], [50], [60], [70]])
	db._INF = array([['a'], ['b'], ['c'], ['d'], ['e'], ['f'], ['g'],['h']])
	view = db.views.View(db, array([1, 2, 3, 4, 5, 6]))
	view2 = db.views.View(view, array([3, 5]))
	print("-----------")
	for i in range(db._X.shape[0]):
		print(db[i])
	print("-----------")
	for v in db:
		print(v)
	print("-----------")
	for v in view:
		print(v)
	print("-----------")
	for v in view2:
		print(v)


def main():
	print("-- main --")
	test_optimizer()
	test_view()

if __name__ == "__main__" : main()
