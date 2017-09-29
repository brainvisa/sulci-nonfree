
#include <si/graph/anneal.h>
#include <si/graph/cgraph.h>
#include <si/graph/mgraph.h>
#include <si/finder/modelFinder.h>
#include <si/graph/sgiterator.h>
#include <neur/rand/rand.h>
#include <si/graph/attrib.h>
#include <si/graph/annealExtension.h>
#ifdef CARTO_THREADED_RCPTR
#include <cartobase/thread/loopContext.h>
#include <cartobase/thread/threadedLoop.h>
#include <cartobase/thread/thread.h>
#include <cartobase/thread/cpu.h>
#endif
#include <iomanip>
#include <iostream>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


struct Anneal::Private
{
#ifdef CARTO_THREADED_RCPTR
  class PotContext : public LoopContext
  {
  public:
    struct CliquePot
    {
      CliquePot( Clique *cl = 0, map<Vertex*, string> *v = 0, double p = 0 )
        : clique( cl ), vertices( v ), pot( p ) {}
      Clique			*clique;
      map<Vertex*, string>	*vertices;
      double			pot;
    };

    PotContext( vector<CliquePot> &, ModelFinder & );
    virtual void doIt( int startIndex, int count );

    vector<CliquePot>	*cliques;
    ModelFinder		*mf;
  };

  int		threaded;
  PotContext	*potcontext;
  ThreadedLoop	*threadloop;
#endif

  void reorderVertices( CGraph & );

  unsigned      maxIterations;
  unsigned      mpmUnrecordedIterations;
  map<long, Vertex *> tractable_vert;

  Private();
  ~Private();
};


Anneal::Private::Private() :
#ifdef CARTO_THREADED_RCPTR
  threaded( -1 ), potcontext( 0 ), threadloop( 0 ),
#endif
  maxIterations( 0 ), mpmUnrecordedIterations( 0 )
{
}


Anneal::Private::~Private()
{
#ifdef CARTO_THREADED_RCPTR
  delete potcontext;
  delete threadloop;
#endif
}


#ifdef CARTO_THREADED_RCPTR
Anneal::Private::PotContext::PotContext( vector<CliquePot> & c, 
                                         ModelFinder & mfind )
  : LoopContext(), cliques( &c ), mf( &mfind )
{
}


void Anneal::Private::PotContext::doIt( int startIndex, int count )
{
  //cout << "PotContext::doIt " << startIndex << endl;
  int		i, n = startIndex + count;
  for( i=startIndex; i<n; ++i )
    {
      CliquePot	& cp = (*cliques)[ i ];
      cp.pot =  mf->potential( cp.clique, *cp.vertices );
    }
}
#endif



void Anneal::Private::reorderVertices( CGraph & cgraph )
{
  if( !tractable_vert.empty() )
    return;

  CGraph::iterator iv, ev=cgraph.end();
  int index;
  long key;
  for( iv=cgraph.begin(); iv!=ev; ++iv )
  {
    if( !(*iv)->getProperty( "index", index )
        && !(*iv)->getProperty( "skeleton_label", index ) )
      key = reinterpret_cast<long
#if defined( _WIN64 )
                             long int
#endif
                            >( *iv ); // NON-TRACTABLE.
    else
      key = index;
    tractable_vert[ key ] = *iv;
  }
}


Anneal::Anneal( CGraph & cg, MGraph & rg )
  : _cgraph( cg ), _mgraph( rg ), _modeI( GIBBS ), _mode( GIBBS ), 
    _tempI( 50 ), _temp( 50 ), _finished( false ), _tmult( 0.98 ), 
    _tICM( 0 ), _stopProp( 0.05 ), _ntrans( 0 ), _maxtrans( 0 ), 
    _gibbsMaxTrans( 1 ), _stepDeltaE( 0 ), _deltaE( 0 ), _initEnergy( 0 ), 
    _verbose( false ), _iterType( VERTEX ), _sgProvider( 0 ), 
    _plotStream( 0 ), _initialLabelsType( INITLABELS_NONE ), 
    _voidLabel( "unknown" ), _voidmode( VOIDMODE_NONE ), _voidoccurency( 0 ), 
    _extensionPassOccurency( 20 ), _doubleDrawingLots( false ), 
    d( new Private )
{
}


Anneal::~Anneal()
{
  clear();
  deleteExtensions();
  delete d;
}


