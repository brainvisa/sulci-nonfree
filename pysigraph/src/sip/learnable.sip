
namespace sigraph
{

class Learnable
{
%TypeHeaderCode
#include <si/learnable/learnable.h>
%End

public:
  virtual ~Learnable();

protected:
  Learnable();
};


class VectorLearnable /Abstract/
{
%TypeHeaderCode
#include <si/learnable/vectorLearnable.h>
#ifndef PYAIMSSIP_VECTOR_DOUBLE_DEFINED
#define PYAIMSSIP_VECTOR_DOUBLE_DEFINED
typedef std::vector<double> vector_DOUBLE;
#endif
%End

%TypeCode
#if SIP_VERSION < 0x040700
#include "sipsigraphsipsigraphVectorLearnable.h"
#endif
#include <pysigraph/learnable.h>
%End

public:
  virtual ~VectorLearnable();
};


class SiDBLearnable
{
%TypeHeaderCode
#define PY_ARRAY_TYPES_PREFIX PyAIMSTypes
#include <pyaims/numpyarray.h>
#include <pysigraph/pysigraph.h>
#include <si/learnable/dbLearnable.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_FLOAT.h"
#endif
#include <iostream>

%End

public:
  SiDBLearnable();
  ~SiDBLearnable();

  int size() const;
  int getXcolsNumber() const;
  int getYcolsNumber() const;
  int getINFcolsNumber() const;

  void setSplit(int);
  void setCycles(int);
  int getSplit();
  int getCycles();

  void setLabels(vector_STRING &);

  SIP_PYOBJECT getX() /Factory/;
%MethodCode
  npy_intp *dims = (npy_intp *) malloc(sizeof(npy_intp) * 2);
  dims[1] = sipCpp->getXcolsNumber();
  dims[0] = sipCpp->size();
  sipRes = PyArray_SimpleNewFromData( 2, dims, NPY_DOUBLE,
                                    (char *) sipCpp->getX() );
  if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

  SIP_PYOBJECT getY() /Factory/;
%MethodCode
  npy_intp *dims = (npy_intp *) malloc(sizeof(npy_intp) * 2);
  dims[1] = sipCpp->getYcolsNumber();
  dims[0] = sipCpp->size();
  sipRes = PyArray_SimpleNewFromData( 2, dims, NPY_DOUBLE,
                                    (char *) sipCpp->getY() );
  if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

  SIP_PYOBJECT getINF() /Factory/;
%MethodCode
  PyArray_Descr   *descr = (PyArray_Descr*) malloc(sizeof(PyArray_Descr));
  memcpy(descr, PyArray_DescrFromType(NPY_STRING), sizeof(PyArray_Descr));
  PyDataType_SET_ELSIZE( descr, 32 );

  npy_intp *dims = (npy_intp *) malloc(sizeof(npy_intp) * 2);
  dims[0] = sipCpp->size();
  dims[1] = sipCpp->getINFcolsNumber();
  sipRes = PyArray_NewFromDescr(descr->typeobj, descr, 2, dims, 0,
                                (char *) sipCpp->getINF(), 0, 0 );
  if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

  SiDBLearnable(SIP_PYOBJECT /Transfer/, SIP_PYOBJECT /Transfer/,
	SIP_PYOBJECT /Transfer/);
%MethodCode
  PyArrayObject *arr0 = NULL;
  PyArrayObject *arr1 = NULL;
  PyArrayObject *arr2 = NULL;

  bool	a0isnone = (a0 == Py_None);
  bool	a1isnone = (a1 == Py_None);
  bool	a2isnone = (a2 == Py_None);

  if ((!a0isnone and !PyArray_Check(a0)) or
     (!a1isnone and !PyArray_Check(a1)) or
     (!a2isnone and!PyArray_Check(a2)))
  {
    sipIsErr = 1;
    PyErr_SetString(PyExc_TypeError, "wrong argument type");
  }
  else
  {
      if (!a0isnone) arr0 = (PyArrayObject *) a0;
      if (!a1isnone) arr1 = (PyArrayObject *) a1;
      if (!a2isnone) arr2 = (PyArrayObject *) a2;
      if ((!a0isnone and !PyArray_ISCONTIGUOUS(arr0)) or
          (!a1isnone and !PyArray_ISCONTIGUOUS(arr1)) or
          (!a2isnone and !PyArray_ISCONTIGUOUS(arr2)))
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError,
                         "Building a DBLearnable from a non-contiguous array "
                         "is not supported" );
      }
      else if ((!a0isnone and PyArray_NDIM( arr0 ) != 2) or
               (!a1isnone and PyArray_NDIM( arr1 ) != 2) or
	       (!a2isnone and PyArray_NDIM( arr2 ) != 2))
      {
         sipIsErr = 1;
         PyErr_SetString( PyExc_RuntimeError,
                       "Array dimensions are not compatible with DBLearnable" );
      }
      else if( (!a0isnone and PyArray_DESCR( arr0 )->type_num != NPY_DOUBLE) ||
               (!a1isnone and PyArray_DESCR( arr1 )->type_num != NPY_DOUBLE) ||
               (!a2isnone and PyArray_DESCR( arr2 )->type_num != NPY_STRING) )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_TypeError, "wrong array data type" );
      }
  }
  if( !sipIsErr )
  {
    double		*X = NULL;
    double		*Y = NULL;
    char		*I = NULL;

    if (!a0isnone) X = (double *) PyArray_DATA( arr0 );
    if (!a1isnone) Y = (double *) PyArray_DATA( arr1 );
    if (!a2isnone) I = (char *) PyArray_DATA( arr2 );

    std::vector<int>	dims(4);
    if (!a0isnone) dims[0] = PyArray_DIMS( arr0 )[1]; //cols_x
    else dims[0] = 0;
    if (!a1isnone) dims[1] = PyArray_DIMS( arr1 )[1]; //cols_y
    else dims[1] = 0;
    if (!a2isnone) dims[2] = PyArray_DIMS( arr2 )[1]; //cols_inf
    else dims[2] = 0;
    //size
    if (!a0isnone) dims[3] = PyArray_DIMS( arr0 )[0];
    else if (!a1isnone) dims[3] = PyArray_DIMS( arr1 )[0];
    else if (!a2isnone) dims[3] = PyArray_DIMS( arr2 )[0];
    else dims[3] = 0;
    sipCpp = new sigraph::SiDBLearnable(X, Y, I, dims, false);
  }
  else
  {
    sipCpp = NULL;
  }
%End

};



};

