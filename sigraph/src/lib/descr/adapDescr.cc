
#include <si/descr/adapDescr.h>
#include <si/model/learnParam.h>
#include <si/learnable/learnable.h>
#include <si/model/adaptiveLeaf.h>
#include <cartobase/type/string_conversion.h>
#include <string.h>

using namespace sigraph;


AdapDescr::AdapDescr() : CliqueDescr(), _learnable(NULL)
{
}


AdapDescr::~AdapDescr()
{
  delete _learnable;
}

void		AdapDescr::addGeneratedVector(const LearnConstParam *lp)
{
	GeneratedVector				*gv = NULL;
	AdaptiveLeaf				*al = lp->adap;
	assert(al);

	carto::GenericObject		*ao = al->graphObject();
	std::vector<double>		vec;
		
	if (!makeLearnVector(lp->clique, vec, ao)) return;
	
	preProcess(vec, ao);
	gv = new GeneratedVector(vec, lp->outp, lp->class_id, lp->cycle);
	_generated_vectors.push_back(gv);
}


void	AdapDescr::updateSiDBLearnable(void)
{
	std::list<GeneratedVector *>::iterator	it, et;
	int					size, dimX, i;
	char					*offset;
	double					*X = NULL, *Y = NULL;
	char					*INF = NULL;
	std::vector<int>			dims(4);


	size = _generated_vectors.size();
	dimX = (*(_generated_vectors.begin()))->getVector().size();
	X = new double[dimX * size];
	Y = new double[2 * size];
	INF = new char[32 * size];
	for (i = 0, it = _generated_vectors.begin(),
			et = _generated_vectors.end();
			it != et; ++it, ++i)
	{
		const std::vector<double> &vec = (*it)->getVector();
		memcpy(&X[i * dimX], &vec[0], dimX * sizeof(double));
		Y[i * 2] = (*it)->getOutp();
		Y[i * 2 + 1] = (*it)->getClassID();
		std::string	s = carto::toString<int>((*it)->getCycle());
		offset = INF + 32 * i;
		memcpy(offset, s.c_str(), s.size());
		memset(offset + s.size(), '\0', 32 - s.size());
	}

	dims[0] = dimX;
	dims[1] = 2;
	dims[2] = 1;
	dims[3] = size;

	_learnable = new SiDBLearnable(X, Y, INF, dims, true);

	// delete generated vectors
	for (it = _generated_vectors.begin(), et = _generated_vectors.end();
			it != et; ++it)
		delete *it;
	_generated_vectors.clear();
}

SiDBLearnable	&AdapDescr::getSiDBLearnable()
{
	return *_learnable;
}


void AdapDescr::clearDB()
{
  delete _learnable;
  _learnable = 0;
}