void Anneal::init( Anneal::Mode mode, double temp, double tmult, double tICM, 
		   double stopProp, unsigned gibbsMaxTrans, bool verbosity, 
		   IterType itType, Anneal::InitLabelsType initialLabelsType, 
		   const string & voidLabel, ostream* plotStream, 
                   unsigned niterBelowStopProp )
{
  _modeI = mode;
  _tempI = temp;
  _finished = false;
  _tmult = tmult;
  _tICM = tICM;
  _stopProp = stopProp;
  _gibbsMaxTrans = gibbsMaxTrans;
  _verbose = verbosity;
  _iterType = itType;
  _niter = 0;
  delete _sgProvider;
  _initialLabelsType = initialLabelsType;
  _plotStream = plotStream;
  _voidLabel = voidLabel;
  _niterBelowStopProp = niterBelowStopProp;

  switch( itType )
    {
    case VERTEX:
      _sgProvider = new VertexProvider( _cgraph );
      break;
    case CLIQUE:
      _sgProvider = new VertexCliqueProvider( _cgraph );
      break;
    case CUSTOM:
      cerr << "Anneal iterType CUSTOM : je sais pas encore faire.\n";
      cerr << "Je prends VERTEX.\n";
      _iterType =  VERTEX;
      _sgProvider = new VertexProvider( _cgraph );
      break;
    }

  d->maxIterations = 0;
  d->mpmUnrecordedIterations = 0;
  d->tractable_vert.clear();
}


void Anneal::setVoidMode( VoidMode mode, unsigned occurency )
{
  if( occurency == 0 )
    {
      _voidmode = VOIDMODE_NONE;
      _voidoccurency = 0;
      return;
    }
  _voidmode = mode;
  _voidoccurency = occurency;
}


void Anneal::addExtension( AnnealExtension* ae, unsigned occurency )
{
  _annealExtensions.push_back( ae );
  _extensionPassOccurency = occurency;
}


void Anneal::deleteExtensions()
{
  for( unsigned i=0; i<_annealExtensions.size(); ++i )
  delete _annealExtensions[i];
  _annealExtensions.erase( _annealExtensions.begin(), 
			   _annealExtensions.end() );
}


void Anneal::clear()
{
  CGraph::const_iterator	ig, fg = _cgraph.end();
  Vertex			*v;

  for( ig=_cgraph.begin(); ig!=fg; ++ig )
    {
      v = *ig;
      if( v->hasProperty( "potential" ) )
	v->removeProperty( "potential" );
    }
}


namespace
{

  void resetMPM( CGraph & cgraph )
  {
    Graph::iterator i, e = cgraph.end();
    for( i=cgraph.begin(); i!=e; ++i )
      if( (*i)->hasProperty( "used_labels" ) )
        (*i)->removeProperty( "used_labels" );
  }

}


void Anneal::reset()
{
  _mode = _modeI;
  _temp = _tempI;
  _finished = false;
  _ntrans = _maxtrans = 0;
  _deltaE = _stepDeltaE = 0;
  _niter = 0;
  _currentNiterBelowStopProp = 0;

  switch( _initialLabelsType )
    {
    case INITLABELS_VOID:
      _cgraph.setAllLabels( _voidLabel );
      break;
    case INITLABELS_RANDOM:
      _cgraph.randLabels();
      break;
    default:
      break;
    }
  processAllPotentials();	// initials cliques energies
  resetMPM( _cgraph );
}


