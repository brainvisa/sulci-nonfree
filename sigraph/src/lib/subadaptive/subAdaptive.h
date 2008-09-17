

#ifndef SI_SUBADAPTIVE_SUBADAPTIVE_H
#define SI_SUBADAPTIVE_SUBADAPTIVE_H

#include <si/learnable/dbLearnable.h>
#include <si/subadaptive/subAdResponse.h>
#include <vector>
#include <set>
#include <map>
#include <string>

class Tree;


namespace sigraph
{

class SiVectorLearnable;
class AdaptiveLeaf;



/**	Sous-élément adaptatif (classe abstraite).

	Partie de Adaptive donnant soit la sortie proporement dite, soit 
	la partie évaluation de la validité de la réponse
*/
class SubAdaptive
{

public:
	struct Stat
	{
		double	mean;
		double	sigma;
	};

	enum RealClassMode
	{
		RealClassFromClassID,
		RealClassFromPotential
	};

	enum RelianceWeightMethod
	{
		MeanGenError,
		MisclassFrequency,
	};

	/// Facteur d'oubli pour moyenner les taux d'erreurs
	static double ForgetFactor;
	/// Facteur d'oubli pour moyenner les taux d'erreurs de généralisation
	static double GenForgetFactor;
	/**	Facteur d'oubli pour moyenner les taux d'erreurs de
	 *      généralisation des bons exemples */
	static double GenGoodForgetFactor;
	/**     Facteur d'oubli pour moyenner les taux d'erreurs de
	 *      généralisation des mauvais exemples */
	static double GenBadForgetFactor;
	///	Facteur d'oubli pour les min et max locaux
	static double LocalForgetFactor;

	virtual ~SubAdaptive();

	virtual SubAdaptive & operator = (const SubAdaptive & sa);
	///	copie
	virtual SubAdaptive* clone() const = 0;

	/**@name	Fonctions de base */
	//@{

	/// Cumule phase d'apprentissage (learn) et phase de test si besoin
	virtual SubAdResponse *train(AdaptiveLeaf &, 
			const SiDBLearnable &tr, const SiDBLearnable &tst) = 0;
	/// Apprentissage incrémental (vecteur par vecteur) ?
	virtual bool hasIncrementalLearning(void) const = 0;
	virtual double normalizedPotential(double outp) const;
	virtual double unNormalizedPotential(double nout) const;
	virtual double getLearnedLabel(const SiVectorLearnable &vl) const;
	//Appel de la fonction de test sur l'ensemble d'une DB
	virtual SubAdResponse *test(const SiDBLearnable &tst);
	/**	Test: met à jour le taux d'erreur de généralisation
	  Renvoie la valeur prédite par le classifieur*/
	virtual double test(const SiVectorLearnable &vl);
	/**	Propagation (model answer).
	  The output is normalized between minout and maxout, but not weighted 
	  by the relianceWeight() yet: this weight will be applied at a higher
	  level by AdaptiveLeaf.
	  */
	virtual double prop(const std::vector<double> & vec) = 0;
	///	Initialise l'apprentissage (fonction abstraite)
	virtual void init();
	/** Normalise le vecteur d'entrée et ne retourne que les composantes
	  sélectionnées dans le modèle
	  */
	virtual std::vector<double>*
		normalizeSelected(const double *vec, unsigned int size);

	/**	Ouvre un fichier pour écrire les vecteurs de données
	  @param	filename	nom du fichier à ouvrir. Si 
	  filename = "", prend un mon par défaut
	  avec l'extension .dat. Si filename
	  finit par '/' c'est considéré comme la base 
	  d'un nom qui sera complété automatiquement.
	  */
	virtual bool openFile(const std::string & filename = "");
	///	Ferme le fichier
	virtual void closeFile();
	///	répond si le fichier est ouvert ou non
	virtual bool fileOpened() const
	{ return(_stream != 0); }
	///	Choisit un nom de fichier par défaut
	virtual std::string
	chooseFilename(const std::string & basename = "") const;
	///	Donne la liste des fichiers sous le modèle
	virtual void subFiles(const std::string & prefix, 
			std::set<std::string> & listNames) const;

	//@}
	void	setRealClassMode(RealClassMode m) { _real_class_mode = m; }
	std::string getRealClassMode(void) const
	{
		static std::string   mode[] = {"class", "potential"};
		return mode[_real_class_mode];
	}


