#include <neur/mlp/pat.h>
#include <neur/mlp/misc.h>
#include <fstream>
#include <string>

using namespace std;

/*	BASE D'EXEMPLES		*/

template<class T> void pat<T>::init( const char *nom, int np, int ni, int no)
{
    int	i;

    strcpy(_nm,nom);
    _np=np;
    _ni=ni;
    _no=no;

    _vi= new T*[_np];
    _vo= new T*[_np];

    for(i=0; i<_np; i++)
      {
	_vi[i]= new T[_ni];
	_vo[i]= new T[_no];
      }
}


template<class T> pat<T>::pat( pat<T> & pa )
{
  init( pa._nm, pa._np, pa._ni, pa._no );

  int	i,j;

  for( i=0; i<_np; i++ )
    {
      for( j=0; j<_ni; j++ ) _vi[i][j] = pa._vi[i][j];
      for( j=0; j<_no; j++ ) _vo[i][j] = pa._vo[i][j];
    }
}


template<class T> pat<T>::~pat()
{
    int	i;

    if(_np>0)
	{
	    for(i=_np-1; i>=0; i--)
		{
		    delete[] _vo[i];
		    delete[] _vi[i];
		}
	    delete[] _vo;
	    delete[] _vi;
	}
}


template<class T> void pat<T>::set_np( int np )
{
  if( _np == np )
    return;

  int	i;
  T	**vi, **vo;

  vi = new T*[np];
  vo = new T*[np];

  if( np < _np )
    {
      for( i=0; i<np; i++ )
	{
	  vi[i] = _vi[i];
	  vo[i] = _vo[i];
	}
      for( i=_np-1; i>=np; i-- )
	{
	  delete[] vo[i];
	  delete[] vi[i];
	}
    }
  else
    {
      for( i=0; i<_np; i++ )
	{
	  vi[i] = _vi[i];
	  vo[i] = _vo[i];
	}
      for( i=_np; i<np; i++ )
	{
	  vi[i] = new T[_ni];
	  vo[i] = new T[_no];
	}
    }

  _np = np;
  _vi = vi;
  _vo = vo;
}


template<class T> int pat<T>::save( const char *nom ) const
{
  ofstream	fich( nom, ios::out );
  if( !fich )
    {
      cout << "Impossible d'ecrire " << nom << ".\n";
      return( 1 );
    }

  int	i, j;

  cout << "Ecriture de la base " << nom << "...\n";

  fich << "SNNS pattern definition file V3.2\ngenerated at fev 1997\n\n";
  fich << "No. of patterns : " << _np;
  fich << "\nNo. of input units : " << _ni;
  fich << "\nNo. of output units : " << _no << "\n\n";

  for( i=0; i<_np; i++ )
    {
      fich << "# Input pattern no. " << i+1 << " :\n";
      for( j=0; j<_ni-1; j++ ) fich << _vi[i][j] << " ";
      fich << _vi[i][_ni-1] << "\n";
      fich << "# Output pattern no. " << i+1 << " :\n";
      for( j=0; j< _no-1; j++ ) fich << _vo[i][j] << " ";
      fich << _vo[i][_no-1] << "\n";
    }
  fich.close();

  return( 0 );
}


template<class T> int pat<T>::load( const char *nom )
{
  int	i,j,er,np,ni,no;

  _np=0;
  cout << "Chargement de la base " << nom << " :" << endl;

  ifstream	fich(nom,ios::in);
  if(!fich)
    {
      cout << "Fichier " << nom << " introuvable.\n";
      return(1);
    }

  fich.unsetf(ios::skipws);

  er=cherche(&fich,"No. of patterns :");
  if(er) return(1);
  fich.setf( ios::skipws );
  fich >> np;
  fich.unsetf( ios::skipws );

  er=cherche(&fich,"No. of input units :");
  if(er) return(1);
  fich.setf( ios::skipws );
  fich >> ni;
  fich.unsetf( ios::skipws );

  er=cherche(&fich,"No. of output units :");
  if(er) return(1);
  fich.setf( ios::skipws );
  fich >> no;
  fich.unsetf( ios::skipws );

  // cout << "np : " << np << "\nni : " << ni << "\nno : " << no << "\n";
  init(nom,np,ni,no);

  for(i=0; i<_np; i++)
    {
      char	c;

      do
	{
	  er=cherche(&fich,"\n");
	  if(er) return(1);
	  fich >> c;
	  fich.putback(c);
	} while(c=='#' || c=='\n');
      fich.setf( ios::skipws );
      for(j=0; j<_ni; j++)
	fich >> _vi[i][j];
      fich.unsetf( ios::skipws );
      do
	{
	  er=cherche(&fich,"\n");
	  if(er) return(1);
	  fich >> c;
	  fich.putback(c);
	} while(c=='#' || c=='\n');
      fich.setf( ios::skipws );
      for(j=0; j<_no; j++)
	fich >> _vo[i][j];
      fich.unsetf( ios::skipws );
    }
  fich.close();

  return(0);
}

template<class T> void pat<T>::aff() const
{
    cout << "BASE D'EXEMPLES : " << _nm << "\n\n";
    cout << "Nb d'exemples : " << _np << "\n";
    cout << "Nb d'entrees  : " << _ni << "\n";
    cout << "Nb de sorties : " << _no << "\n";

    for(int i=0; i<_np; i++)
	{
	    cout << "Exemple " << i << " :\n";
	    cout << "Entrees : ";
	    for(int j=0; j<_ni; j++) cout << _vi[i][j] << " , ";
	    cout << "\nSorties : ";
	    for(int j=0; j<_no; j++) cout << _vo[i][j] << " , ";
	    cout << "\n";
	}
    cout << "\n\n";
}


template<class T> void pat<T>::add( T *vi, T *vo )
{
  set_np( _np + 1 );
  int	i;

  for( i=0; i<_ni; i++ ) _vi[_np-1][i] = vi[i];
  for( i=0; i<_no; i++ ) _vo[_np-1][i] = vo[i];
}


//	Opérateurs de la classe pat


template<class T> inline pat<T> & pat<T>::operator = ( const pat<T> & pa )
{
  if ( &pa == this ) return( *this );

  int	i,j;

  if( _np )
    {
      for(i=_np-1; i>=0; i--)
	{
	  delete[] _vo[i];
	  delete[] _vi[i];
	}
      delete[] _vi;
      delete[] _vo;
    }

  init( pa._nm, pa._np, pa._ni, pa._no );

  for( i=0; i<_np; i++ )
    {
      for( j=0; j<_ni; j++ ) _vi[i][j] = pa._vi[i][j];
      for( j=0; j<_no; j++ ) _vo[i][j] = pa._vo[i][j];
    }
  return( *this );
}


template<class T> pat<T> & pat<T>::operator += ( const pat<T> & pa )
{
  if( _ni!=pa._ni || _no!=pa._no ) return( *this );

  int	i, j, np = pa._np;
  set_np( _np + pa._np );

  for( i=0; i<pa._np; i++ )
    {
      for( j=0; j<_ni; j++ ) _vi[i+np][j] = pa._vi[i][j];
      for( j=0; j<_no; j++ ) _vo[i+np][j] = pa._vo[i][j];
    }
  return( *this );
}


template<class T> inline pat<T> pat<T>::operator + ( const pat<T> & pa )
{
  pat<T>	p( *this );

  p += pa;

  return( p );
}



//	Compilation


template class pat<double>;
// template class pat<int>;
