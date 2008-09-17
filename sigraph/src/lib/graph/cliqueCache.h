
#ifndef SI_GRAPH_CLIQUECACHE_H
#define SI_GRAPH_CLIQUECACHE_H

#include <cartobase/object/object.h>

namespace sigraph
{

  /**	Classe fourre-tout qui stocke des résultats plus ou moins 
	intermédiaires de calculs sur les cliques. 

	Le système d'utilisation "classique" pour l'apprentissage est le 
	suivant:
	- un cache est mis dans la clique originale (sans bruitages et 
	  transformations), dans l'attribut \c "original_cache", 
	  c'est un pointeur qui est stocké pour permettre l'accès en 
	  écriture dans une clique const (ce qui est une violation 
	  notoire de la protection du C++ mais bon, c'est pratique)). 
	  Le cache original n'est pas copié quand la clique est 
	  dupliquée, les deux pointent sur le même.
	- un autre cache est mis dans les cliques trafiquées pour y 
	  stocker les résultats temporaires sur les cliques après 
	  modifications, dans l'attribut "{\tt cache}"
	- à la destruction de la clique, les objets pointés par les 
	  attributs \c "cache" et \c "original_cache" sont 
	  détruits; en fait \c "original_cache" n'est détruit 
	  que si la clique n'est pas une copie (attribut \c "is_copy" 
	  absent)
	- les caches ne sont pas pré-crées: on les fabrique quand on 
	  en a besoin
  */
  class CliqueCache
  {
  public:
    CliqueCache();
    virtual ~CliqueCache();

    virtual CliqueCache* clone() const = 0;

  protected:

  private:
  };

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::CliqueCache * )
}

#endif


