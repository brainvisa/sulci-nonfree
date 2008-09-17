
namespace sigraph
{

class Clique : carto::GenericObject /Abstract/
{
%TypeHeaderCode
#include <si/graph/vertexclique.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<sigraph::VertexClique *>( sipCpp ) )
        sipClass = sipClass_sigraph_VertexClique;
  else if( dynamic_cast<sigraph::Clique *>( sipCpp ) )
        sipClass = sipClass_sigraph_Clique;
  else  sipClass = 0;
%End

public:
  virtual ~Clique();
  virtual void clear();
  virtual void deepCopy() const = 0;

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
  Clique();
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
};

};

