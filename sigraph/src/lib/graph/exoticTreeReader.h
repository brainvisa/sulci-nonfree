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

#ifndef SI_GRAPH_EXOTICTREEREADER_H
#define SI_GRAPH_EXOTICTREEREADER_H


#include <cartobase/object/attributed.h>
//#include <cartobase/object/areader.h>
#include <graph/tree/treader.h>
#include <graph/tree/tree.h>


namespace sigraph
{

  /**	Classe ExoticTreeReader : classe de base de lecture d'arbres 
	"exotiques".

	Cette structure de base permet d'encapsuler la lecture d'un arbre, 
	avec une étape de post-processing au cours de laquelle les éléments de 
	l'arbre sont scrutés et éventuellement complétés ou transformés
  */
  class ExoticTreeReader : public TreeReader
  {
  public:
    ExoticTreeReader( const std::string & filename, 
		      const carto::SyntaxSet & attr,
		      const carto::AttributedReader::HelperSet& helpers
		      = carto::AttributedReader::HelperSet());
    ExoticTreeReader( const carto::SyntaxSet & attr,
		      const carto::AttributedReader::HelperSet& helpers
		      = carto::AttributedReader::HelperSet());
    ExoticTreeReader( const TreeFactory & factory, 
		      const std::string & filename, 
		      const carto::SyntaxSet & attr,
		      const carto::AttributedReader::HelperSet& helpers
		      = carto::AttributedReader::HelperSet());
    ExoticTreeReader( const TreeFactory & factory, 
		      const carto::SyntaxSet & attr,
		      const carto::AttributedReader::HelperSet& helpers
		      = carto::AttributedReader::HelperSet());
    virtual ~ExoticTreeReader();

    ///	Crée l'arbre et le lit
    virtual Tree* read();

    static carto::SyntaxSet initSyntax( const std::string & filename );

  protected:
    virtual void readTree( Tree* t );
    void parseTree( Tree* tr );
    virtual void parse( Tree* go ) { parse( (carto::AttributedObject *) go ); };
    virtual void parse( carto::AttributedObject* ) {};

  private:
  };


  inline ExoticTreeReader & operator >> ( ExoticTreeReader & er, Tree & t )
  { t.accept( er ); return( er ); }

}

#endif

