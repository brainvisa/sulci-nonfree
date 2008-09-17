#include <neur/koho/kohonen.h>
#include <cmath>
#include <string>
#include <iomanip>
#include <fstream>
#include <neur/stream/readstream.h>
#include <neur/rand/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;



//	OPérateurs externes


ostream & operator << ( ostream & fich, const Kohonen & koh )
{
  fich << "Kohonen :\nNDim : " << koh._nDim << "\tNCDim : " << koh._nCDim 
       << "\tNVCode : " << koh._vCode.size() 
       << "\tLearnRay : " << koh._learnRay 
       << "\tLearnFactor : " << koh._learnFactor 
       << "\tDensityRay : " << koh._densityRay 
       << "\tUserData : " << koh.UserData.size() << endl;
  unsigned	i, j;
  for( i=0; i<koh._vCode.size(); i++ )
    {
      fich << "Class : " << koh._vCode[i]->cla << "\t";
      fich << "Coord : ";
      for( j=0; j<koh._vCode[i]->coord.size(); j++ )
	fich << setw( 10 ) << koh._vCode[i]->coord[j] << "\t";
      fich << "Vect : ";
      for( j=0; j<koh._vCode[i]->vect.size(); j++ )
	fich << setw( 10 ) << koh._vCode[i]->vect[j] << "\t";
      fich << endl;
    }
  for( i=0; i<koh.UserData.size(); i++ )
    fich << setw(10) << koh.UserData[i] << "\t";
  fich << endl;
  return( fich );
}


istream & operator >> ( istream & fich, Kohonen & koh )
{
  char		ch[100] = "Kohonen";
  unsigned	nvc, i, j, nud;
  VectCode	*vc;
  double		x;

  koh.Empty();

  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais fichier\n";
      exit( 1 );
    }
  strcpy( ch, "NDim" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais NDim\n";
      exit( 1 );
    }
  fich >> koh._nDim;
  if( koh._nDim <=0 )
    {
      cerr << "istream >> Kohonen : mauvais NDim\n";
      exit( 1 );
    }
  strcpy( ch, "NCDim" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais NCDim\n";
      exit( 1 );
    }
  fich >> koh._nCDim;
  if( koh._nCDim <=0 )
    {
      cerr << "istream >> Kohonen : mauvais NCDim\n";
      exit( 1 );
    }
  strcpy( ch, "NVCode" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais NVCode\n";
      exit( 1 );
    }
  fich >> nvc;
  if( nvc <=0 )
    {
      cerr << "istream >> Kohonen : mauvais NVCode\n";
      exit( 1 );
    }
  strcpy( ch, "LearnRay" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais LearnRay\n";
      exit( 1 );
    }
  fich >> koh._learnRay;
  if( koh._learnRay <=0 )
    {
      cerr << "istream >> Kohonen : mauvais LearnRay\n";
      exit( 1 );
    }
  strcpy( ch, "LearnFactor" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais LearnFactor\n";
      exit( 1 );
    }
  fich >> koh._learnFactor;
  if( koh._learnFactor <=0 )
    {
      cerr << "istream >> Kohonen : mauvais LearnFactor\n";
      exit( 1 );
    }
  strcpy( ch, "DensityRay" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais DensityRay\n";
      exit( 1 );
    }
  fich >> koh._densityRay;
  if( koh._densityRay <=0 )
    {
      cerr << "istream >> Kohonen : mauvais DensityRay\n";
      exit( 1 );
    }
  strcpy( ch, "UserData" );
  if( !ReadLabel( fich, ch ) )
    {
      cerr << "istream >> Kohonen : mauvais UserData\n";
      exit( 1 );
    }
  fich >> nud;
  /*if( nud <0 )
    {
      cerr << "istream >> Kohonen : mauvais UserData\n";
      exit( 1 );
      }*/

  for( i=0; i<nvc; i++ )
    {
      vc = new VectCode;
      strcpy( ch, "Class" );
      if( !ReadLabel( fich, ch ) )
	{
	  cerr << "istream >> Kohonen : mauvaise classe vecteur "
	       << i << "\n";
	  exit( 1 );
	}
      fich >> vc->cla;
      strcpy( ch, "Coord" );
      if( !ReadLabel( fich, ch ) )
	{
	  cerr << "istream >> Kohonen : mauvaises coordonnees vecteur "
	       << i << "\n";
	  exit( 1 );
	}
      for( j=0; j<koh._nDim; j++ )
	{
	  fich >> x;
	  vc->coord.push_back( x );
	}

      strcpy( ch, "Vect" );
      if( !ReadLabel( fich, ch ) )
	{
	  cerr << "istream >> Kohonen : mauvaises composantes vecteur "
	       << i << "\n";
	  exit( 1 );
	}
      for( j=0; j<koh._nCDim; j++ )
	{
	  fich >> x;
	  vc->vect.push_back( x );
	}
      koh.AddVectCode( vc );
    }

  for( i=0; i<nud; i++ )
    fich >> x;
  koh.UserData.push_back( x );

  return( fich );
}



