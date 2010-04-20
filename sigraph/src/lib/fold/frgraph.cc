/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/fold/frgraph.h>
#include <si/fold/foldFinder.h>
#include <si/fold/domainBox.h>
#include <si/model/adaptive.h>
#include <si/fold/fattrib.h>
#include <si/fold/foldFakeRel.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/exception/assert.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FRGraph::FRGraph( const string s, const string clqDescr ) : MGraph( s )
{
  setProperty( "clique_descriptor", (string) clqDescr );
  ModelFinder	*mf;
  if( clqDescr == "standard1" )
    {
      mf = new FoldFinder( *this );
      setProperty( "model_finder_ptr", mf );
    }
}

//note: property "model_finder_ptr" is deleted in base class MGraph
FRGraph::~FRGraph() { }


ModelFinder & FRGraph::modelFinder()
{
  ModelFinder	*mf;

  if( !getProperty( "model_finder_ptr", mf ) )
    {
      setProperty( "model_finder", (string) "standard1" );
      mf = new FoldFinder( *this );
      setProperty( "model_finder_ptr", mf );
    }

  return( *mf );
}


void FRGraph::addEdges( const Graph & gr, const Model* mod )
{
  const_iterator		iv, fv=end();
  unsigned			i, n = order();
  string			label, label2, label3;
  Vertex::const_iterator	ie, fe, ism, fsm;
  set<Vertex*>::const_iterator	is, fs;
  Edge::const_iterator		iv2, iv3;
  bool				noadd;
  int				cnt, gcnt;
  Edge				*edg;
  map<string,Edge *>::iterator	im;
  Vertex			*mver;
  set<Edge *>::const_iterator	ime, fme;

  cout << "Adding Random Edges ...       / " << n;

  if( !getProperty( SIA_NBASEGRAPHS, gcnt ) )
    gcnt = 0;
  setProperty( SIA_NBASEGRAPHS, gcnt+1 );

  //	m�moriser les anciens nombres d'occurence des relations
  for( ime=((const FRGraph *) this)->edges().begin(), 
	 fme=((const FRGraph *) this)->edges().end(); ime!=fme; ++ime )
    {
      edg = *ime;
      if( !edg->getProperty( SIA_OCCURENCE_COUNT, cnt ) )
	cnt = 0;
      edg->setProperty( "occurence_copy", cnt );
      edg->setProperty( SIA_OCCURENCE_COUNT, 0 );
    }

  //	comtages et ajouts
  for( iv=begin(), i = 0; iv!=fv; ++iv, ++i )
    {
      cout << "\b\b\b\b\b" << setw( 5 ) << i << flush;
      mver = *iv;
      ASSERT( mver->getProperty( SIA_LABEL, label ) );
      set<Vertex *>		sv = gr.getVerticesWith( SIA_LABEL, label );
      map<string, Edge *>	done;

      if( !sv.empty() )
	{
	  if( !mver->getProperty( SIA_OCCURENCE_COUNT, cnt ) )
	    cnt = 0;
	  mver->setProperty( SIA_OCCURENCE_COUNT, cnt+1 );
	}
      else
	{
	  if( !mver->getProperty( SIA_NOINSTANCE_COUNT, cnt ) )
	    cnt = 0;
	  mver->setProperty( SIA_NOINSTANCE_COUNT, cnt+1 );
	}

      // pour chaque noeud de label 'label'
      for( is=sv.begin(), fs=sv.end(); is!=fs; ++is )
	// chaque relation de chaque noeud
	for( ie=(*is)->begin(), fe=(*is)->end(); ie!=fe; ++ie )
	  if( (*ie)->getSyntax() != SIA_HULLJUNCTION_SYNTAX )
	    // (pas de mod�le pour les jonctions � la bo�te externe)
	    {
	      // trouver la destination du lien
	      iv2 = (*ie)->begin();
	      if( *iv2 == *is )
		++iv2;	// normalement c'est un graphe binaire
	      ASSERT( (*iv2)->getProperty( SIA_LABEL, label2 ) );
	      // si le lien n'existe pas encore
	      if( label2 != label )
		{
		  if( (im = done.find( label2 )) == done.end() )
		    {
		      noadd = false;
		      // dans les liens du mod�le
		      for( ism=(*iv)->begin(), fsm=(*iv)->end(); ism!=fsm; 
			   ++ism )
			{
			  iv3 = (*ism)->begin();
			  if( *iv3 == *iv )
			    ++iv3;
			  ASSERT( (*iv3)->getProperty( SIA_LABEL, label3 ) );
			  if( label3 == label2 )
			    {
			      noadd = true;
			      if( !(*ism)->getProperty( SIA_OCCURENCE_COUNT, 
							 cnt ) )
				cnt = 1;
			      (*ism)->setProperty( SIA_OCCURENCE_COUNT, 
						    cnt+1 );
			      break;
			    }
			}

		      if( noadd )
			edg = *ism;
		      else	// pas trouv�: il faut ajouter
			{
			  set<Vertex *> dest = getVerticesWith( SIA_LABEL, 
								label2 );
			  ASSERT( dest.size() == 1 );
			  cout << label << " <-> " << label2 << endl;
			  edg = makeEdge( *iv, *dest.begin(), label, label2, 
					  mod );
			}

		      done[ label2 ] = edg;
		    }
		  else	// d�j� dans la liste
		    {
		      if( !(*im).second->getProperty( SIA_OCCURENCE_COUNT, 
						       cnt ) )
			cnt = 1;
		      (*im).second->setProperty( SIA_OCCURENCE_COUNT, cnt+1 );
		    }
		}
	    }
    }

  //	comparer les anciens nombres d'occurence aux nouveaux
  int	oldc, noins;
  bool	newr;

  for( ime=((const FRGraph *) this)->edges().begin(); ime!=fme; ++ime )
    {
      edg = *ime;
      newr = false;
      if( !edg->getProperty( SIA_OCCURENCE_COUNT, cnt ) )
	cnt = 0;
      if( !edg->getProperty( "occurence_copy", oldc ) )
	{
	  oldc = 0;
	  newr = true;
	}
      if( !edg->getProperty( SIA_NOINSTANCE_COUNT, noins ) )
	noins = 0;
      if( cnt == 0 )
	++noins;
      cnt = oldc + cnt/2;
      edg->setProperty( SIA_OCCURENCE_COUNT, cnt );
      if( newr )	// si relation nouvelle, elle n'avait pas d'instance
	noins += gcnt;	// pour tous les graphes pr�c�dents de la base
      else
	edg->removeProperty( "occurence_copy" );
      if( noins > 0 )
	edg->setProperty( SIA_NOINSTANCE_COUNT, noins );
    }

  cout << endl;
}


