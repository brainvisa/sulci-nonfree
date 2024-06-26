
%ModuleCode
#include <si/fold/frgraph.h>
#include <si/fold/fgraph.h>
%End

namespace sigraph
{

class MGraph : Graph
{
%TypeHeaderCode
#include <si/fold/frgraph.h>
#include <si/graph/cgraph.h>
#include <si/learner/constLearner.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<Graph *>( sipCpp ) )
  {
    if( dynamic_cast<sigraph::MGraph *>( sipCpp ) )
      if( dynamic_cast<sigraph::FRGraph *>( sipCpp ) )
      {
        sipType = sipType_sigraph_FRGraph;
        *sipCppRet = dynamic_cast<sigraph::FRGraph *>( sipCpp );
      }
      else
      {
        sipType = sipType_sigraph_MGraph;
        *sipCppRet = dynamic_cast<sigraph::MGraph *>( sipCpp );
      }
    else if( dynamic_cast<sigraph::CGraph *>( sipCpp ) )
      if( dynamic_cast<sigraph::FGraph *>( sipCpp ) )
      {
        sipType = sipType_sigraph_FGraph;
        *sipCppRet = dynamic_cast<sigraph::FGraph *>( sipCpp );
      }
      else
      {
        sipType = sipType_sigraph_CGraph;
        *sipCppRet = dynamic_cast<sigraph::CGraph *>( sipCpp );
      }
    else
    {
      sipType = sipType_Graph;
      *sipCppRet = dynamic_cast<Graph *>( sipCpp );
    }
  }
  else if( dynamic_cast<Tree *>( sipCpp ) )
  {
    if( dynamic_cast<sigraph::Learner *>( sipCpp ) )
    {
      sipType = sipType_sigraph_Learner;
      *sipCppRet = dynamic_cast<sigraph::Learner *>( sipCpp );
    }
    else
      sipType = 0;
  }
  else
    sipType = 0;
%End

public:
  virtual ~MGraph();

  enum VersionStatus
  {
    Unversioned, VersionOk, Outdated
  };

  struct VersionCheck
  {
    bool ok;
    sigraph::MGraph::VersionStatus model;
    sigraph::MGraph::VersionStatus data;
    std::string message;
  };

  virtual VersionCheck checkCompatibility( const Graph & ) const = 0;
  sigraph::ModelFinder & modelFinder() = 0;
/* %MethodCode
  sipRes = &sipCpp->modelFinder();
%End */

  virtual void initAdap() = 0;
  virtual void initDomain() = 0;
  virtual void initStats() = 0;
  virtual void closeLearning() = 0;
  virtual void setWeights( double factor = 1 );
  virtual void removeWeights();

private:
  MGraph( const std::string & );
};


class FGraph : sigraph::CGraph
{
%TypeHeaderCode
#include <si/fold/fgraph.h>
%End
public:
	FGraph(const std::string s="");
	virtual ~FGraph();

	virtual void flipHemispheres();
};

class FRGraph : sigraph::MGraph
{
%TypeHeaderCode
#include <si/fold/frgraph.h>
%End

public:
  FRGraph( const std::string & = "", const std::string & = "standard1" );
  virtual ~FRGraph();

  virtual VersionCheck checkCompatibility( const Graph & ) const;
  sigraph::ModelFinder & modelFinder();
  virtual void initAdap();
  virtual void initDomain();
  virtual void initStats();
  virtual void closeLearning();
};


class MGWriter // : GraphWriter
{
%TypeHeaderCode
#include <si/graph/mgWriter.h>
#include <pyaims/object/object.h>
%End

public:
  MGWriter( const std::string &, const carto::PySyntaxSet & );
  virtual ~MGWriter();
  virtual void write( const Graph & );
  virtual std::string dataDirectory( const Graph & ao ) const;
  virtual void parseModel(const GraphObject & /NoCopy/,
		const std::string & = "model_file",
		const std::string & = "model");
%MethodCode
  try
  {
    sipCpp->parseModel(*a0, *a1, *a2);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
  }
%End

  virtual std::string name() const;

private:
  MGWriter( const sigraph::MGWriter & );
};


class FrgWriter : sigraph::MGWriter
{
%TypeHeaderCode
#include <si/fold/frgWriter.h>
%End

public:
  FrgWriter( const std::string & );
  virtual ~FrgWriter();

private:
  FrgWriter( const sigraph::FrgWriter & );
};

};

