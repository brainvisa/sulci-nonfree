#include <Python.h>
#include <stdlib.h>
#include <stdio.h>

static PyObject *init_wrapper(PyObject *self, PyObject *null)
{
  (void)(self); /* compilation warning... */
  (void)(null);
  printf("init libsvm python\n");
  return NULL;
}


static PyMethodDef methods[] = {
  {"init", init_wrapper, METH_NOARGS, "init in python"},
  {NULL, NULL, 0, NULL}
};


static struct PyModuleDef cModSiSvm =
{
  PyModuleDef_HEAD_INIT,
  "_si_svm", /* name of module */
  "",          /* module documentation, may be NULL */
  -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
  methods
};


PyMODINIT_FUNC PyInit__si_libsvm(void)
{
  return PyModule_Create(&cModSiSvm);
}
