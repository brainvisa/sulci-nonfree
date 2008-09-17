
#ifndef SI_MODEL_CONSTMODEL
#define SI_MODEL_CONSTMODEL


#include <si/model/model.h>


namespace sigraph
{

  ///	Modèle très simple: donne un potentiel constant...
  class ConstModel : public Model
  {
  public:
    ConstModel( Model* parent = 0 );
    ConstModel( const ConstModel & m );
    virtual ~ConstModel();

    virtual Model* clone() const;

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, 
			 const std::map<Vertex*, std::string> & changes );
    virtual void buildTree( Tree & tr ) const;

    void setValue( double val );

  protected:

  private:
    ///	Valeur de potentiel
    double	_value;
  };


  //	inline

  inline ConstModel::ConstModel( Model* parent ) : Model( parent ), _value( 0 )
  {
  }


  inline ConstModel::ConstModel( const ConstModel & m ) : Model( m ), 
							  _value( m._value )
  {
  }


  inline Model* ConstModel::clone() const
  {
    return( new ConstModel( *this ) );
  }


  inline double ConstModel::prop( const Clique* )
  {
    return( _value );
  }


  inline double ConstModel::prop( const Clique*, 
				  const std::map<Vertex*, std::string> & )
  {
    return( _value );
  }


  inline void ConstModel::setValue( double val )
  {
    _value = val;
  }

}

#endif