//	Constructeurs - destructeur


Kohonen::Kohonen( int ndim, int ncdim )
{
  if( ndim < 1 )
    {
      cerr << "Kohonen::Kohonen : mauvais ndim\n";
      exit( 1 );
    }
  if( ncdim < 1 )
    {
      cerr << "Kohonen::Kohonen : mauvais ncdim\n";
      exit( 1 );
    }
  _nDim = ndim;
  _nCDim = ncdim;
  _vCWinner = NULL;
  _learnRay = 5.;
  _learnFactor = 1.;
  _densityRay = 5.;
  _sqDistWinner = 0.;
}


Kohonen::Kohonen( const Kohonen & koh )
{
  _nDim = koh._nDim;
  _nCDim = koh._nCDim;
  CopyVC( koh._vCode );
  //_vCode = koh._vCode;
  _winIndex = koh._winIndex;
  _vCWinner = _vCode[ _winIndex ];

  _sqDistWinner = koh._sqDistWinner;
  _learnRay = koh._learnRay;
  _learnFactor = koh._learnFactor;
  _densityRay = koh._densityRay;
  UserData = koh.UserData;
}


Kohonen::~Kohonen()
{
  unsigned	i;

  for( i=0; i<_vCode.size(); i++ )
    delete _vCode[i];
}


//	Opérateurs


Kohonen & Kohonen::operator = ( const Kohonen & koh )
{
  Empty();
  _nDim = koh._nDim;
  _nCDim = koh._nCDim;
  CopyVC( koh._vCode );
  // _vCode = koh._vCode;
  _winIndex = koh._winIndex;
  _vCWinner = _vCode[ _winIndex ];

  _sqDistWinner = koh._sqDistWinner;
  _learnRay = koh._learnRay;
  _learnFactor = koh._learnFactor;
  _densityRay = koh._densityRay;
  UserData = koh.UserData;

  return( *this );
}



//	Manips vecteurs-code


void Kohonen::ChangeDims( unsigned ndim, unsigned ncdim )
{
  if( ndim <= 0 || ncdim <= 0 )
    {
      cerr << "Kohonen::ChangeDims : valeur(s) incorrectes.\n";
      return;
    }
  if( ndim != _nDim || ncdim != _nCDim )
    {
      Empty();
      _nDim = ndim;
      _nCDim = ncdim;
    }
}


void Kohonen::CopyVC( const vector<VectCode*> & vcl )
{
  unsigned	i;
  VectCode	*vc;

  _vCode.erase( _vCode.begin(), _vCode.end() );
  for( i=0; i<vcl.size(); i++ )
    {
      vc = new VectCode( *vcl[i] );
      AddVectCode( vc );
    }
}


