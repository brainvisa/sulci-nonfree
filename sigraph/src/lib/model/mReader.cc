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

#include <si/global/global.h>
#include <si/model/mReader.h>
#include <si/model/adaptiveTree.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/topAdaptive.h>
#include <si/model/nullModel.h>
#include <si/model/constModel.h>
#include <si/subadaptive/saParser.h>
#include <si/mixer/mixerParser.h>
#include <si/descr/descrParser.h>
#include <si/fold/fdParser.h>
#include <si/dimreductor/dimreductorParser.h>
#include <si/optimizer/optimizerParser.h>
#include <si/optimizer/gridOptimizer.h> //FIXME
#include <si/graph/attrib.h>
#include <aims/def/path.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/exception/ioexcept.h>
#include <cartobase/object/pythonwriter.h>
#include <iostream>
#include <assert.h>

using namespace carto;
using namespace aims;
using namespace sigraph;
using namespace std;


SyntaxSet MReader::syntax;
AttributedReader::HelperSet MReader::helpers( MReader::initHelpers() );

PythonReader::HelperSet
MReader::python_helpers( MReader::initPythonHelpers() );


MReader::MReader( const string & filename, 
		  const TreePostParser::FactorySet & fs, 
		  const SyntaxSet & synt,
		  const AttributedReader::HelperSet& helpers )
  : ExoticTreeReader( filename, synt, helpers), _knownElems( fs )
{
  if( syntax.empty() )
  {
    syntax = initSyntax( Path::singleton().syntax()
      + FileUtil::separator() + "adap.stx" );
  }
  if( syntaxSet().empty() )
    setSyntax( syntax );
}


MReader::MReader( const TreePostParser::FactorySet & fs, 
		  const SyntaxSet & synt,
		  const AttributedReader::HelperSet& helpers )
  : ExoticTreeReader( synt, helpers ), _knownElems( fs )
{
  if( syntax.empty() )
  {
    syntax = initSyntax( Path::singleton().syntax()
    + FileUtil::separator() + "adap.stx" );
  }
  if( syntaxSet().empty() )
    setSyntax( syntax );
}


MReader::~MReader()
{
}

static void	gridOptimizerParametersReader(GenericObject &object,
			const std::string &semantic, carto::DataSource& is)
{
	rc_ptr<DataSource>	ds(&is);
	PythonReader		pr(ds, MReader::syntax,
                                   MReader::python_helpers);
	AttributedObject	*val = NULL;
	try {
		val = new AttributedObject("grid_optimizer_parameters");
		pr.readDictionary(*val);

		ds.release();
		//Object(val) : pointer own to an rcptr object
		object.setProperty(semantic, Object(val));
	} catch (exception &e) {
		ds.release();
		throw;
	}
}

static GenericObject * gridOptimizerParameterReader(GenericObject *,
			const std::string &semantic, carto::PythonReader &pr)
{
	ValueObject<GridOptimizerParameter>  *val = NULL;
	val = new ValueObject<GridOptimizerParameter>
				(GridOptimizerParameter(semantic));
	
	/* Syntax is not needed here because we use gridoptimizerparameter
           type to identify syntax in python reader */
	pr.readDictionary(*val);

	return val;
}


AttributedReader::HelperSet	MReader::initHelpers()
{
	AttributedReader::HelperSet	hs =
		carto::AttributedReader::HelperSet();
	hs["grid_optimizer_parameters"] = &gridOptimizerParametersReader;

	return hs;
}

PythonReader::HelperSet	MReader::initPythonHelpers()
{
	PythonReader::HelperSet	hs =
		carto::PythonReader::HelperSet();
	hs["grid_optimizer_parameter"] = &gridOptimizerParameterReader;

	return hs;
}


sigraph::TreePostParser::FactorySet MReader::baseFactorySet()
{
  SAParser		sap;
  MixerParser		mp;
  FDParser		fd;
  DimReductorParser	dr;
  OptimizerParser	op;
  TreePostParser::FactorySet	fs, fs2 = sap.factories();
  TreePostParser::FactorySet	fs3 = mp.factories();
  TreePostParser::FactorySet	fs4 = fd.factories();
  TreePostParser::FactorySet	fs5 = dr.factories();
  TreePostParser::FactorySet	fs6 = op.factories();

  fs[ "top_adaptive" ] = buildTop;
  fs[ "ad_tree"      ] = buildTree;
  fs[ "ad_leaf"      ] = buildLeaf;
  fs[ "null_model"   ] = buildNull;
  fs[ "const_model"  ] = buildConst;

  fs.insert( fs2.begin(), fs2.end() );
  fs.insert( fs3.begin(), fs3.end() );
  fs.insert( fs4.begin(), fs4.end() );
  fs.insert( fs5.begin(), fs5.end() );
  fs.insert( fs6.begin(), fs6.end() );

  return( fs );
}


Model* MReader::readModel()
{
  Model		*mod = 0;
  Tree		tree;

  try
    {
      readTree( &tree );
    }
  catch( parse_error & e )
    {
      cerr << e.what() << " - file : " << e.filename() << ", line " 
	   << e.line() << endl;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }

  if( !tree.getProperty( "pointer", mod ) )
    {
      cerr << "No Model element has been created!\n";
    }

  return( mod );
}


void MReader::parse( Tree * ao )
{
  //cout << "MReader::parse( Tree * )\n";
  TreePostParser::FactorySet::const_iterator 
    ifs = _knownElems.find( ao->getSyntax() );

  //cout << "syntax: " << ao->getSyntax() << endl;  
  if( ifs == _knownElems.end() )
    cerr << "MReader::parse : unknown syntactic attribute " << ao->getSyntax() 
	 << endl;
  else
    try
      {
        (*ifs).second( (Tree*) ao->getParent(), ao, name() );
      }
    catch( exception & e )
      {
        cerr << e.what() << endl << flush;
        throw;
      }
}


