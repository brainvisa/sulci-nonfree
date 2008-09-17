
#ifndef SI_FS_MATRIXFS_H
#define SI_FS_MATRIXFS_H

#include <assert.h>
#include <si/dimreductor/dimreductor.h>


namespace sigraph
{

/**  Matrix Dimension Reductor (for instance svd)

 */
class MatrixDimReductor : public DimReductor
{
  public:
    MatrixDimReductor(std::vector<float> &matrix,
                      std::vector<int> &shape,
                      int selected);
    MatrixDimReductor(const MatrixDimReductor &dimreductor);
    virtual ~MatrixDimReductor();

    virtual DimReductor* clone() const;
    MatrixDimReductor& operator = (const MatrixDimReductor &dimreductor);

    virtual const std::string typeName() const {
      return( "matrix_dimreduction" ); }
    virtual unsigned int reducedDim() const;
    virtual void buildTree(Tree &tr) const;
    virtual void transform(const std::vector<double> &src,
                           std::vector<double> &dst) const;


    int getSelected(void) const;
    const std::vector<int>& getShape(void) const;
    const std::vector<float>& getMatrix(void) const;

  protected:
    std::vector<float> _matrix;
    std::vector<int>   _shape;
    int                _selected;
  private:
};

//  inline
inline MatrixDimReductor::MatrixDimReductor
  (std::vector<float> &matrix, std::vector<int> &shape, int selected) :
  _matrix(matrix), _shape(shape), _selected(selected) {}

inline MatrixDimReductor::MatrixDimReductor
  (const MatrixDimReductor &dimreductor) : DimReductor(),
  _matrix(dimreductor._matrix), _shape(dimreductor._shape),
  _selected(dimreductor._selected) {}

inline DimReductor* MatrixDimReductor::clone() const
{
  return (new MatrixDimReductor(*this));
}

inline MatrixDimReductor &
MatrixDimReductor::operator=(const MatrixDimReductor &d)
{
	if(this != &d)
	{
		DimReductor::operator = (d);
		_matrix = d._matrix;
		_shape = d._shape;
		_selected = d._selected;
	}
	return (*this);
}

inline int MatrixDimReductor::getSelected(void) const { return _selected; }

inline const std::vector<int>& MatrixDimReductor::getShape(void) const
{ return _shape; }


inline const std::vector<float>& MatrixDimReductor::getMatrix(void) const
{ return _matrix; }


inline unsigned int MatrixDimReductor::reducedDim() const
{
  return (unsigned int ) _selected;
}


inline void  MatrixDimReductor::transform(const std::vector<double> &src,
           std::vector<double> &dst) const
{
  assert(src.size() == (unsigned int) _shape[1]);
  assert(dst.size() == (unsigned int) _selected);

  unsigned int  i, j, s = src.size();
  //dst = _matrix * src with troncated output according to selected value.
  for (i = 0; i < (unsigned int) _selected; ++i)
  {
    dst[i] = 0.;
    for (j = 0; j < s; ++j)
      dst[i] += _matrix[i * _shape[1] + j] * src[j];
  }
}
}

#endif

