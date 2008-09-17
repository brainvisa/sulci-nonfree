
#include <si/model/topAdaptive.h>
#include <si/graph/attrib.h>
#include <si/graph/clique.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


TopAdaptive::TopAdaptive( Model* subm )
  : Adaptive(), TopModel(), _model( subm )
{
  if( _model )
    _model->setParent( this );
}


TopAdaptive::TopAdaptive( const TopAdaptive & a )
  : Adaptive( a ), TopModel( a ), _model( a._model ? a._model->clone() : 0 )
{
  if( _model )
    _model->setParent( this );
}


TopAdaptive & TopAdaptive::operator = ( const TopAdaptive & a )
{
  if( this != &a )
    {
      *(Model *) this = a;
      *(TopModel *) this = a;
      delete _model;
      if( a._model )
	{
	  _model = a._model->clone();
	  _model->setParent( this );
	}
      else _model = 0;
    }
  return( *this );
}


TopAdaptive::~TopAdaptive()
{
  delete _model;
}


void TopAdaptive::buildTree( Tree & tr ) const
{
  Adaptive::buildTree( tr );
  TopModel::buildTree( tr );
  tr.setSyntax( "top_adaptive" );

  Tree	*child = new Tree;
  tr.insert( child );
  _model->buildTree( *child );
}


double TopAdaptive::prop( const Clique* cl,
                          const std::map<Vertex*, std::string> & changes )
{
  double pot;

  if( cl->getProperty( SIA_POTENTIAL, pot ) &&
      !doesOutputChange( cl, changes ) )
    return pot;
  return( _model->prop( cl, changes ) );
}


bool TopAdaptive::doesOutputChange
( const Clique* cl, const map<Vertex*, string> & changes ) const
{
  return _model->doesOutputChange( cl, changes );
}

