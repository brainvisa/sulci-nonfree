/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <cstdlib>
#include <si/global/global.h>
#include <si/fold/fgraph.h>
#include <si/fold/frgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgReader.h>
#include <si/learner/selectiveTrainer.h>
#include <si/learner/constLearner.h>
#include <si/graph/attrib.h>
#include <si/finder/modelFinder.h>
#include <aims/getopt/getopt2.h>
#include <aims/selection/selection.h>
#include <aims/io/selectionr.h>
#include <cartobase/exception/assert.h>
#include <cartobase/stream/sstream.h>
#include <cartobase/object/sreader.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

using namespace carto;
using namespace sigraph;
using namespace aims;
using namespace std;


FRGraph	rg;	// global pour permettre l'acc�s dans une interruption


struct Params
{
  string		model;
  vector<string>	graphs;
  vector<string>	subjects;
  string		labelsMap;
  string		atts;
  string		pattern;
  int			verbose;
  string		prefix;
  int			nameDescr;
  string		labelatt;
  int			printLabels;
  int			oneFile;
  string		subjectRegex;
  map<string,string>	descrAlias;
  string		selection;
};


Params	par;


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( EXIT_FAILURE );
}


/*
char escapedchar( char c )
{
  switch( c )
  {
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  default:
    return c;
  };
}
*/

string readstring( istringstream & s )
{
  char c, quote = '\0';
  // bool esc = false;
  string item;

  c = s.get();
  while( s && c )
  {
    /*
    if( esc )
    {
      item += escapedchar( c );
      esc = false;
    }
    else if( c == '\\' )
      esc = true;
    else */
    if( c == '"' || c == '\'' )
    {
      if( quote == '\0' )
        quote = c;
      else
      {
        if( c == quote )
        {
          quote = '\0';
        }
        else
          item += c;
      }
    }
    else if( c == ' ' /* && !esc*/ && quote == '\0' )
    {
      return item;
    }
    else
    {
      item += c;
    }
    c = s.get();
  };

  return item;
}


void loadParams( const string & paramFile, const char* name, Params & par )
{
  SyntaxReader	pr( si().basePath() + "/config/siMorpho.stx" );
  SyntaxSet		ps;

  try
  {
    pr >> ps;
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    exit( EXIT_FAILURE );
  }

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	gf, str, tgf, subjs;

  try
  {
    tr >> t;
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    exit( EXIT_FAILURE );
  }

  if( !t.getProperty( "modelFile", par.model ) )
    paramError( name, paramFile, "modelFile" );
  if( !t.getProperty( "graphFiles", gf ) )
    paramError( name, paramFile, "graphFiles" );
  if( !t.getProperty( "labelsMapFile", par.labelsMap ) )
    par.labelsMap = "";
  t.getProperty( "filter_attributes", par.atts );
  t.getProperty( "filter_pattern", par.pattern );
  t.getProperty( "verbose", par.verbose );
  t.getProperty( "output_prefix", par.prefix );
  t.getProperty( "name_descriptors", par.nameDescr );
  t.getProperty( "label_attribute", par.labelatt );
  string	da;
  t.getProperty( "descriptor_aliases", da );
  t.getProperty( "print_labels", par.printLabels );
  t.getProperty( "subject_regex", par.subjectRegex );
  t.getProperty( "subjects", subjs );
  t.getProperty( "selection", par.selection );

  istringstream	sst( gf.c_str() );

  while( !sst.eof() )
  {
    str = readstring( sst );
    if( !str.empty() )
      par.graphs.push_back( str );
  }
  if( par.graphs.size() == 0 )
    paramError( name, paramFile, "graphFiles" );
  cout << "Graphs : " << par.graphs.size() << endl;

  istringstream	sst2( subjs.c_str() );
  while( !sst2.eof() )
  {
    sst2 >> str;
    if( !str.empty() )
      par.subjects.push_back( str );
  }


  istringstream	sda( da.c_str() );
  string	str2;
  str = "";

  while( !sda.eof() )
  {
    sda >> str;
    if( !str.empty() )
    {
      sda >> str2;
      if( str2.empty() )
        cerr << "warning: descriptor_aliases should be pairs of strings"
          << endl;
      else
        par.descrAlias[ str ] = str2;
    }
  }

  if( par.graphs.size() == 0 )
    paramError( name, paramFile, "graphFiles" );

  if( !par.subjects.empty() && !par.subjectRegex.empty() )
    cerr << "warning: subjects and subject_regex are both provided and are " 
         << "mutually exclusive. subjects_regex will be ignored" << endl;
}


void init_file_with_header( ofstream *& fp, const std::string &filename,
                            int printsubjects,
                            const CGraph::CliqueSet::iterator &ic )
{
  vector<string>		names;
  string			descname;
  bool			naming = (bool) par.nameDescr;
  unsigned		j;

  fp = new ofstream( filename.c_str() );
  if( !*fp )
    cerr << "could not write file "
      << filename << endl;
  if( naming && (*ic)->getProperty( SIA_DESCRIPTOR_NAMES,
                          names ) )
  {
    if( printsubjects )
            *fp << "subject ";
    if( par.printLabels )
            *fp << "label side ";// split label in 2 : label side
    for( j=0; j<names.size(); ++j )
            *fp << names[j] << " ";
    if( (*ic)->getProperty( SIA_DESCRIPTOR, descname ) )
            *fp << descname << "_output";
    *fp << endl;
  }
}


