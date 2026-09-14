
#ifndef SI_GRAPH_SGITERATOR_H
#define SI_GRAPH_SGITERATOR_H

#include <si/graph/cgraph.h>
#include <list>


namespace sigraph
{

/**	It�rateur de sous-graphe
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

  /**@name	It�rateurs */
  //@{
  ///	Produit des pointeurs d'objets allou�s avec new, qu'il faut effacer
  SGIterator begin() const
    { return( SGIterator( _data.begin() ) ); }
  ///	Idem
  SGIterator end() const
    { return( SGIterator( _data.end() ) ); }
  ///	Nombre d'�l�ments
  unsigned size() const { return( _data.size() ); }
  //@}

  /**@name	Pr�paration */
  //@{
  /**	A appeler � chaque fois qu'on veut repr�parer les donn�es
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'al�atoire) */
  virtual void refresh() = 0;
  //@}

protected:
  datatype		_data;
  const CGraph		& _graph;

private:
};



/**	"Functor" de comparaison al�atoire. \\
	C'est imbitable comme nom, hein ? (cf STL)
*/
class RandComp
{
public:
  bool operator () ( const std::set<Vertex *> *s1, 
		     const std::set<Vertex *> *s2 );
  void reset();

private:
  ///	Tirages au sort
  std::map< const std::set<Vertex *> *, float >	dat;
};



/**	Fabrique de vertex simples, it�r�s dans un ordre al�atoire
 */
class VertexProvider : public SGProvider
{
public:
  ///	Le constructeur ne cr�e pas les donn�es: appeler refresh() !
  VertexProvider( const CGraph & gr ) : SGProvider( gr ) {}
  virtual ~VertexProvider();

  /**@name	Pr�paration */
  //@{
  /**	A appeler � chaque fois qu'on veut repr�parer les donn�es
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'al�atoire) */
  virtual void refresh();
  //@}

protected:
  ///	Cr�e la liste, appel� par refresh au besoin
  virtual void init();
  ///	D�truit les sets dans les listes (bref: fait le m�nage)
  virtual void cleanup();

private:
};


/**	Fabrique de cliques de noeuds, it�r�es dans un ordre al�atoire
 */
class VertexCliqueProvider : public SGProvider
{
public:
  ///	Le constructeur ne cr�e pas les donn�es: appeler refresh() !
  VertexCliqueProvider( const CGraph & gr ) : SGProvider( gr ) {}
  virtual ~VertexCliqueProvider();

  /**@name	Pr�paration */
  //@{
  /**	A appeler � chaque fois qu'on veut repr�parer les donn�es
	(par ex. avant chaque boucle begin..end s'il y a qqchose 
	d'al�atoire) */
  virtual void refresh();
  //@}

protected:
  ///	Cr�e la liste, appel� par refresh au besoin
  virtual void init();

private:
};

}

#endif

