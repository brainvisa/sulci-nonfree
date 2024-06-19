#include <pysigraph/pysigraph.h>
#include <pyaims/numpyarray.h>
#include <iostream>
#include <string.h>

namespace sigraph
{

  static PyObject *my_getitem(char *ip, PyArrayObject *)
  {
#if PY_VERSION_HEX >= 0x03000000
    return PyUnicode_FromString(*(char **)ip);
#else
    return PyString_FromString(*(char **)ip);
#endif
  }

  static int my_setitem(PyObject *op, char *ov, PyArrayObject *)
  {
    char *ptr;
    char **ov2 = (char **) ov;

    Py_ssize_t len;
    PyObject *temp = PyObject_Str(op);

    if (temp == NULL) return -1;

#if PY_VERSION_HEX >= 0x03000000
    ptr = const_cast< char* >( PyUnicode_AsUTF8AndSize( temp, &len ) );
    if( !ptr )
#else
    if (PyString_AsStringAndSize(temp, &ptr, &len) == -1)
#endif
    {
      Py_DECREF(temp);
      return -1;
    }
    *ov2 = strdup(ptr);
    Py_DECREF(temp);
    return 0;
  }

  PyArray_Descr	*NewDBLearnableInfoPyArrayDescr(void)
  {
    PyArray_Descr	*descr = NULL;
    descr = PyArray_DescrNewFromType(NPY_VOID);

#if NPY_ABI_VERSION < 0x02000000
    descr->f->getitem = (PyArray_GetItemFunc *) my_getitem;
    descr->f->setitem = (PyArray_SetItemFunc *) my_setitem;
#else
    PyDataType_GetArrFuncs(descr)->getitem = reinterpret_cast<PyObject * (*)(void *, void *)>( my_getitem );
    PyDataType_GetArrFuncs(descr)->setitem = reinterpret_cast<int (*)(PyObject *, void *, void *)>( my_setitem );
#endif
    PyDataType_SET_ELSIZE( descr, sizeof(char *) );
    //PyArray_RegisterDataType(descr);
    //FIXME : marche pas
    std::cerr << "error : NewDBLearnableInfoPyArrayDescr doesn't work!"
            << std::endl;
    exit(1);

    return descr;
  }

}
