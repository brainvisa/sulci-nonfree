
#ifndef SI_FINDER_ADAPFINDER_H
#define SI_FINDER_ADAPFINDER_H


#include <si/finder/modelFinder.h>
#include <si/model/learnParam.h>


namespace sigraph
{

  class LearnConstParam;

  /**	S�lecteur de mod�les adaptatifs (Adaptive). 

	Avec ce type de s�lecteur, on acc�de aux fonctions d'apprentissage
  */
  class AdapFinder : public ModelFinder
  {
  public:
    AdapFinder( MGraph & rg );
    virtual ~AdapFinder();

    /**	Apprends le MGraph � partir d'une clique
	@param	cl	clique � d�crire
	@param	outp	sortie d'apprentissage (potentiel voulu pour la clique)
	@return		erreur d'apprentissage
    */

    ///	peut apprendre ?
    virtual bool isAdaptive() const;
  };


  //	Fonctions inline

  inline bool AdapFinder::isAdaptive() const
  {
    return( true );
  }

}

#endif


