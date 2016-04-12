
#ifndef SI_GRAPH_ANNEALING_H
#define SI_GRAPH_ANNEALING_H

#include <string>
#include <vector>
#include <si/graph/clique.h>

class Vertex;

namespace sigraph
{
  class CGraph;
  class MGraph;
  class AnnealExtension;
  class SGProvider;

  /**	Type utilisé pour les calculs d'énergie : Classe de stockage des 
	champs d'énergie pour l'échantillonneur de Gibbs ou l'ICM
  */
  struct EnergyField
  {
    EnergyField();
    EnergyField( const EnergyField & ef );

    ///	Noeuds dont on modifie les labels
    std::vector<Vertex*>	vertices;
    ///	Labels choisis pour ces noeuds
    std::vector<std::string>	labels;
    ///	Cliques impliquées par ces noeuds, énergies recalculées correspondantes
    std::map<Clique*, double>	involvedCliques;
    ///	Modification d'énergie pour ces changements de labels
    double			energy;
    ///	exp( - Denergy / temp )
    double			expEnergy;
    ///	Probabilité de cette configuration
    double			probability;
    ///	Probabilité cummulée
    double			probabilitySum;
  };

  //	inline


  inline EnergyField::EnergyField()
  {
  }

  inline EnergyField::EnergyField( const EnergyField & ef )
  {
    vertices = ef.vertices;
    labels = ef.labels;
    involvedCliques = ef.involvedCliques;
    energy = ef.energy;
    expEnergy = ef.expEnergy;
    probability = ef.probability;
    probabilitySum = ef.probabilitySum;
  }


  /**	Recuit simulé.


	Fait le recuit simulé à partir d'un graphe à cliques (CGraph) à 
	étiqueter et d'un graphe modèle (MGraph).

	Avant de faire un recuit, il faut avoir initialisé les cliques du 
	graphe à étiqueter (fonction ModelFinder::initCliques()).

	Il faut aussi initialiser les paramètres du recuit en appelant la 
	fonction init().

	Puis l'utilisation se fait par la fonction fit() (recuit complet), 
	ou pas à pas en initialisant d'abord le recuit (fonction reset()) 
	puis en appelant la fonction fitStep() en boucle jusqu'à ce que 
	isFinished() retourne \c true (recuit terminé).


	Plusieurs modes de recuit sont disponibles:
	- METROPOLIS: Algorithme de Metropolis simplifié. Pour chaque 
	  passe de recuit simulé, on essaie, pour chaque noeud du graphe à 
	  étiqueter, un changement aléatoire d'étiquette. L'ordre de 
	  passage des noeuds est aléatoire et change d'une fois sur 
	  l'autre. La température est abaissée après chaque passage de 
	  tous les noeuds (sans attendre stabilisation comme dans 
	  l'algorithme original de Metropolis).
	- GIBBS: Algorithme du Gibbs Sampler. A chaque passe, chaque 
	  clique est traitée dans un ordre aléatoire. (on travaille sur 
	  les cliques, et pas sur les noeuds comme dans l'algorithme de 
	  Metropolis). Pour chaque clique, toutes les transformations 
	  possibles impliquant un nombre de noeuds maximal donné sont 
	  évaluées, puis le choix se fait aléatoirement parmi ces 
	  transformations en fonction de la probabilité de chacune.
	- ICM: Mode déterministe. L'ICM fonctionne comme le Gibbs sampler 
	  (évaluations par cliques, configurations à plusieurs 
	  changements) sauf que le choix final n'est pas aléatoire, mais 
	  on prend systématiquement le choix le plus probable.
        - MPM: Maximum Posterior Marginal. Annealing is done at a constant 
          temperature (ideally the critical temperature), using the Gibbs
          Sampler algorithm. When the process ends (after a determined number
          of iterations), each node is assigned the label which has been most
          taken during the annealing (most probable label). Then the
          process generally switches to ICM mode to end up in a coherent state
          in a satisfactory minimum.


	Les modes Metropolis, Gibbs et MPM basculent vers le mode ICM en
        dessous	d'un certain seuil de température, pour finir le recuit au
        plus vite dans le minimum local le plus proche.

	Le recuit s'arrête lorsque la proportion du nombre de transformations 
	acceptées par rapport au nombre proposé passe en dessous d'un seuil 
	donné lui aussi.


	On peut interroger l'état du système: température, nombres de 
	changements proposés et acceptés, énergie actuelle, différence 
	d'énergie entre le début du recuit et l'état actuel, différence 
	d'énergie au cours de la dernière passe, etc. en utilisant les 
	fonctions qui vont bien.
  */
  class Anneal
  {
  public:
    enum Mode
      {
	///	Algorithme original de Métropolis
	METROPOLIS, 
	///	Gibbs Sampler
	GIBBS,
	///	ICM (déterministe)
	ICM,
        /// MPM (Maximum Posterior Marginal)
        MPM,
        /// Internal mode
        SPECIAL,
      };

