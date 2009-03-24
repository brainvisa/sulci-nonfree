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
#include <si/fold/fgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgraph.h>
#include <si/fold/frgReader.h>
#include <si/fold/labelsTranslator.h>
#include <si/graph/anneal.h>
#include <si/finder/modelFinder.h>
#include <si/global/global.h>
#include <si/graph/attrib.h>
#include <si/model/topAdaptive.h>
#include <aims/getopt/getopt2.h>
#include <graph/tree/tree.h>
#include <graph/tree/twriter.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/sstream.h>
#include <vector>
#include <iostream>

using namespace carto;
using namespace sigraph;
using namespace aims;
using namespace std;


struct Params
{
  string		baseGraphFiles;
  string		testGraphFiles;
  string		genGraphFiles;
  string		resultFile;
  string		translation;
  string		modelGraph;
  float			setWeights;
  vector<string>	graphs;
  int			nbase;
  int			ntest;
  int			ngen;
};


void usage( const char* name )
{
  cerr << "usage : \n" << name << " resultfile.tre foldgraph1.arg ... " 
       << "foldgraphN.arg\n";
  cerr << name << " paramfile.cfg\n\n";
  cerr << "Counts differences between 'label' and 'name' attributes " 
       << "(automatic and manual\n" 
       << "sulci identifications), sulcus by sulcus, and saves a tree " 
       << "with stats in\n'resultfile.tre'\n\n";
  exit( 1 );
}


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( 1 );
}


void fillEmptyFold( Tree* tr, unsigned n )
{
  vector<float>	adds, rejs, sizes;

  for( unsigned i=0; i<n; ++i )
    {
      adds.push_back( 0 );
      rejs.push_back( 0 );
      sizes.push_back( 0 );
    }
  tr->setProperty( "adds_vector", adds );
  tr->setProperty( "rejects_vector", rejs );
  tr->setProperty( "sizes_vector", sizes );
}


void loadParams( const string & paramFile, const char* name, Params & par )
{
  SyntaxReader	pr( si().basePath() 
                    + "/config/siErrorStatsParams.stx" );
  SyntaxSet		ps;

  pr >> ps;

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	str;

  par.setWeights = 0;

  tr >> t;
  t.getProperty( "baseGraphFiles", par.baseGraphFiles );
  t.getProperty( "testGraphFiles", par.testGraphFiles );
  t.getProperty( "genGraphFiles", par.genGraphFiles );
  t.getProperty( "resultFile", par.resultFile );
  t.getProperty( "translation", par.translation );
  t.getProperty( "modelGraph", par.modelGraph );
  t.getProperty( "set_weights", par.setWeights );

  istringstream	sst( par.baseGraphFiles.c_str() );

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
        par.graphs.push_back( str );
    }
  if( par.graphs.size() == 0 )
    paramError( name, paramFile, "baseGraphFiles" );
  par.nbase = par.graphs.size();

  istringstream	tst( par.testGraphFiles.c_str() );

  while( !tst.eof() )
    {
      tst >> str;
      if( str.size() > 0 )
        par.graphs.push_back( str );
    }
  par.ntest = par.graphs.size() - par.nbase;

  istringstream	gst( par.genGraphFiles.c_str() );

  while( !gst.eof() )
    {
      gst >> str;
      if( str.size() > 0 )
        par.graphs.push_back( str );
    }
  par.ngen = par.graphs.size() - par.ntest - par.nbase;
}


