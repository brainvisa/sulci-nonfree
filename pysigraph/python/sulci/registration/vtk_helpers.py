#!/usr/bin/env python

import vtk, numpy


def gen_data_gmm(centers, covariances, n):
	import numpy
	X = []
	size = len(centers)
	for i in range(size):
		c, cov = numpy.asarray(centers[i]).T[0], covariances[i]
		X.append(numpy.random.multivariate_normal(c, cov, n))
	return numpy.asmatrix(numpy.vstack(X).T)


class VtkData(object): pass


class VtkOneData(VtkData):
	def __init__(self):
		self._vtk_init()

	def set_color(self, color):
		p = self._actor.GetProperty()
		p.SetColor(color[0], color[1], color[2])

	def get_actors(self): return [self._actor]

	def _vtk_init(self):
		data = vtk.vtkPolyData()
		data.SetVerts(vtk.vtkCellArray())
		data.SetLines(vtk.vtkCellArray())
		data.SetPolys(vtk.vtkCellArray())

		mapper = vtk.vtkPolyDataMapper()
		mapper.SetInput(data)
		actor = vtk.vtkActor()
		actor.SetMapper(mapper)
		self._data = data
		self._mapper = mapper
		self._actor = actor


class VtkSeveralData(VtkData):
	def __init__(self, n):
		self._vtk_init(n)

	def set_color(self, color):
		for a in self._actors:
			p = a.GetProperty()
			p.SetColor(color[0], color[1], color[2])

	def get_actors(self): return self._actors

	def _vtk_init(self, n):
		self._data = []
		self._mappers = []
		self._actors = []
		for i in range(n):
			data = vtk.vtkPolyData()
			data.SetVerts(vtk.vtkCellArray())
			data.SetLines(vtk.vtkCellArray())
			data.SetPolys(vtk.vtkCellArray())
			mapper = vtk.vtkPolyDataMapper()
			mapper.SetInput(data)
			actor = vtk.vtkActor()
			actor.SetMapper(mapper)

			self._data.append(data)
			self._mappers.append(mapper)
			self._actors.append(actor)


class PointsCloud(VtkOneData):
	def __init__(self, X):
		VtkOneData.__init__(self)
		self.set_X(X)

	def set_X(self, X):
		points = vtk.vtkPoints()
		for i in range(X.shape[1]):
			x = X[:, i]
			points.InsertNextPoint(x[0], x[1], x[2])
		id = vtk.vtkIdList()
		for i in range(X.shape[1]): id.InsertNextId(i)
		conn = vtk.vtkCellArray()
		conn.InsertNextCell(id)

		self._data.SetPoints(points)
		self._data.SetVerts(conn)

	def set_size(self, size):
		self._actor.GetProperty().SetPointSize(size)


class TwoCloudsLinks(VtkOneData):
	def __init__(self, X, Y):
		VtkOneData.__init__(self)
		self.set_XY(X, Y)

	def set_XY(self, X, Y):
		points = vtk.vtkPoints()
		for i in range(X.shape[1]):
			x, y = X[:, i], Y[:, i]
			points.InsertNextPoint(x[0], x[1], x[2])
			points.InsertNextPoint(y[0], y[1], y[2])
		conn = vtk.vtkCellArray()
		for i in range(X.shape[1]):
			conn.InsertNextCell(2)
			conn.InsertCellPoint(2 * i)
			conn.InsertCellPoint(2 * i + 1)

		self._data.SetPoints(points)
		self._data.SetLines(conn)

	def set_size(self, size):
		self._actor.GetProperty().SetLineWidth(size)


def create_arrow2D_split():
	line = vtk.vtkPolyData()
	line.SetVerts(vtk.vtkCellArray())
	line.SetLines(vtk.vtkCellArray())
	line.SetPolys(vtk.vtkCellArray())
	p = vtk.vtkPoints()
	p.InsertNextPoint(0, 0, 0)
	p.InsertNextPoint(1, 0, 0)
	conn = vtk.vtkCellArray()
	conn.InsertNextCell(2)
	conn.InsertCellPoint(0)
	conn.InsertCellPoint(1)
	line.SetPoints(p)
	line.SetLines(conn)
	
	pike = vtk.vtkPolyData()
	pike.SetVerts(vtk.vtkCellArray())
	pike.SetLines(vtk.vtkCellArray())
	pike.SetPolys(vtk.vtkCellArray())
	p = vtk.vtkPoints()
	p.InsertNextPoint(-0.2, -0.1, 0)
	p.InsertNextPoint(0, 0, 0)
	p.InsertNextPoint(-0.2, 0.1, 0)
	conn = vtk.vtkCellArray()
	conn.InsertNextCell(3)
	conn.InsertCellPoint(0)
	conn.InsertCellPoint(1)
	conn.InsertCellPoint(2)
	pike.SetPoints(p)
	pike.SetLines(conn)
	return line, pike


def create_arrow2D():
	arrow2D = vtk.vtkPolyData()
	arrow2D.SetVerts(vtk.vtkCellArray())
	arrow2D.SetLines(vtk.vtkCellArray())
	arrow2D.SetPolys(vtk.vtkCellArray())
	p = vtk.vtkPoints()
	p.InsertNextPoint(0, 0, 0)
	p.InsertNextPoint(1, 0, 0)
	p.InsertNextPoint(0.8, -0.1, 0)
	p.InsertNextPoint(1, 0, 0)
	p.InsertNextPoint(0.8, 0.1, 0)
	conn = vtk.vtkCellArray()
	conn.InsertNextCell(3)
	conn.InsertCellPoint(0)
	conn.InsertCellPoint(1)
	conn.InsertCellPoint(2)
	conn.InsertNextCell(2)
	conn.InsertCellPoint(3)
	conn.InsertCellPoint(4)
	arrow2D.SetPoints(p)
	arrow2D.SetLines(conn)
	return arrow2D