    enum IterType
      {
	///	Itération noeud par noeud, ordre aléatoire
	VERTEX, 
	///	Itération clique par clique, ordre aléatoire
	CLIQUE, 
	///	Itération sur d'autres groupes de noeuds (je verrai + tard)
	CUSTOM
      };

    enum VoidMode
      {
	///	Pas de mode void
	VOIDMODE_NONE, 
	///	Mode régulier
	VOIDMODE_REGULAR, 
	///	Mode stochastique
	VOIDMODE_STOCHASTIC
      };

    enum InitLabelsType
      {
	///	Pas d'initialisation des labels: prendre ceux qui y sont déjà
	INITLABELS_NONE, 
	INITLABELS_VOID, 
	INITLABELS_RANDOM
      };

    Anneal( CGraph & cg, MGraph & rg );
    virtual ~Anneal();

    /**	Fixe l'état initial
	@param	mode	Type de recuit (Anneal::METROPOLIS, Anneal::GIBBS ou 
	Anneal::ICM)
	@param	temp	Température initiale
	@param	tmult	Multiplicateur de décroissance de température (entre 0 
	et 1)
	@param	tICM	Un recuit METROPOLIS ou GIBBS passe en mode ICM quand 
	la température atteint la température tICM
	@param	stopProp	Proportion de changements acceptés en dessous 
	duquel le recuit s'arrête
	@param	gibbsMaxTrans	Nombre max. de transformations à la fois dans 
	une clique par l'algorithme du Gibbs Sampler 
	et l'ICM
	@param	verbosity	Si {\tt true} (par défaut), affiche à chaque 
	pas la température, le mode de recuit, la 
	variation d'énergie du pas, les nombres de 
	changements acceptés et proposés. Si {\tt 
	false}, n'affiche rien.
	@param	itType		Type d'itération sur le graphe: types de 
	groupes de noeuds et ordre de passage. Pour 
	l'instant ça n'est utilisé qu'en modes ICM et 
	Gibbs. Pour Metropolis on verra plus tard.
	@param	plotStream	stream dans lequel les énergies sont écrites
	à chaque pas de recuit, pour traçage de 
	courbes. Si ce paramètre est laissé à 0, rien 
	n'est écrit.
    */
    void init( Mode mode, double temp, double tmult, double tICM, 
	       double stopProp, unsigned gibbsMaxTrans, bool verbose = true, 
	       IterType itType = VERTEX, 
	       InitLabelsType voidInitialLabels = INITLABELS_RANDOM, 
	       const std::string & voidLabel = "", 
               std::ostream *plotStream = 0, 
               unsigned niterBelowStopProp = 1 );
    /**	Mode "void": essais de configs où tous les labels sont remplacés par
	void
	@param	mode	VOIDMODE_NONE: désactivé; VOIDMODE_REGULAR: essais 
	réguliers, une fois sur occurency; VOIDMODE_STOCHASTIC:
	essais irréguliers, avec probabilité 1/occurency
	@param	occurency	périodicité de la passe en mode "void" */
    void setVoidMode( VoidMode mode, unsigned occurency = 0 );
    void deleteExtensions();
    void addExtension( AnnealExtension* ae, unsigned occurency = 20 );
    ///	Libère les structures allouées dans les graphes
    void clear();
    ///	Revient aux conditions initiales
    void reset();
    ///	Teste le critère d'arrêt et fixe l'état correspondant
    void checkStop();
    ///	Effectue une passe de recuit
    void fitStep();
    ///	Effectue tout le recuit
    void fit();
    ///	Effectue une passe en mode Métropolis
    void stepMetropolis();
    ///	Effectue une passe en mode Gibbs Sampler
    void stepGibbs();
    ///	Effectue une passe en mode ICM déterministe
    void stepICM();
    /**	Effectue une passe en mode Void (essais de configs ou tout un label 
	est enlevé) */
    void stepVoid();
    /**	Calcule les différentes énergies des transitions possibles d'un 
	groupe de noeuds, en Gibbs Sampler ou en ICM.
	Cette fonction efface le vecteur et le re-remplit
    */
    void processPotentials( const std::set<Vertex *> & vertices, 
			    std::vector<EnergyField> & en );
    /**	Calcule les potentiels de toutes les cliques. 
	Cette fonction est utilisée à l'initialisation, ou pour calculer 
	l'énergie totale du graphe
	@return	énergie totale du graphe
    */
    double processAllPotentials();

