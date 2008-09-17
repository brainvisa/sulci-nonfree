

#include <iomanip>
#include <graph/graph/graph.h>
#include <si/model/adaptive.h>
#include <si/domain/adapDomain.h>
#include <graph/tree/tree.h>
#include <si/graph/attrib.h>
#include <si/graph/vertexclique.h>



using namespace sigraph;
using namespace std;


Adaptive::~Adaptive()
{
}


void Adaptive::buildTree(Tree & tr) const
{
	tr.setProperty("nb_learn_data", (int) _ndata);
}

void	Adaptive::trainDomain(const list<Clique *> &cliques)
{
	list<Clique *>::const_iterator	ilc, elc;
	carto::AttributedObject			*ao = graphObject();
	unsigned int				i, s;
	Domain					*dom = NULL;

	if (!ao->getProperty(SIA_DOMAIN, dom))
	{
		cout << "no domain" << endl;
		return;
	}

	AdapDomain	*ad = dynamic_cast<AdapDomain *>(dom);
	if( !ad )
	{
		cout << "non-adapative domain" << endl;
		return;
	}
	
	for (ilc = cliques.begin(), elc = cliques.end(), i = 0,
		s = cliques.size(); ilc != elc; ++ilc, ++i)
	{
		const VertexClique	*vcl = (const VertexClique *) *ilc;
		VertexClique::const_iterator  iv, fv = vcl->end();
		Graph                         *g = 0;

		cout << "\r\tCliques " << setw(3) << (i + 1) << " / "
			<< setw(3) << s << "\r" << flush;

		vcl->getProperty(SIA_GRAPH, g);
		for(iv = vcl->begin(); iv != fv; ++iv)
 			ad->learn(*iv, g);
	}
	cout << endl;
}
