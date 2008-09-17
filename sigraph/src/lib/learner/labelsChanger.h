
#ifndef SI_LEARNER_LABELSCHANGER_H
#define SI_LEARNER_LABELSCHANGER_H


#include <si/learner/noiser.h>

class Vertex;

namespace sigraph
{
  class CliqueCache;

  /**	Change des labels dans les cliques
   */
  class LabelsChanger : public Noiser
  {
  public:
    LabelsChanger() : Noiser( "labels_changer" ) {}
    virtual ~LabelsChanger() {}

    ///	@return	distance entre l'exemple d'origine et celui généré
    virtual double noise( Clique* cl, double & outp );
    virtual double openNoise( Clique* cl, double & outp );
    virtual double constrainedNoise( Clique* cl, double & outp, 
				     const std::set<std::string> & 
				     significantLabels, 
				     const std::string & voidLabel );
    ///	Intersection de sets
    template<class T> void intersection( const std::set<T> & s1, 
					 const std::set<T> & s2, 
					 std::set<T> & s3 ) const;
    /**	Tirer un nombre entre 0 et n inclus, avec plus de chances pour les 
	petits nombres */
    virtual unsigned randomGen( unsigned n );

    ///	Distance de l'exemple changé à celui d'origine
    virtual double distance( Vertex* v, const std::string & oldlabel, 
			     const std::string & newlabel, CliqueCache* cc = 0 );

  protected:
    LabelsChanger( const std::string & syntax ) : Noiser( syntax ) {}

  private:
  };

  //	inline

  inline double LabelsChanger::distance( Vertex*, const std::string &, 
					 const std::string &, CliqueCache * )
  {
    return( 1. );
  }

}

#endif

