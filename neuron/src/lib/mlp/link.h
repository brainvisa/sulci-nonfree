

#ifndef NEUR_LINK_H
#define NEUR_LINK_H


#include <neur/mlp/lelm.h>


//	CONNEXION

/**	Connexion à un neurone : classe neur_link. \\ \\
	Cette classe représente l'ensemble des liaisons élémentaires qui 
	arrivent à un même neurone-cible. Elle possède donc un numéro de 
	neurone-cible (ta), un nombre de liaisons élémentaire (nl) et un 
	tableau de liaisons élémentaires (le). \\
	T est le type utilisé pour mémoriser les poids des liaisons 
	élémentaires. */
template<class T> class neur_link   		/* - Connexion: -*/
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Initialise la cible et le nombre de liens à zéro
  neur_link()				{ _ta=0; _nl=0; _le=0; }
  ///	Initialise la cible et le nombre de liens comme précisé
  neur_link(int tar, int nli)		{ init(tar,nli);	}
  ///	Initialise tous les liens avec la liste des origines des connexions
  neur_link(int tar, int nli, int *ori)	{ init(tar,nli,ori);	}
  ///	Constructeur de copie
  neur_link(const neur_link<T> & l);

  ///Destructeur
  ~neur_link()				{ delete[] _le;		}
  //@}

  /**@name	Opérateurs */
  //@{
  ///
  neur_link<T> & operator =(const neur_link<T> & le);
  ///	Ajoute un lien élémentaire à une connexion
  inline neur_link<T> operator +(const lelm<T> & le);
  ///	Enlève un lien élémentaire à une connexion
  inline neur_link<T> operator -(const lelm<T> & le);
  neur_link<T> & operator *(const neur_link<T> & le);
  neur_link<T> & operator /(const neur_link<T> & le);
  neur_link<T> & operator ^(const neur_link<T> & le);
  ///	Ajoute un lien élémentaire à une connexion
  neur_link<T> & operator +=(const lelm<T> & le);
  ///	Enlève un lien élémentaire à une connexion
  neur_link<T> & operator -=(const lelm<T> & le);
  neur_link<T> & operator *=(const neur_link<T> & le);
  neur_link<T> & operator /=(const neur_link<T> & le);
  int operator ==(const neur_link<T> & le) const;
  int operator !=(const neur_link<T> & le) const;
  int operator <(const neur_link<T> & le) const;
  int operator >(const neur_link<T> & le) const;
  int operator <=(const neur_link<T> & le) const;
  int operator >=(const neur_link<T> & le) const;
  //@}

  /**@name	Fonctions */
  //@{
  /**	Initialisation. \\
	Initialise une liaison, ne désalloue pas la mémoire précédemment 
	allouée! */
  void init(int tar, int nli);
  ///	Idem et fixe toutes les caractéristiques de la nouvelle liaison
  void init(int tar, int nli, int *ori);

  ///	Fixe la destination de la liaison
  void	set_ta(int t)      	{ _ta=t;	}
  ///	Ajoute un lien élémentaire
  void	add(int ori, T wgt);
  ///	Enlève un lien élémentaire
  int 	del(int ori);
  ///	Affiche les caractéristiques de la liaison
  void	aff() const;
  //@}

  /**@name	Accès aux champs */
  //@{
  ///	Destination de la liaison
  int		ta() const	{ return(_ta);	}
  ///	Nombre de liens élémentaires
  int		nl() const	{ return(_nl);	}
  ///	Liste des liens élémentaires
  lelm<T>	*le() const	{ return(_le);	}
  //@}


  protected:

  ///	Neurone d'arrivée (target)
  int		_ta;
  ///	Nombre de liens arrivant sur ta
  int		_nl;
  ///	Tableau des liens élémentaires
  lelm<T>	*_le;
};






#endif

