
namespace sigraph
{

class Clique : carto::GenericObject /Abstract/
{
%TypeHeaderCode
#include <si/graph/vertexclique.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<sigraph::VertexClique *>( sipCpp ) )
  {
    sipType = sipType_sigraph_VertexClique;
    *sipCppRet = dynamic_cast<sigraph::VertexClique *>( sipCpp );
  }
  else if( dynamic_cast<sigraph::Clique *>( sipCpp ) )
  {
    sipType = sipType_sigraph_Clique;
    *sipCppRet = dynamic_cast<sigraph::Clique *>( sipCpp );
  }
  else  sipType = 0;
%End

public:
  Clique();
  virtual ~Clique();
  virtual void clear();
  virtual sigraph::Clique* deepCopy() const = 0;

  static set_CliquePtr * fromSetPtrObject( carto::GenericObject );
%MethodCode
  try
  {
    sipRes = a0->value<std::set<sigraph::Clique *> *>();
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End

protected:
  Clique( const sigraph::Clique & );
};


class VertexClique : sigraph::Clique
{
%TypeHeaderCode
#include <si/graph/vertexclique.h>
#include <graph/graph/vertex.h>
%End

public:
  VertexClique();
  ~VertexClique();

  virtual sigraph::Clique* deepCopy() const;

  void addVertex(Vertex*);
  void removeVertex(Vertex*);
  const set_VertexPtr& vertices() const;

  static sigraph::VertexClique* fromObject(carto::GenericObject);
%MethodCode
  try
  {
    sipRes = a0->value<sigraph::VertexClique*>();
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End

  static SIP_PYLIST connectivity2( const set_VertexPtr & vx,
      const set_STRING & syntTypes) /PyName=connectivity/;
%MethodCode
  std::set<sigraph::CComponent *>	s;
  std::set<sigraph::CComponent *>::const_iterator i, e;
  unsigned int n = sigraph::VertexClique::connectivity(*a0, &s, *a1);
  unsigned int ind = 0;
  sipRes = PyList_New(n);
  for (i = s.begin(), e = s.end(); i != e; ++i, ++ind)
    PyList_SetItem(sipRes, ind, sipConvertFromType(*i,
                   sipType_set_VertexPtr, Py_None));
%End
};

};