void Anneal::fitStep()
{
  _ntrans = _maxtrans = 0;
  ++_niter;

  static const string modes_str[] = { "Metropolis", "     Gibbs",
                                      "       ICM", "       MPM",
                                      " Void pass" };

  bool		voidstep = false;
  unsigned	mode = _mode;

  switch( _voidmode )
    {
    case VOIDMODE_REGULAR:
      if( _niter % _voidoccurency == 0 )
	voidstep = true;
      break;
    case VOIDMODE_STOCHASTIC:
      if( ran1() * _voidoccurency <= 1 )
	voidstep = true;
      break;
    default:
      break;
    }

  if( voidstep )
  {
    mode = SPECIAL;
    stepVoid();
  }
  else
    switch( _mode )
    {
    case METROPOLIS:
      stepMetropolis();
      break;
    case GIBBS:
    case MPM:
      stepGibbs();
      break;
    case ICM:
      stepICM();
      break;
    default:
      break;
    }

  if( _verbose )
    cout << setw(5) << _niter << "  - Temp : " << setw(10) << _temp << " - " 
         << modes_str[ mode ] << " - DeltaE = " << setw(10) << _stepDeltaE
         << " - changes : " << setw(4) << _ntrans << " / " << setw(4)
         << _maxtrans << endl;

  if( _plotStream )
    (*_plotStream) << " " << setw(5) << _niter << " " << setw(10) << _temp
                   << " " << setw(10) << _deltaE + _initEnergy << " "
                   << setw(4) << _ntrans << endl;

  if( !voidstep )	// on ne peut pas s'arr�ter sur un voidstep.
    checkStop();

  if( _annealExtensions.size() != 0 
      && (_niter % _extensionPassOccurency) == 0 )
    for( unsigned i=0; i<_annealExtensions.size(); ++i )
      {
	_annealExtensions[i]->specialStep( _niter );

	if( _verbose )
	  cout << setw(5) << _niter << "  - Temp : " << setw(10) << _temp 
	       << " - " << _annealExtensions[i]->name() 
	       << " - DeltaE = " << setw(10) 
	       << _annealExtensions[i]->stepDeltaE() << " - changes : " 
	       << setw(4) 
	       << _annealExtensions[i]->ntrans() << " / " << setw(4) 
	       << _annealExtensions[i]->maxTrans() << endl;

      _deltaE += _annealExtensions[i]->stepDeltaE();
      _maxtrans += _annealExtensions[i]->maxTrans();
      _ntrans += _annealExtensions[i]->ntrans();
      _stepDeltaE += _annealExtensions[i]->stepDeltaE();

      if( _plotStream )
	(*_plotStream) << " " << setw(5) << _niter << " " << setw(10) << _temp 
		       << " " << setw(10) << _deltaE + _initEnergy << " " 
		       << setw(4) << _annealExtensions[i]->ntrans() << endl;
    }

  if( _mode != MPM && _mode != ICM )
    _temp *= _tmult;
}


void Anneal::fit()
{
  reset();
  cout << "Energie initiale : " << _initEnergy << endl;
  if( _plotStream )
    (*_plotStream) << " " << setw(5) << 0 << " " << setw(10) << _temp 
		   << " " << setw(10) << _initEnergy << " " 
		   << setw(4) << _maxtrans << endl;

  while( !_finished )
  {
    fitStep();
  }

#ifdef CARTO_THREADED_RCPTR
  // delete threads
  delete d->threadloop;
  d->threadloop = 0;
  delete d->potcontext;
  d->potcontext = 0;
#endif
}


namespace
{

  void setMPMLabels( CGraph & cgraph )
  {
    Graph::iterator i, e = cgraph.end();
    for( i=cgraph.begin(); i!=e; ++i )
      if( (*i)->hasProperty( "used_labels" ) )
      {
        Object ul = (*i)->getProperty( "used_labels" );
        Object it;
        string  maxlabel;
        int  nmax = -1, n;
        for( it= ul->objectIterator(); it->isValid(); it->next() )
        {
          n = (int) it->currentValue()->getScalar();
          if( nmax < 0 || nmax < n )
          {
            nmax = n;
            maxlabel = it->key();
          }
        }
        if( !maxlabel.empty() )
          (*i)->setProperty( SIA_LABEL, maxlabel );
      }
  }

}


void Anneal::checkStop()
{
  bool  switchmode = false;
  if( _mode != MPM && _ntrans <= _stopProp * _maxtrans )
  {
    ++_currentNiterBelowStopProp;
    if( _currentNiterBelowStopProp >= _niterBelowStopProp )
      switchmode = true;
  }
  else
  {
    _currentNiterBelowStopProp = 0;
    if( _mode != ICM && ( _temp <= _tICM
        || (d->maxIterations != 0 && _niter >= d->maxIterations ) ) )
      switchmode = true;
  }
  if( switchmode )
  {
    if( _mode == ICM )
    {
      _finished = true;
      cout << "Annealing stops. temp = " << _temp << ", energy = "
          << energy() << ", global DeltaE = " << _deltaE << "\n";
    }
    else
    {
      if( _mode == MPM )
      {
        // assign labels
        setMPMLabels( _cgraph );
        // re calculate potentials because all labels have changed
        double en = _initEnergy;
        processAllPotentials();
        cout << "End of MPM, labels set, deltaE: " << _initEnergy - en << endl;
        _deltaE += _initEnergy - en;
        _initEnergy = en;
      }

      _mode = ICM;
      _currentNiterBelowStopProp = 0;
      cout << "Annealing switching to ICM mode\n";
    }
  }
}


