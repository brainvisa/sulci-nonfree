/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <cstdlib>
#include <si/global/global.h>
#include <si/model/mReader.h>
#include <si/domain/domReader.h>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <si/fold/foldReader.h>
#include <si/fold/fdParser.h>
#include <aims/getopt/getopt2.h>
#include <cartobase/exception/assert.h>
#include <cartobase/exception/parse.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/sstream.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


struct Params
{
  string	mgraphFile;
  string	attribute;
  string	pattern;
  string	modelFile;
};


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( EXIT_FAILURE );
}


void loadParams( const string & paramFile, const char* name, Params & params )
{
  SyntaxReader	pr( si().basePath() + "/config/siChangeModels.stx" );
  SyntaxSet	ps;

  pr >> ps;

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	gf, str;

  tr >> t;

  if( !t.getProperty( "mgraph", params.mgraphFile ) )
    paramError( name, paramFile, "mgraph" );
  if( !t.getProperty( "attribute", params.attribute ) )
    params.attribute = "label";
  if( !t.getProperty( "pattern", params.pattern ) )
    paramError( name, paramFile, "pattern" );
  if( !t.getProperty( "model", params.modelFile ) )
    paramError( name, paramFile, "model" );
}


int main( int argc, const char** argv )
{
  try
    {
      Params	params;
      string	paramfile;

      AimsApplication	app( argc, argv, "Replaces models in some model " 
                             "elements according to a matching criterion on " 
                             "a specific attribute" );

      string	p = "input parameters file (tree format) "
        "(richer than other parameter given on the commandline)";
      ifstream	ps( (si().basePath() 
                     + "/config/siChangeModels.stx" ).c_str() );
      if( ps )
        {
          char		s[1024];
          unsigned	i, n;
          p += "\nParameter file attributes:\n";
          while( !ps.eof() )
            {
              ps.getline( s, 1023 );
              if( s[0] != '\n' && s[0] != '\0' )
                {
                  string	s2( s );
                  for( i=0, n=s2.length(); i<n && ( s2[i]==' ' || s2[i]=='\t' );
                       ++i ) {}
                  if( s2.substr( i, i+6 ) != "*BEGIN" 
                      && s2.substr( i, i+4 ) != "*END" )
                    {
                      p += "\n";
                      p += s2.substr( i, s2.length()-i );
                    }
                }
            }
        }
      app.addOption( paramfile, "-p", p, true );
      app.addOption( params.mgraphFile, "-g", "model graph file", true );
      app.addOption( params.attribute, "-a", 
                     "model attribute to discriminate on", true );
      app.addOption( params.pattern, "-e", 
                     "regular expression to match model elements for changes", 
                     true );
      app.addOption( params.modelFile, "-m", 
                     "model file to replace matched models (.mod)", true );

      app.initialize();

      if( !paramfile.empty() )
        loadParams( paramfile, argv[0], params );
      bool	ok = true;
      if( params.mgraphFile.empty() )
	{
	  cerr << "A model graph must be provided" << endl;
          ok = false;
	}
      if( params.modelFile.empty() )
	{
	  cerr << "A model file must be provided" << endl;
          ok = false;
	}
      if( params.attribute.empty() )
	{
	  cerr << "An attribute must be provided" << endl;
          ok = false;
	}
      if( params.pattern.empty() )
	{
	  cerr << "A pattern expression must be provided" << endl;
          ok = false;
	}
      if( !ok )
        return EXIT_FAILURE;

      //	Conversion attributs -> liste

      set<string>	attset;
      istringstream	sst( params.attribute.c_str() );
      string	str;

      while( !sst.eof() )
        {
          sst >> str;
          if( str.size() > 0 )
            attset.insert( str );
        }
      if( attset.size() == 0 )
        {
          cout << "Nothing to do..." << endl;
          exit( 0 );
        }
      cout << attset.size() << " attributes to scan" << endl;

      //	Lecture de l'�l�ment adaptatif

      Model	*mod;

      try
        {
          MReader	ar( params.modelFile );
          FDParser	fdp;
          ar.addFactories( fdp.factories() );
          mod = ar.readModel();
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw;
        }

      ASSERT( mod );

      //	Expression r�guli�re


      regex_t	pat;
      ASSERT( !regcomp( &pat, params.pattern.c_str(), REG_NOSUB ) );


      //	Lecture graphe mod�le

      FRGraph	rg;
      FrgReader	rr( params.mgraphFile );

      try
        {
          rr >> rg;
          cout << "Lecture FRGraph OK." << endl;
        }
      catch( parse_error & e )
        {
          cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
               << endl;
          throw;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw;
        }

      //	Scrutage du graphe

      Graph::iterator			iv, fv = rg.end();
      set<Edge *>::const_iterator	ie, fe;
      string				attval, label1, label2;
      Model				*oldmod, *rplmod;
      set<string>::const_iterator	is, fs=attset.end();

      for( iv=rg.begin(); iv!=fv; ++iv )
        if((*iv)->getProperty( "model", oldmod ) )
          for( is=attset.begin(); is!=fs; ++is )
            if( (*iv)->getProperty( *is, attval ) 
                && !regexec( &pat, attval.c_str(), 0, 0, 0 ) )
              {
                delete oldmod;
                rplmod = mod->clone();
                (*iv)->setProperty( "model", rplmod );
                if( (*iv)->getProperty( "label", label1 ) )
                  mod->setBaseName( label1 );
                break;
              }

      const set<Edge *>	& edg = rg.edges();

      for( ie = edg.begin(), fe=edg.end(); ie!=fe; ++ie )
        if((*ie)->getProperty( "model", oldmod ) )
          for( is=attset.begin(); is!=fs; ++is )
            if( (*ie)->getProperty( *is, attval ) 
                && !regexec( &pat, attval.c_str(), 0, 0, 0 ) )
              {
                delete oldmod;
                rplmod = mod->clone();
                (*ie)->setProperty( "model", rplmod );
                if( (*ie)->getProperty( "label1", label1 ) 
                    && (*ie)->getProperty( "label2", label2 ) )
                  rplmod->setBaseName( label1 + "-" + label2 );
                break;
              }

      //	Sauvegarde du mod�le

      FrgWriter	rw( params.mgraphFile );

      try
        {
          rw << rg;
          cout << "Ecriture FRGraph OK." << endl;
        }
      catch( parse_error & e )
        {
          cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
               << endl;
          throw;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw;
        }

      cout << "OK." << endl;
      return 0;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}
