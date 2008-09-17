
#ifndef SI_FOLD_RBFDOMAIN_H
#define SI_FOLD_RBFDOMAIN_H


#include <si/domain/adapDomain.h>
#include <vector>
template<class T, int D> class AimsVector;
class GaussNet;


namespace sigraph
{

  class DomainRBF : public AdapDomain
  {
  public:
    DomainRBF();
    DomainRBF( const DomainRBF & dom );
    virtual ~DomainRBF();
    virtual Domain* clone() const;
    virtual void learn( const Vertex* v, const Graph* g = 0 );
    virtual void learnBucket( const Vertex* v, const Graph* g = 0 );
    virtual void reset();
    virtual void buildTree( Tree & tr ) const;
    virtual bool canBeFound( const Vertex* v, const Graph* g = 0 );
    virtual bool canBeFound( double x, double y, double z );
    float sigma() const;
    float threshold() const;
    float learnThreshold() const;
    void setSigma( float );
    void setThreshold( float );
    void setLearnThreshold( float );
    unsigned nGauss() const;
    const GaussNet & gaussNet() const;
    GaussNet & gaussNet();

    static void buildDomRBF( Tree* parent, Tree* tr );

  protected:
    void learnVoxel( const std::vector<float> & rot, 
		     const std::vector<float> & scale, 
		     const std::vector<float> & transl, 
		     const std::vector<float> & vsz, 
		     const AimsVector<short, 3> & pt );
    void learnTalVoxel( double x, double y, double z );

  private:
    struct Private;
    Private	*d;
  };

  //	inline

  inline Domain* DomainRBF::clone() const
  {
    return( new DomainRBF( *this ) );
  }


  inline DomainRBF::~DomainRBF()
  {
  }

}

#endif


