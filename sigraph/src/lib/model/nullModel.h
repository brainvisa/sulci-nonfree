
#ifndef SI_MODEL_NULLMODEL_H
#define SI_MODEL_NULLMODEL_H


#include <si/model/model.h>


namespace sigraph
{

  ///	Elément non-adaptatif, qui ne fait RIEN.
  class NullModel : public Model
  {
  public:
    NullModel( Model* parent = 0 );
    NullModel( const NullModel & m );
    virtual ~NullModel();

    virtual Model* clone() const;

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, 
			 const std::map<Vertex*, std::string> & changes );
    virtual void buildTree( Tree & tr ) const;
  };


  //	inline

  inline NullModel::NullModel( Model* parent ) : Model( parent )
  {
  }


  inline NullModel::NullModel( const NullModel & m ) : Model( m )
  {
  }


  inline Model* NullModel::clone() const
  {
    return( new NullModel( *this ) );
  }


  inline double NullModel::prop( const Clique* )
  {
    return( 0 );
  }


  inline double NullModel::prop( const Clique*, 
				 const std::map<Vertex*, std::string> & )
  {
    return( 0 );
  }

}

#endif


