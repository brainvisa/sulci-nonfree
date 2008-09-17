
#include <si/fold/foldReducedLabelsChanger.h>
#include <si/graph/attrib.h>
#include <graph/graph/edge.h>
#include <si/graph/vertexclique.h>

using namespace sigraph;
using namespace std;


double FoldReducedLabelsChanger::constrainedNoise( Clique* cl, double & outp, 
						   const set<string> & sl, 
						   const string & vl )
{
  // cout << "FoldReducedLabelsChanger\n";

  VertexClique::const_iterator	ic, fc=((const VertexClique *)cl)->end();
  Vertex			*v;
  string			label;
  set<string>::const_iterator	insig = sl.end();
  Vertex::const_iterator	ie, fe;
  Edge::const_iterator		iv;
  bool				firstRemoved = false, keepme;

  //	inspecter chaque noeud
  for( ic=((const VertexClique *)cl)->begin(); ic!=fc; )
    {
      v = *ic;
      v->getProperty( SIA_LABEL, label );
      keepme = false;
      if( label == vl || sl.find( label ) == insig )
        {
          // pas significatif. Voisins significatifs ?
          for( ie=v->begin(), fe=v->end(); ie!=fe; ++ie )
            {
              // autre bout de la relation
              iv = (*ie)->begin();
              if( *iv == v )
                ++iv;
              (*iv)->getProperty( SIA_LABEL, label );
              if( label != vl && sl.find( label ) != insig )
                {
                  keepme = true;
                  break;
                }
            }
          if( !keepme )
            {
              // aucun voisin significatif non plus: on jarte v.
              if( ic == ((const VertexClique *)cl)->begin() )
                // petit pb chiant avec les sets: pas de -- sur le 1er
                firstRemoved = true;
              else
                --ic;
              ((VertexClique *) cl)->removeVertex( v );
            }
        }
      if( firstRemoved )
	{
	  ic = ((const VertexClique *)cl)->begin();
	  firstRemoved = false;
	}
      else
	++ic;
    }

  return( FoldLabelsChanger::constrainedNoise( cl, outp, sl, vl ) );
}