	/**@name	Statistiques (pour normaliser les entrées) */
	//@{
	///	Vide toutes les stats
	virtual void resetStats();
	/// SetStats from vectors.
	virtual void setStats(const std::vector<double> &mean,
			const std::vector<double> &sigma);
	///	Ajout à la base de statistiques
	virtual void learnStats(const std::vector<double> &vec, double outp=0);
	unsigned nStats() const;
	void setNStats(unsigned nstats);
	const std::map<unsigned, Stat> & stats() const;
	void setStats(const std::map<unsigned, Stat> & stats);
	double getMean(unsigned ind) { return _stats[ind].mean; }
	double getSigma(unsigned ind) { return _stats[ind].sigma; }
	double errorRate() const { return(_errorRate); }
	double genErrorRate() const { return(_genErrorRate); }
	double genGoodErrorRate() const { return(_ggErrorRate); }
	double genBadErrorRate() const { return(_gbErrorRate); }
	///C'est CETTE fonction qui est utilisée pour la pondération des experts
	double genMeanErrorRate() const;
	void setErrorRate(double rate) { _errorRate = rate; }
	void setGenErrorRate(double rate) { _genErrorRate = rate; }
	double localMinGErr() const { return(_ml); }
	double localMaxGErr() const { return(_Ml); }
	double globalMinGErr() const { return(_mg); }
	double globalMaxGErr() const { return(_Mg); }
	void setLocalMinGErr(double e) { _ml = e; }
	void setLocalMaxGErr(double e) { _Ml = e; }
	void setGlobalMinGErr(double e) { _mg = e; }
	void setGlobalMaxGErr(double e) { _Mg = e; }
	double misclassGoodRate() const { return _misclassGoodRate; }
	void setMisclassGoodRate(double x) { _misclassGoodRate = x; }
	double misclassBadRate() const { return _misclassBadRate; }
	void setMisclassBadRate(double x) { _misclassBadRate = x; }
	double misclassRate() const
	{ return (_misclassGoodRate + _misclassBadRate) / 2; }
	///Met à jour les taux d'erreur de généralisation (appelé par test())
	void updateErrors(double err);
	/**Met à jour les taux d'erreur de généralisation pour les bons exemples
	 * (appelé par test()) */
	void updateGoodErrors(double err);
	/**Met à jour les taux d'erreur de généralisation pour les bons exemples
	 * (appelé par test()) */
	void updateBadErrors(double err);
	void setGenGoodErrorRate(double rate) { _ggErrorRate = rate; }
	void setGenBadErrorRate(double rate) { _gbErrorRate = rate; }
	void setLocalGoodMinGErr(double e) { _mgl = e; }
	void setLocalGoodMaxGErr(double e) { _Mgl = e; }
	void setGlobalGoodMinGErr(double e) { _mgg = e; }
	void setGlobalGoodMaxGErr(double e) { _Mgg = e; }
	double appGoodErrorRate() const { return(_agErrorRate); }
	void setAppGoodErrorRate(double e) { _agErrorRate = e; }
	unsigned stepsSinceGenMin() const { return(_stepsSinceGenMin); }
	void setStepsSinceGenMin(unsigned n) { _stepsSinceGenMin = n; }
	void setGlobalGenGBError(double e) { _mggb = e; }
	void setLearnfinished(bool s) { _learnfinished = s; }
	bool getLearnfinished(void) const { return _learnfinished; }

	//@}

	///	Conversion en arbre (pour IO)
	virtual void buildTree(Tree & tr) const;
	std::string name() const { return(_name); }
	void setName(const std::string name) { _name = name; }
	virtual void setBaseName(const std::string &) {}
	///Transfère les fichiers ouverts d'un autre modèle (en cas de copie)
	virtual void getStreams(SubAdaptive & sa)
	{
		_stream = sa._stream;
		_tstream = sa._tstream; 
		sa._stream = 0;
		sa._tstream = 0;
	}
	///Noms des fichiers de sauvegardes (des réseaux de neurones par ex.)
	virtual void setFileNames(const std::string &) {}
	virtual std::string fileNames() const { return(""); }
	///	Fixe le min de la sortie
	virtual void setMinOut(double mino) { _minOut = mino; }
	///	Fixe le max de la sortie
	virtual void setMaxOut(double maxo) { _maxOut = maxo; }
	virtual double relianceWeight() const;
	RelianceWeightMethod relianceWeightMethod() const
	{ return _relianceWeightMethod; }
	void setRelianceWeightMethod(RelianceWeightMethod x)
	{ _relianceWeightMethod = x; }


protected:
	///	Entrées utiles
	std::string	_name;
	///	Nombre d'exemples dans les stats
	unsigned	_nStats;
	///	Statistiques pour chaque entrée
	std::map<unsigned, Stat>	_stats;
	///fichier de sauvegarde des données reçues en apprentissage
	std::ostream	*_stream;
	///fichier de sauvegarde pour les données de propagation / test
	std::ostream	*_tstream;
	///	Taux d'erreur d'apprentissage
	double	_errorRate;
	///	Taux d'erreur de généralisation
	double	_genErrorRate;
	///	Minimum global d'erreur
	double	_mg;
	///	Maximum global d'erreur
	double	_Mg;
	///	Minimum local d'erreur
	double	_ml;
	///	Maximum local d'erreur
	double	_Ml;
	///	Taux d'erreur de généralisation sur les bons exemples
	double	_ggErrorRate;
	///	Min local d'erreur sur les bons (généralisation)
	double	_mgl;
	///	Max local d'erreur sur les bons
	double	_Mgl;
	///	Min global d'erreur sur les bons
	double	_mgg;
	///	Max global d'erreur sur les bons
	double	_Mgg;
	///	Taux d'erreur d'apprentissage sur les bons exemples
	double	_agErrorRate;
	///	Nb de tests depuis le dernier minimum global
	unsigned	_stepsSinceGenMin;
	///	Taux d'erreur de généralisation sur les mauvais exemples
	double	_gbErrorRate;
	///     Min global de (erreur bons + erreur mauvais) en généralisation
	double	_mggb;
	double	_misclassGoodRate;
	double	_misclassBadRate;
	///	Borne inf de la sortie
	double	_minOut;
	///	Borne sup de la sortie
	double	_maxOut;
	/// L'apprentissage a été effectué
	bool	_learnfinished;

