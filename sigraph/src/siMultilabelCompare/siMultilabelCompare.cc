/*
 *  Copyright (C) 2004-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <cstdlib>
#include <aims/getopt/getopt2.h>
#include <aims/io/reader.h>
#include <graph/graph/graph.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/object/pythonwriter.h>
#include <map>
#include <set>
#include <sys/types.h>
#include <regex.h>

using namespace aims;
using namespace carto;
using namespace std;

struct ID
{
  string	graph;
  string	labelatt;
  string	idatt;
};


struct NodeDescr
{
  NodeDescr() : n( 0 ), size( 0 ) {}

  unsigned	n;
  float		size;
};


struct LabelDescr
{
  LabelDescr() : n( 0 ), size( 0 ) {}

  set<rc_ptr<Graph> >	occur;
  unsigned		n;
  float			size;
  map<int, NodeDescr>	nodes;
};


void writesubj( Object o, ostream & fw, const string & s, 
                bool complete = false )
{
  Dictionary		& d = o->value<Dictionary>();
  Dictionary::iterator	i, e = d.end();
  for( i=d.begin(); i!=e; ++i )
    {
      fw << s << "\t" << i->first << "\t" 
         << i->second->getProperty( "agreement" )->value<float>() << "\t" 
         << i->second->getProperty( "mean_size" )->value<float>() << "\t" 
         << i->second->getProperty( "occurence" )->value<int>();
      if( complete )
        fw << "\t" << i->second->getProperty( "extent_size" )->value<float>() 
           << "\t" 
           << i->second->getProperty( "total_nodes" )->value<unsigned>() 
           << endl;
      else
        fw << endl;
    }
}


int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Evaluation of different labelings." 
                             "This command is a bit similar to siErrorStats " 
                             "but allows to compare an arbitrary number of " 
                             "different labelings for the same graph, and " 
                             "an arbitrary number of subjects" );

      vector<string>	graphs;
      string		output, conf;
      bool		tableout = false, bvstyle = false;

      app.addOption( output, "-o", "output filename for stats" );
      app.addOptionSeries( graphs, "-i", "input graphs filenames and params. " 
                           "They should have the form " 
                           "filename.arg|subjectname|label_attribute|" 
                           "id_attribute, with:\n" 
                           "subjectname: identifier for the subject\n" 
                           "label_attribute: label or name\n" 
                           "id_attribute: attribute (int type) used to " 
                           "identify the same node on different copies of the "
                           "same graph.\n" 
                           "If you are using a BrainVisa-like database, you " 
                           "can either use the -b option, or build the " 
                           "parameters list using a sed command "
                           "of the form:\n"
                           "ls */graphe/*.arg | sed -e \\\n" 
                           "'s%.*/\\([^/]*\\)/graphe/[^/]*\\.arg%\"\\0|\\1|" 
                           "label|index\"%' \\\n" 
                           "| awk '{ printf( \"%s \", $0 )}'" );
      app.addOption( conf, "-c", "config file providing the list of graphs to "
                     "compare. This file only contains a concatenation of the "
                     "-i arguments equivaments.\n" 
                     "If you are using a BrainVisa-like database, you can " 
                     "either use the -b option, or build the config file " 
                     "using a sed command of the form:\n"
                     "ls */graphe/*.arg | sed -e \\\n" 
                     "'s%.*/\\([^/]*\\)/graphe/[^/]*\\.arg%\\0|\\1|" 
                     "label|index%' > configfile", 
                     true );
      app.addOption( tableout, "-t", "output as table (default is pyhton-like "
                     "dictionary object)", true );
      app.addOption( bvstyle, "-b", "BrainVisa style: input parameters are " 
                     "only graph filenames which are organized as a BrainVisa "
                     "database: <basepath>/subjectname/graphe/*.arg\n" 
                     "in this case, the label attribute is 'label' and the " 
                     "identifier attribute is 'index'.", true );
      app.initialize();

      if( !conf.empty() )
        {
          ifstream	cf( conf.c_str() );
          string	z;
          while( !cf.eof() )
            {
              cf >> z;
              // cout << "param: " << z << endl;
              if( !z.empty() )
                graphs.push_back( z );
            }
        }

      unsigned				i, n = graphs.size();
      string::size_type			pos, pos2;
      typedef map<string,set<rc_ptr<ID> > > Subjects;
      Subjects				subj;
      ID				id;
      string				s;
      regex_t				re;
      regmatch_t			match[2];

      if( bvstyle )
        regcomp( &re, "^.*/\\([^/]*\\)/graphe/[^/]*\\.arg$", 0 );

      for( i=0; i<n; ++i )
        {
          if( bvstyle )
            {
              if( regexec( &re, graphs[i].c_str(), 2, match, 0 ) )
                throw invalid_argument( graphs[i] 
                                        + " has not the correct syntax" );
              /* cout << match[1].rm_so << " - " << match[1].rm_eo 
                   << ": " << graphs[i].substr( match[1].rm_so, 
                                                match[1].rm_eo 
                                                - match[1].rm_so) << endl; */
              id.graph = graphs[i];
              id.labelatt = "label";
              id.idatt = "index";
              subj[ graphs[i].substr( match[1].rm_so, 
                                      match[1].rm_eo - match[1].rm_so ) 
              ].insert( rc_ptr<ID>( new ID( id ) ) );
            }
          else
            {
              pos = graphs[i].find( '|' );
              if( pos == string::npos )
                throw invalid_argument( graphs[i] 
                                        + " has not the correct syntax" );
              id.graph = graphs[i].substr( 0, pos );
              pos2 = graphs[i].find( '|', pos+1 );
              if( pos2 == string::npos )
                throw invalid_argument( graphs[i] 
                                        + " has not the correct syntax" );
              s = graphs[i].substr( pos+1, pos2-pos-1 );
              pos = graphs[i].find( '|', pos2+1 );
              if( pos == string::npos )
                throw invalid_argument( graphs[i] 
                                        + " has not the correct syntax" );
              id.labelatt = graphs[i].substr( pos2+1, pos-pos2-1 );
              id.idatt = graphs[i].substr( pos+1, graphs[i].length()-pos-1 );
              subj[s].insert( rc_ptr<ID>( new ID( id ) ) );
            }
          /*
          cout << "subject: " << s << ": arg file: " << id.graph 
               << ", label att: " << id.labelatt << ", id att: " << id.idatt 
               << endl;
          */
        }

      Subjects::iterator		is, es = subj.end();
      set<rc_ptr<ID> >::iterator	ii, ei;
      Graph::iterator			iv, ev;
      string				label;
      int				ind;
      float				sz, s2, agree;
      int				m;
      Object	res( (GenericObject *) new ValueObject<Dictionary> );
      Object	ressub = Object::value( Dictionary() );
      res->setProperty( "subjects", ressub );
      Object	resgen = Object::value( Dictionary() );
      res->setProperty( "general", resgen );

      for( is=subj.begin(); is!=es; ++is )
        if( is->second.size() < 2 )
          throw invalid_argument( string( "subject " ) + is->first 
                                  + " has not 2 or more labelings" );

      for( is=subj.begin(); is!=es; ++is )
        {
          vector<rc_ptr<Graph> >		g;
          map<string,LabelDescr>		labels;
          map<string,LabelDescr>::iterator	ild, eld = labels.end();
          map<int,NodeDescr>::iterator		in, en;

          for( ii=is->second.begin(), ei=is->second.end(), i=0; ii!=ei; 
               ++ii, ++i )
            {
              rc_ptr<ID>	id = *ii;
              Reader<Graph>	gr( id->graph );
              g.push_back( rc_ptr<Graph>( new Graph) );
              cout << "reading " << id->graph << "..." << endl;
              gr.read( *g[i], 0, 0, 0 );

              for( iv=g[i]->begin(), ev=g[i]->end(); iv!=ev; ++iv )
                if( (*iv)->getProperty( id->labelatt, label ) 
                    && (*iv)->getProperty( id->idatt, ind ) 
                    && (*iv)->getProperty( "size", sz ) )
                  {
                    LabelDescr	& ld = labels[ label ];
                    ld.occur.insert( g[i] );
                    NodeDescr	& nd = ld.nodes[ ind ];
                    ++nd.n;
                    ++ld.n;
                    nd.size = sz;
                    ld.size += sz;
                  }
            }

          cout << "analyzing stats for subject " << is->first << "..." << endl;

          Object	rs = Object::value( Dictionary() );
          ressub->setProperty( is->first, rs );
          set<string>		usedlabels;
          set<string>::iterator	iu, eu = usedlabels.end();

          for( ild=labels.begin(); ild!=eld; ++ild )
            {
              // cout << "label: " << ild->first << endl;
              agree = 0;
              sz = 0;
              m = ild->second.occur.size();
              // cout << "occur: " << m << endl;
              for( in=ild->second.nodes.begin(), en=ild->second.nodes.end(); 
                   in!=en; ++in )
                {
                  s2 = in->second.size * in->second.n;
                  /* cout << "node size: " << s2 << ", n: " << in->second.n 
                     << endl; */
                  sz += s2;
                  if( in->second.n > 1 )
                    agree += s2 * (in->second.n - 1) / ( m - 1 );
                }
              if( sz > 0 )
                usedlabels.insert( ild->first );

              agree /= ild->second.size;
              Object	o = Object::value( Dictionary() );
              rs->setProperty( ild->first, o );
              o->setProperty( "agreement", agree );
              o->setProperty( "extent_size", sz );
              o->setProperty( "mean_size", ild->second.size / m );
              o->setProperty( "occurence", m );
              o->setProperty( "total_nodes", ild->second.n );
            }

          for( iu=usedlabels.begin(); iu!=eu; ++iu )
            {
              Object	la = resgen->getProperty( *iu );
              if( !la.get() )
                {
                  la = Object::value( Dictionary() );
                  resgen->setProperty( *iu, la );
                  la->setProperty( "agreement", 0.F );
                  la->setProperty( "mean_size", 0.F );
                  int	toto = 0;
                  la->setProperty( "occurence", toto );
                }
              la->getProperty( "agreement" )->value<float>() 
                += rs->getProperty( *iu )->getProperty( "agreement" )
                ->value<float>();
              ++la->getProperty( "occurence" )->value<int>();
              la->getProperty( "mean_size" )->value<float>() 
                += rs->getProperty( *iu )->getProperty( "mean_size" )
                ->value<float>();
            }
        }

      Dictionary	& pd = resgen->value<Dictionary>();
      Dictionary::iterator	ipd, epd = pd.end();
      int				x;

      for( ipd=pd.begin(); ipd!=epd; ++ipd )
        {
          x = ipd->second->getProperty( "occurence" )->value<int>();
          ipd->second->getProperty( "agreement" )->value<float>() /= x;
          ipd->second->getProperty( "mean_size" )->value<float>() /= x;
        }

      if( tableout )
        {
          ofstream	fw( output.c_str() );
          if( !fw )
            io_error::launchErrnoExcept( output );
          fw << "subject:\tlabel:\tagreement:\tmean_size:\toccurence:\t" 
             << "extent_size\ttotal_nodes\n";
          writesubj( resgen, fw, "*general*" );
          Dictionary	& sd = ressub->value<Dictionary>();
          for( ipd=sd.begin(), epd=sd.end(); ipd!=epd; ++ipd )
            writesubj( ipd->second, fw, ipd->first, true );
        }
      else
        {
          PythonWriter	pw( output );
          pw.write( *res );
        }

      return EXIT_SUCCESS;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
  return EXIT_FAILURE;
}


