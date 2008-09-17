
#ifndef SI_GRAPH_ANNEALEXTENSION_H
#define SI_GRAPH_ANNEALEXTENSION_H

#include <string>


namespace sigraph
{
  class Anneal;

  /**	Extensions au recuit simulé.
	S'occupe des passes spéciales qu'on fait de temps en temps
  */
  class AnnealExtension
  {
  public:
    AnnealExtension( Anneal* ann );
    virtual ~AnnealExtension();
    ///	Passe de recuit spécifique
    virtual void specialStep( unsigned passnum = 0 ) = 0;
    virtual unsigned ntrans() const { return( _ntrans ); }
    virtual unsigned maxTrans() const { return( _maxtrans ); }
    double stepDeltaE() const { return( _stepDeltaE ); }
    virtual std::string name() const = 0;

  protected:
    Anneal	*_anneal;
    unsigned	_ntrans;
    unsigned	_maxtrans;
    double	_stepDeltaE;

  private:
  };

}

#endif