void Anneal::stepMetropolis()
{
  set<Clique*>			*cs;
  set<Clique*>::const_iterator	ic, fc;
  Clique			*cl = 0;
  double			E, E2, rnd;
  map<double,Vertex*>		mv;
  map<double,Vertex*>::const_iterator	im, fm;
  ModelFinder			&mf = _mgraph.modelFinder();
  string			label1, label2;
  Vertex			*v;
  vector<string>		*pl;
  map<Clique*, double>		tmp_pot;
  map<Clique*, double>::iterator	itp, ftp;

  _stepDeltaE = 0;

  //	Tirer au sort l'ordre de passage des vertex
  d->reorderVertices( _cgraph );
  map<long, Vertex *>::iterator iv, ev=d->tractable_vert.end();
  for( iv=d->tractable_vert.begin(); iv!=ev; ++iv )
  {
    do
    {
      rnd = ran1();
    } while( mv.find( rnd ) != mv.end() );

    mv[ rnd ] = iv->second;
  }

  map<Vertex*, string>	changes;


  //	Iteration on eachVertex in drawn order
  for( im=mv.begin(), fm=mv.end(); im!=fm; ++im )
    {
      v = (*im).second;

      if( v->hasProperty( "cliques" ) )
        {
          v->getProperty( "cliques", cs );
          // choose a new label for v
          v->getProperty( "label", label1 );
          v->getProperty( "possible_labels", pl );
          changes.erase( changes.begin(), changes.end() );
          changes[v] = label1;

          label2 = (*pl)[ (int) ran1()*(pl->size()-1) ];
          if( label1 == label2 ) label2 = (*pl)[ pl->size() - 1 ];
          v->setProperty( "label", label2 );
          E = 0;
          tmp_pot.erase( tmp_pot.begin(), tmp_pot.end() );

          for( ic=cs->begin(), fc=cs->end(); ic!=fc; ++ic )
            {
              cl = *ic;
              // Clique energy after label change of vertex *im
              E2 = mf.potential( cl, changes );
              tmp_pot[cl] = E2;
              // Energy difference on all cliques
              E += E2;
              cl->getProperty( "potential", E2 ); // before change
              E -= E2;
            }
          // decision
          ++_maxtrans;
          if( E < 0 || ran1() <= exp( -(E+0.1) / _temp ) )
            { // update accepted
              ++_ntrans;
              // update all new cliques potentials
              for( itp=tmp_pot.begin(), ftp=tmp_pot.end(); itp!=ftp; ++itp )
                (*itp).first->setProperty( SIA_POTENTIAL, (*itp).second );
              _stepDeltaE += E;
              mf.update( cl, changes ); // update internal things
            }
          else	// reject transformation
            { // set back initial label
              v->setProperty( SIA_LABEL, label1 );
            }
        }
    }

  _deltaE += _stepDeltaE;
}