class TwoOrientedCloudsLinks(VtkSeveralData):
	def __init__(self, X, Y):
		VtkSeveralData.__init__(self, 2)
		self.set_XY(X, Y)

	def set_XY(self, X, Y):
		size = X.shape[1]
		line, pike = create_arrow2D_split()

		# vectors
		vectors = vtk.vtkFloatArray()
		vectors.SetNumberOfComponents(3)
		vectors.SetNumberOfTuples(size)
		for i in range(size):
			x, y = X[:, i], Y[:, i]
			d = y - x
			vectors.InsertTuple3(i, d[0], d[1], d[2])

		# line
		points = vtk.vtkPoints()
		for i in range(size):
			x = X[:, i]
			points.InsertNextPoint(x[0], x[1], x[2])
		self._data[0].SetPoints(points)
		self._data[0].GetPointData().SetVectors(vectors)

		glyph = vtk.vtkGlyph3D()
		glyph.ScalingOn()
		glyph.SetScaleModeToScaleByVector()
		glyph.SetInput(0, self._data[0])
		glyph.SetSource(0, line)
		self._mappers[0].SetInputConnection(glyph.GetOutputPort())

		# pike
		points = vtk.vtkPoints()
		for i in range(size):
			y = Y[:, i]
			points.InsertNextPoint(y[0], y[1], y[2])
		self._data[1].SetPoints(points)
		self._data[1].GetPointData().SetVectors(vectors)

		glyph = vtk.vtkGlyph3D()
		glyph.ScalingOff()
		glyph.SetInput(0, self._data[1])
		glyph.SetSource(0, pike)
		self._mappers[1].SetInputConnection(glyph.GetOutputPort())


class FuzzyGaussianLinks(VtkOneData):
	def __init__(self, X, centers, weights):
		VtkOneData.__init__(self)
		self.set(X, centers, weights)

	def set(self, X, centers, weights):
		size = X.shape[1]

		# weights
		scalars = vtk.vtkFloatArray()
		scalars.SetNumberOfComponents(1)
		scalars.SetNumberOfTuples(size * len(centers) * 2)
		j = 0
		for w in weights:
			for i in range(size):
				scalars.InsertTuple1(j, w[:, i])
				scalars.InsertTuple1(j + 1, w[:, i])
				j += 2

		# line
		points = vtk.vtkPoints()
		for c in centers:
			for i in range(size):
				x = X[:, i]
				points.InsertNextPoint(x[0], x[1], x[2])
				points.InsertNextPoint(c[0], c[1], c[2])
		conn = vtk.vtkCellArray()
		for i in range(size * len(centers)):
			conn.InsertNextCell(2)
			conn.InsertCellPoint(2 * i)
			conn.InsertCellPoint(2 * i + 1)
		self._data.SetPoints(points)
		self._data.SetLines(conn)
		self._data.GetPointData().SetScalars(scalars)

		lut = self._mapper.GetLookupTable()
        	lut.SetTableRange(0., 1.);
        	lut.SetValueRange(0, 1);
		lut.SetSaturationRange(0., 0.)
		lut.SetRampToLinear()
		lut.SetAlphaRange(0., 1.)
        	lut.Build()
		self._mapper.SetScalarVisibility(1)
		self._mapper.SetColorModeToMapScalars()
		self._mapper.SetColorMode(1)


class Vector(VtkOneData):
	def __init__(self, pos, dir):
		VtkOneData.__init__(self)
		self.set(pos, dir)

	def set(self, pos, dir):
		if numpy.linalg.norm(dir) == 0: return
		points = vtk.vtkPoints()
		points.InsertNextPoint(pos[0], pos[1], pos[2])
		self._data.SetPoints(points)
		vectors = vtk.vtkFloatArray()
		vectors.SetNumberOfComponents(3)
		vectors.SetNumberOfTuples(1)
		vectors.InsertTuple3(0, dir[0], dir[1], dir[2])
		self._data.GetPointData().SetVectors(vectors)
		glyph = vtk.vtkGlyph3D()
		glyph.ScalingOn()
		glyph.SetScaleModeToScaleByVector()
		glyph.SetInput(0, self._data)
		glyph.SetSource(0, create_arrow2D())
		self._mapper.SetInputConnection(glyph.GetOutputPort())

	def set_size(self, size):
		self._actor.GetProperty().SetLineWidth(size)


class VtkPlot(object):
	def __init__(self, sizex, sizey):
		renderer = vtk.vtkRenderer()
		renderWindow = vtk.vtkRenderWindow()
		renderWindow.SetSize(sizex, sizey)
		renderWindow.AddRenderer(renderer)
		iren = vtk.vtkRenderWindowInteractor()
		iren.SetRenderWindow(renderWindow)
		style = vtk.vtkInteractorStyleTrackballCamera()
		iren.SetInteractorStyle(style)
		self._renderer = renderer
		self._window = renderWindow
		self._window_interactor = iren

	def set_bgcolor(self, color):
		self._renderer.SetBackground(color[0], color[1], color[2])

	def plot(self, objects):
		for o in objects:
			for a in o.get_actors(): self._renderer.AddActor(a)

	def render(self):
		self._window.Render()

	def show(self):
		self._window.Render()
		self._window_interactor.Start()
