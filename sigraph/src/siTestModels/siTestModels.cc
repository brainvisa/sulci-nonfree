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
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#include <aims/getopt/getopt2.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/sstream.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>

using namespace carto;
using namespace aims;
using namespace sigraph;
using namespace std;


struct Params
{
  string		netFiles;
  vector<string>	nets;
  string		learnCfgFiles;
  vector<string>	learnCfgs;
  string		learnerFiles;
  vector<string>	learners;
  string		netTarget;
  string		plotCmd;
  string		outputDir;
  string		plotFields;
  string		outPlotField;
  string		errPlotField;
};


void loadParams( const string & paramFile, const char*, Params & par )
{
  SyntaxReader	pr( si().basePath() + "/config/siTestModels.stx" );
  SyntaxSet		ps;

  try
    {
      pr >> ps;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }

  TreeReader	tr( paramFile, ps );
  Tree		t;

  try
    {
      tr >> t;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }

  t.getProperty( "netFiles", par.netFiles );
  t.getProperty( "learnCfgFiles", par.learnCfgFiles );
  t.getProperty( "learnerFiles", par.learnerFiles );
  t.getProperty( "netTarget", par.netTarget );
  t.getProperty( "outputDir", par.outputDir );
  t.getProperty( "plotFields", par.plotFields );
  t.getProperty( "outPlotField", par.outPlotField );
  t.getProperty( "errPlotField", par.errPlotField );
  if( !t.getProperty( "plotCmd", par.plotCmd ) )
    par.plotCmd = "ploterr-ps.pl";

  istringstream	sst( par.netFiles.c_str() );
  string	str;

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
	par.nets.push_back( str );
    }

  istringstream	lcs( par.learnCfgFiles.c_str() );
  while( !lcs.eof() )
    {
      lcs >> str;
      if( str.size() > 0 )
	par.learnCfgs.push_back( str );
    }

  istringstream	lrs( par.learnerFiles.c_str() );
  while( !lrs.eof() )
    {
      lrs >> str;
      if( str.size() > 0 )
	par.learners.push_back( str );
    }
}


void replaceNets( const string & modelpath, const string & netname )
{
  //	bon, pour le moment je remplace *TOUS* les réseaux
  //	(j'espère que vous ne teniez pas à votre modèle)

  DIR			*dir = opendir( modelpath.c_str() );
  vector<string>	files;
  struct dirent		*dent;

  if( !dir )
    {
      cerr << "could not open directory " << modelpath + "\n";
      return;
    }

  while( ( dent = readdir( dir ) ) )
    {
      string s = dent->d_name;
      if( s.rfind( ".net" ) == s.size() - 4 )	// nom de réseau
	files.push_back( s );
    }
  closedir( dir );

  vector<string>::const_iterator	ifl;

  //	remplacement de tous ces fichiers

  cout << "Remplacement des réseaux...\n";
  for( ifl=files.begin(); ifl!=files.end(); ++ifl )
    system( ( string( "cp -f " ) + netname + " " + *ifl ).c_str() );
  cout << "OK." << endl;
}


void relplaceNets( const string & modelFile, const string & filt, 
		   const string & netname )
{
  string	modpath;
  string::size_type pos;

  pos = modelFile.rfind( '.' );
  if( pos != string::npos )
    modpath = modelFile.substr( 0, pos );
  modpath += ".data/";

  if( filt == "label" )		// filtre les noeuds
    modpath += "adap/nnets";
  else				// filtre les relations
    modpath += "edges/nnets";
  replaceNets( modpath, netname );
}