int Kohonen::AddVectCode( VectCode *vc )
{
  if( vc->coord.size() != _nDim || vc->vect.size() != _nCDim )
    {
      cerr << "Kohonen::AddvectCode : les dimensions ne collent pas\n";
      return( false );
    }
  _vCode.push_back( vc );

  return( true );
}


int Kohonen::RemoveVectCode( const VectCode *vc )
{
  vector<VectCode*>::iterator ivc;

  for( ivc=_vCode.begin(); ivc!=_vCode.end() && (*ivc)!=vc; ivc++ ) {}
  if( ivc != _vCode.end() ) _vCode.erase( ivc );
  else
    {
      cerr << "Kohonen::DelVectCode : vecteur non trouvé\n";
      return( false );
    }
  return( true );
}

int Kohonen::DelVectCode( VectCode *vc )
{
  if( !RemoveVectCode( vc ) ) return( false );

  delete vc;

  return( true );
}


void Kohonen::Empty()
{
  _vCWinner = NULL;
  _winIndex = 0;
  unsigned	i;
  for( i=0; i<_vCode.size(); i++ ) delete _vCode[i];
  _vCode.erase( _vCode.begin(), _vCode.end() );
  UserData.erase( UserData.begin(), UserData.end() );
}


void Kohonen::CreateUniformVC( unsigned nvd, double cmin, double cmax )
{
  Empty();
  double comp[ _nDim ];
  unsigned	i;

  for( i=0; i<_nDim; i++ ) comp[i] = 0.;
  CreateUniformVCFromComp( 0, comp, nvd, cmin, cmax );
}


void Kohonen::CreateUniformVCFromComp( unsigned c, double *comp, unsigned nvd, 
				       double cmin, double cmax )
{
  if( c < _nDim-1 )
    {
      unsigned	i;
      double	ci = comp[c];
      for( i=0; i<nvd; i++, comp[c]+=1. ) 
 	CreateUniformVCFromComp( c+1, comp, nvd, cmin, cmax );
      comp[c] = ci;
    }
  else
    {
      unsigned	i, j;
      VectCode	*vc;
      double	ci = comp[c];
      for( i=0; i<nvd; i++, comp[c]+=1. ) 
	{
	  vc = new VectCode;
	  for( j=0; j<_nDim; j++ )
	    vc->coord.push_back( comp[j] );
	  for( j=0; j<_nCDim; j++ )
	    vc->vect.push_back( cmin + ran1()*(cmax-cmin) );
	  AddVectCode( vc );
	}
      comp[c] = ci;
    }
}


void Kohonen::RandVC( double cmin, double cmax )
{
  unsigned	i, j;

  for( i=0; i<_vCode.size(); i++ )
    for( j=0; j<_vCode[i]->vect.size(); j++ )
      _vCode[i]->vect[j] = cmin + ran1()*( cmax - cmin );
}




//	IO


int Kohonen::Save( const char *nom ) const
{
  char nf[150];

  if( strstr( nom, ".koh" ) != nom + strlen( nom ) - 4 )
    sprintf( nf, "%s.koh", nom );
  else strcpy( nf, nom );

  ofstream	fich( nf, ios::out );
  if( !fich )
    {
      cerr << "Kohonen::Save : ouverture de " << nf << " impossible.\n";
      return( false );
    }
  fich << *this;
  return( true );
}


int Kohonen::Load( const char *nom )
{
  char nf[150];

  if( strstr( nom, ".koh" ) != nom + strlen( nom ) - 4 )
    sprintf( nf, "%s.koh", nom );
  else strcpy( nf, nom );

  ifstream	fich( nf, ios::in );
  if( !fich )
    {
      cerr << "Kohonen::Load : Fichier " << nf << " non trouve.\n";
      return( false );
    }
  fich >> *this;
  return( true );
}


int Kohonen::ByteSize() const
{
  return( 55 + _vCode.size()*( 27 + 12*( _nDim+_nCDim ) ) 
	  + UserData.size()*12 );
}



