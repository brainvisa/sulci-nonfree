/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/global/global.h>
#include <si/fold/frgReader.h>
#include <si/model/mReader.h>
#include <si/fold/foldFinder.h>
#include <si/fold/fdParser.h>
#include <si/domain/domReader.h>
#include <si/fold/ffParser.h>
#include <graph/graph/graph.h>
#include <si/model/topModel.h>
#include <si/fold/foldFakeRel.h>
#include <si/fold/fattrib.h>
#include <si/sulcalsketch/sulcalsketchattrib.h>
#include <si/sulcalsketch/sulcalsketchfinderparser.h>
#include <si/sulcalsketch/sulcalsketchmodel.h>
#include <si/functionalsketch/functionalsketchattrib.h>
#include <si/functionalsketch/functionalsketchfinderparser.h>
#include <si/functionalsketch/functionalsketchmodel.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchattrib.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchfinderparser.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchmodel.h>
#include <si/sulcalSketch_Arnaud/arnaudattrib.h>
#include <si/sulcalSketch_Arnaud/arnaudfinderparser.h>
#include <si/sulcalSketch_Arnaud/arnaudmodel.h>
#include <aims/def/path.h>
#include <cartobase/stream/fileutil.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


SyntaxSet & FrgReader::syntax()
{
  static SyntaxSet	synt( initSyntax( Path::singleton().syntax() + 
                                          FileUtil::separator() + 
                                          "mgraph.stx" ) );
  return synt;
}


MReader & FrgReader::defaultMReader()
{
  static MReader	mr;
  FDParser		fdp;

  mr.addFactories( fdp.factories() );
  mr.addFactory( SIA_FAKEREL_SYNTAX, &readFakeRel );
  mr.addFactory( SIA_SULCALSKETCH_SIMILARITY_MODEL_SYNTAX, 
                 &SulcalSketchSimilarityModel::buildSimilarity );
  mr.addFactory( SIA_SULCALSKETCH_DATADRIVEN_MODEL_SYNTAX, 
                 &SulcalSketchDataDrivenModel::buildDataDriven );
  mr.addFactory( SIA_FUNCTIONALSKETCH_SIMILARITY_MODEL_SYNTAX,
                 &FunctionalSketchSimilarityModel::buildSimilarity );
  mr.addFactory( SIA_FUNCTIONALSKETCH_DATADRIVEN_MODEL_SYNTAX,
                 &FunctionalSketchDataDrivenModel::buildDataDriven );
  mr.addFactory( SIA_FUNCTIONALSKETCH_LOWERSCALEBEST_MODEL_SYNTAX,
                 &FunctionalSketchLowerScaleModel::buildLowerScale );
  mr.addFactory( SIA_FUNCTIONALSKETCH_INTRAPS_MODEL_SYNTAX,
                 &FunctionalSketchIntraPSModel::buildIntraPS );
  mr.addFactory( SIA_SURFACEBASEDFUNCTIONALSKETCH_SIMILARITY_MODEL_SYNTAX,
                 &SurfaceBasedFunctionalSketchSimilarityModel::buildSimilarity );
  mr.addFactory( SIA_SURFACEBASEDFUNCTIONALSKETCH_DATADRIVEN_MODEL_SYNTAX,
                 &SurfaceBasedFunctionalSketchDataDrivenModel::buildDataDriven );
  mr.addFactory( SIA_SURFACEBASEDFUNCTIONALSKETCH_LOWERSCALEBEST_MODEL_SYNTAX,
                 &SurfaceBasedFunctionalSketchLowerScaleModel::buildLowerScale );
  mr.addFactory( SIA_SURFACEBASEDFUNCTIONALSKETCH_INTRAPS_MODEL_SYNTAX,
                 &SurfaceBasedFunctionalSketchIntraPSModel::buildIntraPS );
  mr.addFactory( SIA_ARNAUD_SIMILARITY_MODEL_SYNTAX,
                 &ArnaudSimilarityModel::buildSimilarity );
  mr.addFactory( SIA_ARNAUD_DATADRIVEN_MODEL_SYNTAX,
                 &ArnaudDataDrivenModel::buildDataDriven );
  mr.addFactory( SIA_ARNAUD_LOWERSCALEBEST_MODEL_SYNTAX,
                 &ArnaudLowerScaleModel::buildLowerScale );
  mr.addFactory( SIA_ARNAUD_INTRAPS_MODEL_SYNTAX,
                 &ArnaudIntraPSModel::buildIntraPS );
  return( mr );
}


TreePostParser::FactorySet FrgReader::defaultFFactories()
{
  FFParser	fp;

  TreePostParser::FactorySet	fs = fp.factories();
  fs[ SIA_SULCALSKETCH_FINDER_SYNTAX ] 
    = &SulcalSketchFinderParser::buildSulcalSketchFinder;
  fs[ SIA_FUNCTIONALSKETCH_FINDER_SYNTAX ] 
    = &FunctionalSketchFinderParser::buildFunctionalSketchFinder;
  fs[ SIA_SURFACEBASEDFUNCTIONALSKETCH_FINDER_SYNTAX ] 
      = &SurfaceBasedFunctionalSketchFinderParser::buildSurfaceBasedFunctionalSketchFinder;
  fs[ SIA_ARNAUD_FINDER_SYNTAX ] 
    = &ArnaudFinderParser::buildArnaudFinder;


  return( fs );
}



FrgReader::FrgReader( const string & filename, MReader & mr, 
		      const TreePostParser::FactorySet & fs ) 
  : ExoticGraphReader( filename, FrgReader::syntax() ), _mreader( &mr ), 
    _ffact( fs )
{
}


