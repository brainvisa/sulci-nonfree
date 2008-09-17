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

#ifndef SI_GRAPH_EXOTICTREEWRITER_H
#define SI_GRAPH_EXOTICTREEWRITER_H


#include <cartobase/object/attributed.h>
#include <graph/tree/twriter.h>
#include <graph/tree/tree.h>


namespace sigraph
{

  /**	Classe ExoticTreeWriter : écriture d'arbres "exotiques"
   */
  class ExoticTreeWriter : public TreeWriter
  {
  public:
    ExoticTreeWriter( const std::string & filename, 
		      const carto::SyntaxSet & attr, bool makedir = false );
    virtual ~ExoticTreeWriter();

  protected:
    virtual void writeTree( const Tree* tree, const std::string & type );
    virtual void parse( const carto::AttributedObject* ) {};
    virtual void parseTree( const Tree* ao );

  private:
  };


  inline ExoticTreeWriter & 
  operator << ( ExoticTreeWriter & tw, const Tree & tr )
  { tr.const_accept( tw ); return( tw ); }

}

#endif


