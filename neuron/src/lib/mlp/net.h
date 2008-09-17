

#ifndef NEUR_NET_H
#define NEUR_NET_H


#include <neur/mlp/unit.h>
#include <neur/mlp/link.h>
#include <neur/mlp/pat.h>


/*	RESEAU DE NEURONES	*/

/**	Réseau de neurones : classe net. \\ \\
	template<class T, class U> class net; \\ \\
	Cette classe regroupe les cellules et les liaisons en un réseau
	de neurones complet. Deux types de données sont à
	préciser: T est utilisé pour les entrées, sorties et biais des 
	neurones, et U pour les poids. On utilise deux types distincts car on 
	peut très bien construire un réseau à valeurs discrètes
	(ou binaires) mais dont les poids des connexions sont réels. Toutes les
	fonctions pour manipuler les réseaux (chargement, sauvegarde,
	propagation, apprentissage, modification) sont fournies parmi les
	méthodes de la classe. Les caractéristiques d'un réseau
	sont: son nom (nm), son nombre d'unités (nu), son
	nombre de connexions (nl) (nombre total de liaisons
	élémentaires), son nombre d'arrivées de connexions
	(nac) (nombre de liaisons de type neur_link<U>;), son nombre
	d'entrées (ni) et de sorties (no) ainsi que le
	numéro de la 1ère unité de sortie dans la liste des
	neurones (fo), et la liste des unités (u) et des liaisons (l).*/