    /**@name	Interrogation de l'état du système */
    //@{
    ///	Graphe à étiqueter
    const CGraph & cGraph() const { return( _cgraph ); }
    CGraph & cGraph() { return( _cgraph ); }
    ///	Graphe modèle
    const MGraph & rGraph() const { return( _mgraph ); }
    MGraph & rGraph() { return( _mgraph ); }
    ///	Mode de recuit initial
    Mode modeI() const { return( _modeI ); }
    ///	Mode courant
    Mode mode() const { return( _mode ); }
    ///	Température initiale
    double tempI() const { return( _tempI ); }
    ///	Température courante
    double temp() const { return( _temp ); }
    ///	Retourne true si le recuit est fini
    bool isFinished() const { return( _finished ); }
    ///	Multiplicxateur de température
    double tMult() const { return( _tmult ); }
    ///	Température de passage en ICM
    double tICM() const { return( _tICM ); }
    /**	Proportion de transformations acceptées en dessous de laquelle on 
	arrête le recuit */
    double stopProp() const { return( _stopProp ); }
    ///	Nombre de transformations acceptées au cours de la dernière passe
    unsigned nTrans() const { return( _ntrans ); }
    ///	Nombre de transformations proposées au cours de la dernière passe
    unsigned maxTrans() const { return( _maxtrans ); }
    /**	Nombre max. de transformations par groupe de noeuds dans le 
	Gibbs Sampler (et l'ICM) */
    unsigned gibbsMaxTrans() const { return( _gibbsMaxTrans ); }
    ///	Variation d'énergie du graphe depuis le début du recuit
    double deltaE() const { return( _deltaE ); }
    ///	Variation d'énergie du graphe à la dernière passe
    double stepDeltaE() const { return( _stepDeltaE ); }
    double initialEnergy() const { return( _initEnergy ); }
    double energy() const { return( _initEnergy + _deltaE ); }
    ///	dit si les affichages sont autorisés ou interdits
    bool verbosity() const { return( _verbose ); }
    IterType iterType() const { return( _iterType ); }
    ///	Nb d'itérations de recuit effectuées
    unsigned nIter() const { return( _niter ); }
    /**	Dit si on est en mode DoubleTirage, technologie © JeffProd'00, pour 
	les passes void et extensions */
    bool doubleDrawingLots() const { return( _doubleDrawingLots ); }
    void setDoubleDrawingLots( bool t ) { _doubleDrawingLots = t; }
    const std::string & voidLabel() const { return( _voidLabel ); }
    /** enables or disables the use of multiple threads during annealing.
        When enabled, multiple threads will only be used on platforms that
        support such a feature (threads and thread-safe reference counters),
        and which have multiple CPUs. In such case, one thread by CPU will
        be used.
        Be warned that there will not necessarily be a large benefit for
        it since threads synchronization will use a significant overhead
        and bottlenecks. On a bi-Xeon Linux with hyperthreading enabled,
        I ususally don't get more than 40-50% speedup for 4 logical CPUs
        (but 2 physical CPUs are actually present). On a bi-G5 Mac, I get
        basically almost no speed improvement.
    */
    void setAllowThreads( bool );
    bool threadsAllowed() const;
    void setMaxIterations( unsigned n );
    /** Maximum number of iterations before switching to next mode.
    Especially useful in constant temperature modes such as MPM.
    The default is 0 and means no limit.
     */
    unsigned maxIterations() const;
    /** Number of MPM steps before labels stats are recorded.
        Useful to wait a kind of stabilization before recording labels stats
    */
    unsigned MPMUnrecordedIterations() const;
    void setMPMUnrecordedIterations( unsigned n );
    //@}