void Anneal::stepGibbs()
{
  assert( _sgProvider );

  vector<EnergyField>			ef;
  EnergyField				*en = 0;
  double				fate;
  unsigned				i, n, p, count = 0;
  map<Clique*, double>::const_iterator	ime, fme;
  SGProvider::const_iterator		is, fs;
  map<Vertex *, string>			changes;
  Vertex				*v;
  ModelFinder				& mf = _mgraph.modelFinder();
  Clique				*cl;
  set<Vertex *>::iterator               iv, ev;
  string                                label;

  _stepDeltaE = 0;

  //	Remettre a jour le provider de sous-graphes
  _sgProvider->refresh();

  if( _verbose )
    cout << "      / " << _sgProvider->size() << "\r";

  //	Iteration sur les sous-graphes dans l'ordre qu'il faut
  for( is=_sgProvider->begin(), fs=_sgProvider->end(); is!=fs; ++is )
    {
      if( _verbose )
      {
        string str;
        ++count;
        cout << setw( 5 ) << count << " " << (*is)->size();
        if( (*is)->size() == 1 &&
            (*(*is)->begin())->getProperty( SIA_LABEL, str ) )
          cout << " " << str;
        cout << "                         \r" << flush;
      }
      //	Calcul des transitions possibles et des 
      //	corresponding probabilities
      processPotentials( **is, ef );
      //  cout << "\t" << setw( 3 ) << ef.size() << "\r";

      //	Tirage, c'est la grande roue du hasard...
      fate = ran1();
      for( p=0; ef[p].probabilitySum < fate; ++p ) {}

      if( p != 0 )	// did we change anything after all ?
      {
        en = &ef[p];
        changes.erase( changes.begin(), changes.end() );
        //	Fixer les labels correspondants
        n = en->vertices.size();
        //cout << "clique changed : \n";
        for( i=0; i<n; ++i )
          {
            v = en->vertices[i];
            v->getProperty( SIA_LABEL, changes[v] );
            v->setProperty( SIA_LABEL, en->labels[i] );
            //cout << changes[v] << " -> " << en->labels[i] << endl;
          }
        //	update involved cliques energies
        for( ime=en->involvedCliques.begin(), fme=en->involvedCliques.end();
              ime!=fme; ++ime )
          {
            cl = (*ime).first;
            /* normalement il faudrait prendre dans 'changes' une sous-liste
                dont les noeuds sont effectivement dans cette clique */
            mf.update( cl, changes );

            cl->setProperty( SIA_POTENTIAL, (*ime).second );
          }
        //	Stats, track energy
        ++_ntrans;
        _stepDeltaE += en->energy;
      }
      //else cout << "clique unchanged.\n";
      if( _mode == MPM && _niter > d->mpmUnrecordedIterations )
      {
        // record labels usage statistics
        for( iv=(*is)->begin(), ev=(*is)->end(); iv!=ev; ++iv )
        {
          v = *iv;
          Object  labelsch;
          if( v->hasProperty( "used_labels" ) )
            labelsch = v->getProperty( "used_labels" );
          else
          {
            labelsch = Object::value( map<string, unsigned>() );
            v->setProperty( "used_labels", labelsch );
          }
          v->getProperty( SIA_LABEL, label );
          unsigned x = 0;
          labelsch->getProperty( label, x );
          ++x;
          labelsch->setProperty( label, x );
        }
      }
      ++_maxtrans;
    }

  _deltaE += _stepDeltaE;
}


void Anneal::stepICM()
{
  assert( _sgProvider );

  vector<EnergyField>			ef;
  EnergyField				*en;
  unsigned				i, n, p, count = 0;
  double				Emin;
  map<Clique*, double>::const_iterator	ime, fme;
  SGProvider::const_iterator		is, fs;
  map<Vertex *, string>			changes;
  Vertex				*v;
  ModelFinder				& mf = _mgraph.modelFinder();

  _stepDeltaE = 0;

  //	update subgraph provider
  _sgProvider->refresh();

  if( _verbose )
    cout << "      / " << _sgProvider->size() << "\r";

  //	Iteration sur les sous-graphes dans l'ordre qu'il faut
  for( is=_sgProvider->begin(), fs=_sgProvider->end(); is!=fs; ++is )
    {
      if( _verbose )
	{
	  ++count;
	  cout << setw( 5 ) << count << flush << "\r";
	}
      //	Calcul des transitions possibles et des 
      //	probabilit�s correspondantes
      processPotentials( **is, ef );
      //	Recherche du minimum
      Emin = ef[0].energy;
      p = 0;
      for( i=1; i<ef.size(); ++i )
	if( ef[i].energy < Emin )
	  {
	    Emin = ef[i].energy;
	    p = i;
	  }

      if( p != 0 )	// on a chang� qqch en fin de compte ?
	{
	  en = &ef[p];
	  changes.erase( changes.begin(), changes.end() );
	  //	Fixer les labels correspondants
	  n = en->vertices.size();
	  for( i=0; i<n; ++i )
	    {
	      v = en->vertices[i];
	      v->getProperty( SIA_LABEL, changes[v] );
	      v->setProperty( SIA_LABEL, en->labels[i] );
	    }
	  //	mettre � jour les �nergies des cliques impliqu�es
	  for( ime=en->involvedCliques.begin(), fme=en->involvedCliques.end(); 
	       ime!=fme; ++ime )
	    {
	      mf.update( (*ime).first, changes );
	      (*ime).first->setProperty( SIA_POTENTIAL, (*ime).second );
	    }
	  //	Stats, traces de l'�nergie
	  ++_ntrans;
	  _stepDeltaE += en->energy;
	}
      ++_maxtrans;
    }

  _deltaE += _stepDeltaE;
}


