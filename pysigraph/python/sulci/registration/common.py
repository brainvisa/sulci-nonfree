#!/usr/bin/env python

import numpy
from soma import aims

################################################################################
def transformation_to_motion(R, t):
	m = numpy.identity(4)
	m[0:3, 3] = t.T
	m[:3, :3] = R
	motion = aims.Motion(m.flatten())
	return motion

def save_transformation(filename, R, t):
	motion = transformation_to_motion(R, t)
	aims.Writer().write(motion, filename)
