

#ifndef SI_SUBADAPTIVE_INCREMENTALSUBADAPTIVE_H
#define SI_SUBADAPTIVE_INCREMENTALSUBADAPTIVE_H

#include <si/subadaptive/subAdaptive.h>
#include <si/learnable/vectorLearnable.h>
#include <si/learnable/dbLearnable.h>

#include <string>


namespace sigraph
{
class AdaptiveLeaf;

class IncrementalSubAdaptive : public SubAdaptive
{
	public:
	virtual ~IncrementalSubAdaptive();

	virtual IncrementalSubAdaptive & operator =
		(const IncrementalSubAdaptive & sa);

	/// Apprentissage
	virtual SubAdResponse *train(AdaptiveLeaf &al,
		const SiDBLearnable &train, const SiDBLearnable &test) = 0;
	virtual double learn(AdaptiveLeaf &al,
		const SiDBLearnable &train, const SiDBLearnable &test) = 0;
	//Apprentissage incrémental ?
	virtual bool hasIncrementalLearning(void) const { return true;};

	/**@na Data Access */
	//@{
	///Testing period
	unsigned int testPeriod() const { return _testPeriod; }
	void setTestPeriod(unsigned int testPeriod){ _testPeriod = testPeriod; }
	//@}

	protected:
	IncrementalSubAdaptive(const std::string name = "");
	IncrementalSubAdaptive(const IncrementalSubAdaptive &sa);

	/// Test Period
	unsigned int	_testPeriod;
};


// Fonctions inline
inline IncrementalSubAdaptive::IncrementalSubAdaptive(
	const IncrementalSubAdaptive &sa) :
	SubAdaptive(sa), _testPeriod(sa._testPeriod) {}


inline IncrementalSubAdaptive&
IncrementalSubAdaptive::operator = (const IncrementalSubAdaptive &sa)
{
	if(this != &sa)
        	SubAdaptive::operator = (sa);
	_testPeriod = sa._testPeriod;
	return(*this);
}


}

#endif