int main( int argc, const char** argv )
{
  try
  {
    unsigned		i;
    set<string>	attrs;
    string		paramfile;
    par.verbose = 0;
    par.prefix = "siMorpho_";
    par.nameDescr = 0;
    par.printLabels = 0;
    par.oneFile = 0;

    AimsApplication	app( argc, argv,
                            "Writes morphometry figures to disk files "
                            "according to a model graph.\n"
                            "Each data graph is one single line on each "
                            "file, and each file is a table for a single "
                            "element (sulcus or relation).\nElements to get "
                            "figures on can be filtered by providing regular "
                            "expressions or a selection (newer mechanism)" );
    string	p = "input parameters file (tree format) "
      "(richer than other parameter given on the commandline)";
    ifstream	ps( (si().basePath() + "/config/siMorpho.stx" ).c_str() );
    if( ps )
    {
      char		s[1024];
      unsigned	i, n;
      p += "\nParameter file attributes:\n";
      while( !ps.eof() )
      {
        ps.getline( s, 1023 );
        if( s[0] != '\n' && s[0] != '\0' )
        {
          string	s2( s );
          for( i=0, n=s2.length();
            i<n && ( s2[i]==' ' || s2[i]=='\t' ); ++i ) {}
          if( s2.substr( i, i+6 ) != "*BEGIN"
              && s2.substr( i, i+4 ) != "*END" )
          {
            p += "\n";
            p += s2.substr( i, s2.length()-i );
          }
        }
      }
    }
    app.addOption( paramfile, "-p", p, true );
    app.addOption( par.model, "-m", "model file", true );
    app.addOptionSeries( par.graphs, "-g",
                          "graphs to get morphometry figures on" );
    app.addOption( par.labelsMap, "-l", "correspondance map from labels of "
                    "the graph to learn to those used by the model", true );
    app.addOption( par.prefix, "-o", "append prefix to output data file "
                    "names (followed with region models names) "
                    "default: siMorpho_", true );
    app.addOption( par.labelatt, "--label_attribute", "label attribute used "
                    "to get labels from: usually \"label\" or \"name\", "
                    "\"auto\" means try first label, and if no label is "
                    "present, take name [default:auto]", true );
    app.addOption( par.printLabels, "--print-labels",
                    "add labels/side column", true );
    app.addOption( par.nameDescr, "--name-descriptors",
                    "add header", true );
    app.addOption( par.oneFile, "--one-file",
                    "one uniq output file (default: one file per sulci)",
                      true );
    app.addOption( par.atts, "--filter-attributes",
                    "label, label1 label2, label label1 label2 (default)",
                    true );


    app.initialize();

    if( !paramfile.empty() )
      loadParams( paramfile, argv[0], par );
    if( par.model.empty() )
    {
      cerr << "A model file must be provided" << endl;
      return EXIT_FAILURE;
    }

    if( par.atts != "" )
    {
      istringstream	sst( par.atts.c_str() );
      string		str;

      while( !sst.eof() )
      {
        sst >> str;
        if( str.size() > 0 )
          attrs.insert( str );
      }
    }
    if (attrs.size() == 3)
    {
      if (par.oneFile)
      {
        std::cerr <<
          "--oneFile option can't be used with heterogeneous descriptors\n"
          "(bad option value for --filter-attributes option)" << std::endl;
        exit(1);
      }
    }


    //	read model graph

    MReader			& mr1 = FrgReader::defaultMReader();
    MReader			mr;
    mr.addFactories( mr1.factories() );
    map<string,string>::iterator	is, es = par.descrAlias.end();
    for( is=par.descrAlias.begin(); is!=es; ++is )
      mr.aliasFactory( is->first, is->second );

    FrgReader	rr( par.model, mr );

    try
    {
      rr >> rg;
      cout << "Read FRGraph OK." << endl;
    }
    catch( exception & e )
    {
      cerr << e.what() << endl;
      throw;
    }

    //	Read selection file if provided

    SelectionSet	sel;
    if( !par.selection.empty() )
    {
      SelectionReader	sr( "" );
      if( par.selection == "-" )
        sr.open( cin );
      else
        sr.open( par.selection );
      sr.read( sel );
    }

    //	Correspondance of labels

    if( !par.labelsMap.empty() )
      si().setLabelsTranslPath( par.labelsMap );

    SelectiveTrainer	*seltr = 0;
    Learner		*learn = 0;
    bool		naming = (bool) par.nameDescr;

    if( attrs.size() != 0 )
    {
      learn = new ConstLearner;
      if( par.pattern.empty() )
        par.pattern = ".*";
      seltr = new SelectiveTrainer( rg, learn, par.pattern );
      seltr->setFiltAttributes( attrs );
    }

    CGraph::CliqueSet::iterator	ic, ec;
    ModelFinder		& mf = rg.modelFinder();
    AttributedObject		*modAO;
    Model			*mod;
    vector<double>		potv;
    string			modname, name2;
    string			filename;
    double			outp;
    map<string, ofstream *>	files;
    ofstream			*fp = NULL;
    unsigned			j;
    int			printsubjects = 0;
    regex_t			subjregex;
    regmatch_t		subjmatch[2];


    if( par.subjects.size() == par.graphs.size() )
      printsubjects = 1;
    else if( !par.subjectRegex.empty() )
    {
      if( regcomp( &subjregex, par.subjectRegex.c_str(), 0 ) )
        cerr << "Can't compile subject regex " << par.subjectRegex << endl;
      else
        printsubjects = 2;
    }

    //	processing loop

    for( i=0; i<par.graphs.size(); ++i )
    {
      //	Lecture graphes
      FoldReader	fr( par.graphs[i] );

      FGraph	fg;
      try
      {
        fr >> fg;
        cout << "Read FGraph " << par.graphs[i] << " OK." << endl;
      }
      catch( exception & e )
      {
        cerr << e.what() << endl;
        throw;
      }

      //	choose label attribute
      if( !par.labelatt.empty() &&  par.labelatt != "auto" )
      {
        string	todel;
        if( par.labelatt == "name" )
          todel = "label";
        else
          todel = "name";
        /* right now, we use brutality (until a better system is done
            in LabelsTranslator) */
        Graph::iterator	iv, ev = fg.end();
        for( iv=fg.begin(); iv!=ev; ++iv )
          if( (*iv)->hasProperty( todel ) )
            (*iv)->removeProperty( todel );
      }

      //	clears modelFinder cache
      mf.clear();

      //	Pr�paration des cliques
      cout << "Init cliques..." << endl;
      if( sel.begin() == sel.end() )
        rg.modelFinder().initCliques( fg, par.verbose, false, true,
                                      false );
      else	// using a selection
        rg.modelFinder().initCliques( fg, par.verbose, false, true,
                                      false, &sel );
      cout << "nb of nodes: " << fg.order() << endl;
      cout << "nb of cliques: " << fg.cliques().size() << endl;

      const CGraph::CliqueSet	& cs = fg.cliques();
      for( ic=cs.begin(), ec=cs.end(); ic!=ec; ++ic )
      {
        modAO = mf.selectModel( ic->get() );
        ASSERT( modAO );
        ASSERT( modAO->getProperty( SIA_MODEL, mod ) );
        if( !seltr || seltr->checkAdap( modAO, 0 ) )
        {
          if( !(*ic)->getProperty( SIA_LABEL, modname ) )
          {
            if( (*ic)->getProperty( SIA_LABEL1, modname )
                && (*ic)->getProperty( SIA_LABEL2, name2 ) )
              modname += "-" + name2;
            else
            {
              cerr << "warning : unnamed model" << endl;
              modname = "unknown";
            }
          }
          outp = mod->printDescription( ic->get(), naming );
          if( (*ic)->getProperty( SIA_POT_VECTOR, potv ) )
          {
            ofstream    **fp_ref;
            if( !par.oneFile )
            {
              filename = par.prefix + modname + ".dat";
              fp_ref = &files[ modname ];
              fp = *fp_ref;
            }
            else
            {
              filename = par.prefix + ".dat";
              fp_ref = &fp;
            }
            if( !fp ) // new file
              init_file_with_header(*fp_ref, filename,
                      printsubjects, ic);
            ofstream	& f = **fp_ref;
            switch( printsubjects )
            {
            case 1:
              if( !par.subjects.empty() )
                f << par.subjects[i] << " ";
              break;
            case 2:
              if( regexec( &subjregex, par.graphs[i].c_str(),
                            2, subjmatch, 0 ) )
                f << "(unknown) ";
              else
              {
                int x=0;
                if( subjmatch[1].rm_so >= 0 )
                  x = 1;
                f << par.graphs[i].substr( subjmatch[x].rm_so,
                                            subjmatch[x].rm_eo
                                            - subjmatch[x].rm_so
                                            )
                  << " ";
              }
              break;
            }

            if( par.printLabels )
            {
              // split label in 2 : label side
              string _left("_left");
              string _right("_right");
              string label, side;

              string::size_type pos = modname.rfind(_left);
              if( pos != string::npos
                  && ( pos+_left.size() == modname.size() ) )
              {
                label = modname.substr(0,pos);
                side  = modname.substr(pos+1);
              }
              else
              {
                pos = modname.rfind(_right);
                if( pos != string::npos
                    && ( pos+_right.size() == modname.size() ) )
                {
                  label = modname.substr(0,pos);
                  side  = modname.substr(pos+1);
                }
                else
                {
                  label = modname;
                  side = "NA";
                }
              }
              f << label << " " << side << " ";
              //f << modname << " ";
            }
            for( j=0; j<potv.size(); ++j )
              f << potv[j] << " ";
            f << outp << endl;
          }
          else
            cout << "warning : model " << modname
                  << " does not provide a description vector" << endl;
        }
      }
    }

    delete seltr;
    delete learn;

    cout << "OK." << endl;
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
