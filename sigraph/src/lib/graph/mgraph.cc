
#include <si/graph/mgraph.h>
#include <si/model/adaptive.h>
#include <si/finder/modelFinder.h>
#include <si/domain/domain.h>
#include <si/model/adaptive.h>
#include <si/domain/adapDomain.h>
#include <si/graph/attrib.h>
#include <cartobase/stream/fileutil.h>
#include <iostream>
#include <stdio.h>

using namespace sigraph;
using namespace carto;
using namespace std;


MGraph::~MGraph()
{
  iterator	in, fn=end();

  //	attributs globaux graphe
  parseDelete( this );

  //	noeuds
  for ( in=begin(); in!=fn; ++in )
    parseDelete( *in );

  //	relations
  set<Edge*> ed = edges();
  set<Edge*>::iterator	ir, fr=ed.end();

  for ( ir=ed.begin(); ir!=fr; ++ir )
    parseDelete( *ir );
}


void MGraph::parseDelete( AttributedObject *ao )
{
  Model	*ptr;
  ModelFinder	*mf;
  Domain	*dom;

  if ( ao->getProperty( "model", ptr ) )
    delete ptr;

  if ( ao->getProperty( "model_finder_ptr", mf ) )
    delete mf;

  if ( ao->getProperty( "domain", dom ) )
    delete dom;
}


void MGraph::clearAll()
{
  iterator	in, fn=end();

  //	attributs globaux graphe
  parseDelete( this );

  //	noeuds
  for ( in=begin(); in!=fn; ++in )
    parseDelete( *in );

  //	relations
  set<Edge*> ed = edges();
  set<Edge*>::iterator	ir, fr=ed.end();

  for ( ir=ed.begin(); ir!=fr; ++ir )
    parseDelete( *ir );

  clearProperties();
  clear();
}


string MGraph::domainFile( const string & graphname, 
			   const AttributedObject* ao )
{
  string	endn;

  if ( ao->getProperty( "domain_file", endn ) )
    {
      //	prefix
      string	name = FileUtil::dirname( graphname );

      if ( !name.empty() )
        name += FileUtil::separator();
      name += endn;
      return name;
    }
  else
    return "";
}


string MGraph::modelFile( const string & graphname, 
			  const AttributedObject* ao, 
			  set<string> & otherFiles )
{
  string	endn;

  if ( ao->getProperty( "model_file", endn ) )
    {
      //	prefix
      string	name = FileUtil::dirname( graphname );
      Model	*mod;

      if ( !name.empty() )
        name += FileUtil::separator();

      otherFiles.erase( otherFiles.begin(), otherFiles.end() );

      if ( ao->getProperty( "model", mod ) )
	{
	  string	tmp = FileUtil::dirname( name + endn );
	  mod->subFiles( tmp, otherFiles );
	}

      name += endn;
      return name;
    }
  else
    return "";
}


void MGraph::closeLearning()
{
  const_iterator	imv, fmv=end();
  Adaptive		*ad;
  Model			*mod;

  cout << "Closing learning\n";

  for ( imv=begin(); imv!=fmv; ++imv )
    {
      if ( (*imv)->getProperty( "model", mod ) )
	{
	  ad = dynamic_cast<Adaptive *>( mod );
	  if ( ad )
	    ad->forceLearnFinished();
	}
    }

  //	pareil pour les relations...
  set<Edge *>::const_iterator	ime, fme=edges().end();

  for ( ime=edges().begin(); ime!=fme; ++ime )
    {
      if ( (*ime)->getProperty( "model", mod ) )
	{
	  ad = dynamic_cast<Adaptive *>( mod );
	  if ( ad )
	    ad->forceLearnFinished();
	}
    }
}


void MGraph::initDomain()
{
  iterator	iv, fv=end();
  Domain	*domain;
  AdapDomain	*adapDom;

  for ( iv=begin(); iv!=fv; ++iv )
    if ( (*iv)->getProperty( "domain", domain ) )
      {
	adapDom = dynamic_cast<AdapDomain *>( domain );
	if ( adapDom )
	  adapDom->reset();
      }
}


void MGraph::initAdap()
{
  iterator	iv, fv=end();
  Model		*model;
  Adaptive	*adaptive;

  for ( iv=begin(); iv!=fv; ++iv )
    if ( (*iv)->getProperty( "model", model ) )
      {
	adaptive = dynamic_cast<Adaptive *>( model );
	if ( adaptive )
	  adaptive->init();
      }

  set<Edge *>::const_iterator	ie, fe=edges().end();

  for ( ie=edges().begin(); ie!=fe; ++ie )
    if ( (*ie)->getProperty( "model", model ) )
      {
	adaptive = dynamic_cast<Adaptive *>( model );
	if ( adaptive )
	  adaptive->init();
      }
}