int main( int argc, const char** argv )
{
  try
    {
      Params	par;
      string	paramfile;
      unsigned	ng, i;

      AimsApplication	app( argc, argv, "Counts differences between 'label' " 
                             "and 'name' attributes (automatic and manual " 
                             "sulci identifications), sulcus by sulcus, and " 
                             "saves a tree with stats in 'resultfile.tre'" );

      app.addOption( paramfile, "-c", "parameters file (if you don't provide " 
                     "other parameters on the commandline options). You have " 
                     "to provide either this parameters file, or the other " 
                     "arguments (result tree file, input graphs)", true );
      app.addOption( par.resultFile, "-o", "result tree which will contain " 
                     "the difference stats", true );
      app.addOptionSeries( par.graphs, "-i", "Graphs to count differences " 
                           "from" );

      app.initialize();

      if( !paramfile.empty() )
        loadParams( paramfile, argv[0], par );
      if( par.resultFile.empty() )
        throw invalid_argument( "No output file specified" );

      ng = par.graphs.size();

      FGraph	gr[ ng ];
      string	names[ ng ];
      FRGraph	*frg = 0;

      if( !par.modelGraph.empty() )
        {
          try
            {
              FrgReader	frr( par.modelGraph );

              frg = new FRGraph;
              frr >> *frg;
            }
          catch( exception & e )
            {
              cerr << e.what() << endl;
              throw;
            }
          if( par.setWeights < 0 )
            frg->removeWeights();
          else if( par.setWeights > 0 )
            frg->setWeights( par.setWeights );
        }

      for( i=0; i<ng; ++i )
        {
          try
            {
              FoldReader	fr( par.graphs[i] );

              fr >> gr[i];
              cout << "Lecture de " << par.graphs[i] << " OK.\n";
            }
          catch( exception & e )
            {
              cerr << e.what() << endl;
              throw;
            }

          names[ i ] = par.graphs[i];
          // effacer path du nom
          string::size_type pos = names[i].rfind( '/' );
          if( pos != string::npos )
            names[i].erase( 0, pos+1 );
          // effacer extension
          pos = names[i].find( '.' );
          if( pos != string::npos )
            names[i].erase( pos, names[i].size() - pos );
          // effacer préfixe Base ou Auto
          pos = names[i].rfind( "Auto" );
          if( pos == string::npos )
            pos = names[i].rfind( "Base" );
          if( pos != string::npos )
            names[i].erase( pos, names[i].size() - pos );
        }

      FoldLabelsTranslator	tr;
      if( par.translation.size() == 0 )
        par.translation = si().labelsTranslPath();
      else
        si().setLabelsTranslPath( par.translation );
      if( frg )
        tr.makeFromModel( *frg, par.translation );
      else
        tr.readLabels( par.translation );

      for( i=0; i<ng; ++i )
        {
          tr.translate( gr[i] );
          tr.translate( gr[i], "label", "label" );
        }

      //	arbre de résultats
      Tree			errTr( true, "siErrorStats" );
      Graph::const_iterator	iv, fv;
      string		label, name;
      map<string, Tree *>	mtr;
      map<string, Tree *>::const_iterator	imt, fmt;
      float			com, add, rej, size;
      Vertex		*v;
      set<Tree *>		usedTrees;
      set<Tree *>::const_iterator	it, ft=usedTrees.end();
      Tree			*t, *folds, *graphs, *tg;
      int			ngraphs, nerr, terr, nadd, ncom, nrej, nfld, npG, nprs;
      float			err, merr, Merr, serr;
      float			mcom, mincom, scom, madd, Madd, sadd, mrej, Mrej, srej;
      bool			ha, hc, hr;
      float			mass, perr, padd, prej, totmass;
      map<FGraph *, Tree *>	graphTrees;
      vector<float>		vadd, vrej, vsize;

      folds = new Tree( true, "folds" );
      graphs = new Tree( true, "graphs" );
      errTr.insert( folds );
      errTr.insert( graphs );
      errTr.setProperty( "baseGraphs", (int) par.nbase );
      errTr.setProperty( "testGraphs", (int) par.ntest );
      errTr.setProperty( "genGraphs", (int) par.ngen );

      for( i=0; i<ng; ++i )
        {
          usedTrees.erase( usedTrees.begin(), ft );
          mass = 0;
          merr = 0;
          nerr = 0;

          for( iv=gr[i].begin(), fv=gr[i].end(); iv!=fv; ++iv )
            if( (v = *iv)->getProperty( SIA_LABEL, label ) 
                && v->getProperty( SIA_NAME, name ) 
                && v->getProperty( "size", size ) )
              {
                //cout << "name: " << name << ", label: " << label << endl;
                Tree	*& tro = mtr[ name ];
                if( !tro )
                  {
                    tro = new Tree( true, "fold" );
                    tro->setProperty( "label", name );
                    folds->insert( tro );
                    fillEmptyFold( tro, i );
                  }
                Tree	*& trn = mtr[ label ];
                if( !trn )
                  {
                    trn = new Tree( true, "fold" );
                    trn->setProperty( "label", label );
                    folds->insert( trn );
                    fillEmptyFold( trn, i );
                  }
                usedTrees.insert( tro );
                usedTrees.insert( trn );
                mass += size;

                if( label == name )	// bon label
                  {
                    com = 0;
                    ncom = 0;
                    tro->getProperty( "common", com );
                    com += size;
                    tro->setProperty( "common", com );
                    tro->getProperty( "n_common", ncom );
                    ++ncom;
                    tro->setProperty( "n_common", ncom );
                  }
                else	// mauvais label
                  {
                    merr += size;
                    ++nerr;
                    add = 0;
                    rej = 0;
                    nadd = nrej = 0;
                    tro->getProperty( "rejected", rej );
                    trn->getProperty( "added", add );
                    tro->getProperty( "n_rejected", nrej );
                    trn->getProperty( "n_added", nadd );
                    add += size;
                    rej += size;
                    ++nadd;
                    ++nrej;
                    tro->setProperty( "rejected", rej );
                    trn->setProperty( "added", add );
                    tro->setProperty( "n_rejected", nrej );
                    trn->setProperty( "n_added", nadd );
                  }
              }

          //	erreurs graphe par graphe

          tg = new Tree( true, "graph" );
          graphs->insert( tg );
          graphTrees[ &gr[i] ] = tg;
          tg->setProperty( "name", names[i] );
          tg->setProperty( "mass", mass );
          tg->setProperty( "nodes_count", (int) gr[i].order() );
          tg->setProperty( "node_errors_count", nerr );
          tg->setProperty( "pmass_error", merr / mass );

          nfld = npG = 0;

          //	erreurs graphe par graphe sur les labels rencontrés
          for( it=usedTrees.begin(); it!=ft; ++it )
            {
              t = *it;
              if( !t->getProperty( "graphs_count", ngraphs ) )
                ngraphs = 0;
              ++ngraphs;
              t->setProperty( "graphs_count", ngraphs );
              com = 0;
              add = 0;
              rej = 0;
              merr = 0;
              Merr = 0;
              serr = 0;
              nerr = terr = 0;
              mcom = mincom = scom = madd = Madd = sadd = mrej = Mrej = srej = 0;
              ncom = nrej = nadd = 0;
              nprs = 0;
              hc = t->getProperty( "common", com );
              ha = t->getProperty( "added", add );
              hr = t->getProperty( "rejected", rej );
              t->getProperty( "mean_common", mcom );
              t->getProperty( "mean_added", madd );
              t->getProperty( "mean_rejected", mrej );
              t->getProperty( "min_common", mincom );
              t->getProperty( "max_added", Madd );
              t->getProperty( "max_rejected", Mrej );
              t->getProperty( "sigma_common", scom );
              t->getProperty( "sigma_added", sadd );
              t->getProperty( "sigma_rejected", srej );
              t->getProperty( "mean_error", merr );
              t->getProperty( "max_error", Merr );
              t->getProperty( "sigma_error", serr );
              t->getProperty( "errors_count", nerr );
              t->getProperty( "total_errors_count", terr );
              t->getProperty( "n_rejected", nrej );
              t->getProperty( "n_added", nadd );
              t->getProperty( "n_common", ncom );
              t->getProperty( "occurence_count", nprs );
              t->getProperty( "adds_vector", vadd );
              t->getProperty( "rejects_vector", vrej );
              t->getProperty( "sizes_vector", vsize );
              assert( add + rej + com > 0 );
              if( ncom + nrej != 0 )
                {
                  ++nfld;
                  ++nprs;
                  if( ncom > 0 )
                    ++npG;
                }
              if( add + rej != 0 )
                ++nerr;	// erreurs partielles
              if( com == 0 )
                ++terr;	// erreurs totales
              totmass = com + add + rej;
              vadd.push_back( add / totmass );
              vrej.push_back( rej / totmass );
              vsize.push_back( com + rej );
              err = ( add + rej ) / totmass;
              mcom += com;
              if( ngraphs == 1 || com < mincom )
                mincom = com;
              scom += com * com;
              madd += add;
              if( Madd < add )
                Madd = add;
              sadd += add * add;
              mrej += rej;
              if( Mrej < rej )
                Mrej = rej;
              srej += rej * rej;
              merr += err;
              if( Merr < err )
                Merr = err;
              serr += err * err;

              if( hc )
                {
                  t->removeProperty( "common" );
                  t->removeProperty( "n_common" );
                }
              if( ha )
                {
                  t->removeProperty( "added" );
                  t->removeProperty( "n_added" );
                }
              if( hr )
                {
                  t->removeProperty( "rejected" );
                  t->removeProperty( "n_rejected" );
                }
              t->setProperty( "mean_common", mcom );
              t->setProperty( "mean_added", madd );
              t->setProperty( "mean_rejected", mrej );
              t->setProperty( "min_common", mincom );
              t->setProperty( "max_added", Madd );
              t->setProperty( "max_rejected", Mrej );
              t->setProperty( "sigma_common", scom );
              t->setProperty( "sigma_added", sadd );
              t->setProperty( "sigma_rejected", srej );
              t->setProperty( "mean_error", merr );
              t->setProperty( "max_error", Merr );
              t->setProperty( "sigma_error", serr );
              t->setProperty( "errors_count", nerr );
              t->setProperty( "total_errors_count", terr );
              t->setProperty( "occurence_count", nprs );
              t->setProperty( "adds_vector", vadd );
              t->setProperty( "rejects_vector", vrej );
              t->setProperty( "sizes_vector", vsize );
            }

          tg->setProperty( "partly_recogn_folds", npG );
          tg->setProperty( "folds_count", nfld );

          // mettre à jour les arbres inutilisés (sillons pas présents)
          for( imt=mtr.begin(), fmt=mtr.end(); imt!=fmt; ++imt )
            if( usedTrees.find( (*imt).second ) == ft )
              {
                t = (*imt).second;
                t->getProperty( "adds_vector", vadd );
                t->getProperty( "rejects_vector", vrej );
                t->getProperty( "sizes_vector", vsize );
                vadd.push_back( 0 );
                vrej.push_back( 0 );
                vsize.push_back( 0 );
                t->setProperty( "adds_vector", vadd );
                t->setProperty( "rejects_vector", vrej );
                t->setProperty( "sizes_vector", vsize );
              }
        }

      //	maintenant on divise

      for( imt=mtr.begin(), fmt=mtr.end(); imt!=fmt; ++imt )
        {
          t = (*imt).second;
          assert( t->getProperty( "graphs_count", ngraphs ) );
          assert( ngraphs > 0 );
          assert( t->getProperty( "mean_common", mcom ) );
          assert( t->getProperty( "mean_added", madd ) );
          assert( t->getProperty( "mean_rejected", mrej ) );
          assert( t->getProperty( "sigma_common", scom ) );
          assert( t->getProperty( "sigma_added", sadd ) );
          assert( t->getProperty( "sigma_rejected", srej ) );
          assert( t->getProperty( "mean_error", merr ) );
          assert( t->getProperty( "max_error", Merr ) );
          assert( t->getProperty( "sigma_error", serr ) );

          mass = mcom + mrej;
          if( mass == 0 )
            {
              padd = 2;
              prej = 2;
            }
          else
            {
              padd = madd / mass;
              prej = mrej / mass;
            }
          perr = (madd + mrej) / (mass + madd);

          mcom /= ng;
          scom = scom / ng - mcom * mcom;
          if( scom < 0 )
            {
              cerr << "sigma négatif: " << scom << ", taille : " << mcom 
                   << endl;
              scom = 0;
            }
          scom = sqrt( scom );

          madd /= ng;
          sadd = sadd / ng - madd * madd;
          if( sadd < 0 )
            sadd = 0;
          sadd = sqrt( sadd );

          mrej /= ng;
          srej = srej / ng - mrej * mrej;
          if( srej < 0 )
            srej = 0;
          srej = sqrt( srej );

          merr /= ng;
          serr = serr / ng - merr * merr;
          if( serr < 0 )
            serr = 0;
          serr = sqrt( serr );

          t->setProperty( "mean_common", mcom );
          t->setProperty( "mean_added", madd );
          t->setProperty( "mean_rejected", mrej );
          t->setProperty( "sigma_common", scom );
          t->setProperty( "sigma_added", sadd );
          t->setProperty( "sigma_rejected", srej );
          t->setProperty( "mean_error", merr );
          t->setProperty( "max_error", Merr );
          t->setProperty( "sigma_error", serr );
          t->setProperty( "pmass_error", perr );
          t->setProperty( "pmass_added", padd );
          t->setProperty( "pmass_rejected", prej );
          t->setProperty( "mass", mass );
        }

      errTr.setProperty( "graphs_count", (int) ng );

      //	si graphe modèle: mettre aussi les énergies et les poids
      if( frg )
        {
          float			en, wt, err, wte;
          Graph::const_iterator	iv, fv=frg->end();
          Vertex			*v;
          Edge			*e;
          Model			*mod;
          TopAdaptive		*tad;
          Vertex::const_iterator	ie, fe;

          for( i=0; i<ng; ++i )
            {
              cout << "making cliques of " << par.graphs[i] << endl;
              frg->modelFinder().initCliques( gr[i], true, false, false, 
                                              false );

              Anneal	an( gr[i], *frg );

              gr[i].ensureAllLabelsPossible();	// don't do this anymore!
              en = an.processAllPotentials();
              t = graphTrees[ &gr[i] ];
              t->setProperty( "final_energy", en );

              // recopier name -> label
              tr.translate( gr[i], "name", "label" );
              //cout << "(back to names)\n";
              gr[i].ensureAllLabelsPossible();
              en = an.processAllPotentials();
              t->setProperty( "initial_energy", en );
            }

          for( iv=frg->begin(); iv!=fv; ++iv )
            {
              v = *iv;
              if( v->getProperty( "model", mod ) 
                  && v->getProperty( "label", label ) )
                {
                  tad = dynamic_cast<TopAdaptive *>( mod );
                  if( tad )
                    {
                      wt = tad->weight();
                      err = tad->genErrorRate();
                      t = mtr[ label ];
                      if( !t )
                        cerr << "Warning : label " << label 
                             << " has no instance\n";
                      else
                        {
                          t->setProperty( "weight_size", wt );
                          t->setProperty( "weight_genErrorRate", 
                                           (float) 1. - err );
                          if( err < 0.5 )
                            wt *= 1. - err * 2;
                          else
                            wt = 0;
                          t->setProperty( "weight_fold", wt );

                          for( ie=v->begin(), fe=v->end(); ie!=fe; ++ie )
                            {
                              e = *ie;
                              if( e->getProperty( "model", mod ) )
                                {
                                  tad = dynamic_cast<TopAdaptive *>( mod );
                                  if( tad )
                                    {
                                      wte = tad->weight();
                                      err = tad->genErrorRate();
                                      if( err < 0.5 )
                                        wt += wte * ( 1. - err * 2 );
                                    }
                                }
                            }
                          t->setProperty( "weight_global", wt );
                        }
                    }
                }
            }
        }

      SyntaxReader	sr( si().basePath() + "/config/siErrorStats.stx" );
      SyntaxSet	ss;

      sr >> ss;

      TreeWriter	tw( par.resultFile, ss );
      tw << errTr;

      return EXIT_SUCCESS;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }

  return EXIT_FAILURE;
}
