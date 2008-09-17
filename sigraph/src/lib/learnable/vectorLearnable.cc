#include <si/learnable/vectorLearnable.h>
#include <assert.h>

using namespace sigraph;

/******************************************************************************/
GaussVectorLearnable::GaussVectorLearnable(std::vector<double> &X, double y) :
_X(X), _y(y) {};

std::vector<double> GaussVectorLearnable::X() const
{
	return _X;
}

std::vector<double> GaussVectorLearnable::Y() const
{
	std::vector<double> v(1);
	v[0] = _y;
	return v;
}

std::string GaussVectorLearnable::INF(unsigned int) const
{
	return std::string("");
}

int GaussVectorLearnable::sizeX() const
{
	return _X.size();
}

int GaussVectorLearnable::sizeY() const
{
	return 1;
}

int GaussVectorLearnable::sizeINF() const
{
	return 0;
}

double GaussVectorLearnable::getX(unsigned int ind) const
{
	return _X[ind];
}

double GaussVectorLearnable::getY(unsigned int) const
{
	return _y;
}

double  GaussVectorLearnable::y() const
{
	return _y;
}

char *GaussVectorLearnable::getINF(unsigned int) const
{
	return NULL;
}


/******************************************************************************/
DBVectorLearnable::DBVectorLearnable(const DBLearnable *db, unsigned int ind) :
_db(db), _ind(ind) { assert(db); };

std::vector<double> DBVectorLearnable::X() const
{
	int			l = _db->getXcolsNumber();
	std::vector<double>	v(l);
	double			*X = _db->getX();
	for (int i = 0; i < l; ++i) v[i] = X[_ind * l + i];
	return v;
}

std::vector<double> DBVectorLearnable::Y() const
{
	int			l = _db->getYcolsNumber();
	std::vector<double>	v(l);
	double			*Y = _db->getY();
	for (int i = 0; i < l; ++i) v[i] = Y[_ind * l + i];
	return v;
}

std::string DBVectorLearnable::INF(unsigned int ind) const
{
	int			l = _db->getINFcolsNumber();
	char			*INF = _db->getINF();
	std::string		v(INF + (_ind * l + ind)* 32, 32);
	return v;
}

std::string DBVectorLearnable::INF(std::string &label) const
{
	int	ind = _db->getIndiceFromLabel(label);
	return INF(ind);
}


/******************************************************************************/
SiVectorLearnable::SiVectorLearnable(const DBLearnable *db, unsigned int ind) :
	DBVectorLearnable(db, ind)
{
	std::string	label = "cycle";
	_cycle_ind = _db->getIndiceFromLabel(label);
}