void Anneal::processPotentials( const set<Vertex *> & ver, 
				vector<EnergyField> & ef )
{
  ef.erase( ef.begin(), ef.end() );

  unsigned		nn;
  static EnergyField	noChange;

  //	nb de noeuds � choisir
  if( ver.size() < _gibbsMaxTrans ) nn = ver.size();
  else nn = _gibbsMaxTrans;

  //	1er �l�ment du tableau: celui ou rien ne change
  noChange.energy = 0.;
  noChange.expEnergy = 1.;
  ef.push_back( noChange );

  bool		npos[ver.size()];
  string*	orLab = new string[ver.size()];
  unsigned	i;
  set<Vertex *>::const_iterator	ic;

  //	memorize initial labels
  for( i=0, ic=ver.begin(); i<ver.size(); ++i, ++ic )
    (*ic)->getProperty( "label", orLab[i] );

  //	calcule toutes les configs
  processNodes( ver, ef, npos, 0, nn, orLab );

  delete[] orLab;

  //	calcule les probas
  double		sumE = 0., sumP = 0.;

  for( i=0; i<ef.size(); ++i )
    sumE += ef[i].expEnergy;

  for( i=0; i<ef.size(); ++i )
    {
      ef[i].probability = ef[i].expEnergy / sumE;
      sumP += ef[i].probability;
      ef[i].probabilitySum = sumP;
    }
}


void Anneal::processNodes( const set<Vertex *> & ver, 
			   vector<EnergyField> & ef, bool* npos, 
			   unsigned first, unsigned nn, string* orLab )
{
  unsigned			i, j, p = ver.size() - nn + 1;
  vector<string>		*pl;
  vector<string>::const_iterator	il, fl;
  string			label;
  set<Vertex *>::const_iterator	ic;
  Vertex			*v;

  for( i=first+1; i<ver.size(); ++i )
    npos[i] = false;	// vider la fin de la ligne

  for( i=first; i<p; ++i )
    {
      npos[i] = true;
      // il faut tester les labels : un coup pour chaque label possible
      // d'abord r�cup�rer le noeud qui nous interesse: le i-�me
      for( ic=ver.begin(), j=0; j<i; ++j ) ++ic;
      v = *ic;					// c'est celui-l�.
      v->getProperty( "label", label );	// r�cup�rer le label d'origine
      v->getProperty( "possible_labels", pl );
      for( il=pl->begin(), fl=pl->end(); il!=fl; ++il )
	{
	  v->setProperty( "label", *il );
	  if( nn == 1 )	// neud terminal ?
	    processConfig( ver, ef, npos, orLab );	// calcule vraiment
	  else		// encore plusieurs noeuds non affect�s
	    processNodes( ver, ef, npos, i+1, nn-1, orLab );
	}
       //	remettre le label en place
       v->setProperty( "label", label );
       npos[i] = false;
    }
}


