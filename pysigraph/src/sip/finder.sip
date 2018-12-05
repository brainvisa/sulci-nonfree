
namespace sigraph
{

class ModelFinder /Abstract/
{
%TypeHeaderCode
#include <si/finder/modelFinder.h>
#include <cartobase/object/object.h>
%End

public:
	virtual void initCliques(sigraph::CGraph &, bool=true, bool=false, 
				bool=true, bool=true /*, 
				const aims::SelectionSet *=0 */ ) = 0 /AutoGen/;
	virtual bool isAdaptive() const = 0 /AutoGen/;
	carto::GenericObject* selectModel2(const sigraph::Clique*) /PyName=selectModel, AutoGen/;
%MethodCode
	sipRes = sipCpp->selectModel(a0);
%End

};


class AdapFinder : sigraph::ModelFinder /Abstract/
{
%TypeHeaderCode
#include <si/finder/adapFinder.h>
%End

public:
	AdapFinder(sigraph::MGraph &);
	virtual ~AdapFinder();

	virtual bool isAdaptive() const;
};

class FoldFinder : sigraph::AdapFinder
{
%TypeHeaderCode
#include <si/fold/foldFinder.h>
%End


public:
	FoldFinder(sigraph::MGraph &);
	virtual ~FoldFinder();
};

};

