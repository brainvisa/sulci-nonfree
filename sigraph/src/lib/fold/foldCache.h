
#ifndef SI_FOLD_FOLDCACHE_H
#define SI_FOLD_FOLDCACHE_H


#include <si/graph/cliqueCache.h>
#include <vector>


namespace sigraph
{

  /**	Cache de clique de sillon
   */
  class FoldCache : public CliqueCache
  {
  public:
    FoldCache();
    virtual ~FoldCache();

    virtual CliqueCache* clone() const;

    std::vector<double>	inputVector;
    bool		vecValid;
    std::vector<bool>	subVecValid;

  protected:

  private:
  };


  //	inline

  inline CliqueCache* FoldCache::clone() const
  {
    return( new FoldCache );
  }

}

#endif


