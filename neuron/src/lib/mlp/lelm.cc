
#include <neur/mlp/lelm.h>
#include <iostream>

using namespace std;

//	LIENS ELEMENTAIRES

template<class T> lelm<T> & lelm<T>::operator = (const lelm<T> & le)
{
    _or=le.org();
    _w=le.w();

    return(*this);
}


template<class T> void lelm<T>::aff() const
{
  cout << "Origine : " << _or << " ; Poids : " << _w << "\n";
}




//	Compilation


template class lelm<double>;
// template class lelm<int>;
