
#ifndef SI_SUBADAPTIVE_SUBADMLP_H
#define SI_SUBADAPTIVE_SUBADMLP_H



#include <si/subadaptive/incrementalSubAdaptive.h>
#include <cartobase/object/object.h>
#include <neur/mlp/mlp.h>
class Tree;


namespace sigraph
{

  /**	Classe élémentaire contenant un perceptron multicouches
   */
  class SubAdMlp : public IncrementalSubAdaptive
  {
  public:
    SubAdMlp( const std::string name = "" );
    /**	Constructeur qui charge le réseau
	@param	name	nom du SubAdMlp (identifiant pour work / eval)
	@param	file	nom de fichier réseau (.net) écrit dans le fichier 
	du modèle
	@param	filename	nom complet du fichier réseau
    */
    SubAdMlp( const std::string name, const std::string file, 
	      const std::string filename );
    SubAdMlp( const char* nom, int nc, int* couch );
    SubAdMlp( const SubAdMlp & sa );
    virtual ~SubAdMlp();

    virtual SubAdMlp & operator = ( const SubAdMlp & sa );

    ///	copie
    virtual SubAdaptive* clone() const;

    /**@name	Fonctions de base */
    //@{

    /**	Prépare le réseau avec les stats sur le vecteur etc. */
    virtual void prepare(const std::vector<double> &v);
    virtual void prepare(const SiVectorLearnable &vl);
    virtual void prepare(const double *vec, unsigned int size);
    /** Return learned label of the vector. */
    virtual double getLearnedLabel(const SiVectorLearnable &vl) const;

    virtual SubAdResponse *train(AdaptiveLeaf &, 
		const SiDBLearnable &tr, const SiDBLearnable &tst);
   double learn(AdaptiveLeaf &al, const SiDBLearnable &train,
				const SiDBLearnable &tst);
    /**	Apprentissage
	@return	erreur sur l'exemple avant l'apprentissage
    */
    double learn(const SiVectorLearnable &vl);
    ///	Propagation
    virtual double prop( const std::vector<double> & );
    ///	Test: met à jour le taux d'erreur de généralisation
    virtual double test(const SiVectorLearnable &vl);
    ///	{\tt basename} doit être vide ou terminé par '/'
    virtual std::string chooseFilename( const std::string 
					& basename = "" ) const;
    ///	Donne la liste des fichiers sous le modèle
    virtual void subFiles( const std::string & prefix, 
			   std::set<std::string> & listNames ) const;
    ///	Ajout à la base de statistiques
    virtual void learnStats( const std::vector<double> &vec, double outp = 0 );
 
    virtual void init();

    //@}

    /**@na Data Access */
    //@{
    ///Network access (R/W)
    mlp<double,double> & net() { return( _net ); }
    ///Learning step
    double eta() const { return( _eta ); }
    void setEta( double eta ) { _eta = eta; }
    //@}

    ///	Convert to tree (for IO)
    virtual void buildTree( Tree & tr ) const;
    virtual void setBaseName( const std::string & basename );
    ///	Noms des fichiers de sauvegardes (des réseaux de neurones par ex.)
    virtual void setFileNames( const std::string & name )
      { _netFileName = name; }
    virtual std::string fileNames() const { return( _netFileName ); }


  protected:
    ///	Multi layer perceptron
    mlp<double,double>	_net;
    ///	Learning step
    double		_eta;
    ///	netfile name
    std::string	_netFileName;

  private:
  };


  //	Fonctions inline

  inline SubAdMlp::SubAdMlp( const SubAdMlp & sa )
    : IncrementalSubAdaptive( sa ), _net( sa._net ), _eta ( sa._eta ), 
      _netFileName( sa._netFileName )
  {
  }


  inline SubAdMlp & SubAdMlp::operator = ( const SubAdMlp & sa )
  {
    if( this != &sa )
      {
	IncrementalSubAdaptive::operator = ( sa );
	_net = sa._net;
	_eta = sa._eta;
	_netFileName = sa._netFileName;
      }
    return( *this );
  }


  inline SubAdaptive* SubAdMlp::clone() const
  {
    return( new SubAdMlp( *this ) );
  }

}


namespace carto
{
#define _TMP_ mlp<double, double> *
  DECLARE_GENERIC_OBJECT_TYPE( _TMP_ )
#undef _TMP_
}

#endif


