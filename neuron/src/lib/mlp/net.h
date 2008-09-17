

#ifndef NEUR_NET_H
#define NEUR_NET_H


#include <neur/mlp/unit.h>
#include <neur/mlp/link.h>
#include <neur/mlp/pat.h>


/*	RESEAU DE NEURONES	*/

/**	R�seau de neurones : classe net. \\ \\
	template<class T, class U> class net; \\ \\
	Cette classe regroupe les cellules et les liaisons en un r�seau
	de neurones complet. Deux types de donn�es sont �
	pr�ciser: T est utilis� pour les entr�es, sorties et biais des 
	neurones, et U pour les poids. On utilise deux types distincts car on 
	peut tr�s bien construire un r�seau � valeurs discr�tes
	(ou binaires) mais dont les poids des connexions sont r�els. Toutes les
	fonctions pour manipuler les r�seaux (chargement, sauvegarde,
	propagation, apprentissage, modification) sont fournies parmi les
	m�thodes de la classe. Les caract�ristiques d'un r�seau
	sont: son nom (nm), son nombre d'unit�s (nu), son
	nombre de connexions (nl) (nombre total de liaisons
	�l�mentaires), son nombre d'arriv�es de connexions
	(nac) (nombre de liaisons de type neur_link<U>;), son nombre
	d'entr�es (ni) et de sorties (no) ainsi que le
	num�ro de la 1�re unit� de sortie dans la liste des
	neurones (fo), et la liste des unit�s (u) et des liaisons (l).*/