//	Utilisation / Apprentissage


/*VectCode *Kohonen::FindWinner( void *toto )
{
  cout << "KOhonen::FindWinner\n";
}*/

template class vector<double>;

VectCode *Kohonen::FindWinner( const vector<double> *vec )
{
  unsigned	i, ind=0;
  double		dmin = 0, d;
  VectCode	*vc = NULL;

  if( _vCode.size() != 0 )
    {
      vc = _vCode[0];
      dmin = SqVectDist( vec, &vc->vect );
      for( i=1; i<_vCode.size(); i++ )
	{
	  d = SqVectDist( vec, &_vCode[i]->vect );
	  if( d < dmin )
	    {
	      ind = i;
	      dmin = d;
	      vc = _vCode[i];
	    }
	}
    }

  _vCWinner = vc;
  _sqDistWinner = dmin;
  _winIndex = ind;
  return( vc );
}


double Kohonen::SqVectDist( const vector<double> *v1, const vector<double> *v2 )
{
  unsigned	n;

  if( (n=v1->size()) != v2->size() )
    {
      cerr << "Kohonen::VectDist : tailles de vecteurs incompatibles\n";
      exit( 1 );
    }

  unsigned	i;
  double	d=0., x;

  for( i=0; i<n; i++ )
  {
    x = (*v1)[i] - (*v2)[i];
    d += x * x;
  }

  return( d );
}


void Kohonen::PrintMap() const
{
  unsigned	i;

  for( i=0; i<_vCode.size(); i++ ) PrintVC( _vCode[i] );
}


void Kohonen::PrintVC( const VectCode *vc )
{
  cout << "Coord:";
  unsigned	i;
  for( i=0; i<vc->coord.size(); i++ ) cout << "\t" << vc->coord[i];
  cout << "\tVect:";
  for( i=0; i<vc->vect.size(); i++ ) cout << "\t" << vc->vect[i];
  cout << "\n";
}


void Kohonen::PrintVect( const vector<double> *vect )
{
  unsigned	i;
  for( i=0; i<vect->size(); i++ ) cout << "\t" << (*vect)[i];
  cout << "\n";
}


int Kohonen::Learn( const vector<double> *vect )
{
  VectCode *vc = FindWinner( vect );

  if( !vc )
    {
      cerr << "Probleme: vecteur-code non trouve\n";
      return( 0 );
    }
  unsigned	i, j;
  double	d;

  for( i=0; i<_vCode.size(); i++ ) 
    if( (d=MapDist( _vCode[i], _vCWinner )) < _learnRay )
      for( j=0; j<_nCDim; j++ )
	_vCode[i]->vect[j] += 
	  DistFact( d ) * ( (*vect)[j] - _vCode[i]->vect[j] );
  return( _winIndex );
}


double Kohonen::DistFact( double d ) const
{
  return( _learnFactor * exp( - d/_learnRay ) );
}


double Kohonen::MapDist( int i1, int i2 ) const
{
  return( (double) abs( i1-i2 ) );
}


double Kohonen::MapDist( const VectCode *vc1, const VectCode *vc2 ) const
{
  double	x, d;
  unsigned	i, n;

  if( (n=vc1->coord.size()) != vc2->coord.size() || n==0 )
  {
    cerr << "Kohonen::MapDist : pas le bon nb de coordonnees.\n";
    exit( 1 );
  }
x=vc1->coord[0]-vc2->coord[0];
  d = x * x;
  for( i=1; i<n; i++ )
  {
    x = vc1->coord[i]-vc2->coord[i];
    d += x * x;
  }

  return( d );
}


int Kohonen::Density( const vector<double> *vect ) const
{
  vector<VectCode*>::const_iterator	ivc;
  int					n=0;

  for( ivc=_vCode.begin(); ivc!=_vCode.end(); ivc++ )
    if( SqVectDist( vect, &(*ivc)->vect ) <= _densityRay ) n++;

  return( n );
}