  protected:
    ///	Graphe à étiqueter
    CGraph				&_cgraph;
    ///	Graphe modèle
    MGraph				&_mgraph;
    ///	Mode initial
    Mode				_modeI;
    ///	Mode courant
    Mode				_mode;
    ///	Température initiale
    double				_tempI;
    ///	Température courante
    double				_temp;
    bool				_finished;
    double				_tmult;
    double				_tICM;
    double				_stopProp;
    unsigned				_ntrans;
    unsigned				_maxtrans;
    unsigned				_gibbsMaxTrans;
    double				_stepDeltaE;
    double				_deltaE;
    double				_initEnergy;
    bool				_verbose;
    IterType				_iterType;
    SGProvider				*_sgProvider;
    unsigned				_niter;
    std::ostream			*_plotStream;
    InitLabelsType			_initialLabelsType;
    std::string				_voidLabel;
    VoidMode				_voidmode;
    unsigned				_voidoccurency;
    std::vector<AnnealExtension *>	_annealExtensions;
    unsigned				_extensionPassOccurency;
    bool				_doubleDrawingLots;
    unsigned				_niterBelowStopProp;
    unsigned				_currentNiterBelowStopProp;

  private:
    struct Private;
    Private	*d;

    /**	Fonction utilisée par processPotentials().
	Calcule les potentiels pour les changements de nn noeuds parmi ceux de 
	numéro au moins \c first
	@param	ver	groupe de noeuds à tester
	@param	ef	vecteur d'énergies à remplir. Les nouvelles valeurs 
	sont ajoutées au vecteur
	@param	npos	tableau de booléens des positions de tous les noeuds 
	(de taille \c cl->size())
	@param	first	numéro du 1er noeud qu'on a le droit d'utiliser
	@param	nn	nombre de noeuds à choisir (parmi les <tt>cl->size() 
	- first</tt>)
	@param	orLab	tableau des labels d'origine des noeuds de la clique
    */
    void processNodes( const std::set<Vertex *> & ver, std::vector<EnergyField> & ef, 
		       bool* npos, unsigned first, unsigned nn, 
		       std::string* orLab );
    /**	Fonction utilisée par processPotentials().
	Calcule le potentiel d'une configuration, labels déjà fixés sur les 
	noeuds
	@param	ver	groupe de noeuds
	@param	ef	vecteur de champs d'énergies auquel on ajoute une 
	entrée
	@param	npos	liste des noeuds modifiés
	@param	orLab	tableau des labels d'origine des noeuds de la clique
    */
    void processConfig( const std::set<Vertex *> & ver, 
			std::vector<EnergyField> & ef, 
			bool* npos, std::string* orLab );
  };

}

#endif


