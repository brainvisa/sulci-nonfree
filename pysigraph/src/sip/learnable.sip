
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
  int		*dims = (int *) malloc(sizeof(int) * 2);
  dims[1] = sipCpp->getXcolsNumber();
  dims[0] = sipCpp->size();
  sipRes = PyArray_FromDimsAndData(2, dims, PyArray_DOUBLE,
					(char *) sipCpp->getX());
  if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

  SIP_PYOBJECT getY() /Factory/;
%MethodCode
  int		*dims = (int *) malloc(sizeof(int) * 2);
  dims[1] = sipCpp->getYcolsNumber();
  dims[0] = sipCpp->size();
  sipRes = PyArray_FromDimsAndData(2, dims, PyArray_DOUBLE,
					(char *) sipCpp->getY());
  if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

  SIP_PYOBJECT getINF() /Factory/;
%MethodCode
  PyArray_Descr   *descr = (PyArray_Descr*) malloc(sizeof(PyArray_Descr));
  memcpy(descr, PyArray_DescrFromType(NPY_STRING), sizeof(PyArray_Descr));
  descr->elsize = 32;

  int             *dims = (int *) malloc(sizeof(int) * 2);
  dims[0] = sipCpp->size();
  dims[1] = sipCpp->getINFcolsNumber();
  sipRes = PyArray_FromDimsAndDataAndDescr(2, dims, descr,
	(char *) sipCpp->getINF());
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
      else if ((!a0isnone and arr0->nd != 2) or
               (!a1isnone and arr1->nd != 2) or
	       (!a2isnone and arr2->nd != 2))
      {
         sipIsErr = 1;
         PyErr_SetString( PyExc_RuntimeError,
                       "Array dimensions are not compatible with DBLearnable" );
      }
      else if ((!a0isnone and arr0->descr->type_num != PyArray_DOUBLE) or
               (!a1isnone and arr1->descr->type_num != PyArray_DOUBLE) or
               (!a2isnone and arr2->descr->type_num != PyArray_STRING))
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

    if (!a0isnone) X = (double *) arr0->data;
    if (!a1isnone) Y = (double *) arr1->data;
    if (!a2isnone) I = (char *) arr2->data;

    std::vector<int>	dims(4);
    if (!a0isnone) dims[0] = arr0->dimensions[1]; //cols_x
    else dims[0] = 0;
    if (!a1isnone) dims[1] = arr1->dimensions[1]; //cols_y
    else dims[1] = 0;
    if (!a2isnone) dims[2] = arr2->dimensions[1]; //cols_inf
    else dims[2] = 0;
    //size
    if (!a0isnone) dims[3] = arr0->dimensions[0];
    else if (!a1isnone) dims[3] = arr1->dimensions[0];
    else if (!a2isnone) dims[3] = arr2->dimensions[0];
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

