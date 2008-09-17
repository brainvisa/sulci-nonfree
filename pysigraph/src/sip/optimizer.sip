
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <si/optimizer/gridOptimizer.h>
#endif
%End


namespace sigraph
{

class Optimizer /Abstract/
{
%TypeHeaderCode
#include <si/optimizer/optimizer.h>
%End

%ConvertToSubClassCode
if (dynamic_cast<sigraph::GridOptimizer *>(sipCpp))
	sipClass = sipClass_sigraph_GridOptimizer;
else  sipClass = sipClass_sigraph_Optimizer;
%End

public:
	virtual ~Optimizer();
};



class GridOptimizerParameter
{
%TypeHeaderCode
#include <si/optimizer/gridOptimizer.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_S32.h"
#endif
%End

public:
	GridOptimizerParameter();
	virtual ~GridOptimizerParameter();


	std::string		&getName();
	vector_S32		&getRanges();
	std::string		&getScale();

	static sigraph::GridOptimizerParameter*
	fromObject(carto::GenericObject);
%MethodCode
	try
	{
		sipRes = &(a0->value<sigraph::GridOptimizerParameter>());
	}
	catch( std::exception & e )
	{
		sipIsErr = 1;
		PyErr_SetString( PyExc_RuntimeError, e.what() );
		sipRes = 0;
	}
%End
};

class GridOptimizer: sigraph::Optimizer
{
%TypeHeaderCode
#include <si/optimizer/gridOptimizer.h>
%End

public:
	GridOptimizer(const carto::Object &, const std::string &);
	virtual ~GridOptimizer();

	std::string getStrategy() const;
	carto::Object getParameters() const;

};

};
