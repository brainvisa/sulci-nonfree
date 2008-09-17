
#ifndef SI_GRAPH_VERTEXCLIQUE_H
#define SI_GRAPH_VERTEXCLIQUE_H

#include <set>
#include <graph/graph/vertex.h>
#include <si/graph/clique.h>
// DEBUG
#include <iostream>


namespace sigraph
{

  ///	Composante connexe
  typedef	std::set<Vertex *>	CComponent;


  /**	VertexClique - clique of graph vertices
  */
  class VertexClique : public Clique
  {
  public:
    typedef std::set<Vertex*>::iterator iterator;
    typedef std::set<Vertex*>::const_iterator const_iterator;

    VertexClique();
    VertexClique( const std::set<Vertex*> & vert );
    ///	Ne copie que les attributs, pas les noeuds contenus !
    VertexClique( const VertexClique & cl );
    ~VertexClique();
    virtual void clear();

    virtual Clique* deepCopy() const;

    /**@name	Gestion des noeuds de la clique */
    //@{
    const std::set<Vertex*> & vertices() const;
    void addVertex( Vertex* vert );
    void removeVertex( Vertex* vert );
    const_iterator begin() const;
    const_iterator end() const;
    size_t size() const;
    //@}

    /**@name	Fonctions de recherche */
    //@{
    ///	Find the vertices which contain a given semantic attribute/value pair
    template <class T> std::set <Vertex*> 
    getVerticesWith( const std::string& s, const T& t ) const;
    ///	Find the vertices which contain a given semantic attribute
    std::set <Vertex*> getVerticesWith( const std::string& s ) const;
    /**	Trouve les relations qui relient les noeuds de label {\tt label1} 
	aux noeuds de label {\tt label2}
	@param	label1	1er label
	@param	label2	2e label
	@param	edges	(retour) les relations trouvées y sont stockées */
    virtual void edgesBetweenLabels( const std::string & label1, 
				     const std::string & label2, 
				     std::set<Edge *> & ed ) const;
    ///	Trouve les relations entre deux sous-ensembles
    virtual void edgesBetween( const std::set<Vertex *> & s1, 
			       const std::set<Vertex *> & s2, 
			       std::set<Edge *> & ed ) const;
    //@}

    /**@name	Composantes connexes */
    //@{
    /**	Calcule les composantes connexes pour une étiquette. 
	@param	label	étiquette pour laquelle on cherche les composantes
	@param	sc	si non NULL, la fonction remplit la liste des 
	composantes, il faut les effacer...
	@param	syntType	Type syntaxique de relation concerné. Si ce 
	paramètre n'est pas donné, on connecte par 
	cherche n'importe quel type de relation.
	@return	nombre de composantes connexes
    */
    virtual unsigned connectivity( const std::string & label, 
				   std::set<CComponent *> *sc = 0, 
				   const std::string & SyntType = "" ) const;
    ///	Calcule les composantes connexes pour un ensemble de noeuds
    static unsigned connectivity( const std::set<Vertex *> & vx, 
				  std::set<CComponent *> *sc = 0, 
				  const std::string & syntType = "" );
    /**	Calcule les composantes connexes pour un ensemble de noeuds et de 
	syntaxes de relations */
    static unsigned connectivity( const std::set<Vertex *> & vx, 
				  std::set<CComponent *> *sc, 
				  const std::set<std::string> & syntTypes );
    /**	Propagation d'une composante connexe. 
	@param	v	noeud d'origine de la composante (graine)
	@param	vx	ensemble des noeuds possibles
	@param	done	ceux qui ont déjà été traités
	@param	cc	composante connexe à remplir (si non NULL)
    */
    static void connPropagate( Vertex* v, const std::set<Vertex *> & vx, 
			       std::set<Vertex *> & done, CComponent *cc, 
			       const std::set<std::string> & syntTypes );
    //@}

  protected:
    std::set<Vertex*>	_vertices;

    ///	Les itérateurs non-constants sont en accès protégé.
    iterator begin();
    ///	Les itérateurs non-constants sont en accès protégé.
    iterator end();

  private:
  };

  //	Fonctions inline

  inline VertexClique::VertexClique() : Clique()
  {
  }


  inline const std::set<Vertex*> & VertexClique::vertices() const
  {
    return _vertices;
  }


  inline VertexClique::const_iterator VertexClique::begin() const
  {
    return _vertices.begin();
  }


  inline VertexClique::iterator VertexClique::begin()
  {
    return _vertices.begin();
  }


  inline VertexClique::const_iterator VertexClique::end() const
  {
    return _vertices.end();
  }


  inline VertexClique::iterator VertexClique::end()
  {
    return _vertices.end();
  }


  inline size_t VertexClique::size() const
  {
    return _vertices.size();
  }


  template <class T> inline std::set <Vertex*> 
  VertexClique::getVerticesWith( const std::string& s, const T& t ) const
  {
    std::set<Vertex*> vertices;

    for( const_iterator v = begin(); v != end(); ++v )
      {
        carto::Object tmp = (*v)->getProperty( s );
        if( !tmp )
          std::cerr << "in vertex " << *v << ": prop " << s 
                    << " is None\n";
        else
          try
            {
              if( tmp->GenericObject::value<T>() == t )
                vertices.insert( *v );
            }
          catch( ... )
            {
            }
      }

    return vertices;
  }

}

namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( std::set<sigraph::VertexClique *> * )
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::VertexClique * )
}

#endif
