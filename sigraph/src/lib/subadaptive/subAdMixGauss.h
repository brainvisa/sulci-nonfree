
#ifndef SI_SUBADAPTIVE_SUBADMIXGAUSS_H
#define SI_SUBADAPTIVE_SUBADMIXGAUSS_H

#include <si/subadaptive/nonIncrementalSubAdaptive.h>
#include <si/subadaptive/subAdaptive.h>
#include <vector>
#include <list>

namespace sigraph
{

///	Mixture of Gaussians
class SubAdMixGauss : public NonIncrementalSubAdaptive
{
public:
	SubAdMixGauss();
	SubAdMixGauss(const std::string &name,
			const std::vector<float> &sqrtdets);
	SubAdMixGauss(const SubAdMixGauss &g);
	virtual ~SubAdMixGauss();

	SubAdMixGauss & operator = (const SubAdMixGauss &g);
	virtual SubAdaptive* clone() const;

	virtual void init();
	virtual double prop( const std::vector<double> & vec );
	// Fake learn function : do nothing, all is done in python.
	virtual double learn(const SiDBLearnable &db);
	virtual void buildTree( Tree & tr ) const;
	virtual void addMatrix(const std::vector<float> &matrix);
	virtual void reset();
	virtual void addSqrtDet(double sqrtdet);

protected:
	std::list<std::vector<float> >	_metrics;
	std::vector<float>		_sqrtdets;
};


//	inline

inline SubAdMixGauss & SubAdMixGauss::operator = ( const SubAdMixGauss & g )
{
	if( this != &g )
	{
		_metrics = g._metrics;
		_sqrtdets = g._sqrtdets;
	}
	return( *this );
}


inline SubAdaptive* SubAdMixGauss::clone() const
{
	return( new SubAdMixGauss( *this ) );
}

}

#endif


