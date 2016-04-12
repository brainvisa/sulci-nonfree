
#include <cstdlib>
#include <si/fold/annealConnectExtension.h>
#include <si/graph/anneal.h>
#include <si/graph/vertexclique.h>
#include <si/graph/cgraph.h>
#include <si/fold/fattrib.h>
#include <si/finder/modelFinder.h>
#include <neur/rand/rand.h>
#include <iostream>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


AnnealConnectExtension::~AnnealConnectExtension()
{
}


void AnnealConnectExtension::specialStep( unsigned )
{
  //	liste des cliques de sillon et tirage de l'ordre de passage

  map<unsigned, VertexClique*>	cliques;
  map<unsigned, VertexClique*>::const_iterator	icm, fcm = cliques.end();
  string			label;
  map<VertexClique*, string>		labels;
  unsigned			p;
  ModelFinder			& mf = _anneal->rGraph().modelFinder();
  CGraph::CliqueSet::const_iterator ic, fc=_anneal->cGraph().cliques().end();
  AttributedObject		*mao;
  map<Vertex *, string>		changes;
  string			voidl = _anneal->voidLabel();
  map<string, Clique *>          clord;
  map<string, Clique *>::const_iterator iclo, eclo = clord.end();

  //  cout << "Passe connexe...\n";
  _ntrans = 0;
  _maxtrans = 0;
  _stepDeltaE = 0;

  // order cliques by label
  for( ic=_anneal->cGraph().cliques().begin(); ic!=fc; ++ic )
  {
    // modèle associé à la clique
    mao = mf.selectModel( ic->get() );
    // ne garder que les modèles de noeuds (1 label)
    if( mao && mao->getProperty( SIA_LABEL, label )
        && label != voidl )
      clord[label] = ic->get();
  }

  // from this reproducible order, randomly reorder them
  for( iclo=clord.begin(); iclo!=eclo; ++iclo )
  {
    // tirer un numéro
    do
      {
        p = rand();
      } while( cliques.find( p ) != fcm );
    // stocker la clique numérotée et le label
    cliques[p] = (VertexClique *) iclo->second;
    labels[(VertexClique *) iclo->second] = label;
  }

  //	pour chaque clique dans l'ordre de tirage, 
  //	découpage en composantes connexes

  set<Vertex *>					voidNodes;
  set<string>					syntTypes;
  set<CComponent *>				cc;
  set<CComponent *>::const_iterator		isc, cend = cc.end();
  unsigned					n;
  map<unsigned, CComponent *>			ccm;
  map<unsigned, CComponent *>::iterator		icc, fcc = ccm.end();
  EnergyField					ef;
  CComponent::const_iterator			iv, fv;
  set<Clique *>					*sc;
  set<Clique *>::const_iterator                 ics, ecs;
  map<Clique *, double>::iterator		ic2, fc2;
  double					E, eE, limit;
  bool						accept;
  Clique					*cl;
  map<long, CComponent *>                 ccord;
  map<long, CComponent *>::const_iterator ico, eco = ccord.end();
  Vertex                                  *v;
  int                                     index;
  long                                    key;

  syntTypes.insert( SIA_JUNCTION_SYNTAX );
  syntTypes.insert( SIA_PLI_DE_PASSAGE_SYNTAX );

  for( icm=cliques.begin(); icm!=fcm; ++icm )
    {
      cc.erase( cc.begin(), cend );
      ccm.erase( ccm.begin(), fcc );
      label = labels[ (*icm).second ];
      // noeuds de la clique de label "unknown"
      voidNodes = (*icm).second->getVerticesWith( SIA_LABEL, voidl );
      // si la clique est vide, on saute
      if( !voidNodes.empty() )
	{
	  // découpage en CC
	  n = VertexClique::connectivity( voidNodes, &cc, syntTypes );
	  cout << "label " << label << " : " << n 
	       << " composantes             \r" 
	       << flush;

          // reorder cc in a reproducible way
          ccord.clear();
          for( isc=cc.begin(); isc!=cend; ++isc )
          {
            Vertex *v = *(*isc)->begin();
            if( v->getProperty( "index", index )
                || v->getProperty( "skeleton_label", index ) )
              key = index;
            else
              key = reinterpret_cast<long>( v );
            ccord[ key ] = *isc;
          }
          /* in this order, randomly re-order
            (this is done in 2 passes to ensure reproducibility of random
            numbers order when we want to control srand)
          */
          for( ico=ccord.begin(); ico!=eco; ++ico )
          {
            do
              {
                p = rand();
              } while( ccm.find( p ) != fcc );
            ccm[p] = ico->second;
          }

	  //	essai pour chaque composante
	  for( icc=ccm.begin(); icc!=fcc; ++icc )
	    {
	      //ef.vertices.erase( ef.vertices.begin(), ef.vertices.end() );
	      ef.involvedCliques.erase( ef.involvedCliques.begin(), 
					ef.involvedCliques.end() );
	      changes.erase( changes.begin(), changes.end() );

	      //	config à tester: passer tout de "unknown" à 'label'
	      for( iv=(*icc).second->begin(), fv=(*icc).second->end(); 
		   iv!=fv; ++iv )
		{
		  // répertorier les noeuds et mettre leur label à 'label'
		  //ef.vertices.push_back( *iv );
		  changes[ *iv ] = voidl;
		  (*iv)->setProperty( SIA_LABEL, label );
		  // cliques concernées par les changements
		  if( (*iv)->getProperty( SIA_CLIQUES, sc ) )
		    {
		      for( ics=sc->begin(), ecs=sc->end(); ics!=ecs; ++ics )
			ef.involvedCliques[ *ics ] = 0;
		    }
		}
	      ef.energy = 0;

	      //	calcul des différences de potentiels
	      for( ic2=ef.involvedCliques.begin(), 
		     fc2=ef.involvedCliques.end(); 
		   ic2!=fc2; ++ic2 )
		{
		  (*ic2).first->getProperty( SIA_POTENTIAL, E );
		  (*ic2).second = mf.potential( (*ic2).first, changes );
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
		  for( iv=(*icc).second->begin(); iv!=fv; ++iv )
		    (*iv)->setProperty( SIA_LABEL, voidl );
		}
	      ++_maxtrans;
	    }	// CC suivante
	}	// (if clique pas vide)
    }		// clique suivante
}
