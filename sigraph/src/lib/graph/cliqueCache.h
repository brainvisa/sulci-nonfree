
#ifndef SI_GRAPH_CLIQUECACHE_H
#define SI_GRAPH_CLIQUECACHE_H

#include <cartobase/object/object.h>

namespace sigraph
{

  /**	Classe fourre-tout qui stocke des r�sultats plus ou moins 
	interm�diaires de calculs sur les cliques. 

	Le syst�me d'utilisation "classique" pour l'apprentissage est le 
	suivant:
	- un cache est mis dans la clique originale (sans bruitages et 
	  transformations), dans l'attribut \c "original_cache", 
	  c'est un pointeur qui est stock� pour permettre l'acc�s en 
	  �criture dans une clique const (ce qui est une violation 
	  notoire de la protection du C++ mais bon, c'est pratique)). 
	  Le cache original n'est pas copi� quand la clique est 
	  dupliqu�e, les deux pointent sur le m�me.
	- un autre cache est mis dans les cliques trafiqu�es pour y 
	  stocker les r�sultats temporaires sur les cliques apr�s 
	  modifications, dans l'attribut "{\tt cache}"
	- � la destruction de la clique, les objets point�s par les 
	  attributs \c "cache" et \c "original_cache" sont 
	  d�truits; en fait \c "original_cache" n'est d�truit 
	  que si la clique n'est pas une copie (attribut \c "is_copy" 
	  absent)
	- les caches ne sont pas pr�-cr�es: on les fabrique quand on 
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