void MGraph::initStats()
{
  iterator			iv, fv=end();
  Model				*model;
  Adaptive			*adaptive;
  set<Edge*>::const_iterator	ie, fe = edges().end();

  for ( iv=begin(); iv!=fv; ++iv )
    if ( (*iv)->getProperty( "model", model ) )
      {
	adaptive = dynamic_cast<Adaptive *>( model );
	if ( adaptive )
	  adaptive->resetStats();
      }

  for ( ie = edges().begin(); ie!=fe; ++ie )
    if ( (*ie)->getProperty( "model", model ) )
      {
	adaptive = dynamic_cast<Adaptive *>( model );
	if ( adaptive )
	  adaptive->resetStats();
      }
}


void MGraph::removeUnusedModels( bool, const string & prefix )
{
  iterator		iv, iv2, fv=end();
  // Vertex::iterator	ie, fe;
  Domain		*dom;
  AdapDomain		*adapDom;
  bool			mustRemove;
  string		filename;
  set<string>		other;
  set<string>::iterator	io, fo;

  for ( iv=begin(); iv!=fv; )
    {
      mustRemove = false;
      if ( (*iv)->getProperty( SIA_DOMAIN, dom ) )
	{
	  adapDom = dynamic_cast<AdapDomain *>( dom );
	  if ( adapDom && adapDom->nData() == 0 )
	    mustRemove = true;

	  if ( mustRemove )
	    {
	      filename = domainFile( prefix, *iv );
	      if ( !filename.empty() )
		remove( filename.c_str() );
	      filename = modelFile( prefix, *iv, other );
	      if ( !filename.empty() )
		{
		  remove( filename.c_str() );
		  for ( io=other.begin(), fo=other.end(); io!=fo; ++io )
		    remove( (*io).c_str() );
		}
	      parseDelete( *iv );
	      iv2 = iv;
	      ++iv;
	      removeVertex( *iv2 );
	    }
	  else
	    ++iv;
	}
      // else edges...
      else
	++iv;
    }
}


void MGraph::setWeights( double fac )
{
  MGraph::const_iterator	iv, fv=end();
  Model				*mod;
  TopModel			*tm;

  for ( iv=begin(); iv!=fv; ++iv )
    {
      (*iv)->getProperty( "model", mod );
      tm = mod->topModel();
      if ( tm )
	tm->setWeight( ((double) (*iv)->size()) * fac / 2 );
    }
}


void MGraph::removeWeights()
{
  MGraph::const_iterator	iv, fv=end();
  Model				*mod;
  TopModel			*tm;

  for ( iv=begin(); iv!=fv; ++iv )
    {
      (*iv)->getProperty( "model", mod );
      tm = mod->topModel();
      if ( tm )
	tm->setWeight( 1. );
    }
}


unsigned MGraph::removeRareEdges( float freqmin )
{
  unsigned 			nr = 0;
  const set<Edge *>		& e = edges();
  set<Edge *>::const_iterator	ie, fe=e.end(), ie2;
  int				cnt, ng;

  if ( !getProperty( SIA_NBASEGRAPHS, ng ) || ng == 0 )
    {
      cout << "Pas de nombre de graphes dans les stats\n";
      return 0;
    }

  for ( ie=e.begin(); ie!=fe; ++ie )
    {
      if ( (*ie)->getProperty( SIA_NOINSTANCE_COUNT, cnt ) 
	  && 1.-((float)cnt)/ng <= freqmin )
	{
	  ie2 = ie;
	  --ie;
	  parseDelete( *ie2 );
	  removeEdge( *ie2 );
	}
    }

  return nr;
}


void MGraph::removeEdgesToVoid()
{
  const set<Edge *>		& se = edges();
  set<Edge *>::const_iterator	ie, fe=se.end();
  Edge::const_iterator		iv;
  string			label1, label2;
  bool				remove, firstremoved = false;
  Edge				*edge;
  unsigned			i;
  string			voidl = SIV_VOID_LABEL;

  getProperty( SIA_VOID_LABEL, voidl );
  cout << "Elimination des relations avec '" << voidl << "'...\n";
  //cout << "Nb : " << order() << endl;

  for ( ie=se.begin(), i=0; ie!=fe; ++ie, ++i )
    {
      if ( firstremoved )
	{
	  ie = se.begin();
	  //cout << "on revient au debut.\n";
	  firstremoved = false;
	}
      //cout << i << "  ";
      iv = (*ie)->begin();
      remove = false;
      assert( (*iv)->getProperty( SIA_LABEL, label1 ) );
      ++iv;
      assert( (*iv)->getProperty( SIA_LABEL, label2 ) );
      if ( label1 == voidl )
	remove = true;
      else
	{
	  if ( label2 == voidl )
	    remove = true;
	}
      if ( remove )
	{
	  edge = *ie;
	  cout << "Removing " << label1 << "  -  " << label2 << endl;
	  if ( ie == se.begin() )
	    firstremoved = true;
	  else
	    --ie;
	  parseDelete( edge );
	  removeEdge( edge );
	}
    }
  cout << "OK.\n";
}


