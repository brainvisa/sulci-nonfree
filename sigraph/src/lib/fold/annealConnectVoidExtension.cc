
#include <cstdlib>
#include <si/fold/annealConnectVoidExtension.h>
#include <si/graph/anneal.h>
#include <si/graph/vertexclique.h>
#include <si/graph/cgraph.h>
#include <si/fold/fattrib.h>
#include <si/finder/modelFinder.h>
#include <neur/rand/rand.h>
#include <math.h>

using namespace sigraph;
using namespace std;


AnnealConnectVoidExtension::~AnnealConnectVoidExtension()
{
}


void AnnealConnectVoidExtension::specialStep( unsigned )
{
  //  cout << "Passe connexe VOID...\n";
  _ntrans = 0;
  _maxtrans = 0;
  _stepDeltaE = 0;

  // partition des noeuds selon leur label

  map<string, set<Vertex *> >	sg;
  CGraph::const_iterator	iv, fv=_anneal->cGraph().end();
  string			label;
  double			r;
  map<double, string>		order;
  unsigned			maxs = 0;

  for( iv=_anneal->cGraph().begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_LABEL, label ) 
	&& label != _anneal->voidLabel() )
      {
	set<Vertex *>	& sv = sg[label];
	if( sv.empty() )
	  {
	    do
	      {
		r = ran1();
	      } while( order.find( r ) != order.end() );
	    order[ r ] = label;
	  }
	sv.insert( *iv );
	if( sv.size() > maxs )
	  maxs = sv.size();
      }

  // itération sur chaque groupe

  map<double, string>::const_iterator	ig, fg=order.end();
  set<Vertex *>::const_iterator		isv, fsv;
  EnergyField				ef;
  set<Clique *>				*sc;
  set<Clique *>::const_iterator		ic, fc;
  map<Clique *, double>::iterator	ic2, fc2;
  double				E, eE, limit;
  ModelFinder				&mf = _anneal->rGraph().modelFinder();
  bool					accept;
  map<Vertex *, string>			changes;
  Clique				*cl;
  set<CComponent *>			cc;
  set<CComponent *>::const_iterator	isc, cend = cc.end();
  unsigned				n;
  set<string>				syntTypes;
  map<double, CComponent *>		ccm;
  map<double, CComponent *>::iterator	icc, fcc = ccm.end();

  ef.vertices.reserve( maxs );
  syntTypes.insert( SIA_JUNCTION_SYNTAX );
  syntTypes.insert( SIA_PLI_DE_PASSAGE_SYNTAX );

  for( ig=order.begin(); ig!=fg; ++ig )
    {
      label = (*ig).second;
      set<Vertex *>	& sv = sg[ label ];

      // séparer chaque sillon en composantes connexes
      cc.erase( cc.begin(), cend );
      ccm.erase( ccm.begin(), fcc );
      n = VertexClique::connectivity( sv, &cc, syntTypes );

      //	ordonnancement aléatoire des composantes
      for( isc=cc.begin(); isc!=cend; ++isc )
	{
	  do
	    {
	      r = rand();
	    } while( ccm.find( r ) != fcc );
	  ccm[r] = *isc;
	}

      //	essai pour chaque composante
      for( icc=ccm.begin(); icc!=fcc; ++icc )
	{
	  //ef.vertices.erase( ef.vertices.begin(), ef.vertices.end() );
	  ef.involvedCliques.erase( ef.involvedCliques.begin(), 
				    ef.involvedCliques.end() );
	  changes.erase( changes.begin(), changes.end() );

	  for( isv=(*icc).second->begin(), fsv=(*icc).second->end(); 
	       isv!=fsv; ++isv )
	    {
	      //ef.vertices.push_back( *isv );
	      changes[ *isv ] = label;
	      (*isv)->setProperty( SIA_LABEL, _anneal->voidLabel() );
	      // cliques concernées par les changements
	      if( (*isv)->getProperty( SIA_CLIQUES, sc ) )
		{
		  for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
		    ef.involvedCliques[ *ic ] = 0;
		}
	    }

	  ef.energy = 0;

	  for( ic2=ef.involvedCliques.begin(), 
		 fc2=ef.involvedCliques.end(); ic2!=fc2; ++ic2 )
	    {
	      cl = (*ic2).first;
	      cl->getProperty( SIA_POTENTIAL, E );
	      /* normalement il faudrait prendre dans 'changes' une 
		 sous-liste dont les noeuds sont effectivement dans cette 
		 clique */
	      (*ic2).second = mf.potential( cl, changes );
	      ef.energy += (*ic2).second - E;
	    }

	  E = ef.energy;
	  //	on triche un peu si E~0 sinon ca ne converge pas
	  //	(-> favoriser la config sans changements)
	  if( E >= 0 && E < 0.1 )
	    E = 0.1;
	  eE = - E / _anneal->temp();
	  if( eE > 700 )	// anti-SIGFPE
	    eE = 700;
	  ef.expEnergy = exp( eE );

	  //	décision
	  accept = false;

	  if( _anneal->mode() == Anneal::ICM )	// déterministe
	    {
	      if( ef.energy < 0 )
		accept = true;
	    }
	  else			// mode non-déterministe
	    {
	      //	tirage
	      limit = ef.expEnergy / (ef.expEnergy + 1);
	      // technologie du DoubleTirage © JeffProd'00
	      if( ran1() < limit 
		  && ( !_anneal->doubleDrawingLots() || ran1() < limit ) )
		accept = true;
	    }
	  if( accept )	  //	accepte: mettre les nouveaux potentiels
	    {
	      for( ic2=ef.involvedCliques.begin(), 
		     fc2=ef.involvedCliques.end(); ic2!=fc2; ++ic2 )
		{
		  cl = (*ic2).first;
		  cl->setProperty( SIA_POTENTIAL, (*ic2).second );
		  mf.update( cl, changes );
		}

	      //	Stats, traces de l'énergie
	      ++_ntrans;
	      _stepDeltaE += ef.energy;
	    }
	  else	// rejet: remettre les labels
	    {
	      for( isv=(*icc).second->begin(); isv!=fsv; ++isv )
		(*isv)->setProperty( SIA_LABEL, label );
	    }
	  ++_maxtrans;
	}	// composante suivante
    }		// label suivant
}
