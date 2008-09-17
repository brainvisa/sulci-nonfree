/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_GRAPH_EXOTICGRAPHREADER_H
#define SI_GRAPH_EXOTICGRAPHREADER_H


#include <cartobase/object/attributed.h>
#include <graph/graph/greader.h>
class Vertex;
class Edge;


namespace sigraph
{

  /**	Classe ExoticGraphReader : classe de base de lecture de graphes 
	"exotiques".

	Cette structure de base permet d'encapsuler la lecture d'un graphe, 
	avec une étape de post-processing au cours de laquelle les éléments du 
	graphe sont scrutés et éventuellement complétés
  */
  class ExoticGraphReader : public GraphReader
  {
  public:
    ExoticGraphReader( const std::string & filename, 
		       const carto::SyntaxSet & attr );
    ExoticGraphReader( const carto::SyntaxSet & attr );
    virtual ~ExoticGraphReader();

    static carto::SyntaxSet initSyntax( const std::string & filename );

  protected:
    virtual void read( Graph & gr );
    virtual void parse( Graph & sg, carto::AttributedObject* go ) = 0;
    virtual void parse( Graph & sg, Vertex* go );
    virtual void parse( Graph & sg, Edge* go );
    virtual void parse( Graph & sg, Graph* go );

  private:
  };

  //	Fonctions inline

  inline void ExoticGraphReader::parse( Graph & sg, Vertex* go )
  {
    parse( sg, (carto::AttributedObject *) go );
  }


  inline void ExoticGraphReader::parse( Graph & sg, Edge* go )
  {
    parse( sg, (carto::AttributedObject *) go );
  }


  inline void ExoticGraphReader::parse( Graph & sg, Graph* go )
  {
    parse( sg, (carto::AttributedObject *) go );
  }

}

#endif

