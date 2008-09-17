
#ifndef NEUR_UNIT_H
#define NEUR_UNIT_H


#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>


///
enum Neur_Types
{ 
  ///
  ENTREE, 
  ///
  CACHE, 
  ///
  SORTIE 
};


//	NEURONES

/**	Unit� : neurone de base, classe unit. \\ \\
	template<class T> class unit; \\ \\
	Cette classe repr�sente un neurone dans un r�seau. Une unit� garde en 
	m�moire son nom (nm), son type (t), son biais (bi), son �tat 
	de sortie (o) et ses coordonn�es (x, y, z), qui sont utilis�es pour la 
	repr�sentation graphique par le logiciel SNNS. \\
	La classe T est le type num�rique utilis� pour repr�senter
	le biais et la sortie: par exemple: int ou double. */
template <class T> class unit
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Constructeur
  unit( const char *nom="", int typ=ENTREE, int xx=0, int yy=0, int zz=0, 
       T bia=0, T sor=0 )
  {	set(nom,typ,xx,yy,zz,bia,sor);	}

  ///	Destructeur
  ~unit()	{ }
  //@}

  /**@name	Op�rateurs */
  //@{
  ///
  unit<T> & operator = (const unit<T> & u);
  unit<T> & operator +(const unit<T> & u);
  unit<T> & operator -(const unit<T> & u);
  unit<T> & operator *(const unit<T> & u);
  unit<T> & operator /(const unit<T> & u);
  unit<T> & operator ^(const unit<T> & u);
  unit<T> & operator ++();
  unit<T> & operator +=(const unit<T> & u);
  unit<T> & operator -=(const unit<T> & u);
  unit<T> & operator *=(const unit<T> & u);
  unit<T> & operator /=(const unit<T> & u);
  ///	Egalit� de nom, position, type, biais
  inline int operator ==(const unit<T> & u) const;
  ///	In�galit�
  inline int operator !=(const unit<T> & u) const;
  /**	Comparaison de type. \\
	ENTREE < CACHE < SORTIE */
  inline int operator <(const unit<T> & u) const;
  ///
  inline int operator >(const unit<T> & u) const;
  ///
  inline int operator <=(const unit<T> & u) const;
  ///
  inline int operator >=(const unit<T> & u) const;
  //@}

  /**@name	Acc�s aux champs */
  //@{
  ///	Nom du neurone
  const char *nm() const	{ return(_nm);	}
  ///	Biais
  T	bi() const	{ return(_bi);	}
  ///	Type de neurone
  int	t() const	{ return(_t);	}
  ///	Sortie
  T	o() const	{ return(_o);	}
  ///	Position x
  int	x() const	{ return(_x);	}
  ///	Position y
  int	y() const	{ return(_y);	}
  ///	Position z
  int	z() const	{ return(_z);	}
  //@}

  /**@name	Modification des champs */
  //@{
  ///	Fixe tous les champs � la fois
  void set( const char *nom, int typ, int xx, int yy, int zz, 
		   T bia, T sor);
  ///	Fixe le nom du neurone
  void set_nm(const char *nom);
  /**	Fixe le type de neurone. \\
	Les types possibles sont :\\ ENTREE, CACHE, SORTIE. */
  void set_t(int typ);
  ///	Fixe les coordonn�es
  void set_coord(int xx, int yy, int zz);
  ///	Fixe le biais
  void set_bi(T bia)	{ _bi=bia;	}
  ///	Fixe l'�tat de sortie
  void set_o(T sor)	{ _o=sor;	}
  //@}

  /**@name	Fonctions */
  //@{
  ///	Affiche les caract�ristiques du neurone
  void	aff() const;
  //@}


  protected:

  ///	Affiche un message d'erreur de type
  void errtyp() const;

  ///	Champ nom
  char	_nm[12];	       	/* nom du neurone */
  ///	Champ biais
  T		_bi;		/* biais */
  ///	Champ type
  int		_t;		/* type */
  ///	Champ sortie
  T		_o;		/* etat de sortie */
  ///	Champs coordonn�es
  int		_x;
  ///
  int		_y;
  ///
  int		_z;      	/* coordonnees du neurone */
};


// inline functions

template<class T> inline int unit<T>::operator == ( const unit<T> & u ) const
{
  return( (_t==u._t) && (_bi==u._bi) && (_x==u._x) && (_y==u._y) && (_z==u._z) 
	  && !(strcmp(_nm, u._nm)) );
}


template<class T> inline int unit<T>::operator != ( const unit<T> & u ) const
{
  return( (_t!=u._t) || (_bi!=u._bi) || (_x!=u._x) || (_y!=u._y) || (_z!=u._z) 
	  || strcmp(_nm, u._nm) );
}


template<class T> inline int unit<T>::operator < ( const unit<T> & u ) const
{
  return( _t < u._t );
}


template<class T> inline int unit<T>::operator > ( const unit<T> & u ) const
{
  return( _t > u._t );
}


template<class T> inline int unit<T>::operator <= ( const unit<T> & u ) const
{
  return( _t <= u._t );
}


template<class T> inline int unit<T>::operator >= ( const unit<T> & u ) const
{
  return( _t >= u._t );
}




#endif

