#include <Python.h>
#include <stdlib.h>
#include <stdio.h>

static PyObject *init_wrapper(PyObject *self, PyObject *null) {
	printf("init libsvm python\n");
	return NULL;
}


static PyMethodDef methods[] = {
	{"init", init_wrapper, METH_NOARGS, "init in python"},
	{NULL, NULL, 0, NULL}
};


void	init_si_libsvm(void)
{
	Py_InitModule("_si_libsvm", methods);
}
