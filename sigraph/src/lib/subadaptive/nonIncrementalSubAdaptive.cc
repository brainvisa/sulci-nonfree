#include <si/subadaptive/nonIncrementalSubAdaptive.h>

using namespace sigraph;


NonIncrementalSubAdaptive::NonIncrementalSubAdaptive(const std::string name)
        : SubAdaptive(name)
{
}

NonIncrementalSubAdaptive::~NonIncrementalSubAdaptive()
{
}


SubAdResponse	*NonIncrementalSubAdaptive::train(AdaptiveLeaf &, 
	const SiDBLearnable &tr, const SiDBLearnable &tst)
{
	learn(tr);
	SubAdResponse	*response = test(tst);
	return response;
}
