
#include <neur/mlp/link.h>
#include <iostream>

using namespace std;

//	CONNEXION

template<class T> void neur_link<T>::init(int tar, int nli)
{
  _ta = tar;
  _nl = nli;
  if(nli>0) _le = new lelm<T>[nli];
  else _le = 0;
}

template<class T> void neur_link<T>::init(int tar, int nli, int *ori)
{
  int	i;

  init(tar,nli);
  for(i=0; i<nli; i++) _le[i].set_or(ori[i]);
}

template<class T> neur_link<T>::neur_link(const neur_link<T> & l)
{
  init(l.ta(),l.nl());
  for(int i=0; i<_nl; i++) _le[i]=l.le()[i];
}



//	Opérateurs de la classe neur_link


template<class T> neur_link<T> & neur_link<T>::operator = (const neur_link<T> & l)
{
    if(_nl!=l.nl())
	{
	    if(_nl!=0) delete _le;
	    _nl=l.nl();
	    if (_nl>0) _le= new lelm<T>[_nl];
	    else _le = 0;
	}
    _ta=l.ta();
    for(int i=0; i<_nl; i++) _le[i]=l.le()[i];

    return(*this);
}

template<class T> inline neur_link<T> neur_link<T>::operator + (const lelm<T> & l)
{
  neur_link<T>	lnk( *this );

  lnk.add( l.org(), l.w() );

  return( lnk );
}


template<class T> inline neur_link<T> neur_link<T>::operator - (const lelm<T> & l)
{
  neur_link<T>	lnk( *this );

  lnk.del( l.org() );

  return( lnk );
}


template<class T> inline neur_link<T> & neur_link<T>::operator += (const lelm<T> & l)
{
  add( l.org(), l.w() );
  return( *this );
}


template<class T> inline neur_link<T> & neur_link<T>::operator -= (const lelm<T> & l)
{
  del( l.org() );
  return( *this );
}


//	Fonctions de la classe neur_link


template<class T> void neur_link<T>::add(int ori, T wgt)
{
    lelm<T>	*lold=_le;
    int		i;

    _le= new lelm<T>[_nl+1];

    for(i=0; (i<_nl) && (lold[i].org()<ori); i++) _le[i]=lold[i];

    if((i<_nl) && (lold[i].org()==ori))
	{
	    cout << "Il y avait deja un lien entre " << ori << " et "
		 << _ta << ".\n";
	    delete[] _le;
	    _le=lold;
	    _le[i].set(ori,wgt);
	    return;
	}

    _le[i].set(ori,wgt);

    for(int j=i; j<_nl; j++) _le[j+1]=lold[j];

    _nl++;
    if(_nl>1) delete[] lold;
}

template<class T> int neur_link<T>::del(int ori)
{
    int		i,j;
    lelm<T>	*lold=_le;

    for(i=0; i<_nl; i++) if(_le[i].org()==ori) break;
    if(i==_nl)
	{
	    cout << "Pas de lien entre " << ori << " et " << _ta << ".\n";
	    return(1);
	}

    if(_nl>1) _le= new lelm<T>[_nl-1];

    for(j=0; j<i; j++) _le[j]=lold[j];
    for(j=i+1; j<_nl; j++) _le[j-1]=lold[j];

    delete[] lold;
    _nl--;
    return(0);
}

template <class T> void neur_link<T>::aff() const
{
    int	i;

    cout << "Liaisons vers le neurone " << _ta << " : " << _nl
	 << " liaisons.\n";
    for(i=0; i<_nl; i++) _le[i].aff();
}



//	Compilation


template class neur_link<double>;
// template class neur_link<int>;

