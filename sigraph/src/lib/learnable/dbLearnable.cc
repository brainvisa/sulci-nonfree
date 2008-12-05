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

DBLearnable::~DBLearnable()
{
  clear();
}

DBVectorLearnable *DBLearnable::operator[](unsigned int ind) const
{
	DBVectorLearnable *v = new DBVectorLearnable(this, ind);
	return v;
};

void DBLearnable::clear()
{
  if( _owned_data )
  {
    delete _X;
    delete _Y;
    delete _INF;
  }
  _X = 0;
  _Y = 0;
  _INF = 0;
  _cols_numbers.clear();
  _labels_to_indices.clear();
}

SiVectorLearnable *SiDBLearnable::operator[](unsigned int ind) const
{
	SiVectorLearnable *v = new SiVectorLearnable(this, ind);
	return v;
};

