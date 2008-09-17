
#ifndef SI_DIMREDUCTOR_FAKEDIMREDUCTOR_H
#define SI_DIMREDUCTOR_FAKEDIMREDUCTOR_H

#include <si/dimreductor/dimreductor.h>
#include <assert.h>


namespace sigraph
{

class FakeDimReductor : public DimReductor
{
	public:
	FakeDimReductor(unsigned int size=0.);
	FakeDimReductor(const FakeDimReductor &dimreductor);
	virtual ~FakeDimReductor();

	virtual DimReductor* clone() const;
	FakeDimReductor& operator = (const FakeDimReductor &dimreductor);

	virtual const std::string typeName() const
		{ return( "fake_dimreductor" ); }

	virtual unsigned int reducedDim() const;
	virtual void transform(const std::vector<double> &src,
				std::vector<double> &dst) const;

	virtual void buildTree(Tree &tr) const;

	protected:
	unsigned int	_size;
};


//  inline
inline   FakeDimReductor::FakeDimReductor(unsigned int size) :
	DimReductor(), _size(size) {}


inline FakeDimReductor::FakeDimReductor(const FakeDimReductor &dimreductor) :
	DimReductor(dimreductor), _size(dimreductor._size) {}


inline DimReductor* FakeDimReductor::clone() const
{
	return (new FakeDimReductor(*this));
}

inline FakeDimReductor &
FakeDimReductor::operator=(const FakeDimReductor &d)
{
	if(this != &d)
	{
		DimReductor::operator = (d);
		_size = d._size;
	}
	return (*this);
}

inline unsigned int FakeDimReductor::reducedDim() const
{
	return _size;
}

inline void  FakeDimReductor::transform(const std::vector<double> &src,
           std::vector<double> &dst) const
{
	assert(dst.size() == src.size());
	dst = src;
}
}

#endif