void Anneal::processConfig( const set<Vertex *> & ver, 
			    vector<EnergyField> & ef, bool* npos, 
			    string* orLab )
{
  EnergyField			pot;
  unsigned			i;
  set<Vertex *>::const_iterator	ic, fc=ver.end();
  string			label;
  ModelFinder			&mf = _mgraph.modelFinder();
  map<Clique*, map<Vertex*, string> >	cens;
  map<Clique*, map<Vertex*, string> >::iterator	icens, fcens;
  set<Clique*>			*cs;
  set<Clique*>::const_iterator	is, fs;
  Vertex			*v;
  double			E = 0, E0;

  for( i=0, ic=ver.begin(); ic!=fc; ++ic, ++i )
    if( npos[i] )
      {
	v = *ic;
	v->getProperty( SIA_LABEL, label );
	if( label != orLab[i] && v->getProperty( SIA_CLIQUES, cs ) )
	  {	// ce label a chang� ?
	    pot.vertices.push_back( v );
	    pot.labels.push_back( label );

	    for( is=cs->begin(), fs=cs->end() ; is!=fs; ++is )
	      {
		// ins�re ou r�cup�re la description de la clique dans cens
		map<Vertex*, string> & chVert = cens[ *is ];
		//chVert[v] = label;	// c'est pas plut�t orlab ??
		chVert[v] = orLab[i];
	      }
	  }
      }
  /*	OK. maintenant, cens est l'ensemble des cliques concern�es par le 
	chgt, et chaque clique est associ�e � un ensemble de noeuds qui 
	changent, chacun lui-m�me reli� � son label d'origine
  */

  if( pot.vertices.size() > 0 )	// si rien n'a chang� c'est pas la peine
    {
      //	Calcul du potentiel, pour chaque clique de cens

#ifdef CARTO_THREADED_RCPTR
      if( d->threaded < 0 )
        {
          if( cpuCount() > 1 )
            d->threaded = 1;
          else
            d->threaded = 0;
          // FIXME : threads are disabled during debug
          //d->threaded = 0;
        }
      if( d->threaded == 1 )
        {
          unsigned					i, n = cens.size();
          vector<Private::PotContext::CliquePot>	cens2( n );

          for( icens=cens.begin(), fcens=cens.end(), i=0; icens!=fcens; 
               ++icens, ++i )
            cens2[i] = Private::PotContext::CliquePot( icens->first, 
                                                       &icens->second, 0 );

          if( !d->potcontext )
            d->potcontext = new Private::PotContext( cens2, mf );
          else
            {
              d->potcontext->cliques = &cens2;
              d->potcontext->mf = &mf;
            }
          if( !d->threadloop )
            {
              cout << "new threaded loop for " << rint( 1 * cpuCount() ) 
                   << " threads\n";
              d->threadloop = new ThreadedLoop( d->potcontext, 0, n );
              d->threadloop->setEndThreadsWhenDone( false );
            }
          else
            d->threadloop->setLoopCount( n );
          //cout << "start threads for " << n << " pots\n";
          d->threadloop->launch();
          //cout << "stop threads\n";

          for( icens=cens.begin(), i=0; icens!=fcens; 
               ++icens, ++i )
            {
              E0 = cens2[i].pot;
              pot.involvedCliques[ (*icens).first ] = E0;
              E += E0;
              (*icens).first->getProperty( SIA_POTENTIAL, E0 );
              E -= E0;
            }

          // DEBUG
          //delete d->threadloop;
          //d->threadloop = 0;
          //delete d->potcontext;
          //d->potcontext = 0;
        }
      else
        {
#endif

          for( icens=cens.begin(), fcens=cens.end(); icens!=fcens; ++icens )
            {
              E0 = mf.potential( (*icens).first, (*icens).second );
              //double debug = E0;
              pot.involvedCliques[ (*icens).first ] = E0;
              E += E0;
              (*icens).first->getProperty( SIA_POTENTIAL, E0 );
              /*cout << "cl " << (*icens).first << " : " << E0 << " -> " 
                << debug;
                if( (*icens).second.size() == 1 )
                {
                cout << ",  " << (*(*icens).second.begin()).first << " : " 
                << (*(*icens).second.begin()).second;
                string	l;
                (*(*icens).second.begin()).first->getProperty( SIA_LABEL, l );
                cout << " -> " << l;
                }
                cout << endl;*/
              E -= E0;
            }
#ifdef CARTO_THREADED_RCPTR
        }
#endif
      pot.energy = E;
      //	on triche un peu si E~0 sinon ca ne converge pas
      //	(-> favoriser la config sans changements)
      if( E >= 0 && E < 0.1 )
	E = 0.1;
      E /= _temp;
      if( E < -700 )	// protection contre SIGFPE.
	E = -700;
      pot.expEnergy = exp( - E );

      ef.push_back( pot );
    }
}


double Anneal::processAllPotentials()
{
  double		ener = 0, pot;
  CGraph::CliqueSet::const_iterator	ic, fc=_cgraph.cliques().end();
  ModelFinder	&mf = _mgraph.modelFinder();
  Clique	*cl;

  mf.clear();	// vide le cache �ventuel avant de commencer

  for( ic=_cgraph.cliques().begin(); ic!=fc; ++ic )
    {
      cl = ic->get();
      pot = mf.update( cl );
      //pot = mf.potential( cl );
      ener += pot;
      cl->setProperty( "potential", pot );
    }

  _initEnergy = ener;
  return( ener );
}


