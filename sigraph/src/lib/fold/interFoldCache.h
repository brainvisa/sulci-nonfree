
#ifndef SI_FOLD_INTERFOLDCACHE_H
#define SI_FOLD_INTERFOLDCACHE_H


#include <si/graph/cliqueCache.h>
#include <vector>


namespace sigraph
{
  class InterFoldDescr;

  /**	Cache de clique de relation sillon-sillon
   */
  class InterFoldCache : public CliqueCache
  {
  public:
    ///
    InterFoldCache();
    ///
    virtual ~InterFoldCache();

    ///
    virtual CliqueCache* clone() const;

    ///
    std::vector<double>	inputVector;
    ///
    bool			vecValid;
    ///
    std::vector<bool>		subVecValid;
    ///	Changements taille sillon 1
    double		cSize1;
    ///
    double		cSize2;
    ///	Changements taille relations corticales
    double		cSzCort;
    ///	Changements taille relations jonction
    double		cSzJunc;

  protected:

  private:
  };


  //	inline

  inline CliqueCache* InterFoldCache::clone() const
  {
    return( new InterFoldCache );
  }

}

#endif


