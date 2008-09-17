

#ifndef SI_LEARNABLE_VECTORLEARNABLE_H
#define SI_LEARNABLE_VECTORLEARNABLE_H

#include <vector>
#include <si/learnable/learnable.h>
#include <si/learnable/dbLearnable.h>
#include <cartobase/type/string_conversion.h>

namespace sigraph
{

class VectorLearnable : public Learnable
{
	public:
	virtual ~VectorLearnable() {};

	//@{
	/// Converter
	public:
	virtual std::vector<double>	X() const = 0;
	virtual std::vector<double>	Y() const = 0;
	virtual std::string		INF(unsigned int ind) const = 0;
	//@}

	//@{
	/// inline / accessors
	public:
	virtual int	sizeX() const = 0;
	virtual int	sizeY() const = 0;
	virtual int	sizeINF() const = 0;
	virtual double	getX(unsigned int ind) const = 0;
	virtual double	getY(unsigned int ind) const = 0;
	virtual char	*getINF(unsigned int ind) const = 0;
	//@}
};


class GaussVectorLearnable : public Learnable
{
	public:
	GaussVectorLearnable(std::vector<double> &X, double y);
	virtual ~GaussVectorLearnable() {};

	//@{
	/// Converter
	public:
	virtual std::vector<double>	X() const;
	virtual std::vector<double>	Y() const;
	virtual std::string		INF(unsigned int ind) const;
	//@}

	//@{
	/// inline / accessors
	public:
	virtual int	sizeX() const;
	virtual int	sizeY() const;
	virtual int	sizeINF() const;
	virtual double	getX(unsigned int ind) const;
	virtual double	getY(unsigned int ind) const;
	virtual double  y() const;
	virtual char	*getINF(unsigned int ind) const;
	//@}
	
	protected:
	std::vector<double>	_X;
	double			_y;
};


class DBVectorLearnable : public VectorLearnable
{
	public:
	DBVectorLearnable(const DBLearnable *db, unsigned int ind);
	virtual ~DBVectorLearnable() {};

	//@{
	/// Converter
	public:
	virtual std::vector<double>	X() const;
	virtual std::vector<double>	Y() const;
	virtual std::string		INF(unsigned int ind) const;
	virtual std::string		INF(std::string &label) const;
	//@}

	//@{
	/// inline / accessors
	public:
	virtual int	sizeX() const;
	virtual int	sizeY() const;
	virtual int	sizeINF() const;
	virtual double	getX(unsigned int ind) const;
	virtual double	getY(unsigned int ind) const;
	virtual char	*getINF(unsigned int ind) const;
	virtual char	*getINF(std::string &label) const;
	//@}

	protected:
	///db which owned data
	const DBLearnable	*_db;
	///indice of vector in database (row indice)
	unsigned int		_ind;
};


///Specialization for sigraph : Y as only one dim
class SiVectorLearnable : public DBVectorLearnable
{
	public:
	SiVectorLearnable(const DBLearnable *db, unsigned int ind);
	virtual ~SiVectorLearnable() {};

	int	cycle() const;
	double	y() const;
	
	private:
	unsigned int	_cycle_ind;
};


/******************************************************************************/
inline int	DBVectorLearnable::sizeX() const
{
	return _db->getXcolsNumber();
}

inline int	DBVectorLearnable::sizeY() const
{
	return _db->getYcolsNumber();
}

inline int	DBVectorLearnable::sizeINF() const
{
	return _db->getINFcolsNumber();
}

inline double	DBVectorLearnable::getX(unsigned int ind) const
{
	double	*X = _db->getX();
	int	l = _db->getXcolsNumber();
	return X[_ind * l + ind]; //ind : column indice.
}

inline double	DBVectorLearnable::getY(unsigned int ind) const
{
	double	*Y = _db->getY();
	int	l = _db->getYcolsNumber();
	return Y[_ind * l + ind]; //ind : column indice.
}

inline char	*DBVectorLearnable::getINF(unsigned int ind) const
{
	char	*INF = _db->getINF();
	int	l = _db->getINFcolsNumber();
	return INF + (_ind * l * 32 + ind); //ind : column indice.
}

inline char	*DBVectorLearnable::getINF(std::string &label) const
{
	int	ind = _db->getIndiceFromLabel(label);
	return getINF(ind);
}


inline int	SiVectorLearnable::cycle() const
{
	unsigned int		d;
	const std::string	cycle_string = INF(_cycle_ind);

	carto::stringTo<unsigned int>(cycle_string, d);
	return d;
}


/******************************************************************************/
inline double SiVectorLearnable::y() const
{
	return getY(0);
}

}

#endif
