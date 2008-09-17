

#ifndef SI_SUBADAPTIVE_NONINCREMENTALSUBADAPTIVE_H
#define SI_SUBADAPTIVE_NONINCREMENTALSUBADAPTIVE_H

#include <si/subadaptive/subAdaptive.h>
#include <string>


namespace sigraph
{

class NonIncrementalSubAdaptive : public SubAdaptive
{
	public:
	virtual ~NonIncrementalSubAdaptive();

	virtual NonIncrementalSubAdaptive & operator =
		(const NonIncrementalSubAdaptive & sa);

	///Apprentissage
	virtual SubAdResponse *train(AdaptiveLeaf &, 
		const SiDBLearnable &tr, const SiDBLearnable &tst);
	virtual double learn(const SiDBLearnable &train) = 0;
	///Apprentissage incrémental ?
	virtual bool hasIncrementalLearning(void) const { return false;};

	protected:
	NonIncrementalSubAdaptive(const std::string name = "");
	NonIncrementalSubAdaptive(const NonIncrementalSubAdaptive &sa);
};


// Fonctions inline
inline
NonIncrementalSubAdaptive::NonIncrementalSubAdaptive(
const NonIncrementalSubAdaptive &sa) : SubAdaptive(sa) { }


inline NonIncrementalSubAdaptive&
NonIncrementalSubAdaptive::operator = (const NonIncrementalSubAdaptive &sa)
{
	if(this != &sa)
        	SubAdaptive::operator = (sa);
	return(*this);
}


}

#endif



