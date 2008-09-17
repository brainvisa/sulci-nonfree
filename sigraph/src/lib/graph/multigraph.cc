/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/graph/multigraph.h>
#include <graph/graph/graph.h>
#include <si/graph/attrib.h>
#include <iostream>

using namespace sigraph;
using namespace std;

namespace sigraph
{

  void concatenateGraphs( const vector<Graph*> in, Graph & out, 
                          const string & subjectatt )
  {
    vector<Graph*>::const_iterator	ig, eg = in.end();
    Graph::iterator			iv, ev;
    string				subject;

    for( ig=in.begin(); ig!=eg; ++ig )
      {
        (*ig)->getProperty( subjectatt, subject );
        for( iv=(*ig)->begin(), ev=(*ig)->end(); iv!=ev; ++iv )
          (*iv)->setProperty( SIA_SUBJECT, subject );
        (*ig)->extract( out, (*ig)->begin(), (*ig)->end() );
      }
    out.setProperty( SIA_SUBJECT, subjectatt );
  }


  void recoverIndividualGraph( const std::vector<Graph*> subjects, 
                               Graph & multi, const std::string & vertexatt )
  {
    Graph::const_iterator				imv, emv=multi.end();
    vector<Graph*>::const_iterator		ig, eg = subjects.end();
    map<string, map<int, Vertex *> >		smap;
    map<string, map<int, Vertex *> >::iterator	is, es = smap.end();
    string					subjatt, s;
    int						index;
    Graph::const_iterator				isv, esv;
    map<int, Vertex *>::iterator			iv;
    string					l;

    multi.getProperty( SIA_SUBJECT, subjatt );

    for( ig=subjects.begin(); ig!=eg; ++ig )
      {
        (*ig)->getProperty( subjatt, s );
        map<int, Vertex *>	& m = smap[ s ];
        for( isv=(*ig)->begin(), esv=(*ig)->end(); isv!=esv; ++isv )
          {
            (*isv)->getProperty( vertexatt, index );
            m[index] = *isv;
          }
      }

    for( imv=multi.begin(); imv!=emv; ++imv )
      {
        (*imv)->getProperty( SIA_SUBJECT, s );
        is = smap.find( s );
        if( is != es )
          {
            (*imv)->getProperty( vertexatt, index );
            iv = is->second.find( index );
            if( iv == is->second.end() )
              cerr << "warning: unfortunately, index " << index 
                   << " in subject " << s << " doesn't exist\n";
            else
              {
                (*imv)->getProperty( SIA_LABEL, l );
                iv->second->setProperty( SIA_LABEL, l );
              }
          }
      }
  }

}