	/// Real Class mode
	RealClassMode	  _real_class_mode;
	RelianceWeightMethod  _relianceWeightMethod;


	SubAdaptive(const std::string name = "");
	SubAdaptive(const SubAdaptive & sa);

private:
};


//	Fonctions inline

inline SubAdaptive::SubAdaptive(const SubAdaptive & sa) 
	: _name(sa._name), _nStats(sa._nStats), 
	_stats(sa._stats), _stream(0), _tstream(0), 
	_errorRate(sa._errorRate), 
	_genErrorRate(sa._genErrorRate), _mg(sa._mg), _Mg(sa._Mg), 
	_ml(sa._ml), _Ml(sa._Ml), _ggErrorRate(sa._ggErrorRate), 
	_mgl(sa._mgl), _Mgl(sa._Mgl), _mgg(sa._mgg), _Mgg(sa._Mgg), 
	_agErrorRate(sa._agErrorRate), 
	_stepsSinceGenMin(sa._stepsSinceGenMin),
	_gbErrorRate(sa._gbErrorRate), _mggb(sa._mggb),
	_misclassGoodRate(sa._misclassGoodRate),
	_misclassBadRate(sa._misclassBadRate),
	_minOut(sa._minOut), _maxOut(sa._maxOut), _learnfinished(false),
	_real_class_mode(sa._real_class_mode),
	_relianceWeightMethod(MeanGenError)
{
}


inline SubAdaptive & SubAdaptive::operator = (const SubAdaptive & sa)
{
	if(this != &sa)
	{
		_name = sa._name;
		_nStats = sa._nStats;
		_stats = sa._stats;
		_errorRate = sa._errorRate;
		_genErrorRate = sa._genErrorRate;
		_mg = sa._mg;
		_Mg = sa._Mg;
		_ml = sa._ml;
		_Ml = sa._Ml;
		_ggErrorRate = sa._ggErrorRate;
		_mgl = sa._mgl;
		_Mgl = sa._Mgl;
		_mgg = sa._mgg;
		_Mgg = sa._Mgg;
		_agErrorRate = sa._agErrorRate;
		_stepsSinceGenMin = sa._stepsSinceGenMin;
		_gbErrorRate = sa._gbErrorRate;
		_mggb = sa._mggb;
		_misclassGoodRate = sa._misclassGoodRate;
		_misclassBadRate = sa._misclassBadRate;
		_minOut = sa._minOut;
		_maxOut = sa._maxOut;
		_learnfinished = sa._learnfinished;
		_real_class_mode = sa._real_class_mode;
		_relianceWeightMethod = sa._relianceWeightMethod;
	}
	return(*this);
}


inline unsigned SubAdaptive::nStats() const
{
	return(_nStats);
}


inline void SubAdaptive::setNStats(unsigned nstats)
{
	_nStats = nstats;
}


inline 
const std::map<unsigned, SubAdaptive::Stat> & SubAdaptive::stats() const
{
	return(_stats);
}


inline void SubAdaptive::setStats(const std::map<unsigned, Stat> & stats)
{
	_stats = stats;
}

inline double	SubAdaptive::normalizedPotential(double outp) const
{
	return (outp - _minOut) / (_maxOut == _minOut ? 1 : _maxOut - _minOut);
}

inline double SubAdaptive::unNormalizedPotential(double nout) const
{
	double	outp;

	outp = _minOut + nout * (_maxOut == _minOut ? 1 : _maxOut - _minOut);
	/* truncate subadaptive unNormalized output : to prevent errors from
	 * crazy subadaptive models. */
	if (outp < _minOut) outp = _minOut;
	else if (outp > _maxOut) outp = _maxOut;
	return outp;
}

}

#endif



