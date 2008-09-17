
#ifndef SI_FS_RANKSFS_H
#define SI_FS_RANKSFS_H

#include <si/dimreductor/dimreductor.h>
#include  <assert.h>


namespace sigraph
{

/**  Ranks Feature Selector

 */
class RanksDimReductor : public DimReductor
{
	public:
	RanksDimReductor(std::vector<int> &ranks, int selected);
	RanksDimReductor(const RanksDimReductor &dimreductor);
	virtual ~RanksDimReductor();

	virtual DimReductor* clone() const;
	RanksDimReductor& operator = (const RanksDimReductor &dimreductor);

	virtual const std::string typeName() const
		{ return( "ranks_dimreductor" ); }

	virtual unsigned int reducedDim() const;
	virtual void transform(const std::vector<double> &src,
				std::vector<double> &dst) const;

	virtual void buildTree(Tree &tr) const;
	const std::vector<int>& getRanks(void) const;
	int getSelected(void) const;
	std::string getMode(void) const;

	protected:
	std::vector<int>	_ranks;
	int			_selected;
	private:
};


//  inline
inline   RanksDimReductor::RanksDimReductor
	(std::vector<int> &ranks, int selected) :
	DimReductor(), _ranks(ranks), _selected(selected) {}

inline RanksDimReductor::RanksDimReductor
	(const RanksDimReductor &dimreductor) : DimReductor(),
	_ranks(dimreductor._ranks), _selected(dimreductor._selected) {}



inline DimReductor* RanksDimReductor::clone() const
{
	return (new RanksDimReductor(*this));
}

inline RanksDimReductor &
RanksDimReductor::operator=(const RanksDimReductor &d)
{
	if (this != &d)
	{
		DimReductor::operator = (d);
		_ranks = d._ranks;
		_selected = d._selected;
	}
	return (*this);
}

inline const std::vector<int>& RanksDimReductor::getRanks(void) const
{ return _ranks; }

inline int RanksDimReductor::getSelected(void) const { return _selected; }

inline unsigned int RanksDimReductor::reducedDim() const
{
	return (unsigned int ) _selected;
}

inline void  RanksDimReductor::transform(const std::vector<double> &src,
           std::vector<double> &dst) const
{
	assert(src.size() == (unsigned int) _ranks.size());
	assert(dst.size() == (unsigned int) _selected);

	for (unsigned int i = 0; i < (unsigned int) _selected; ++i)
		dst[i] = src[_ranks[i]];
}
}

#endif

