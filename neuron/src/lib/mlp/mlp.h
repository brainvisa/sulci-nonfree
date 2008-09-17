

#ifndef NEUR_MLP_MLP_H
#define NEUR_MLP_MLP_H


#include <neur/mlp/net.h>


//	PERCEPTRONS MULTI-COUCHES

/**	Perceptron Multi-couches : classe mlp. \\ \\
	template<class T, class U> class mlp : public net<T,U> \\ \\
	Une classe d�riv�e de net se sp�cialise dans
	les perceptrons multi-couches (Multi-Layer Perceptrons, ou MLP). Elle
	offre � peu pr�s les m�mes m�thodes que la classe
	net, mais celles-ci ont �t� r�ecrites pour tenir
	compte de la structure optimis�e des r�seaux organis�s en
	couches. Dans la classe mlp, les neurones et les liaisons sont
	class�s par couche, et sont toujours maintenus dans l'ordre au fil des
	modifications. Ainsi les op�rations de propagation et
	r�tropropagation sont effectu�es beaucoup plus vite. La classe
	doit garder en m�moire, en plus de la structure de net, le nombre
	de couches et la liste du nombre d'unit�s sur chaque couche. */
template<class T, class U> class mlp: public net<T,U>
{
  public:

  /**@name	Constructeurs - Destructeur */
  //@{
  ///	R�seau vide
  mlp(): net<T,U>()		{ _nc=0; _couch = (int*) NULL;	}
  /**	Couches initialis�es. \\
	Cr�e un r�seau de nom nom, de nc
	couches, et dont le nombre de neurones sur chaque couche est donn� par
	le tableau couch. Chaque neurone d'une couche cach�e ou de sortie
	est reli� � tous les neurones de la couche
	pr�c�dente. */
  mlp(const char *nom, int nc, int *couch)
    :net<T,U>(nom,nc,couch)
  {	init(nom,nc,couch);	}
  mlp(const mlp<T,U> & pmc);

  virtual ~mlp()	{ if(_couch!=NULL) delete[] _couch;	}

  mlp<T,U> & operator = (const mlp<T,U> & pmc);

  /**@name	Fonctions */
  //@{
  /**	Initialisation du r�seau. \\
	Initialise le PMC, sans d�sallouer les �ventuelles allocations
	pr�c�dentes ! */
  virtual void init( const char *nom, int nc, int *couch);

  ///	Vidange du r�seau
  virtual void empty();
  ///	Affichage du contenu du r�seau
  virtual void aff() const;
  /**	Ajout d'un neurone. \\
	Ajoute un neurone de nom nom sur la couche c. Ce nouveau neurone
	sera le dernier de la couche, et sera initialis� avec le biais
	bia et la sortie sor. En cas de r�ussite, la fonction
	retourne 0. */
  virtual void add_neur(const char *nom, int c, T bia, T sor);
  /**	Elimination d'un neurone. \\
	Enl�ve le no-�me neurone de la couche c, ainsi que
	toutes les liaisons qui en partemt ou y aboutissent. La fonction 
	retourne 0 en cas de r�ussite. */
  virtual int	 del_neur(int c, int no);
  /**	Ajout d'une liaison. \\
	Ajoute une liaison vers le neurone no tar de la couche c. Cette
	liaison poss�dera nli liens �l�mentaires. Les
	origines et poids de ces liaisons seront � pr�ciser ensuite. La
	fonction renvoie 0 pour un succ�s. */
  virtual int  add_link(int c, int tar,int nli);
  /**	Ajout d'une liaison. \\
	Idem mais le nombre de liaisons �l�mentaires est ici �gal
	au nombre de neurones sur la couche pr�c�dente, et chaque neurone
	de cette couche pr�c�dente est reli� � cette cible.
	Les poids sont s d�finir. */
  virtual int  add_link(int c, int tar);
  /**	Elimination d'une liaison. \\
	Enl�ve une liaison vers le neurone no no de la couche c.
	Le retour est 0 en cas de succ�s. */
  virtual int  del_link(int c, int no);
  /**	Ajoute un lien �l�mentaire. \\
	Ajoute un lien �l�mentaire du neurone or de la couche
	co vers le neurone tar de la couche c, avec le poids
	w. 0 signifie un succ�s. */
  virtual int	 add_le(int c, int tar, int co, int org, U w);
  /**	Elimination d'un lien �l�mentaire. \\
	Enl�ve un lien �l�mentaire du neurone or de la
	couche co vers le neurone tar de la couche c, et retourne
	0 en cas de r�ussite. */
  virtual int	 del_le(int c, int tar, int co, int org);
  /**	Num�ro d'un neurone. \\
	Renvoie le num�ro du neurone n de la couche c. */
  int		get_neur(int c, int n)
  {	int x=0; for(int i=0; i<c; i++) x+=_couch[i]; return(x+n);	}
  /**	Num�ro de liaison. \\
	Renvoie le num�ro de la liaison vers le neurone n de la couche c. */
  int get_link(int c, int n)
  {	return(net<T,U>::get_link(get_neur(c,n)));	}
  /**	Trie du r�seau. \\
	Trie le r�seau, en essayant de rep�rer les
	diff�rentes couches, et place les unit�s et les liaisons dans
	l'ordre des couches. Si tout se passe bien, la fonction renvoie 0. */
  virtual int  trie();
  /**	Charge le r�seau. \\
	Charge un PMC � partir d'un fichier
	SNNS. Le r�seau est lu puis tri�, � la
	diff�rence de la m�thode load() de la classe net.
	Comme toujours, 0 signifie une r�ussite. */
  virtual int load( const char *nom);
  ///	Charge le r�seau � partir d'un stream d�ja ouvert
  virtual int load( std::istream & fich );
  /**	Fait fonctionner le r�seau. \\
	Effectue la propagation avant de l'exemple n de la base d'exemples
	pa � travers le r�seau. L'algorithme utilis�
	suppose que les neurones et les liaisons sont class�s dans l'ordre des
	couches, comme c'est le cas normalement. Ainsi l'op�ration est 
	nettement plus rapide que la m�thode prop() de la classe net. 0 est
	renvoy� si tout s'est bien pass�. */
  virtual int	 prop(const pat<T> & pa, int n);
  ///	Propage les valeurs d'entr�es (pr�alablemet positionn�es)
  virtual int prop();
  /**	Apprentissage. \\
	Effectue l'apprentissage du r�seau � partir de la base
	d'apprentissage pa. nc cycles sont accomplis sur l'ensemble de la
	base, et l'ordre de passage des exemples est tir� au sort �
	chaque cycle. eta pr�cise le coefficient de modification des
	poids de l'algorithme du gradient et flg pr�cise s'il faut
	afficher tous les (nc/10) cycles o� l'on en est (0: pas
	d'affichage). De m�me que pour prop(), cette m�thode est
	optimis�e pour un perceptron multi-couches bien tri�. */
  virtual int	 backprop(const pat<T> & pa, int nc, double eta, int flg=0, 
			  T *err=NULL );
  /**	Apprentissage, 1 passe, � partir d'un �tat d�j� propag�. \\
	Fonction utilis�e par #backprop(..)# avec param�tres. Elle suppose 
	que l'exemple a d�j� �t� propag� (par #prop()#).
	@param	learnO	Tableau des sorties d'apprentissage (d�sir�es)
	@param	eta	facteur d'apprentissage
	@return	erreur entre les sorties d�sir�es et obtenues (*AVANT* 
	apprentissage !)
   */
  virtual T backprop( T *learnO, double eta );
  ///	Fait la r�duction pour 1 neurone donn�, j-�me de la couche cach�e
  virtual void ReductionIndex( int j );
  /**	R�duit pour un PMC � 3 couches. \\
	Pour un PMC � 3 couches, �limine les neurones redondants, selon 
	l'algorithme de Fukumizu */
  virtual void Reduction();
  //@}

  /**@name	Acc�s aux membres */
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


/**@name	Op�rateurs externes */
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

