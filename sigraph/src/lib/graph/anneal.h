
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

  /**	Type utilis� pour les calculs d'�nergie : Classe de stockage des 
	champs d'�nergie pour l'�chantillonneur de Gibbs ou l'ICM
  */
  struct EnergyField
  {
    EnergyField();
    EnergyField( const EnergyField & ef );

    ///	Noeuds dont on modifie les labels
    std::vector<Vertex*>	vertices;
    ///	Labels choisis pour ces noeuds
    std::vector<std::string>	labels;
    ///	Cliques impliqu�es par ces noeuds, �nergies recalcul�es correspondantes
    std::map<Clique*, double>	involvedCliques;
    ///	Modification d'�nergie pour ces changements de labels
    double			energy;
    ///	exp( - Denergy / temp )
    double			expEnergy;
    ///	Probabilit� de cette configuration
    double			probability;
    ///	Probabilit� cummul�e
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


  /**	Recuit simul�.


	Fait le recuit simul� � partir d'un graphe � cliques (CGraph) � 
	�tiqueter et d'un graphe mod�le (MGraph).

	Avant de faire un recuit, il faut avoir initialis� les cliques du 
	graphe � �tiqueter (fonction CGraph::initCliques() de la classe 
	CGraph).

	Il faut aussi initialiser les param�tres du recuit en appelant la 
	fonction init().

	Puis l'utilisation se fait par la fonction fit() (recuit complet), 
	ou pas � pas en initialisant d'abord le recuit (fonction reset()) 
	puis en appelant la fonction fitStep() en boucle jusqu'� ce que 
	isFinished() retourne \c true (recuit termin�).


	Plusieurs modes de recuit sont disponibles:
	- METROPOLIS: Algorithme de Metropolis simplifi�. Pour chaque 
	  passe de recuit simul�, on essaie, pour chaque noeud du graphe � 
	  �tiqueter, un changement al�atoire d'�tiquette. L'ordre de 
	  passage des noeuds est al�atoire et change d'une fois sur 
	  l'autre. La temp�rature est abaiss�e apr�s chaque passage de 
	  tous les noeuds (sans attendre stabilisation comme dans 
	  l'algorithme original de Metropolis).
	- GIBBS: Algorithme du Gibbs Sampler. A chaque passe, chaque 
	  clique est trait�e dans un ordre al�atoire. (on travaille sur 
	  les cliques, et pas sur les noeuds comme dans l'algorithme de 
	  Metropolis). Pour chaque clique, toutes les transformations 
	  possibles impliquant un nombre de noeuds maximal donn� sont 
	  �valu�es, puis le choix se fait al�atoirement parmi ces 
	  transformations en fonction de la probabilit� de chacune.
	- ICM: Mode d�terministe. L'ICM fonctionne comme le Gibbs sampler 
	  (�valuations par cliques, configurations � plusieurs 
	  changements) sauf que le choix final n'est pas al�atoire, mais 
	  on prend syst�matiquement le choix le plus probable.
        - MPM: Maximum Posterior Marginal. Annealing is done at a constant 
          temperature (ideally the critical temperature), using the Gibbs
          Sampler algorithm. When the process ends (after a determined number
          of iterations), each node is assigned the label which has been most
          taken during the annealing (most probable label). Then the
          process generally switches to ICM mode to end up in a coherent state
          in a satisfactory minimum.


	Les modes Metropolis, Gibbs et MPM basculent vers le mode ICM en
        dessous	d'un certain seuil de temp�rature, pour finir le recuit au
        plus vite dans le minimum local le plus proche.

	Le recuit s'arr�te lorsque la proportion du nombre de transformations 
	accept�es par rapport au nombre propos� passe en dessous d'un seuil 
	donn� lui aussi.


	On peut interroger l'�tat du syst�me: temp�rature, nombres de 
	changements propos�s et accept�s, �nergie actuelle, diff�rence 
	d'�nergie entre le d�but du recuit et l'�tat actuel, diff�rence 
	d'�nergie au cours de la derni�re passe, etc. en utilisant les 
	fonctions qui vont bien.
  */
  class Anneal
  {
  public:
    enum Mode
      {
	///	Algorithme original de M�tropolis
	METROPOLIS, 
	///	Gibbs Sampler
	GIBBS,
	///	ICM (d�terministe)
	ICM,
        /// MPM (Maximum Posterior Marginal)
        MPM,
        /// Internal mode
        SPECIAL,
      };

    enum IterType
      {
	///	It�ration noeud par noeud, ordre al�atoire
	VERTEX, 
	///	It�ration clique par clique, ordre al�atoire
	CLIQUE, 
	///	It�ration sur d'autres groupes de noeuds (je verrai + tard)
	CUSTOM
      };

    enum VoidMode
      {
	///	Pas de mode void
	VOIDMODE_NONE, 
	///	Mode r�gulier
	VOIDMODE_REGULAR, 
	///	Mode stochastique
	VOIDMODE_STOCHASTIC
      };

    enum InitLabelsType
      {
	///	Pas d'initialisation des labels: prendre ceux qui y sont d�j�
	INITLABELS_NONE, 
	INITLABELS_VOID, 
	INITLABELS_RANDOM
      };

    Anneal( CGraph & cg, MGraph & rg );
    virtual ~Anneal();

    /**	Fixe l'�tat initial
	@param	mode	Type de recuit (Anneal::METROPOLIS, Anneal::GIBBS ou 
	Anneal::ICM)
	@param	temp	Temp�rature initiale
	@param	tmult	Multiplicateur de d�croissance de temp�rature (entre 0 
	et 1)
	@param	tICM	Un recuit METROPOLIS ou GIBBS passe en mode ICM quand 
	la temp�rature atteint la temp�rature tICM
	@param	stopProp	Proportion de changements accept�s en dessous 
	duquel le recuit s'arr�te
	@param	gibbsMaxTrans	Nombre max. de transformations � la fois dans 
	une clique par l'algorithme du Gibbs Sampler 
	et l'ICM
	@param	verbosity	Si {\tt true} (par d�faut), affiche � chaque 
	pas la temp�rature, le mode de recuit, la 
	variation d'�nergie du pas, les nombres de 
	changements accept�s et propos�s. Si {\tt 
	false}, n'affiche rien.
	@param	itType		Type d'it�ration sur le graphe: types de 
	groupes de noeuds et ordre de passage. Pour 
	l'instant �a n'est utilis� qu'en modes ICM et 
	Gibbs. Pour Metropolis on verra plus tard.
	@param	plotStream	stream dans lequel les �nergies sont �crites
	� chaque pas de recuit, pour tra�age de 
	courbes. Si ce param�tre est laiss� � 0, rien 
	n'est �crit.
    */
    void init( Mode mode, double temp, double tmult, double tICM, 
	       double stopProp, unsigned gibbsMaxTrans, bool verbose = true, 
	       IterType itType = VERTEX, 
	       InitLabelsType voidInitialLabels = INITLABELS_RANDOM, 
	       const std::string & voidLabel = "", 
               std::ostream *plotStream = 0, 
               unsigned niterBelowStopProp = 1 );
    /**	Mode "void": essais de configs o� tous les labels sont remplac�s par
	void
	@param	mode	VOIDMODE_NONE: d�sactiv�; VOIDMODE_REGULAR: essais 
	r�guliers, une fois sur occurency; VOIDMODE_STOCHASTIC:
	essais irr�guliers, avec probabilit� 1/occurency
	@param	occurency	p�riodicit� de la passe en mode "void" */
    void setVoidMode( VoidMode mode, unsigned occurency = 0 );
    void deleteExtensions();
    void addExtension( AnnealExtension* ae, unsigned occurency = 20 );
    ///	Lib�re les structures allou�es dans les graphes
    void clear();
    ///	Revient aux conditions initiales
    void reset();
    ///	Teste le crit�re d'arr�t et fixe l'�tat correspondant
    void checkStop();
    ///	Effectue une passe de recuit
    void fitStep();
    ///	Effectue tout le recuit
    void fit();
    ///	Effectue une passe en mode M�tropolis
    void stepMetropolis();
    ///	Effectue une passe en mode Gibbs Sampler
    void stepGibbs();
    ///	Effectue une passe en mode ICM d�terministe
    void stepICM();
    /**	Effectue une passe en mode Void (essais de configs ou tout un label 
	est enlev�) */
    void stepVoid();
    /**	Calcule les diff�rentes �nergies des transitions possibles d'un 
	groupe de noeuds, en Gibbs Sampler ou en ICM.
	Cette fonction efface le vecteur et le re-remplit
    */
    void processPotentials( const std::set<Vertex *> & vertices, 
			    std::vector<EnergyField> & en );
    /**	Calcule les potentiels de toutes les cliques. 
	Cette fonction est utilis�e � l'initialisation, ou pour calculer 
	l'�nergie totale du graphe
	@return	�nergie totale du graphe
    */
    double processAllPotentials();

    /**@name	Interrogation de l'�tat du syst�me */
    //@{
    ///	Graphe � �tiqueter
    const CGraph & cGraph() const { return( _cgraph ); }
    CGraph & cGraph() { return( _cgraph ); }
    ///	Graphe mod�le
    const MGraph & rGraph() const { return( _mgraph ); }
    MGraph & rGraph() { return( _mgraph ); }
    ///	Mode de recuit initial
    Mode modeI() const { return( _modeI ); }
    ///	Mode courant
    Mode mode() const { return( _mode ); }
    ///	Temp�rature initiale
    double tempI() const { return( _tempI ); }
    ///	Temp�rature courante
    double temp() const { return( _temp ); }
    ///	Retourne true si le recuit est fini
    bool isFinished() const { return( _finished ); }
    ///	Multiplicxateur de temp�rature
    double tMult() const { return( _tmult ); }
    ///	Temp�rature de passage en ICM
    double tICM() const { return( _tICM ); }
    /**	Proportion de transformations accept�es en dessous de laquelle on 
	arr�te le recuit */
    double stopProp() const { return( _stopProp ); }
    ///	Nombre de transformations accept�es au cours de la derni�re passe
    unsigned nTrans() const { return( _ntrans ); }
    ///	Nombre de transformations propos�es au cours de la derni�re passe
    unsigned maxTrans() const { return( _maxtrans ); }
    /**	Nombre max. de transformations par groupe de noeuds dans le 
	Gibbs Sampler (et l'ICM) */
    unsigned gibbsMaxTrans() const { return( _gibbsMaxTrans ); }
    ///	Variation d'�nergie du graphe depuis le d�but du recuit
    double deltaE() const { return( _deltaE ); }
    ///	Variation d'�nergie du graphe � la derni�re passe
    double stepDeltaE() const { return( _stepDeltaE ); }
    double initialEnergy() const { return( _initEnergy ); }
    double energy() const { return( _initEnergy + _deltaE ); }
    ///	dit si les affichages sont autoris�s ou interdits
    bool verbosity() const { return( _verbose ); }
    IterType iterType() const { return( _iterType ); }
    ///	Nb d'it�rations de recuit effectu�es
    unsigned nIter() const { return( _niter ); }
    /**	Dit si on est en mode DoubleTirage, technologie � JeffProd'00, pour 
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
    ///	Graphe � �tiqueter
    CGraph				&_cgraph;
    ///	Graphe mod�le
    MGraph				&_mgraph;
    ///	Mode initial
    Mode				_modeI;
    ///	Mode courant
    Mode				_mode;
    ///	Temp�rature initiale
    double				_tempI;
    ///	Temp�rature courante
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

    /**	Fonction utilis�e par processPotentials().
	Calcule les potentiels pour les changements de nn noeuds parmi ceux de 
	num�ro au moins \c first
	@param	ver	groupe de noeuds � tester
	@param	ef	vecteur d'�nergies � remplir. Les nouvelles valeurs 
	sont ajout�es au vecteur
	@param	npos	tableau de bool�ens des positions de tous les noeuds 
	(de taille \c cl->size())
	@param	first	num�ro du 1er noeud qu'on a le droit d'utiliser
	@param	nn	nombre de noeuds � choisir (parmi les <tt>cl->size() 
	- first</tt>)
	@param	orLab	tableau des labels d'origine des noeuds de la clique
    */
    void processNodes( const std::set<Vertex *> & ver, std::vector<EnergyField> & ef, 
		       bool* npos, unsigned first, unsigned nn, 
		       std::string* orLab );
    /**	Fonction utilis�e par processPotentials().
	Calcule le potentiel d'une configuration, labels d�j� fix�s sur les 
	noeuds
	@param	ver	groupe de noeuds
	@param	ef	vecteur de champs d'�nergies auquel on ajoute une 
	entr�e
	@param	npos	liste des noeuds modifi�s
	@param	orLab	tableau des labels d'origine des noeuds de la clique
    */
    void processConfig( const std::set<Vertex *> & ver, 
			std::vector<EnergyField> & ef, 
			bool* npos, std::string* orLab );
  };

}

#endif


