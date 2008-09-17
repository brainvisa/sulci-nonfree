#ifndef SI_MODEL_LEARNPARAM_H
#define SI_MODEL_LEARNPARAM_H

#include <si/graph/clique.h>
#include <si/graph/cgraph.h>
#include <list>

namespace sigraph
{
	class AdaptiveLeaf;
	class AdapDescr;

	//FIXME : ces 2 structs devraient être des Learnables :
	// peut-être CliqueLearnable
	typedef struct LearnConstParam
	{
		const Clique				*clique;
		Graph					*graph;
		AdapDescr				*descr;
		AdaptiveLeaf				*adap;
		// Potentiel de sortie à apprendre
		double					outp;
		int					class_id;
		int					cycle;

		public:
		LearnConstParam() : clique(NULL), graph(NULL),
					descr(NULL), adap(NULL) {};
		~LearnConstParam() {};
		LearnConstParam(const struct LearnParam &lp) {
			LearnConstParam::operator = (lp);
		};
  		inline LearnConstParam& operator = (const LearnParam &lp);
  		inline LearnConstParam& operator = (const LearnConstParam &lp);
	} LearnConstParam;

	typedef struct LearnParam
	{
		Clique					*clique;
		Graph					*graph;
		AdapDescr				*descr;
		AdaptiveLeaf				*adap;
		// Potentiel de sortie à apprendre
		double					outp;
		int					class_id;
		int					cycle;

		public:
		LearnParam() : clique(NULL), graph(NULL),
					descr(NULL), adap(NULL) {};
		~LearnParam() {};
		LearnParam(const LearnConstParam &lp) {
			LearnParam::operator = (lp);
		};
  		inline LearnParam& operator = (const LearnConstParam &lp);
	} LearnParam;

  	inline LearnConstParam&
	LearnConstParam::operator = (const LearnParam &lp)
	{
		clique = lp.clique;
		graph = lp.graph;
		descr = lp.descr;
		adap = lp.adap;
		outp = lp.outp;
		class_id = lp.class_id;
		cycle = lp.cycle;
		return *this;
	}

	/** la clique, la liste et le graphe sont partagés,
		outp et class_id sont copiés */
  	inline LearnConstParam&
	LearnConstParam::operator = (const LearnConstParam &lp)
	{
		clique = lp.clique;
		graph = lp.graph;
		descr = lp.descr;
		adap = lp.adap;
		outp = lp.outp;
		class_id = lp.class_id;
		cycle = lp.cycle;
		return *this;
	}

	//FIXME : vérifier pourquoi cette fonction est nécessaire?
	/** la clique, la liste et le graphe sont partagés,
		outp et class_id sont copiés */
  	inline LearnParam&
	LearnParam::operator = (const LearnConstParam &lp)
	{
		clique = const_cast<Clique*>(lp.clique);
		graph = lp.graph;
		descr = lp.descr;
		adap = lp.adap;
		outp = lp.outp;
		class_id = lp.class_id;
		cycle = lp.cycle;
		return *this;
	}
}

#endif


