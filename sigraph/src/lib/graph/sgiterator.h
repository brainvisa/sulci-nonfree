
#ifndef SI_GRAPH_SGITERATOR_H
#define SI_GRAPH_SGITERATOR_H

#include <si/graph/cgraph.h>
#include <list>


namespace sigraph
{

/**	Itérateur de sous-graphe
 */
class SGIterator
{
public:
  SGIterator() {}
  SGIterator( const std::list< const std::set< Vertex *> * >::const_iterator 
	      & it ) 
    : _iter( it )  {}
  ~SGIterator() {}
  SGIterator & operator = ( const SGIterator & i )
    { if( this != &i ) { _iter = i._iter; } return( *this ); }
  const std::set<Vertex *> * operator * () const
    { return( *_iter ); }
  bool operator == ( const SGIterator & i ) const
    { return( _iter == i._iter ); }
  bool operator != ( const SGIterator & i ) const
    { return( !operator ==( i ) ); }
  SGIterator & operator ++ ()
    { ++_iter;  return( *this ); }

private:
  std::list< const std::set< Vertex *> * >::const_iterator	_iter;
};


/**	Fabrique de sous-graphes
 */
class SGProvider
{
public:
  typedef std::list< const std::set< Vertex *> * >	datatype;
  typedef SGIterator iterator;
  typedef SGIterator const_iterator;

  SGProvider( const CGraph & gr ) : _graph( gr ) {}
  virtual ~SGProvider();

  /**@name	Itérateurs */
  //@{
  ///	Produit des pointeurs d'objets alloués avec new, qu'il faut effacer
  SGIterator begin() const
    { return( SGIterator( _data.begin() ) ); }
  ///	Idem
  SGIterator end() const
    { return( SGIterator( _data.end() ) ); }
  ///	Nombre d'éléments
  unsigned size() const { return( _data.size() ); }
  //@}

  /**@name	Préparation */
  //@{
  /**	A appeler à chaque fois qu'on veut repréparer les données
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'aléatoire) */
  virtual void refresh() = 0;
  //@}

protected:
  datatype		_data;
  const CGraph		& _graph;

private:
};



/**	"Functor" de comparaison aléatoire. \\
	C'est imbitable comme nom, hein ? (cf STL)
*/
class RandComp : public std::binary_function< const std::set<Vertex *> *, 
		 const std::set<Vertex *> *, bool >
{
public:
  bool operator () ( const std::set<Vertex *> *s1, 
		     const std::set<Vertex *> *s2 );
  void reset();

private:
  ///	Tirages au sort
  std::map< const std::set<Vertex *> *, float >	dat;
};



/**	Fabrique de vertex simples, itérés dans un ordre aléatoire
 */
class VertexProvider : public SGProvider
{
public:
  ///	Le constructeur ne crée pas les données: appeler refresh() !
  VertexProvider( const CGraph & gr ) : SGProvider( gr ) {}
  virtual ~VertexProvider();

  /**@name	Préparation */
  //@{
  /**	A appeler à chaque fois qu'on veut repréparer les données
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'aléatoire) */
  virtual void refresh();
  //@}

protected:
  ///	Crée la liste, appelé par refresh au besoin
  virtual void init();
  ///	Détruit les sets dans les listes (bref: fait le ménage)
  virtual void cleanup();

private:
};


/**	Fabrique de cliques de noeuds, itérées dans un ordre aléatoire
 */
class VertexCliqueProvider : public SGProvider
{
public:
  ///	Le constructeur ne crée pas les données: appeler refresh() !
  VertexCliqueProvider( const CGraph & gr ) : SGProvider( gr ) {}
  virtual ~VertexCliqueProvider();

  /**@name	Préparation */
  //@{
  /**	A appeler à chaque fois qu'on veut repréparer les données
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'aléatoire) */
  virtual void refresh();
  //@}

protected:
  ///	Crée la liste, appelé par refresh au besoin
  virtual void init();

private:
};

}

#endif