void Anneal::stepVoid()
{
  _stepDeltaE = 0;

  // partition des noeuds selon leur label

  map<string, set<Vertex *> >	sg;
  CGraph::const_iterator	iv, fv=_cgraph.end();
  string			label;
  double			r;
  map<double, string>		order;
  unsigned			maxs = 0;

  // partion nodes according to their label.
  for( iv=_cgraph.begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_LABEL, label ) && label != _voidLabel )
    {
      set<Vertex *>	& sv = sg[label];
      sv.insert( *iv );
      if( sv.size() > maxs )
        maxs = sv.size();
    }

  // randomly reorder labels (assigning them in a controlled order)
  map<string, set<Vertex *> >::iterator isg, esg = sg.end();
  for( isg=sg.begin(); isg!=esg; ++isg )
  {
    do
      {
        r = ran1();
      } while( order.find( r ) != order.end() );
    order[ r ] = isg->first;
  }


  // iterate on each group

  map<double, string>::const_iterator	ig, fg=order.end();
  set<Vertex *>::const_iterator		isv, fsv;
  EnergyField				ef;
  set<Clique *>				*sc;
  set<Clique *>::const_iterator		ic, fc;
  map<Clique *, double>::iterator	ic2, fc2;
  double				E, eE, limit;
  ModelFinder				&mf = _mgraph.modelFinder();
  bool					accept;
  map<Vertex *, string>			changes;
  Clique				*cl;

  ef.vertices.reserve( maxs );

  for( ig=order.begin(); ig!=fg; ++ig )
    {
      label = (*ig).second;
      set<Vertex *>	& sv = sg[ label ];
      //ef.vertices.erase( ef.vertices.begin(), ef.vertices.end() );
      ef.involvedCliques.erase( ef.involvedCliques.begin(), 
				ef.involvedCliques.end() );
      changes.erase( changes.begin(), changes.end() );

      for( isv=sv.begin(), fsv=sv.end(); isv!=fsv; ++isv )
	{
	  //ef.vertices.push_back( *isv );
	  changes[ *isv ] = label;
	  (*isv)->setProperty( SIA_LABEL, _voidLabel );
	  if( (*isv)->getProperty( SIA_CLIQUES, sc ) )
	    {
	      for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
		ef.involvedCliques[ *ic ] = 0;
	    }
	}

      /* si aucune clique n'est en cause (label "brain_hull" dans les sillons 
	 par exemple), rien � faire */
      if( !ef.involvedCliques.empty() )
	{
	  ef.energy = 0;

	  for( ic2=ef.involvedCliques.begin(), fc2=ef.involvedCliques.end(); 
	       ic2!=fc2; ++ic2 )
	    {
	      cl = (*ic2).first;
	      cl->getProperty( SIA_POTENTIAL, E );
	      /* normalement il faudrait prendre dans 'changes' une sous-liste
		 dont les noeuds sont effectivement dans cette clique */
	      (*ic2).second = mf.potential( cl, changes );
	      ef.energy += (*ic2).second - E;
	    }

	  E = ef.energy;
	  //	on triche un peu si E~0 sinon ca ne converge pas
	  //	(-> favoriser la config sans changements)
	  if( E >= 0 && E < 0.1 )
	    E = 0.1;
	  eE = - E / _temp;
	  if( eE > 700 )	// anti-SIGFPE
	    eE = 700;
	  ef.expEnergy = exp( eE );

	  //	decision
	  accept = false;

	  if( _mode == ICM )	// d�terministe
	    {
	      if( ef.energy < 0 )
		accept = true;
	    }
	  else	// non-deterministic mode
	    {
	      //	tirage
	      limit = ef.expEnergy / (ef.expEnergy + 1);
	      // syst�me du DoubleTirage � JeffProd'00
	      if( ran1() < limit && ( !_doubleDrawingLots || ran1() < limit ) )
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

	      //	Stats, traces de l'�nergie
	      ++_ntrans;
	      _stepDeltaE += ef.energy;
	    }
	  else	// rejet: remettre les labels
	    {
	      for( isv=sv.begin(), fsv=sv.end(); isv!=fsv; ++isv )
		(*isv)->setProperty( SIA_LABEL, label );
	    }
	  ++_maxtrans;
	}	// fin si involvedCliques n'est pas vide
    }		// label suivane

  _deltaE += _stepDeltaE;
}


void Anneal::setAllowThreads( bool x )
{
#ifdef CARTO_THREADED_RCPTR
  if( x )
    d->threaded = -1;
  else
    d->threaded = 0;
#endif
}


bool Anneal::threadsAllowed() const
{
#ifdef CARTO_THREADED_RCPTR
  if( d->threaded < 0 )
  {
    if( cpuCount() > 1 )
      return true;
    else
      return false;
  }
  else
    return d->threaded > 0;
#else
  return false;
#endif
}


void Anneal::setMaxIterations( unsigned x )
{
  d->maxIterations = x;
}


unsigned Anneal::maxIterations() const
{
  return d->maxIterations;
}


void Anneal::setMPMUnrecordedIterations( unsigned n )
{
  d->mpmUnrecordedIterations = n;
}


unsigned Anneal::MPMUnrecordedIterations() const
{
  return d->mpmUnrecordedIterations;
}