template<class T, class U> class net
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	R�seau vide
  net();
  /**	R�seau initialis�. \\
	Initialise toutes les valeurs du r�seau: tar est un tableau contenant 
	la liste des cibles des liaisons, nli est un
	tableau contenant la liste des nombres de liens �l�mentaires de
	ces liaisons, et ori est un tableau � double entr�e qui
	contient, pour chaque liaison, la liste des origines des liens
	�l�mentaires. */
  net(const char *nom, int nu, int nl, int nac, int ni, int no, int fo,
      int *tar, int *nli, int **ori)
  { init(nom,nu,nl,nac,ni,no,fo,tar,nli,ori); }
  /**	R�seau � couches. \\
	Cr�e un r�seau de type Perceptron Multi-Couches, de nom nom, 
	de nc couches, avec sur chaque couche le nombre de
	neurones donn� par le tableau couch. chaque neurone d'une couche
	donn�e est reli� � l'ensemble des neurones de la couche
	pr�c�dente en entr�e, et � l'ensemble des neurones
	de la couche suivante en sortie. */
  net( const char *nom, int nc, int *couch )
  {	pmc(nom,nc,couch);	}
  /**	Lit un r�seau dans un fichier. \\
	Lit le r�seau dans un fichier de nom nom. 
	Le fichier doit �tre au format SNNS, le r�seau
	ne doit pas avoir de sites, et les fonctions d'activation sont de type
	Act_Logistic (sigmo�de). Les neurones et les liaisons sont repris dans
	l'ordre du fichier. */
  net( const char *nom );
  ///	Constructeur de copie
  net(const net<T,U> & n);

  ///	Destructeur
  virtual ~net();
  //@}

  /**@name	Op�rateurs */
  //@{
  ///	Copie
  net<T,U> & operator = ( const net<T,U> & n );
  ///	Ajout d'un neurone
  net<T,U> operator + (const unit<T> & u );
  ///	Ajout d'une connexion
  net<T,U> operator + ( const neur_link<U> & l );
  ///	Suppression d'un neurone
  net<T,U> operator - (const unit<T> & u );
  net<T,U> & operator *(const net<T,U> & le);
  net<T,U> & operator /(const net<T,U> & le);
  net<T,U> & operator ^(const net<T,U> & le);
  ///	Ajout d'un neurone
  net<T,U> & operator +=(const unit<T> & n );
  ///	Ajout d'une connexion
  net<T,U> & operator +=( const neur_link<U> & l );
  ///	Suppression d'un neurone
  net<T,U> & operator -=(const unit<T> & n );
  //@}

  /**@name	Fonctions */
  //@{
  /**	Initialisation du r�seau. \\
	Initialise le r�seau comme pour le constructeur correspondant. Cette
	fonction ne d�salloue pas d'�ventuelles d�finitions
	pr�c�dentes ! */
  virtual void init( const char *nom, int nu, int nl, int nac, int ni, 
		     int no, int fo, int *tar, int *nli, int **ori);

  ///	Vidange du r�seau
  virtual void empty();
  ///	Affichage du contenu du r�seau
  virtual void aff() const;
  /**	Ajout d'un neurone. \\
	Ajoute un neurone au r�seau, de nom nom, de type typ (0:
	entr�e, 1: cach�, 2: sortie), � la position (x,
	y, z), ayant le biais initialis� � bia et la
	sortie � sor. Le neurone est ajout� en fin de liste (plus
	grand num�ro). Le nouveau neurone n'est pas connect�, il faut
	ensuite ajouter les liaisons. */
  virtual void add_neur( const char *nom, int typ, int xx, int yy, int zz,
			 T bia, T sor );
  /**	Elimination d'un neurone. \\
	Enl�ve un neurone du r�seau, de
	num�ro no dans la liste des neurones. Les liaisons qui le
	concernent sont �galement effac�es. Les neurones suivants dans la
	liste sont translat�s, donc leur num�ro est
	d�cr�ment�, mais les liaisons sont remises � jour automatiquement. 
	La fonction renvoie 0 en cas de r�ussite (si le neurone
	en question existe effectivement). */
  virtual int	 del_neur(int no);
  /**	Ajoute une liaison. \\
	Ajoute une liaison au r�seau, de cible tar. Cette liaison
	poss�dera nli liens �l�mentaires (qu'il faudra
	ensuite initialiser). La fonction renvoie 0 en cas de r�ussite (le
	neurone tar existe). */
  virtual int  add_link(int tar,int nli);
  /**	Elimination d'une liaison. \\
	Efface une liaison, de cible tar, avec
	les liens �l�mentaires qu'elle contient. Renvoie 0 en cas de
	r�ussite. */
  virtual int  del_link(int no);
  /**	Ajout d'un lien �l�mentaire. \\
	Ajoute un lien �l�mentaire du neurone or vers le neurone
	tar, avec un poids initialis� � w. Si la liaison
	arrivant en tar n'existe pas, il faut la cr�er auparavant.
	Renvoie 0 en cas de succ�s. */
  virtual int	 add_le(int tar,int org, U w);
  /**	Elimine un lien �l�mentaire. \\
	Efface un lien �l�mentaire du neurone or vers le neurone
	tar. Si la liaison vers tar n'a plus de liens
	�l�mentaires, elle est supprim�e. Renvoie 0 si tout se
	passe bien. */
  virtual int	 del_le(int tar, int org);
  /**	Initialise un Perceptron-Multi-Couches. \\
	Initialise le r�seau en tant que Perceptron Multi-Couches. cf
	constructeur correspondant. Cette fonction ne d�salloue pas
	d'�ventuelles d�finitions pr�c�dentes ! */
  virtual void pmc( const char *nom, int nc, int *couch);
  /**	Charge un r�seau. \\
	Charge le r�seau � partir d'un fichier de nom "nom". 
	Le format est celui des fichiers SNNS, avec
	les m�mes restrictions que pour le constructeur correspondant. Cette
	fonction ne d�salloue pas d'�ventuelles d�finitions
	pr�c�dentes ! */
  virtual int  load( const char *nom);
  ///	Lecture dans un stream d�ja ouvert
  virtual int load( std::istream & fich );
  /**	Sauve le r�seau sur disque. \\
	Sauvegarde le r�seau dans un fichier de nom
	nom, sous le format <b>SNNS</b>. On peut par la suite reprendre ce
	r�seau avec le logiciel SNNS. */
  virtual int save( const char *nom) const;
  ///	Ecriture dans un stream d�ja ouvert
  virtual int save( std::ostream & fich ) const;
  ///	Renvoie le num�ro d'un neurone
  virtual int get_neur( const unit<T> & u );
  /**	Recherche d'une liaison. \\
	Renvoie le num�ro de la liaison vers le
	neurone tar. Renvoie -1 si cette liaison n'existe pas. */
  virtual int	 get_link(int tar);
  /**	Fait fonctionner le r�seau. \\
	Propage l'exemple num�ro n de la base d'exemples pa
	� travers le r�seau. Renvoie 0 en cas de succ�s. */
  virtual int	 prop(const pat<T> & pa, int n);
  ///	Propage les valeurs d'entr�es (pr�alablement positionn�es)
  virtual int prop();
  /**	Initialisation des poids. \\
	Initialise tous les poids du r�seau al�atoirement, avec des
	valeurs comprises entre min et max (r�partition uniforme
	de probabilit�s). */
  virtual void rand_w(U min, U max);
  /**	Apprentissage. \\
	Effectue l'apprentissage du r�seau par l'algorithme de
	r�tropropagation du gradient classique. Pour cela la base d'exemple
	pa est utilis�e. nc cycles d'apprentissage sont
	effectu�s sur toute la base d'apprentissage. A chaque cycle, l'ordre de
	passage des exemples est tir� au sort. eta est le coefficient de
	modification des poids appliqu� au gradient. flg pr�cise
	s'il faut (flg!=0) ou non (flg=0) afficher o�
	l'apprentissage en est tous les dixi�mes du nombre de cycles. */
  virtual int backprop(const pat<U> & pa, int nc, double eta, int flg=0, 
		       T *err=NULL );
  /**	Apprentissage d'une liaison. \\
	effectue les modifications de poids sur une liaison particuli�re
	arrivant en tar. Les poids de chaque liaison �l�mentaire
	arrivant en tar seront donc modifi�s de: \\
	eta * dlt * f'(entr�e totale) * entr�e particuli�re : \\
	entr�e particuli�re est la sortie du neurone � l'origine
	du lien �l�mentaire consid�r�, entr�e totale
	est la somme pond�r�e de toutes les entr�es de tar.
	f' est la d�riv�e de la fonction d'activation (sigmo�de).
	Pour ce faire, l'�tat du r�seau doit avoir �t�
	pr�alablement calcul� (� l'aide de prop() par exemple). */
  virtual void learn(int n, T dlt, double eta);
  /**	Nombre de liens. \\
	Retourne le nombre de liens �l�mentaires arrivant en ta. */
  virtual int nlink(int ta)
  {	int n; if( (n=get_link(ta))<0 ) return(0); return(_l[n].nl());	    }
  /**	Affiche les sorties du reseau. \\
	Affiche les sorties du r�seau. Ne pr�cise les num�ros et
	noms des cellules que si fl est non nul. */
  virtual void af_out( int fl = 1 ) const;
  ///	Calcule la taille du fichier de sauvegarde
  virtual int bytesize() const;
  //@}

  /**@name	Acc�s aux membres */
  //@{
  ///	Nom du r�seau
  const char	*nm() const	{ return(_nm);	}
  ///	Nombre d'unit�s
  int		nu() const	{ return(_nu);	}
  ///	Nombre total de liens �l�mentaires
  int		nl() const	{ return(_nl);	}
  ///	Nombre d'arriv�es de connexion (neur_links)
  int		nac() const	{ return(_nac);	}
  ///	Nombre d'entr�es
  int		ni() const	{ return(_ni);	}
  ///	Nombre de sorties
  int		no() const	{ return(_no);	}
  ///	Num�ro de la 1�re unit� de sortie
  int		fo() const	{ return(_fo);	}
  ///	Tableau des unit�s du r�seau
  unit<T>	*u() const	{ return(_u);	}
  ///	Tableau des liaisons
  neur_link<U>	*l() const	{ return(_l);	}
  //@}


  protected:

  /**@name	Fonctions */
  //@{
  /**	Apprentissage des poids d'une liaison. \\
	Cette fonction est utilis�e par #backprop()# et #learn()#. */
  virtual T	 learn_p(int ta, T dlt, double eta);
  /**	R�ordonnancement du r�seau. \\
	Remet en ordre toutes les cellules et toutes les liaisons de mani�re 
	� l'organiser en couches de propagation des entr�es vers les sorties. 
	*/
  virtual int  renum(int org, int ta);
  //@}

  /**@name	Champs */
  //@{
  ///	Nom du r�seau
  char		_nm[20];
  ///	Nombre d'unit�s (de neurones)
  int		_nu;
  ///	Nombre de connexions
  int		_nl;
  ///	Nombre d'arriv�es de connexions
  int		_nac;
  ///	Nombre d'entr�es
  int	       	_ni;
  ///	Nombre de sorties
  int		_no;
  ///	Num�ro de la 1�re sortie
  int		_fo;
  ///	Tableau des neurones
  unit<T>	*_u;
  ///	Tableau des connexions
  neur_link<U>	*_l;
  //@}
};


/**@name	Op�rateurs externes */
//@{
///
template<class T, class U>
inline std::ostream & operator << ( std::ostream & fich, 
                                    const net<T,U> & res );
///
template<class T, class U>
inline std::istream & operator >> ( std::istream & fich, 
                                    net<T,U> & res );
//@}





#endif