int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Tests some model configurations" );
      string		paramfile;
      string	p = "input parameters file (tree format) "
        "(richer than other parameter given on the commandline)";
      ifstream	ps( (si().basePath() 
                     + "/config/siTestModels.stx" ).c_str() );
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
                  for( i=0, n=s2.length();
                    i<n && ( s2[i]==' ' || s2[i]=='\t' ); ++i ) {}
                  if( s2.substr( i, i+6 ) != "*BEGIN" 
                      && s2.substr( i, i+4 ) != "*END" )
                    {
                      p += "\n";
                      p += s2.substr( i, s2.length()-i );
                    }
                }
            }
        }
      app.addOption( paramfile, "-p", p );
      app.initialize();

      Params	par;
      string	tmpLrnCfg = "/tmp/tmpSiLearn.cfg";

      loadParams( argv[1], argv[0], par );

      //	syntaxe de siLearn

      SyntaxReader	lrnsr( si().basePath() + "/config/siLearn.stx" );
      SyntaxSet	baseStx;

      lrnsr >> baseStx;

      vector<string>::const_iterator	net, base, lr;
      Tree					lrnCfg;
      string				datafile, cmd;
      string				pnetname, pcfgname, plrnname;

      if( par.netTarget.rfind( ".net" ) == par.netTarget.size() - 4 )
        datafile = par.netTarget.substr( 0, par.netTarget.size() - 4 );
      else
        datafile = par.netTarget;

      for( net=par.nets.begin(); net!=par.nets.end(); ++net )
        {
          cout << "Réseau " << *net << "...\n";
          // remplacer le réseau
          system( ( string( "cp -f " ) + *net + " " + par.netTarget ).c_str() );
          // nom du réseau sans / et sans .net
          string::size_type pos = (*net).rfind( '/' );
          if( pos == string::npos )
            pnetname = *net;
          else
            pnetname = (*net).substr( pos+1, (*net).size() - pos -1 );
          if( pnetname.rfind( ".net" ) == pnetname.size() - 4 )
            pnetname.erase( pnetname.size() - 4, 4 );

          for( base=par.learnCfgs.begin(); base!=par.learnCfgs.end(); ++base )
            {
              cout << "fichier config siLearn : " << *base << "..." << endl;

              // lire le fichier de config pour siLearn

              try
                {
                  TreeReader	tr( *base, baseStx );
                  tr >> lrnCfg;
                }
              catch( exception & e )
                {
                  cerr << e.what() << endl;
                  remove( tmpLrnCfg.c_str() );
                  exit( 1 );
                }

              // nom de la config sans / et sans .
              pos = (*base).rfind( '/' );
              if( pos == string::npos )
                pcfgname = *base;
              else
                pcfgname = (*base).substr( pos+1, (*base).size() - pos -1 );
              pos = pcfgname.rfind( '.' );
              if( pos != string::npos )
                pcfgname.erase( pos, pcfgname.size() - pos );

              //	remplacer les réseaux (si pas déjà fait)
              /*if( !netrplc )
                {
                netrplc = true;
                if( !lrnCfg.getProperty( "filter_attributes", filt ) )
		{
                cerr << "Pas de filtre trouvé: je peux pas essayer tous les "
                << "réseaux à la fois...\n";
                remove( tmpLrnCfg.c_str() );
                exit( 1 );
		}
                assert( lrnCfg.getProperty( "modelFile", mod ) );
                relplaceNets( mod, filt, *net );
                }*/

              for( lr=par.learners.begin(); lr!=par.learners.end(); ++lr )
                {
                  cout << "fichier Learners : " << *lr << "..." << endl;

                  // utiliser le bon learner dans la config de siLearn
                  lrnCfg.setProperty( "trainschemeFile", *lr );
                  // et la réécrire dans /tmp
                  try
                    {
                      TreeWriter	tw( tmpLrnCfg, baseStx );
                      tw << lrnCfg;
                    }
                  catch( exception & e )
                    {
                      cerr << e.what() << endl;
                      remove( tmpLrnCfg.c_str() );
                      exit( 1 );
                    }

                  // maintenant, lancer l'apprentissage
                  cout << "Apprentissage..." << endl;
                  cmd = string( "siLearn " ) + tmpLrnCfg;
                  cout << cmd << endl;
                  system( cmd.c_str() );
                  cout << "Apprentissage fini" << endl;

                  // nom du learner sans / et sans .
                  pos = (*lr).rfind( '/' );
                  if( pos == string::npos )
                    plrnname = *lr;
                  else
                    plrnname = (*lr).substr( pos+1, (*lr).size() - pos -1 );
                  pos = plrnname.rfind( '.' );
                  if( pos != string::npos )
                    plrnname.erase( pos, plrnname.size() - pos );

                  // sorties, base d'apprentissage
                  cmd = par.plotCmd + " " + par.outputDir + "/app-sorties-" 
                    + pnetname + "-" + pcfgname + "-" + plrnname + ".eps " 
                    + datafile + ".dat bim " + par.plotFields + " " 
                    + par.outPlotField;
                  system( cmd.c_str() );
                  // sorties, base de test
                  cmd = par.plotCmd + " " + par.outputDir + "/gen-sorties-" 
                    + pnetname + "-" + pcfgname + "-" + plrnname + ".eps " 
                    + datafile + "-tst.dat bim " + par.plotFields + " " 
                    + par.outPlotField;
                  system( cmd.c_str() );
                  // erreur, base de test
                  cmd = par.plotCmd + " " + par.outputDir + "/gen-erreur-" 
                    + pnetname + "-" + pcfgname + "-" + plrnname + ".eps " 
                    + datafile + "-tst.dat b " + par.plotFields + " " 
                    + par.errPlotField;
                  system( cmd.c_str() );
                }
            }
        }
      remove( tmpLrnCfg.c_str() );
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }
}
