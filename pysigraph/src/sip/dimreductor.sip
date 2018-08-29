
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <si/dimreductor/ranksDimreductor.h>
#include <si/dimreductor/matrixDimreductor.h>
#endif
%End


namespace sigraph
{

class DimReductor /Abstract/
{
%TypeHeaderCode
#include <si/dimreductor/dimreductor.h>
%End

%ConvertToSubClassCode
if (dynamic_cast<sigraph::RanksDimReductor *>(sipCpp))
	sipClass = sipClass_sigraph_RanksDimReductor;
else if (dynamic_cast<sigraph::MatrixDimReductor *>(sipCpp))
	sipClass = sipClass_sigraph_MatrixDimReductor;
else  sipClass = sipClass_sigraph_DimReductor;
%End

public:
	virtual ~DimReductor();

	virtual unsigned int reducedDim() const = 0;
};

class RanksDimReductor : sigraph::DimReductor
{
%TypeHeaderCode
#include <si/dimreductor/ranksDimreductor.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_S32.h"
#endif
%End

public:
	RanksDimReductor(vector_S32&, int);
	virtual ~RanksDimReductor();

	const vector_S32& getRanks() const;
	int getSelected() const;
};

class FakeDimReductor : sigraph::DimReductor
{
%TypeHeaderCode
#include <si/dimreductor/fakeDimreductor.h>
%End

public:
	FakeDimReductor(unsigned int = 0);
	virtual ~FakeDimReductor();
};


class MatrixDimReductor : sigraph::DimReductor
{
%TypeHeaderCode
#define PY_ARRAY_TYPES_PREFIX PyAIMSTypes
#include <si/dimreductor/matrixDimreductor.h>
#include <pyaims/numpyarray.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_S32.h"
#include "sipaimssipvector_FLOAT.h"
#endif
%End

public:
	MatrixDimReductor(vector_FLOAT&, vector_S32&, int);
	virtual ~MatrixDimReductor();
	
	int getSelected() const;
	const vector_S32& getShape() const;

	SIP_PYOBJECT getMatrix() /Factory/;
%MethodCode
	int				*dims = (int *) malloc(sizeof(int) * 2);
	const std::vector<int>	&shape = sipCpp->getShape();
	dims[1] = shape[1];
	dims[0] = shape[0];
	sipRes = PyArray_FromDimsAndData(2, dims, NPY_FLOAT,
					(char *) &(sipCpp->getMatrix()[0]));
	if(sipRes) sipRes = PyArray_Return((PyArrayObject *) sipRes);
%End

};



};

