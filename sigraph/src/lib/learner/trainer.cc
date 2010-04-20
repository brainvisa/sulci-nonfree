
#include <si/learner/trainer.h>
#include <si/domain/adapDomain.h>
#include <si/graph/attrib.h>
#include <neur/rand/rand.h>
#include <iostream>
#include <iomanip>

#include <si/finder/modelFinder.h>
#include <si/subadaptive/subAdaptive.h>
#include <si/subadaptive/nonIncrementalSubAdaptive.h>
#include <si/model/adaptive.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/topAdaptive.h>
#include <cartobase/exception/assert.h>
#include <cartobase/stream/fileutil.h>


using namespace carto;
using namespace sigraph;
using namespace std;


Trainer::Trainer( MGraph & mg, Learner *learner) 
  : _mgraph( mg ), _learner(learner), _pass( 0 ), _learnfinished( false ), 
    _mode(GenerateAndTrain)
{
	if (_learner) _learner->setProperty(SIA_MODEL, &mg);
}


Trainer::~Trainer()
{
}

void Trainer::resetDomains()
{
	MGraph::iterator	im, fm=_mgraph.end();
	Domain			*dom;
	AdapDomain		*ad;

	for (im=_mgraph.begin(); im!=fm; ++im)
	if ((*im)->getProperty( SIA_DOMAIN, dom))
	{
		ad = dynamic_cast<AdapDomain *>(dom);
		if(ad) ad->reset();
	}
}

std::set<Model *>	*Trainer::modelsFromCliquesModelMap(
						const CliquesModelMap *cllrn,
						const CliquesModelMap *cltst)
{
	CliquesModelMap::const_iterator		icl, fcl;
	MGraph::iterator			im, fm;
	std::set<Model *>			*models = NULL;
	Model					*mod = NULL;
	set<Edge*>				ed = _mgraph.edges();
	set<Edge*>::iterator 			ir, fr = ed.end();
	unsigned int				i;
	  
	
	models = new std::set<Model *>();

	for (i = 0, icl = cllrn->begin(), fcl = cllrn->end();
					icl != fcl; ++icl, ++i)
		models->insert((*icl).first);
	if (cltst)
	for (i = 0, icl = cltst->begin(), fcl = cltst->end();
					icl != fcl; ++icl, ++i)
		models->insert((*icl).first);

	if (models->size() != 0) return models;

	for (im = _mgraph.begin(), fm = _mgraph.end(); im != fm; ++im)
		if ((*im)->getProperty(SIA_MODEL, mod))
			models->insert(mod);
	for (ir = ed.begin(); ir != fr; ++ir)
		if ((*ir)->getProperty(SIA_MODEL, mod))
			models->insert(mod);

	return models;
}


Trainer::CliquesModelMap *
Trainer::dataBaseToCliquesModelMap(const set<CGraph *> &lrn)
{
  set<CGraph *>::const_iterator		ig, fg = lrn.end();
  CliquesModelMap				*cl = new CliquesModelMap;
  std::list<Clique *>::const_iterator	ic, fc;
  Model					*mod;
  ModelFinder   				&mf = _mgraph.modelFinder();

  //Regrouper les cliques selon leur modèle
  for (ig = lrn.begin(); ig != fg; ++ig)
  {
    const CGraph::CliqueSet		&cs = (**ig).cliques();
    CGraph::CliqueSet::const_iterator	ic, fc = cs.end();

    for(ic = cs.begin(); ic != fc; ++ic)
    {
      AttributedObject	*modV = mf.selectModel(ic->get());

      if(modV)
      {
        ASSERT(modV->getProperty(SIA_MODEL, mod));
        (*cl)[mod].push_front(ic->get());
      }
    }
  }
  return cl;
}


/** Redefined in python in pysigraph.
 *  In C++, used only by siDomTrain.
 */
void Trainer::trainOne( Trainer::iterator &it, carto::Object &)
{
	std::set<std::string>::const_iterator	il, el;
	Model					*mod = it.model();

	if(!mod)
	{
		cerr << "Trainer::trainOne: no model\n";
		return;
	}
	if (!mod->isAdaptive())
	{
		std::cerr << "skip learning of non adaptive model"
			<< std::endl;
		return;
	}
	if (!mod->topModel())
	{
		cerr << "Trainer::trainOne: no TopModel\n";
		return;
	}
	const std::set<std::string> &labels =
		mod->topModel()->significantLabels();

	std::cout << "* ";
	for (il = labels.begin(), el = labels.end(); il != el; ++il)
		if (*il != "unknown")
			std::cout << " " << *il;
	std::cout << " : " << std::endl;

	Adaptive        *adap = ((Adaptive *) mod);
	if (!adap)
	{
		std::cerr << "model is (NULL)" << std::endl;
		std::cerr << "stop learning" << std::endl;
		return; // FIXME: throw an exception ?
	}

	switch (_mode)
	{
		case TrainDomain:
			trainDomain(it);
			break;
		case GenerateOnly:
		case GenerateAndTrain:
		case ReadAndTrain:
		case TrainStats:
		default:
			std::cerr << "mode '" << _mode << "' : invalid used "
				  << "from C++, used it from python siLearn.py"
				  << std::endl;
			break;
	};
}

//Fonction principale
void    Trainer::train(const set<CGraph *> *lrnBase,
			const set<CGraph *> *tstBase,
			int cycles, int cycles_tst)
{
	iterator i(*this, lrnBase, tstBase, cycles, cycles_tst);

	carto::Object	o; //empty object

	while (i.isValid())
	{
		if (i.adaptive()) i.train(o);
		i.next();
	}
}


void	Trainer::init(TrainerMode mode, unsigned pass )
{
	_mode = mode;
	_pass = pass;
	_learnfinished = false;
}


Trainer::iterator 
Trainer::trainIterator( const std::set<CGraph *> *lrnBase,
                        const std::set<CGraph *> *tstBase, int c, int ct )
{
	return TrainerIterator(*this, lrnBase,tstBase, c, ct);
}


void Trainer::generateDataBase(Trainer::iterator &i, const std::string &prefix)
{
  Adaptive		*adap = i.adaptive();
  CliquesModelMap	*cmtst = i.testBase();
  adap->generateDataBase( *_learner, prefix,
                          &(*i.learnBase())[adap],
                          cmtst ? &((*cmtst)[adap]) : 0,
                          i.cycles(), i.testCycles() );
}


void Trainer::trainDomain( Trainer::iterator & i )
{
  Adaptive	*adap = i.adaptive();
  adap->trainDomain( (*i.learnBase())[adap] );
}


void Trainer::trainStats( Trainer::iterator & i )
{
  Adaptive	*adap = i.adaptive();
  adap->trainStats(*_learner, (*i.learnBase())[adap] );
}



