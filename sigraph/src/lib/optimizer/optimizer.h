
#ifndef SI_OPTIMIZER_OPTIMIZER_H
#define SI_OPTIMIZER_OPTIMIZER_H


#include <cartobase/object/object.h>

class Tree;


namespace sigraph
{

/**	Optimizer
 */
class Optimizer
{
	public:
		virtual ~Optimizer();

		virtual Optimizer* clone() const = 0;
		virtual const std::string typeName() const = 0;

		virtual void buildTree(Tree &tr) const = 0;
	        Optimizer& operator=(const Optimizer&);

	protected:
		Optimizer();
		Optimizer(const Optimizer& optimizer);
};

//	inline
inline Optimizer::Optimizer() {}

inline Optimizer::Optimizer(const Optimizer &) {}

inline Optimizer& Optimizer::operator=(const Optimizer&)
{
	return (*this);
}

}

namespace carto
{
	DECLARE_GENERIC_OBJECT_TYPE(sigraph::Optimizer *)
}

#endif

