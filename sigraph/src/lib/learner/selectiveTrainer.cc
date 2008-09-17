
#include <si/learner/selectiveTrainer.h>
#include <si/model/adaptive.h>
#include <si/finder/modelFinder.h>
#include <si/model/mWriter.h>
#include <si/graph/attrib.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


SelectiveTrainer::SelectiveTrainer(MGraph & mg, Learner *learner,
				    const string & pattern )
  : Trainer( mg, learner)
{
	assert(!regcomp(&_pattern, pattern.c_str(), REG_NOSUB | REG_EXTENDED));
}


SelectiveTrainer::~SelectiveTrainer()
{
	regfree(&_pattern);
}


void SelectiveTrainer::setFiltAttributes(const set<string> &atts)
{
	_atts = atts;
}


void SelectiveTrainer::setPattern( const string & pat )
{
	regfree(&_pattern);
	assert(!regcomp(&_pattern, pat.c_str(), REG_NOSUB | REG_EXTENDED));
}


Trainer::CliquesModelMap *
SelectiveTrainer::dataBaseToCliquesModelMap(const set<CGraph *> &lrn)
{	
	set<CGraph *>::const_iterator		ig, fg = lrn.end();
	CliquesModelMap				*cl = new CliquesModelMap;
	std::list<Clique *>::const_iterator	ic, fc;
	Model					*mod;
	Adaptive				*adap = NULL;
	ModelFinder   				&mf = _mgraph.modelFinder();

	//Regrouper les cliques selon leur modèle
	for (ig = lrn.begin(); ig != fg; ++ig)
	{
		const set<Clique*>		&cs = (**ig).cliques();
		set<Clique*>::const_iterator	ic, fc = cs.end();
	
  		for(ic = cs.begin(); ic != fc; ++ic)
		{
			AttributedObject	*modV = mf.selectModel(*ic);

			if(modV)
			{
				assert(modV->getProperty(SIA_MODEL, mod));
				adap = (Adaptive *) mod;
				if(_usedAdap.find(adap) != _usedAdap.end() ||
					checkAdap(modV, adap))
					(*cl)[mod].push_front(*ic);
			}
		}
	}
	return cl;
}

std::set<Model *>	*SelectiveTrainer::modelsFromCliquesModelMap(
						const CliquesModelMap *cllrn,
						const CliquesModelMap *cltst)
{
	CliquesModelMap::const_iterator		icl, fcl;
	MGraph::iterator			im, fm;
	std::set<Model *>			*models = NULL;
	AttributedObject			*modV = NULL;
	Adaptive				*adap = NULL;
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
	{
		modV = mod->graphObject();
		adap = dynamic_cast<Adaptive *>(mod);
		if(adap && (_usedAdap.find(adap) != _usedAdap.end() ||
			checkAdap(modV, adap)))
			models->insert(mod);
	}
	for (ir = ed.begin(); ir != fr; ++ir)
	if ((*ir)->getProperty(SIA_MODEL, mod))
	{
		modV = mod->graphObject();
		adap = dynamic_cast<Adaptive *>(mod);
		if(adap && (_usedAdap.find(adap) != _usedAdap.end() ||
			checkAdap(modV, adap)))
			models->insert(mod);
	}

	return models;
}

bool SelectiveTrainer::checkAdap( AttributedObject* ao, Adaptive* adap )
{
	set<string>::const_iterator	ia, fa=_atts.end();
	string				str;

	for(ia=_atts.begin(); ia!=fa; ++ia)
	if(ao->getProperty(*ia, str) &&
		!regexec( &_pattern, str.c_str(), 0, 0, 0 ) )
	{
		cout << "Train model attribute " << *ia << " = " << str << endl;
		_usedAdap.insert( adap );
		return( true );
	}
	return( false );
}


void SelectiveTrainer::save(MGWriter &mw)
{
	set<Adaptive *>::const_iterator	ia, fa=_usedAdap.end();

	for(ia = _usedAdap.begin(); ia != fa; ++ia)
		mw.parseModel(*(GraphObject *) (*ia)->topModel()->parentAO());
}


void SelectiveTrainer::init(TrainerMode mode, unsigned pass )
{
	Trainer::init(mode, pass);
	_usedAdap.clear();
}
