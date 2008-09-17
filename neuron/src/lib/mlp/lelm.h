
#ifndef NEUR_LELM_H
#define NEUR_LELM_H



//	LIENS ELEMENTAIRES

/**	Liens élémemtaires : classe lelm. \\ \\
	template<class T> class lelm; \\ \\
	La classe lelm représente un lien élémentaire: c'est une des liaisons 
	arrivant sur un neurone donné, l'ensemble de ces liaisons etant 
	représenté par la classe link. \\
	Un lien élémentaire mémorise un numéro et un poids (w). Le numéro 
	(or) est celui du neurone à l'origine de la liaison. Le lien 
	élémentaire est utilisé par la cible de cette liaison, ainsi la 
	destination n'est pas prise en compte par le lien élémentaire, mais 
	par la liaison globale (voir liaison: type link). Le type T est 
	utilisé pour représenter le poids de la liaison. */
template <class T> class lelm
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Constructeur
  lelm(int ori=0,T wgt=0)	{ _or=ori; _w=wgt; }
  ///	Destructeur
  ~lelm()		{ }
  //@}

  /**@name	Opérateurs */
  //@{
  ///
  lelm<T> & operator =(const lelm<T> & le);
  lelm<T> & operator +(const lelm<T> & le);
  lelm<T> & operator -(const lelm<T> & le);
  lelm<T> & operator *(const lelm<T> & le);
  lelm<T> & operator /(const lelm<T> & le);
  lelm<T> & operator ^(const lelm<T> & le);
  lelm<T> & operator +=(const lelm<T> & le);
  lelm<T> & operator -=(const lelm<T> & le);
  lelm<T> & operator *=(const lelm<T> & le);
  lelm<T> & operator /=(const lelm<T> & le);
  ///
  inline int operator ==(const lelm<T> & le) const;
  ///
  inline int operator !=(const lelm<T> & le) const;
  ///	Comparaison des poids
  inline int operator <(const lelm<T> & le) const;
  ///
  inline int operator >(const lelm<T> & le) const;
  ///
  inline int operator <=(const lelm<T> & le) const;
  ///
  inline int operator >=(const lelm<T> & le) const;
  //@}

  /**@name	Modification des champs */
  //@{
  ///	Fixe l'origine et le poids en même temps
  void set(int ori, T wgt)	{ _or=ori; _w=wgt; }
  ///	Fixe le poids
  void set_w(T wgt)		{ _w=wgt;	}
  ///	Fixe l'origine
  void set_or(int ori)		{ _or=ori;	}
  //@}

  /**@name	Accès aux membres */
  //@{
  ///	Origine
  int	org() const		{ return(_or);	}
  ///	Poids
  T	w() const		{ return(_w);	}
  //@}

  /**@name	Fonctions */
  //@{
  ///	Affichage des caractéristiques de la liaison
  void aff() const;
  //@}


  protected:

  ///	Champ origine de la connexion
  int	_or;
  ///	Champ poids
  T	_w;
};


// inline functions


template<class T> inline int lelm<T>::operator == (const lelm<T> & le) const
{
  return( (_or==le._or) && (_w==le._w) );
}


template<class T> inline int lelm<T>::operator != (const lelm<T> & le) const
{
  return( (_or!=le._or) || (_w!=le._w) );
}


template<class T> inline int lelm<T>::operator < (const lelm<T> & le) const
{
  return( _w < le._w );
}


template<class T> inline int lelm<T>::operator > (const lelm<T> & le) const
{
  return( _w > le._w );
}


template<class T> inline int lelm<T>::operator <= (const lelm<T> & le) const
{
  return( _w <= le._w );
}


template<class T> inline int lelm<T>::operator >= (const lelm<T> & le) const
{
  return( _w >= le._w );
}



#endif


