
#ifndef SI_FS_FS_H
#define SI_FS_FS_H


#include <cartobase/object/object.h>

class Tree;


namespace sigraph
{

/**  Feature Selector :
 */
class DimReductor
{
  public:
    virtual ~DimReductor();

    virtual DimReductor* clone() const = 0;
    virtual const std::string typeName() const = 0;

    virtual unsigned int reducedDim() const = 0;
    virtual std::vector<double> *
    transform(const std::vector<double> &v) const;
    virtual void transform(const std::vector<double> &src,
                           std::vector<double> &dst) const = 0;
    virtual void buildTree(Tree &tr) const = 0;
    DimReductor& operator=(const DimReductor&);

  protected:
    DimReductor();
    DimReductor(const DimReductor& dimreductor);
};

//  inline
inline DimReductor::DimReductor()
{
}

inline DimReductor::DimReductor(const DimReductor &)
{
}

inline DimReductor& DimReductor::operator=(const DimReductor&)
{
  return (*this);
}

inline std::vector<double> *
DimReductor::transform(const std::vector<double> &v) const
{
  std::vector<double>  *v2 = new std::vector<double>;
  v2->reserve(reducedDim());
  transform(v, *v2);
  return v2;
}

}



namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE(sigraph::DimReductor *)
}

#endif