template<class T, class U> class net
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Réseau vide
  net();
  /**	Réseau initialisé. \\
	Initialise toutes les valeurs du réseau: tar est un tableau contenant 
	la liste des cibles des liaisons, nli est un
	tableau contenant la liste des nombres de liens élémentaires de
	ces liaisons, et ori est un tableau à double entrée qui
	contient, pour chaque liaison, la liste des origines des liens
	élémentaires. */
  net(const char *nom, int nu, int nl, int nac, int ni, int no, int fo,
      int *tar, int *nli, int **ori)
  { init(nom,nu,nl,nac,ni,no,fo,tar,nli,ori); }
  /**	Réseau à couches. \\
	Crée un réseau de type Perceptron Multi-Couches, de nom nom, 
	de nc couches, avec sur chaque couche le nombre de
	neurones donné par le tableau couch. chaque neurone d'une couche
	donnée est relié à l'ensemble des neurones de la couche
	précédente en entrée, et à l'ensemble des neurones
	de la couche suivante en sortie. */
  net( const char *nom, int nc, int *couch )
  {	pmc(nom,nc,couch);	}
  /**	Lit un réseau dans un fichier. \\
	Lit le réseau dans un fichier de nom nom. 
	Le fichier doit être au format SNNS, le réseau
	ne doit pas avoir de sites, et les fonctions d'activation sont de type
	Act_Logistic (sigmoïde). Les neurones et les liaisons sont repris dans
	l'ordre du fichier. */
  net( const char *nom );
  ///	Constructeur de copie
  net(const net<T,U> & n);

  ///	Destructeur
  virtual ~net();
  //@}

  /**@name	Opérateurs */
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
  /**	Initialisation du réseau. \\
	Initialise le réseau comme pour le constructeur correspondant. Cette
	fonction ne désalloue pas d'éventuelles définitions
	précédentes ! */
  virtual void init( const char *nom, int nu, int nl, int nac, int ni, 
		     int no, int fo, int *tar, int *nli, int **ori);

  ///	Vidange du réseau
  virtual void empty();
  ///	Affichage du contenu du réseau
  virtual void aff() const;
  /**	Ajout d'un neurone. \\
	Ajoute un neurone au réseau, de nom nom, de type typ (0:
	entrée, 1: caché, 2: sortie), à la position (x,
	y, z), ayant le biais initialisé à bia et la
	sortie à sor. Le neurone est ajouté en fin de liste (plus
	grand numéro). Le nouveau neurone n'est pas connecté, il faut
	ensuite ajouter les liaisons. */
  virtual void add_neur( const char *nom, int typ, int xx, int yy, int zz,
			 T bia, T sor );
  /**	Elimination d'un neurone. \\
	Enlève un neurone du réseau, de
	numéro no dans la liste des neurones. Les liaisons qui le
	concernent sont également effacées. Les neurones suivants dans la
	liste sont translatés, donc leur numéro est
	décrémenté, mais les liaisons sont remises à jour automatiquement. 
	La fonction renvoie 0 en cas de réussite (si le neurone
	en question existe effectivement). */
  virtual int	 del_neur(int no);
  /**	Ajoute une liaison. \\
	Ajoute une liaison au réseau, de cible tar. Cette liaison
	possèdera nli liens élémentaires (qu'il faudra
	ensuite initialiser). La fonction renvoie 0 en cas de réussite (le
	neurone tar existe). */
  virtual int  add_link(int tar,int nli);
  /**	Elimination d'une liaison. \\
	Efface une liaison, de cible tar, avec
	les liens élémentaires qu'elle contient. Renvoie 0 en cas de
	réussite. */
  virtual int  del_link(int no);
  /**	Ajout d'un lien élémentaire. \\
	Ajoute un lien élémentaire du neurone or vers le neurone
	tar, avec un poids initialisé à w. Si la liaison
	arrivant en tar n'existe pas, il faut la créer auparavant.
	Renvoie 0 en cas de succès. */
  virtual int	 add_le(int tar,int org, U w);
  /**	Elimine un lien élémentaire. \\
	Efface un lien élémentaire du neurone or vers le neurone
	tar. Si la liaison vers tar n'a plus de liens
	élémentaires, elle est supprimée. Renvoie 0 si tout se
	passe bien. */
  virtual int	 del_le(int tar, int org);
  /**	Initialise un Perceptron-Multi-Couches. \\
	Initialise le réseau en tant que Perceptron Multi-Couches. cf
	constructeur correspondant. Cette fonction ne désalloue pas
	d'éventuelles définitions précédentes ! */
  virtual void pmc( const char *nom, int nc, int *couch);
  /**	Charge un réseau. \\
	Charge le réseau à partir d'un fichier de nom "nom". 
	Le format est celui des fichiers SNNS, avec
	les mêmes restrictions que pour le constructeur correspondant. Cette
	fonction ne désalloue pas d'éventuelles définitions
	précédentes ! */
  virtual int  load( const char *nom);
  ///	Lecture dans un stream déja ouvert
  virtual int load( std::istream & fich );
  /**	Sauve le réseau sur disque. \\
	Sauvegarde le réseau dans un fichier de nom
	nom, sous le format <b>SNNS</b>. On peut par la suite reprendre ce
	réseau avec le logiciel SNNS. */
  virtual int save( const char *nom) const;
  ///	Ecriture dans un stream déja ouvert
  virtual int save( std::ostream & fich ) const;
  ///	Renvoie le numéro d'un neurone
  virtual int get_neur( const unit<T> & u );
  /**	Recherche d'une liaison. \\
	Renvoie le numéro de la liaison vers le
	neurone tar. Renvoie -1 si cette liaison n'existe pas. */
  virtual int	 get_link(int tar);
  /**	Fait fonctionner le réseau. \\
	Propage l'exemple numéro n de la base d'exemples pa
	à travers le réseau. Renvoie 0 en cas de succès. */
  virtual int	 prop(const pat<T> & pa, int n);
  ///	Propage les valeurs d'entrées (préalablement positionnées)
  virtual int prop();
  /**	Initialisation des poids. \\
	Initialise tous les poids du réseau aléatoirement, avec des
	valeurs comprises entre min et max (répartition uniforme
	de probabilités). */
  virtual void rand_w(U min, U max);
  /**	Apprentissage. \\
	Effectue l'apprentissage du réseau par l'algorithme de
	rétropropagation du gradient classique. Pour cela la base d'exemple
	pa est utilisée. nc cycles d'apprentissage sont
	effectués sur toute la base d'apprentissage. A chaque cycle, l'ordre de
	passage des exemples est tiré au sort. eta est le coefficient de
	modification des poids appliqué au gradient. flg précise
	s'il faut (flg!=0) ou non (flg=0) afficher où
	l'apprentissage en est tous les dixièmes du nombre de cycles. */
  virtual int backprop(const pat<U> & pa, int nc, double eta, int flg=0, 
		       T *err=NULL );
  /**	Apprentissage d'une liaison. \\
	effectue les modifications de poids sur une liaison particulière
	arrivant en tar. Les poids de chaque liaison élémentaire
	arrivant en tar seront donc modifiés de: \\
	eta * dlt * f'(entrée totale) * entrée particulière : \\
	entrée particulière est la sortie du neurone à l'origine
	du lien élémentaire considéré, entrée totale
	est la somme pondérée de toutes les entrées de tar.
	f' est la dérivée de la fonction d'activation (sigmoïde).
	Pour ce faire, l'état du réseau doit avoir été
	préalablement calculé (à l'aide de prop() par exemple). */
  virtual void learn(int n, T dlt, double eta);
  /**	Nombre de liens. \\
	Retourne le nombre de liens élémentaires arrivant en ta. */
  virtual int nlink(int ta)
  {	int n; if( (n=get_link(ta))<0 ) return(0); return(_l[n].nl());	    }
  /**	Affiche les sorties du reseau. \\
	Affiche les sorties du réseau. Ne précise les numéros et
	noms des cellules que si fl est non nul. */
  virtual void af_out( int fl = 1 ) const;
  ///	Calcule la taille du fichier de sauvegarde
  virtual int bytesize() const;
  //@}

  /**@name	Accès aux membres */
  //@{
  ///	Nom du réseau
  const char	*nm() const	{ return(_nm);	}
  ///	Nombre d'unités
  int		nu() const	{ return(_nu);	}
  ///	Nombre total de liens élémentaires
  int		nl() const	{ return(_nl);	}
  ///	Nombre d'arrivées de connexion (neur_links)
  int		nac() const	{ return(_nac);	}
  ///	Nombre d'entrées
  int		ni() const	{ return(_ni);	}
  ///	Nombre de sorties
  int		no() const	{ return(_no);	}
  ///	Numéro de la 1ère unité de sortie
  int		fo() const	{ return(_fo);	}
  ///	Tableau des unités du réseau
  unit<T>	*u() const	{ return(_u);	}
  ///	Tableau des liaisons
  neur_link<U>	*l() const	{ return(_l);	}
  //@}


  protected:

  /**@name	Fonctions */
  //@{
  /**	Apprentissage des poids d'une liaison. \\
	Cette fonction est utilisée par #backprop()# et #learn()#. */
  virtual T	 learn_p(int ta, T dlt, double eta);
  /**	Réordonnancement du réseau. \\
	Remet en ordre toutes les cellules et toutes les liaisons de manière 
	à l'organiser en couches de propagation des entrées vers les sorties. 
	*/
  virtual int  renum(int org, int ta);
  //@}

  /**@name	Champs */
  //@{
  ///	Nom du réseau
  char		_nm[20];
  ///	Nombre d'unités (de neurones)
  int		_nu;
  ///	Nombre de connexions
  int		_nl;
  ///	Nombre d'arrivées de connexions
  int		_nac;
  ///	Nombre d'entrées
  int	       	_ni;
  ///	Nombre de sorties
  int		_no;
  ///	Numéro de la 1ère sortie
  int		_fo;
  ///	Tableau des neurones
  unit<T>	*_u;
  ///	Tableau des connexions
  neur_link<U>	*_l;
  //@}
};


/**@name	Opérateurs externes */
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


