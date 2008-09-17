#include <si/learnable/dbLearnable.h>
#include <si/learnable/vectorLearnable.h>

using namespace carto;
using namespace sigraph;


DBLearnable::DBLearnable(double *X, double *Y, char *INF,
			std::vector<int> &dims, bool owned_data = false)
 : _X(X), _Y(Y), _INF(INF), _size(dims[3]), _owned_data(owned_data) { 
	_cols_numbers.reserve(3);
	_cols_numbers[0] = dims[0];
	_cols_numbers[1] = dims[1];
	_cols_numbers[2] = dims[2];
}

DBVectorLearnable *DBLearnable::operator[](unsigned int ind) const
{
	DBVectorLearnable *v = new DBVectorLearnable(this, ind);
	return v;
};

SiVectorLearnable *SiDBLearnable::operator[](unsigned int ind) const
{
	SiVectorLearnable *v = new SiVectorLearnable(this, ind);
	return v;
};

