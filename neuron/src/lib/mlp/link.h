

#ifndef NEUR_LINK_H
#define NEUR_LINK_H


#include <neur/mlp/lelm.h>


//	CONNEXION

/**	Connexion � un neurone : classe neur_link. \\ \\
	Cette classe repr�sente l'ensemble des liaisons �l�mentaires qui 
	arrivent � un m�me neurone-cible. Elle poss�de donc un num�ro de 
	neurone-cible (ta), un nombre de liaisons �l�mentaire (nl) et un 
	tableau de liaisons �l�mentaires (le). \\
	T est le type utilis� pour m�moriser les poids des liaisons 
	�l�mentaires. */
template<class T> class neur_link   		/* - Connexion: -*/
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Initialise la cible et le nombre de liens � z�ro
  neur_link()				{ _ta=0; _nl=0; _le=0; }
  ///	Initialise la cible et le nombre de liens comme pr�cis�
  neur_link(int tar, int nli)		{ init(tar,nli);	}
  ///	Initialise tous les liens avec la liste des origines des connexions
  neur_link(int tar, int nli, int *ori)	{ init(tar,nli,ori);	}
  ///	Constructeur de copie
  neur_link(const neur_link<T> & l);

  ///Destructeur
  ~neur_link()				{ delete[] _le;		}
  //@}

  /**@name	Op�rateurs */
  //@{
  ///
  neur_link<T> & operator =(const neur_link<T> & le);
  ///	Ajoute un lien �l�mentaire � une connexion
  inline neur_link<T> operator +(const lelm<T> & le);
  ///	Enl�ve un lien �l�mentaire � une connexion
  inline neur_link<T> operator -(const lelm<T> & le);
  neur_link<T> & operator *(const neur_link<T> & le);
  neur_link<T> & operator /(const neur_link<T> & le);
  neur_link<T> & operator ^(const neur_link<T> & le);
  ///	Ajoute un lien �l�mentaire � une connexion
  neur_link<T> & operator +=(const lelm<T> & le);
  ///	Enl�ve un lien �l�mentaire � une connexion
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
	Initialise une liaison, ne d�salloue pas la m�moire pr�c�demment 
	allou�e! */
  void init(int tar, int nli);
  ///	Idem et fixe toutes les caract�ristiques de la nouvelle liaison
  void init(int tar, int nli, int *ori);

  ///	Fixe la destination de la liaison
  void	set_ta(int t)      	{ _ta=t;	}
  ///	Ajoute un lien �l�mentaire
  void	add(int ori, T wgt);
  ///	Enl�ve un lien �l�mentaire
  int 	del(int ori);
  ///	Affiche les caract�ristiques de la liaison
  void	aff() const;
  //@}

  /**@name	Acc�s aux champs */
  //@{
  ///	Destination de la liaison
  int		ta() const	{ return(_ta);	}
  ///	Nombre de liens �l�mentaires
  int		nl() const	{ return(_nl);	}
  ///	Liste des liens �l�mentaires
  lelm<T>	*le() const	{ return(_le);	}
  //@}


  protected:

  ///	Neurone d'arriv�e (target)
  int		_ta;
  ///	Nombre de liens arrivant sur ta
  int		_nl;
  ///	Tableau des liens �l�mentaires
  lelm<T>	*_le;
};






#endif

