#!/usr/bin/env python
from __future__ import print_function
import sigraph

r = sigraph.MReader("data/model.mod")
w = sigraph.MWriter("/tmp/model.mod", True)
m = r.readModel()
del r
ad = m.model()
sa = ad.workEl()
dr = ad.dimreductor()
opt = ad.optimizer()
opt_param = opt.getParameters()
opt_strategy = opt.getStrategy()

# printing
print("labels = ", m.significantLabels())
print("subadaptive name =", sa.name())
print("dimreductor =", dr.getSelected(), dr.getShape(), dr.getMatrix())
print("grid opt = ", opt_strategy)
print("grid parameters = ")
for p in opt_param:
	sp = opt_param[p]
	gp = sigraph.GridOptimizerParameter.fromObject(sp.get())
	print("\t", gp.getName(), ":", gp.getRanges(), gp.getScale())
#print("opt_param = ", opt_param)


w.write(m)
print("cat /tmp/model.mod")
