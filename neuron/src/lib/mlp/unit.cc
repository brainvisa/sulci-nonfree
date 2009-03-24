
#include <cstdlib>
#include <neur/mlp/unit.h>
#include <iostream>
#include <string>
#include <string.h>

using namespace std;

//	NEURONES

template<class T> 
void unit<T>::set( const char *nom, int typ, int xx, int yy, 
			  int zz, T bia, T sor)
{
  if(typ>SORTIE || typ<ENTREE) errtyp();
  strcpy(_nm,nom);
  _t=typ;
  _x=xx;
  _y=yy;
  _z=zz;
  _bi=bia;
  _o=sor;
}


template<class T> void unit<T>::set_nm(const char *nom)
{
  strcpy(_nm,nom);
}


template<class T> void unit<T>::set_t( int typ )
{
  if(typ>SORTIE || typ<ENTREE) errtyp();
  _t=typ;
}


template<class T> void unit<T>::set_coord(int xx, int yy, int zz)
{
  _x=xx;
  _y=yy;
  _z=zz;
}


template<class T> void unit<T>::errtyp() const
{
    cout << "Type de neurone inconnu.\n";
    exit(-1);
}

template<class T> void unit<T>::aff() const
{
    char	tp[3][7]= { "ENTREE", "CACHE", "SORTIE" };

    cout << "Neurone " << _nm << " :\n";
    cout << "Type   : " << tp[_t] << "\n";
    cout << "biais  : " << _bi << "\n";
    cout << "sortie : " << _o << "\n";
    cout << "Coord  : " << _x << " , " << _y << " , " << _z << "\n\n";
}

template<class T> unit<T> & unit<T>::operator = (const unit<T> & u)
{
    strcpy(_nm,u.nm());
    _bi=u.bi();
    _t=u.t();
    _o=u.o();
    _x=u.x();
    _y=u.y();
    _z=u.z();

    return(*this);
}




//	Compilation


template class unit<double>;
// template class unit<int>;