namespace
{

  vector<int> string2version( const string & v )
  {
    vector<int>	vn;
    if( v.empty() )
      return vn;
    string::size_type	pos = 0, pos2 = 0, n = v.length();
    string		num;
    int			x;

    while( pos <= n && pos != string::npos )
      {
        pos2 = pos;
        pos = v.find( '.', pos2 );
        if( pos != string::npos )
          {
            num = v.substr( pos2, pos - pos2 );
            ++pos;
          }
        else
          num = v.substr( pos2, n - pos2 );
        istringstream	ss( num );
        ss >> x;
        vn.push_back( x );
      }

    return vn;
  }


  int versioncmp( const vector<int> & v1, const vector<int> & v2 )
  {
    int	i, n1 = v1.size(), n2 = v2.size(), m = std::max( n1, n2 );
    int	x1, x2;

    for( i=0; i<m; ++i )
      {
        if( i < n1 )
          x1 = v1[i];
        else
          x1 = 0;
        if( i < n2 )
          x2 = v2[i];
        else
          x2 = 0;
      if( x1 < x2 )
        return -1;
      else if( x1 > x2 )
        return 1;
      }
    // equal
    return 0;
  }

}


MGraph::VersionCheck MGraph::checkCompatibility( const Graph & data ) const
{
  VersionCheck	res;
  string	v1, v2, cv1, cv2;

  getProperty( SIA_MODEL_VERSION, v1 );
  getProperty( SIA_MODEL_COMPAT_DATA_VERSION, cv1 );
  if( cv1.empty() )
    cv1 = v1;
  if( !data.getProperty( SIA_DATAGRAPH_VERSION, v2 ) 
      && !data.getProperty( "CorticalFoldArg_VERSION", v2 ) 
      && !data.getProperty( "RoiArg_VERSION", v2 ) 
      && !data.getProperty( "ClusterArg_VERSION", v2 ) 
      )
    data.getProperty( "NucleusArg_VERSION", v2 );
  data.getProperty( SIA_DATAGRAPH_COMPAT_MODEL_VERSION, cv2 );
  if( cv2.empty() )
    cv2 = v2;

  static vector<int> minver;
  if( minver.empty() )
    {
      minver.reserve(2);
      minver.push_back(3);
      minver.push_back(1);
    }

  if( v1.empty() )
    {
      if( v2.empty() )
        {
          // no version at all is considered OK.
          res.ok = true;
          return res;
        }
      vector<int>	v2n = string2version( v2 );
      if( versioncmp( v2n, minver ) < 0 )
        {
          // oldest data verstions were not checked, so are OK
          res.ok = true;
          res.data = VersionOk;
          return res;
        }
      res.message = string( "Outdated model with no version is incompatible " 
                            "with data version " ) + v2;
      res.data = VersionOk;
      return res;
    }
  if( v2.empty() )
    {
      res.message = string( "Outdated data with no version is incompatible " 
                            "with model version " ) + v1;
      res.model = VersionOk;
      return res;
    }
  // here both the model and data have a version
  vector<int>	v1n = string2version( v1 );
  vector<int>	cv1n = string2version( cv1 );
  vector<int>	v2n = string2version( v2 );
  vector<int>	cv2n = string2version( cv2 );
  if( versioncmp( v1n, cv2n ) < 0 )
    {
      res.model = Outdated;
      res.data = VersionOk;
      res.message = string( "Outdated model (version " ) + v1 
        + ") is incompatible with data version " + v2 
        + " (minimum model version expected: " + cv2 + ")";
      return res;
    }
  if( versioncmp( v2n, cv1n ) < 0 )
    {
      res.model = VersionOk;
      res.data = Outdated;
      res.message = string( "Outdated data (version " ) + v2 
        + ") is incompatible with model version " + v1 
        + " (minimum data version expected: " + cv1 + ")";
      return res;
    }

  // here everything should be OK
  res.model = VersionOk;
  res.data = VersionOk;
  res.ok = true;
  return res;
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( MGraph * )

