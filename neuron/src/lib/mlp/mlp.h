

#ifndef NEUR_MLP_MLP_H
#define NEUR_MLP_MLP_H


#include <neur/mlp/net.h>


//	PERCEPTRONS MULTI-COUCHES

/**	Perceptron Multi-couches : classe mlp. \\ \\
	template<class T, class U> class mlp : public net<T,U> \\ \\
	Une classe dérivée de net se spécialise dans
	les perceptrons multi-couches (Multi-Layer Perceptrons, ou MLP). Elle
	offre à peu près les mêmes méthodes que la classe
	net, mais celles-ci ont été réecrites pour tenir
	compte de la structure optimisée des réseaux organisés en
	couches. Dans la classe mlp, les neurones et les liaisons sont
	classés par couche, et sont toujours maintenus dans l'ordre au fil des
	modifications. Ainsi les opérations de propagation et
	rétropropagation sont effectuées beaucoup plus vite. La classe
	doit garder en mémoire, en plus de la structure de net, le nombre
	de couches et la liste du nombre d'unités sur chaque couche. */
template<class T, class U> class mlp: public net<T,U>
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	Réseau vide
  mlp(): net<T,U>()		{ _nc=0; _couch = (int*) NULL;	}
  /**	Couches initialisées. \\
	Crée un réseau de nom nom, de nc
	couches, et dont le nombre de neurones sur chaque couche est donné par
	le tableau couch. Chaque neurone d'une couche cachée ou de sortie
	est relié à tous les neurones de la couche
	précédente. */
  mlp(const char *nom, int nc, int *couch)
    :net<T,U>(nom,nc,couch)
  {	init(nom,nc,couch);	}
  mlp(const mlp<T,U> & pmc);

  virtual ~mlp()	{ if(_couch!=NULL) delete[] _couch;	}

  mlp<T,U> & operator = (const mlp<T,U> & pmc);

  /**@name	Fonctions */
  //@{
  /**	Initialisation du réseau. \\
	Initialise le PMC, sans désallouer les éventuelles allocations
	précédentes ! */
  virtual void init( const char *nom, int nc, int *couch);

  ///	Vidange du réseau
  virtual void empty();
  ///	Affichage du contenu du réseau
  virtual void aff() const;
  /**	Ajout d'un neurone. \\
	Ajoute un neurone de nom nom sur la couche c. Ce nouveau neurone
	sera le dernier de la couche, et sera initialisé avec le biais
	bia et la sortie sor. En cas de réussite, la fonction
	retourne 0. */
  virtual void add_neur(const char *nom, int c, T bia, T sor);
  /**	Elimination d'un neurone. \\
	Enlève le no-ème neurone de la couche c, ainsi que
	toutes les liaisons qui en partemt ou y aboutissent. La fonction 
	retourne 0 en cas de réussite. */
  virtual int	 del_neur(int c, int no);
  /**	Ajout d'une liaison. \\
	Ajoute une liaison vers le neurone no tar de la couche c. Cette
	liaison possèdera nli liens élémentaires. Les
	origines et poids de ces liaisons seront à préciser ensuite. La
	fonction renvoie 0 pour un succès. */
  virtual int  add_link(int c, int tar,int nli);
  /**	Ajout d'une liaison. \\
	Idem mais le nombre de liaisons élémentaires est ici égal
	au nombre de neurones sur la couche précédente, et chaque neurone
	de cette couche précédente est relié à cette cible.
	Les poids sont s définir. */
  virtual int  add_link(int c, int tar);
  /**	Elimination d'une liaison. \\
	Enlève une liaison vers le neurone no no de la couche c.
	Le retour est 0 en cas de succès. */
  virtual int  del_link(int c, int no);
  /**	Ajoute un lien élémentaire. \\
	Ajoute un lien élémentaire du neurone or de la couche
	co vers le neurone tar de la couche c, avec le poids
	w. 0 signifie un succès. */
  virtual int	 add_le(int c, int tar, int co, int org, U w);
  /**	Elimination d'un lien élémentaire. \\
	Enlève un lien élémentaire du neurone or de la
	couche co vers le neurone tar de la couche c, et retourne
	0 en cas de réussite. */
  virtual int	 del_le(int c, int tar, int co, int org);
  /**	Numéro d'un neurone. \\
	Renvoie le numéro du neurone n de la couche c. */
  int		get_neur(int c, int n)
  {	int x=0; for(int i=0; i<c; i++) x+=_couch[i]; return(x+n);	}
  /**	Numéro de liaison. \\
	Renvoie le numéro de la liaison vers le neurone n de la couche c. */
  int get_link(int c, int n)
  {	return(net<T,U>::get_link(get_neur(c,n)));	}
  /**	Trie du réseau. \\
	Trie le réseau, en essayant de repérer les
	différentes couches, et place les unités et les liaisons dans
	l'ordre des couches. Si tout se passe bien, la fonction renvoie 0. */
  virtual int  trie();
  /**	Charge le réseau. \\
	Charge un PMC à partir d'un fichier
	SNNS. Le réseau est lu puis trié, à la
	différence de la méthode load() de la classe net.
	Comme toujours, 0 signifie une réussite. */
  virtual int load( const char *nom);
  ///	Charge le réseau à partir d'un stream déja ouvert
  virtual int load( std::istream & fich );
  /**	Fait fonctionner le réseau. \\
	Effectue la propagation avant de l'exemple n de la base d'exemples
	pa à travers le réseau. L'algorithme utilisé
	suppose que les neurones et les liaisons sont classés dans l'ordre des
	couches, comme c'est le cas normalement. Ainsi l'opération est 
	nettement plus rapide que la méthode prop() de la classe net. 0 est
	renvoyé si tout s'est bien passé. */
  virtual int	 prop(const pat<T> & pa, int n);
  ///	Propage les valeurs d'entrées (préalablemet positionnées)
  virtual int prop();
  /**	Apprentissage. \\
	Effectue l'apprentissage du réseau à partir de la base
	d'apprentissage pa. nc cycles sont accomplis sur l'ensemble de la
	base, et l'ordre de passage des exemples est tiré au sort à
	chaque cycle. eta précise le coefficient de modification des
	poids de l'algorithme du gradient et flg précise s'il faut
	afficher tous les (nc/10) cycles où l'on en est (0: pas
	d'affichage). De même que pour prop(), cette méthode est
	optimisée pour un perceptron multi-couches bien trié. */
  virtual int	 backprop(const pat<T> & pa, int nc, double eta, int flg=0, 
			  T *err=NULL );
  /**	Apprentissage, 1 passe, à partir d'un état déjà propagé. \\
	Fonction utilisée par #backprop(..)# avec paramètres. Elle suppose 
	que l'exemple a déjà été propagé (par #prop()#).
	@param	learnO	Tableau des sorties d'apprentissage (désirées)
	@param	eta	facteur d'apprentissage
	@return	erreur entre les sorties désirées et obtenues (*AVANT* 
	apprentissage !)
   */
  virtual T backprop( T *learnO, double eta );
  ///	Fait la réduction pour 1 neurone donné, j-ème de la couche cachée
  virtual void ReductionIndex( int j );
  /**	Réduit pour un PMC à 3 couches. \\
	Pour un PMC à 3 couches, élimine les neurones redondants, selon 
	l'algorithme de Fukumizu */
  virtual void Reduction();
  //@}

  /**@name	Accès aux membres */
  //@{
  ///	Nombre de couches
  int		nc() const     	{ return(_nc);		}
  ///	Tableau des nombres de neurones sur chaque couche
  int		*couch() const	{ return(_couch);	}
  //@}


  protected:

  ///	Nombre de couches
  int		_nc;
  ///	Tableau des nombres de neurones par couche
  int		*_couch;
};


/**@name	Opérateurs externes */
//@{
///
template<class T, class U>
inline std::ostream & operator << ( std::ostream & fich, 
                                    const mlp<T,U> & res );
///
template<class T, class U>
inline std::istream & operator >> ( std::istream & fich, 
                                    mlp<T,U> & res );
//@}

#endif

