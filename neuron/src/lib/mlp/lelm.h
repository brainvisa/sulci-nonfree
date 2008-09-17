
#ifndef NEUR_LELM_H
#define NEUR_LELM_H



//	LIENS ELEMENTAIRES

/**	Liens �l�memtaires : classe lelm. \\ \\
	template<class T> class lelm; \\ \\
	La classe lelm repr�sente un lien �l�mentaire: c'est une des liaisons 
	arrivant sur un neurone donn�, l'ensemble de ces liaisons etant 
	repr�sent� par la classe link. \\
	Un lien �l�mentaire m�morise un num�ro et un poids (w). Le num�ro 
	(or) est celui du neurone � l'origine de la liaison. Le lien 
	�l�mentaire est utilis� par la cible de cette liaison, ainsi la 
	destination n'est pas prise en compte par le lien �l�mentaire, mais 
	par la liaison globale (voir liaison: type link). Le type T est 
	utilis� pour repr�senter le poids de la liaison. */
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

  /**@name	Op�rateurs */
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
  ///	Fixe l'origine et le poids en m�me temps
  void set(int ori, T wgt)	{ _or=ori; _w=wgt; }
  ///	Fixe le poids
  void set_w(T wgt)		{ _w=wgt;	}
  ///	Fixe l'origine
  void set_or(int ori)		{ _or=ori;	}
  //@}

  /**@name	Acc�s aux membres */
  //@{
  ///	Origine
  int	org() const		{ return(_or);	}
  ///	Poids
  T	w() const		{ return(_w);	}
  //@}

  /**@name	Fonctions */
  //@{
  ///	Affichage des caract�ristiques de la liaison
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


