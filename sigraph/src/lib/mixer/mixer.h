
#ifndef SI_MIXER_MIXER_H
#define SI_MIXER_MIXER_H


#include <cartobase/object/object.h>

class Tree;


namespace sigraph
{

  /**	Mixeur d'experts.

	Cette classe effectue les mélanges de sorties / d'apprentissage entre 
	un jeu d'éléments adaptatifs. Il peut s'agir d'un vrai système de 
	"mixture of experts", ou d'un simple moyennage, ou de toute autre 
	bidouille qui prend une décision en fin de compte.
  */
  class Mixer
  {
  public:
    virtual ~Mixer();

    virtual Mixer* clone() const = 0;
    virtual const std::string typeName() const = 0;

    virtual float mix( const std::vector<double> & vec ) = 0;
    virtual void buildTree( Tree & tr ) = 0;

  protected:
    Mixer();
    Mixer( const Mixer & mix );
    Mixer & operator = ( const Mixer & mix );

  private:
  };

  //	inline

  inline Mixer::Mixer()
  {
  }


  inline Mixer::Mixer( const Mixer & )
  {
  }


  inline Mixer & Mixer::operator = ( const Mixer & )
  {
    return( *this );
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::Mixer * )
}

#endif

