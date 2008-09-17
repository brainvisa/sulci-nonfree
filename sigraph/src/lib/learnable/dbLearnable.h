

#ifndef SI_LEARNABLE_DBLEARNABLE_H
#define SI_LEARNABLE_DBLEARNABLE_H

#include <si/learnable/learnable.h>
#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <iostream> //FIXME : todel

namespace sigraph
{

class DBVectorLearnable;
class SiVectorLearnable;

class DBLearnable : public Learnable
{
	public:
	DBLearnable() : _X(NULL), _Y(NULL), _INF(NULL), _size(0),
		_cols_numbers(std::vector<int>(3, 0)), _owned_data(false) {};
	/** Create DBLearnable from arrays and dims.
            X :    X data matrix.
	    Y :    Y data matrix.
            INF :  INF data matrix.
	    dims : std::vector of 4 elements : 
	           first : (x, y, z) number of columns and then number of rows.
	*/
	DBLearnable(double *X, double *Y, char *INF, std::vector<int> &dims,
		bool owned_data);

	virtual ~DBLearnable() {};

	inline DBLearnable(const DBLearnable & db)
		: Learnable(db)
	{
		*this = db;
	}


	inline DBLearnable& operator = (const DBLearnable &db)
	{
		if (this == &db) return *this;
		_size = db._size;
		_cols_numbers = db._cols_numbers;
		if (_owned_data == false)
		{
			_X = db._X;
			_Y = db._Y;
			_INF = db._INF;
		} else {
			_X = new double[_cols_numbers[0] * _size];
			_Y = new double[_cols_numbers[1] * _size];
			_INF = new char[_cols_numbers[2] * 32 * _size];
			memcpy(_X, db._X, _cols_numbers[0] * _size);
			memcpy(_Y, db._Y, _cols_numbers[1] * _size);
			//FIXME : vrai pour siDBLearnable uniquement
			memcpy(_INF, db._INF, _cols_numbers[2] * 32 * _size);
		}
	return( *this );
	}



	//@{
	///Accessors :
	public:
	inline double *getX() const { return _X; };
	inline double *getY() const { return _Y; };
	inline char *getINF() const { return _INF; };
	inline int size() const { return _size; };
	inline const std::vector<int> &getColsNumber() const {
		return _cols_numbers; }
	inline int getXcolsNumber() const { return _cols_numbers[0]; };
	inline int getYcolsNumber() const { return _cols_numbers[1]; };
	inline int getINFcolsNumber() const { return _cols_numbers[2]; };
	inline unsigned int getIndiceFromLabel(std::string &label) const {
		if (_labels_to_indices.find(label) == _labels_to_indices.end())
			throw;
		return (*_labels_to_indices.find(label)).second;
		
	};
	// New allocated VectorLearnable from database slice
	DBVectorLearnable *operator[](unsigned int ind) const;
	//@}
	
	public:
	void	setLabels(std::vector<std::string> &labels) {
		std::vector<std::string>::const_iterator	il, el;
		unsigned int					ind = 0;

		for (il = labels.begin(), el = labels.end();
					il != el; ++il, ++ind)
			_labels_to_indices[*il] = ind;
	}

	protected:
	/// X data matrix
	double			*_X;
	/// Y data matrix
	double			*_Y;
	/// meta information data matrix
	char			*_INF;
	/// number of vectors
	int			_size;
	/// number of columns (X, Y, INF)
	std::vector<int>	_cols_numbers;
	/// data are owned by database
	bool			_owned_data;
	/// info labels -> indices
	std::map<std::string, unsigned int>	_labels_to_indices;
};


class SiDBLearnable : public DBLearnable
{
	public:
	SiDBLearnable() : DBLearnable() {};
	SiDBLearnable(double *X, double *Y, char *INF,
		std::vector<int> &dims, bool owned_data) :
		DBLearnable(X, Y, INF, dims, owned_data), _cycles(0) {};

	virtual ~SiDBLearnable() {};

	//@{
	///Accessors
	public:
	inline void setSplit(int split) { _split = split; };
	inline void setCycles(int cycles) { _cycles = cycles; };
	inline int getSplit() const { return _split; };
	inline int getCycles() const { return _cycles; };
	// New allocated SiVectorLearnable from database slice
	SiVectorLearnable *operator[](unsigned int ind) const;
	//@}

	private:
	// split value between train and test vectors
	int	_split;
	// number of train cycles.
	int	_cycles;
};

}

#endif
