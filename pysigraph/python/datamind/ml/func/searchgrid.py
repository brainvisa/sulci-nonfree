from __future__ import absolute_import
from __future__ import print_function
import numpy as N
from six.moves import range


class SearchGrid(object):

    """
    Search grid function : scan a space defined by a grid, and evaluate a function with the values of parameters

    Input :
            func : function with eval and setParams functions
            grid : a grid with a set of parameters for each point of the grid

    Output :
            the eval function get (X,Y) and eval function(X,Y) for each point of the grid, by setting the function with the set of parameters define by the grid.

    ex :
            scan from 1 to 5 with a step of 1 for the dimension 1, and from 0 to 100 with a log step of 1 : we the have the grid :


            array([[[    1.,     1.],
            [    1.,    10.],
            [    1.,   100.],
            [    1.,  1000.]],

    [[    2.,     1.],
            [    2.,    10.],
            [    2.,   100.],
            [    2.,  1000.]],

    [[    3.,     1.],
            [    3.,    10.],
            [    3.,   100.],
            [    3.,  1000.]],

    [[    4.,     1.],
            [    4.,    10.],
            [    4.,   100.],
            [    4.,  1000.]],

    [[    5.,     1.],
            [    5.,    10.],
            [    5.,   100.],
            [    5.,  1000.]]])


            We coudl create this grid by using the create_grid function, with this vecotr in input :
            array([['1', '5', '1', 'linear'],
                    ['0', '1000', '1', 'log']],
                    dtype='|S6')


    Examples :
            import numpy as N
            from datamind.ml.func import *
            X = N.asarray(range(0,4))
            Y = N.array([1])

            limits = N.array([1,5,1,"linear",0,1000,1,"log"])
            limits = N.reshape(limits,[2,4])
            func  = Testfunction()
            s = SearchGrid(func)
            s.create_grid(limits)
            grid = s.getGrid()
            print(grid)
            results = s.eval(X,Y)
            print(results)


    """

    def __init__(self, func, verbose=False):
        self.func = func
        self.verbose = verbose

    def setGrid(self, grid):
        self.grid = grid

    def setParams(self, grid):
        self.grid = grid

    def getGrid(self):
        return self.grid

    def eval(self, X, Y):
        func = self.func
        grid = self.grid

        shape_grid = N.shape(grid)
        shape_grid = shape_grid[0:N.size(shape_grid) - 1]
        total_size = N.prod(shape_grid)
        nb_params = N.shape(grid)[N.size(shape_grid)]
        grid_ravel = N.reshape(grid, [total_size, nb_params])
        results_ravel = N.zeros(total_size)
        for i in range(total_size):
            params = N.reshape(grid_ravel[i, :], [nb_params])
            func.setParams(params)
            results_ravel[i] = func.eval(X, Y)
            if self.verbose == True:
                print(i, params, results_ravel[i])
        results = N.reshape(results_ravel, shape_grid)
        return results

    def create_grid(self, limits):
        '''
        Create the grid with an array :
        the array in input must have the following shape

        Number of dimensions to scan * Parameters for the dimension (size = 4)

        with :
        parameters for dimension = [min,max,step,"log" or "linear"]

        getGrid() to get the grid
        '''

        size_grid = N.zeros(N.size(limits, 0))
        for i in range(N.size(size_grid)):
            min_i = N.float(limits[i, 0])
            max_i = N.float(limits[i, 1])
            step_i = N.float(limits[i, 2])
            if limits[i, 3] == "linear":
                size_grid[i] = N.int((max_i - min_i) / step_i) + 1
            if limits[i, 3] == "log":
                size_grid[i] = N.int(N.log10(max_i - min_i) / step_i) + 1

        grid = []
        step = N.zeros(N.size(size_grid))
        for i in range(N.size(N.zeros(size_grid))):
            value_i = N.zeros(N.size(size_grid))
            for j in range(N.size(size_grid)):
                if limits[j, 3] == "linear":
                    value_i[j] = N.float(limits[j, 0]) + step[
                        j] * N.float(limits[j, 2])
                if limits[j, 3] == "log":
                    value_i[j] = N.float(limits[j, 0]) + (
                        10**(step[j] * N.float(limits[j, 2])))
            grid.append(value_i)
            step[N.size(size_grid) - 1] = step[N.size(size_grid) - 1] + 1
            if i != N.size(N.zeros(size_grid)) - 1:
                for j in range(N.size(size_grid)):
                    if step[N.size(size_grid) - 1 - j] >= size_grid[N.size(size_grid) - 1 - j]:
                        step[N.size(size_grid) - 1 - j] = 0
                        step[N.size(size_grid) - 1 - j - 1] = step[
                            N.size(size_grid) - 1 - j - 1] + 1

        shape_grid = N.hstack((size_grid, N.size(limits, 0)))
        grid = N.reshape(grid, shape_grid)
        self.grid = grid
