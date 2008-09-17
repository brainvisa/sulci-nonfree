
#ifndef SI_MIXER_MEANMIXER_H
#define SI_MIXER_MEANMIXER_H


#include <si/mixer/mixer.h>


namespace sigraph
{

  /**	Mixeur moyen. 

	Cette classe effectue les mélanges de sorties / d'apprentissage entre 
	un jeu d'éléments adaptatifs. Ici le mélange est juste une moyenne.
  */
  class MeanMixer : public Mixer
  {
  public:
    MeanMixer();
    MeanMixer( const MeanMixer & mix );
    virtual ~MeanMixer();

    virtual Mixer* clone() const;
    MeanMixer & operator = ( const MeanMixer & mix );

    const std::string typeName() const { return( "mean_mixer" ); }
    virtual float mix( const std::vector<double> & vec );

    virtual void buildTree( Tree & tr );

  protected:

  private:
  };

  //	inline

  inline MeanMixer::MeanMixer() : Mixer()
  {
  }


  inline MeanMixer::MeanMixer( const MeanMixer & ) : Mixer()
  {
  }


  inline Mixer* MeanMixer::clone() const
  {
    return( new MeanMixer( *this ) );
  }


  inline MeanMixer & MeanMixer::operator = ( const MeanMixer & )
  {
    return( *this );
  }


  inline float MeanMixer::mix( const std::vector<double> & vec )
  {
    std::vector<double>::const_iterator	i, f=vec.end();
    float	m = 0;

    for( i=vec.begin(); i!=f; ++i ) m += *i;

    return( m / vec.size() );
  }

}

#endif