void MReader::parseModel( Model* mod, carto::AttributedObject* parent, 
                          Tree* ao )
{
  Model* adp;
  int	nl;

  if( mod->isAdaptive() && ao->getProperty( SIA_NB_LEARN_DATA, nl ) )
    ((Adaptive *) mod)->setNbLearnData( (unsigned) nl );

 if( parent )
   {
     if( parent->getProperty( "pointer", adp ) )
       {
	 mod->setParent( adp );
	 string	psynt = parent->getSyntax();
	 if( mod->isAdaptive() && psynt == "ad_tree" )
	   ((AdaptiveTree *)adp)->insert( (Adaptive *) mod );
	 else if( psynt == "top_adaptive" )
	   ((TopAdaptive *)adp)->setModel( mod );
	 else
	   cerr << "model parent is NOT a ad_tree or top_ad !\n";
       }
     else
       {
	 cerr << "model parent has no pointer\n";
       }
   }
}


void MReader::buildTop( carto::AttributedObject* parent, Tree* ao, 
			const string & )
{
  if( ao->size() == 0 )
    {
      cerr << "warning : empty TopAdaptive\n";
    }

  TopAdaptive*	ad = new TopAdaptive;
  ao->setProperty( "pointer", (Model *) ad );
  parseModel( ad, parent, ao );

  string	s;
  float		w;

  if( ao->getProperty( "void_label", s ) )
    ad->setVoidLabel( s );

  if( ao->getProperty( "significant_labels", s ) )
    {
      string		sub;
      set<string>	ss;

      while( s.size() > 0 )
	{
	  while( s[0] == ' ' || s[0] == '\t' )
	    s.erase( 0, 1 );	// vide début de chaîne
	  string::size_type pos = s.find( ' ' );
	  if( pos == string::npos )
	    pos = s.size();
	  if( pos != 0 )
	    {
	      sub = s.substr( 0, pos );
	      s.erase( 0, pos );
	      ss.insert( sub );
	    }
	}
      if( ss.size() == 0 )
	cout << "unconsistant significant_labels.\n";
      else
	ad->significantLabels() = ss;
    }
  if( !ao->getProperty( "weight", w ) )
    w = 1.;
  ad->setWeight( w );
}


void MReader::buildTree( carto::AttributedObject* parent, Tree* ao, 
			 const string & )
{
  if( ao->size() == 0 )
    {
      cerr << "warning : empty AdTree\n";
    }
  string meth;
  ao->getProperty( "mix_method", meth );
  AdaptiveTree	*adt = new AdaptiveTree( meth );
  ao->setProperty( "pointer", (Model *) adt );
  parseModel( adt, parent, ao );
}


void MReader::buildLeaf( carto::AttributedObject* parent, Tree* ao, 
			 const string & )
{
  if( ao->size() < 2 || ao->size() > 5 )
    {
      cerr << "warning : AdLeaf nb of children is " << ao->size() 
	   << ", should be between 2 and 5\n";
    }
 AdaptiveLeaf	*adl = new AdaptiveLeaf;
 ao->setProperty( "pointer", (Model *) adl );

 int	n;
 if( ao->getProperty( SIA_LEARN_STATE, n ) )
   adl->setLearnState( (AdaptiveLeaf::State) n );
 else
   adl->setLearnState( AdaptiveLeaf::LEARNING );
 if( !ao->getProperty( SIA_NB_LEARN_DATA_MEMO, n ) )
   n = 0;
 adl->setNDataMemo( n );
 //	lire aussi les workMemo et evalMemo...

 std::vector<float>	vec;
 if( ao->getProperty( "mean", vec ) )
   adl->setMean(vec);
 if( ao->getProperty( "std", vec ) )
   adl->setStd(vec);

 parseModel( adl, parent, ao );
}


void MReader::buildNull( carto::AttributedObject* parent, Tree* ao, 
			 const string & )
{
  if( ao->size() != 0 )
    cerr << "warning : NullModel with children\n";
  NullModel	*nm = new NullModel;

  ao->setProperty( "pointer", (Model *) nm );
  parseModel( nm, parent, ao );
}


void MReader::buildConst( carto::AttributedObject* parent, Tree* ao, 
			  const string & )
{
  if( ao->size() != 0 )
    cerr << "warning : ConstModel with chiuldren\n";

  ConstModel	*nm = new ConstModel;
  float		val;

  ao->setProperty( "pointer", (Model *) nm );
  assert( ao->getProperty( "value", val ) );
  nm->setValue( val );

  parseModel( nm, parent, ao );
}


void MReader::addFactory( const string & synt, TreePostParser::Factory fact )
{
  _knownElems[ synt ] = fact;
}


void MReader::addFactories( const TreePostParser::FactorySet & fs )
{
  TreePostParser::FactorySet::const_iterator	ifs, ffs = fs.end();

  for( ifs=fs.begin(); ifs!=ffs; ++ifs )
    _knownElems[ (*ifs).first ] = (*ifs).second;
}


void MReader::setFactories( const TreePostParser::FactorySet & fs )
{
    _knownElems = fs;
}


void MReader::aliasFactory( const std::string & dest, 
			    const std::string & source )
{
  TreePostParser::FactorySet::iterator	ifc = _knownElems.find( source );
  if( ifc == _knownElems.end() )
    {
      cerr << "aliasFactory: alias an inexistent descriptor: " << source 
	   << endl;
      return;
    }
  _knownElems[ dest ] = ifc->second;
}


const TreePostParser::FactorySet & MReader::factories()
{
  return _knownElems;
}