FrgReader::FrgReader( MReader & mr, const TreePostParser::FactorySet & fs )
  : ExoticGraphReader( FrgReader::syntax() ), _mreader( &mr ), _ffact( fs )
{
}


FrgReader::~FrgReader()
{
}


void FrgReader::read( FRGraph & gr )
{
  gr.clearAll();

  ExoticGraphReader::read( gr );
}


void FrgReader::parse( Graph & sg, AttributedObject* go )
{
  string		synt, filename;

  synt = go->getSyntax();

  if( go->getProperty( "model_file", filename ) )
    readModel( sg, go, "model", filename );

  if( go->getProperty( "domain_file", filename ) )
    readDomain( sg, go, "domain", filename);
}


void FrgReader::readModel( Graph & sg, AttributedObject* go, 
			   const string & attrib, const string & filename )
{
  //	Trouver le directory
  string	file = FileUtil::dirname( GraphReader::name() );
  string	fbase;

  if( !file.empty() )
    file += FileUtil::separator();
  if( sg.getProperty( "filename_base", fbase ) )
  {
    if( fbase == "*" )
    {
      fbase = FileUtil::basename( name() );
      string::size_type pos = fbase.rfind( '.' );
      if( pos != string::npos )
        fbase = fbase.substr( 0, pos ) + ".data";
      else
        fbase = fbase + ".data";
    }
    file += fbase + FileUtil::separator();
  }
  file += filename;

  /*string nm;

  if( !go->getProperty( "name", nm ) )
  nm = "***";*/

  //	Lire le modele

  _mreader->open( file );
  Model		*mod = _mreader->readModel();
  _mreader->close();

  go->setProperty( attrib, mod );

  TopModel	*tm = dynamic_cast<TopModel *>( mod );
  if( tm )
  {
    tm->setParentAO( go );
    tm->setMGraph( static_cast<MGraph *>( &sg ) );
  }

  /*cout << "Model dans l'element " << nm 
       << " , Synt : " << go->getSyntax() 
       << ", file : " << file << ", ptr = " << ad << endl;*/
}


void FrgReader::readDomain( Graph & sg, AttributedObject* go, 
			    const string & attrib, const string & filename )
{
  //	Trouver le directory
  string	file = FileUtil::dirname( GraphReader::name() ); 
  string	fbase;

  if( !file.empty() )
    file += FileUtil::separator();
  if( sg.getProperty( "filename_base", fbase ) )
  {
    if( fbase == "*" )
    {
      fbase = FileUtil::basename( name() );
      string::size_type pos = fbase.rfind( '.' );
      if( pos != string::npos )
        fbase = fbase.substr( 0, pos ) + ".data";
      else
        fbase = fbase + ".data";
    }
    file += fbase + FileUtil::separator();
  }
  file += filename;

  string nm;

  if( !go->getProperty( "name", nm ) )
    nm = "***";

  //	Lire le domaine

  DomReader		fdr( file );
  Domain		*fd = fdr.readDom();
  go->setProperty( attrib, fd );

  /*cout << "Domain dans l'element " << nm 
       << " , Synt : " << go->getSyntax() 
       << ", file : " << file << ", ptr = " << fd << endl;*/
}


void FrgReader::parse( Graph & sg, Graph* rg )
{
  string	str;
  ModelFinder	*mf;
  string	filename;

  //cout << "Parse graph\n";

  if( !rg->getProperty( "model_finder", str ) )
    {
      str = "fold_finder";
      rg->setProperty( "model_finder", str );
      //cout << "clique_descriptor par defaut\n";
    }

  if( rg->getProperty( "model_finder_ptr", mf ) )
    delete mf;

  // cout << "FrgReader: read finder for " << str << endl;
  TreePostParser::FactorySet::const_iterator	ifs = _ffact.find( str );

  if( ifs != _ffact.end() )
    {
      // cout << "finder factory found\n";
      (*ifs).second( rg, 0, "" );
    }
  else
    {
      mf = new FoldFinder( *(FRGraph *) rg );
      rg->setProperty( "model_finder_ptr", mf );
      cout << "FRGraph : model finder standard v1\n";
    }

  if( rg->getProperty( "default_model_file", filename ) )
    readModel( sg, rg, "default_model", filename );

  //cout << "Parse graph fini\n";
  parse( sg, (GraphObject *) rg );

  Model	*mod;
  if( sg.getProperty( SIA_MODEL, mod ) )
    {
      FoldFakeRel	*ffr = dynamic_cast<FoldFakeRel *>( mod );
      MGraph		*mg = dynamic_cast<MGraph *>( &sg );
      if( ffr && mg )
	{
	  ffr->setMGraph( mg );
	  sg.setProperty( SIA_FAKEREL_MODEL, ffr );
	}
    }
}


void FrgReader::readFakeRel( AttributedObject*, Tree* ao, 
			     const string & )
{
  //cout << "read fake rel\n";
  if( ao->size() != 0 )
    cerr << "warning : FoldFakeRel with children\n";

  FoldFakeRel	*ffr = new FoldFakeRel;
  ao->setProperty( "pointer", (Model *) ffr );
}

// ---------------

LowLevelFRGArgReader::LowLevelFRGArgReader()
  : LowLevelArgReader()
{
}


LowLevelFRGArgReader::~LowLevelFRGArgReader()
{
}


Graph* LowLevelFRGArgReader::read( const string & filename, int )
{
  FrgReader	r( filename );
  FRGraph	*g = new FRGraph;
  try
    {
      r >> *g;
    }
  catch( exception & )
    {
      delete g;
      throw;
    }

  return( g );
}