Edge* FRGraph::makeEdge( Vertex* v1, Vertex* v2, const string & label1, 
			 const string & label2, const Model* mod )
{
  Edge	*edge;
  Model	*mod2 = mod->clone();
  edge = addUndirectedEdge( v1, v2, SIA_MODEL_EDGE );
  edge->setProperty( SIA_LABEL1, label1 );
  edge->setProperty( SIA_LABEL2, label2 );
  edge->setProperty( SIA_MODEL_FILE, "edges/edg" + label1 + "-" +
		      label2 + ".mod" );
  edge->setProperty( SIA_MODEL, mod2 );
  mod2->setBaseName( label1 + "-" + label2 );
  string			voidl = SIV_VOID_LABEL;

  getProperty( SIA_VOID_LABEL, voidl );

  TopModel	*tm = mod2->topModel();
  if( tm )
    {
      set<string>	ls;
      ls.insert( label1 );
      ls.insert( label2 );
      ls.insert( voidl );
      tm->significantLabels() = ls;
      tm->setVoidLabel( voidl );
    }
  else
    cerr << "FRGraph::makeEdge " << label1 << " - " << label2 
	 << " : pas de TopModel\n";

  return( edge );
}


void FRGraph::createTriangDomainFiles( const string & dir )
{
  string		bname = dir + FileUtil::separator();
  string		name, label;
  const_iterator	iv, fv=end();
  Vertex		*v;
  Domain		*dom;
  DomainBox		*dbox;
  double		nx, ny, nz, v1x, v1y, v1z, v2x, v2y, v2z, d;
  set<string>		sl;
  ofstream		file;
  vector<vector<double> >	pts;

  if( !hasProperty( "fold.tri" ) )
    {
      setProperty( SIA_TYPETRI, string( "fold.tri" ) );
      setProperty( "fold.tri", string( SIA_MODEL_NODE ) + " " 
		    + SIA_TMTKTRI_FILENAME );
      vector<int>	vi;
      vi.push_back( 255 );
      vi.push_back( 255 );
      vi.push_back( 0 );
      setProperty( SIA_TMTKTRI_FILENAME, vi );
    }

  for( iv=begin(); iv!=fv; ++iv )
    {
      v = *iv;
      if( v->getProperty( SIA_DOMAIN, dom ) 
	  && v->getProperty( SIA_LABEL, label ) )
	{
	  dbox = dynamic_cast<DomainBox *>( dom );
	  if( dbox )
	    {
	      dbox->cubeTalairach( pts );
	      /*cout << "label : " << label << " : " << pts[0][0] << ", " 
		   << pts[0][1] << ", " << pts[0][2] << "; " << pts[1][0] 
		   << ", " << pts[1][1] << ", " << pts[1][2] 
		   << endl;*/
	      name = bname + "dom_" + label + ".tri";
	      file.open( name.c_str() );
	      if( !file )
		{
		  cerr << "cannot open " << name << " for writing.\n";
		  break;
		}
	      v->setProperty( SIA_TMTKTRI_FILENAME, name );
	      //	points
	      file << "- 24\n";
	      v1x = pts[1][0] - pts[0][0];
	      v1y = pts[1][1] - pts[0][1];
	      v1z = pts[1][2] - pts[0][2];
	      v2x = pts[3][0] - pts[0][0];
	      v2y = pts[3][1] - pts[0][1];
	      v2z = pts[3][2] - pts[0][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[0][0] << " " << pts[0][1] << " " << pts[0][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[1][0] << " " << pts[1][1] << " " << pts[1][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[2][0] << " " << pts[2][1] << " " << pts[2][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[3][0] << " " << pts[3][1] << " " << pts[3][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;

	      v1x = pts[2][0] - pts[3][0];
	      v1y = pts[2][1] - pts[3][1];
	      v1z = pts[2][2] - pts[3][2];
	      v2x = pts[4][0] - pts[3][0];
	      v2y = pts[4][1] - pts[3][1];
	      v2z = pts[4][2] - pts[3][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[3][0] << " " << pts[3][1] << " " << pts[3][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[2][0] << " " << pts[2][1] << " " << pts[2][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[7][0] << " " << pts[7][1] << " " << pts[7][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[4][0] << " " << pts[4][1] << " " << pts[4][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;

	      v1x = pts[6][0] - pts[1][0];
	      v1y = pts[6][1] - pts[1][1];
	      v1z = pts[6][2] - pts[1][2];
	      v2x = pts[2][0] - pts[1][0];
	      v2y = pts[2][1] - pts[1][1];
	      v2z = pts[2][2] - pts[1][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[1][0] << " " << pts[1][1] << " " << pts[1][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[6][0] << " " << pts[6][1] << " " << pts[6][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[7][0] << " " << pts[7][1] << " " << pts[7][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[2][0] << " " << pts[2][1] << " " << pts[2][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;

	      v1x = pts[3][0] - pts[0][0];
	      v1y = pts[3][1] - pts[0][1];
	      v1z = pts[3][2] - pts[0][2];
	      v2x = pts[5][0] - pts[0][0];
	      v2y = pts[5][1] - pts[0][1];
	      v2z = pts[5][2] - pts[0][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[0][0] << " " << pts[0][1] << " " << pts[0][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[3][0] << " " << pts[3][1] << " " << pts[3][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[4][0] << " " << pts[4][1] << " " << pts[4][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[5][0] << " " << pts[5][1] << " " << pts[5][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;

	      v1x = pts[5][0] - pts[0][0];
	      v1y = pts[5][1] - pts[0][1];
	      v1z = pts[5][2] - pts[0][2];
	      v2x = pts[1][0] - pts[0][0];
	      v2y = pts[1][1] - pts[0][1];
	      v2z = pts[1][2] - pts[0][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[0][0] << " " << pts[0][1] << " " << pts[0][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[5][0] << " " << pts[5][1] << " " << pts[5][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[6][0] << " " << pts[6][1] << " " << pts[6][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[1][0] << " " << pts[1][1] << " " << pts[1][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;

	      v1x = pts[4][0] - pts[5][0];
	      v1y = pts[4][1] - pts[5][1];
	      v1z = pts[4][2] - pts[5][2];
	      v2x = pts[6][0] - pts[5][0];
	      v2y = pts[6][1] - pts[5][1];
	      v2z = pts[6][2] - pts[5][2];
	      nx = v1y * v2z - v1z * v2y;
	      ny = v1z * v2x - v1x * v2z;
	      nz = v1x * v2y - v1y * v2x;
	      d = sqrt( nx * nx + ny * ny + nz * nz );
	      nx /= -d;
	      ny /= -d;
	      nz /= -d;
	      file << pts[7][0] << " " << pts[7][1] << " " << pts[7][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[6][0] << " " << pts[6][1] << " " << pts[6][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[5][0] << " " << pts[5][1] << " " << pts[5][2] 
		   << "  " << nx << " " << ny << " " << nz << endl;
	      file << pts[4][0] << " " << pts[4][1] << " " << pts[4][2] 
	      << "  " << nx << " " << ny << " " << nz << endl;

	      //	triangles
	      file << "- 12 12 12\n";
	      file << "0 1 2\n0 2 3\n4 5 6\n4 6 7\n8 9 10\n8 10 11\n";
	      file << "12 13 14\n12 14 15\n16 17 18\n16 18 19\n";
	      file << "20 21 22\n20 22 23\n";
	      file.close();
	    }
	}
    }
}


void FRGraph::createFakeRel()
{
  FoldFakeRel	*ffr;

  if( getProperty( SIA_FAKEREL_MODEL, ffr ) )
    delete ffr;
  ffr = new FoldFakeRel;
  ffr->setMGraph( this );
  setProperty( SIA_FAKEREL_MODEL, ffr );
}
