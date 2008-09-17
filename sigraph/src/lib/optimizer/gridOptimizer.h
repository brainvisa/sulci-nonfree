
#ifndef SI_OPTIMIZER_RANKSOPTIMIZER_H
#define SI_OPTIMIZER_RANKSOPTIMIZER_H

#include <cartobase/object/object.h>
#include <cartobase/object/property.h>
#include <si/optimizer/optimizer.h>


namespace sigraph
{


class GridOptimizerParameter : public carto::PropertySet
{
public:
	GridOptimizerParameter();
	GridOptimizerParameter(const std::string &name);
	GridOptimizerParameter(const GridOptimizerParameter &g);

	void			initBuiltins();

	std::string		&getName();
	std::vector<int>	&getRanges();
	std::string		&getScale();
protected:
	std::string		_name;
	std::vector<int>	_ranges;
	std::string		_scale;
};

inline std::string	&GridOptimizerParameter::getName() { return _name; }
inline std::vector<int>	&GridOptimizerParameter::getRanges() { return _ranges; }
inline std::string	&GridOptimizerParameter::getScale() { return _scale; }

/**	Grid Optimizer

 */
class GridOptimizer : public Optimizer
{
public:
	GridOptimizer(const carto::Object &parameters,
			const std::string &strategy);
	GridOptimizer(const GridOptimizer &optimizer);
	virtual ~GridOptimizer();

	virtual Optimizer* clone() const;
	GridOptimizer& operator = (const GridOptimizer &optimizer);

	const std::string typeName() const { return("grid_optimizer"); }

	virtual void buildTree(Tree &tr) const;

	std::string	getStrategy() const;
	carto::Object	getParameters() const;

protected:
	carto::Object	_parameters;
	std::string	_strategy;
};


//	inline
inline 	GridOptimizer::GridOptimizer(const GridOptimizer &go) :
Optimizer(go), _parameters(go._parameters), _strategy(go._strategy) {}


inline Optimizer* GridOptimizer::clone() const
{
	return (new GridOptimizer(*this));
}

inline GridOptimizer &
GridOptimizer::operator=(const GridOptimizer &)
{
	return (*this);
}

inline std::string      GridOptimizer::getStrategy() const { return _strategy;}
inline carto::Object	GridOptimizer::getParameters() const
{
	return _parameters;
}

}

#include <cartobase/object/syntobject.h>

namespace carto
{
	DECLARE_GENERIC_OBJECT_TYPE(sigraph::GridOptimizerParameter)

template<> inline std::string DataTypeCode<sigraph::GridOptimizerParameter>::dataType()
  {
    return "grid_optimizer_parameter";
  }
}


#endif
