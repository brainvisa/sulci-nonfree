
#include <si/model/topModel.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


TopModel::TopModel( const TopModel & tm )
  : _voidLabel( tm._voidLabel ), _significantLabels( tm._significantLabels ), 
    _parentAO( 0 ), _weight( tm._weight ), _mgraph( tm._mgraph )
{
}


TopModel::~TopModel()
{
}


TopModel & TopModel::operator = ( const TopModel & tm )
{
  if( this != &tm )
    {
      _voidLabel = tm._voidLabel;
      _significantLabels = tm._significantLabels;
      _weight = tm._weight;
      _mgraph = tm._mgraph;
    }
  return( *this );
}


void TopModel::buildTree( Tree & tr ) const
{
  if( significantLabels().size() != 0 )
    {
      string			sl;
      set<string>::iterator	is, fs=significantLabels().end();

      for( is=significantLabels().begin(); is!=fs; ++is )
	sl += *is + ' ';
      sl.erase( sl.size()-1, 1 );	// enlever dernier ' '
      tr.setProperty( "significant_labels", sl );
    }
  if( voidLabel() != "" )
    tr.setProperty( "void_label", voidLabel() );
  tr.setProperty( "weight", (float) weight() );
}




