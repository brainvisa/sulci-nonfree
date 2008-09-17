#ifndef SI_SUBADAPTIVE_SUBADSVM_H
#define SI_SUBADAPTIVE_SUBADSVM_H

#ifndef SI_NO_SVMLIB

#include <si/subadaptive/nonIncrementalSubAdaptive.h>
#include <datamind/libsvm/libsvm.h>
#include <cartobase/object/attributed.h>
#include <iostream>


namespace sigraph
{

  class SubAdSvm : public NonIncrementalSubAdaptive
  {
  public:
    enum SvmMode 
      {
        Classifier, 
        Probability, 
        Regression, 
        Quality, 
        Decision, 
	OneClass,
      };

    // SubAdSvm( const std::string name = "" );
    SubAdSvm( const std::string name, const std::string file, 
	      const std::string filename );
    SubAdSvm( const SubAdSvm & sa );
    virtual ~SubAdSvm();

    virtual SubAdSvm & operator = ( const SubAdSvm & sa );

    virtual SubAdaptive* clone() const;

    /** Prépare le réseau avec les stats sur le vecteur etc. */
    virtual void prepare(const std::vector<double> &v);
    virtual void prepare(const SiVectorLearnable &vl);
    virtual void prepare( const double *vec, unsigned int size);
    virtual struct svm_problem *prepare(const SiDBLearnable &vl);
    virtual std::string getSvmMode(void) const;
    virtual double learn(const SiDBLearnable &train);
    virtual double getLearnedLabel(const SiVectorLearnable &vl) const;
    virtual SubAdResponse *crossvalidation(const SiDBLearnable &train,
		const unsigned int nbfolds);
    virtual double prop( const std::vector<double> & );
    virtual double test(const SiVectorLearnable &vl);
    /// {\tt basename} doit être vide ou terminé par '/'
    virtual std::string chooseFilename( const std::string
                                        & basename = "" ) const;
    virtual void subFiles( const std::string & prefix,
                           std::set<std::string> & listNames ) const;
    /// Ajout à la base de statistiques
    virtual void learnStats( const std::vector<double> &vec, double outp = 0 );
    SvmMode svmMode() const { return _svmmode; }
    void setSvmMode( SvmMode m ) { _svmmode = m; }
    float qualitySlope() const { return _qualityslope; }
    void setQualitySlope( float x ) { _qualityslope = x; }
    /** in shitfed output mode (the new default), examples classified as 
        bad always get an output >= 0.5 */
    bool qualityShiftedBadOutput() const { return _qualityshifted; }
    void setQualityShiftedBadOutput ( bool x ) { _qualityshifted = x; }
    struct svm_parameter &getSvmParameter() { return _svm_param; }

    virtual void buildTree( Tree & tr ) const;
    virtual void setBaseName( const std::string & basename );
    // SAParser factory function
    static void buildSubSvm( carto::AttributedObject* parent, Tree* t, 
                             const std::string & filename );

    virtual void setFileNames( const std::string & name )
      { _netFileName = name; }
    virtual std::string fileNames() const { return( _netFileName ); }
    /** Set / Get svm parameters */
    inline void setSvmWeight(int label, double weight) {
	    _svm_param.weight[label] = weight;
    };
    void setGamma(double gamma) { _svm_param.gamma = gamma; };
    void setC(double c) { _svm_param.C = c; };
    void setEpsilon(double p) { _svm_param.p = p; };
    void setNu(double nu) { _svm_param.nu = nu; };
    double getGamma(void) { return _svm_param.gamma; }
    double getC(void) { return _svm_param.C; }
    double getEpsilon(void) { return _svm_param.p; }
    double getNu(void) { return _svm_param.nu; }
                                                                                
  protected:
    svm_model	*_svm;
    /// Nom du fichier réseau
    std::string _netFileName;

  private:
    std::vector<double>		_inputs;
    SvmMode			_svmmode;
    float			_qualityslope;
    bool			_qualityshifted;
    struct svm_parameter	_svm_param;
    struct svm_problem		_svm_prob;
    struct svm_node		*_svm_nodes;


    static void parseSubSvm( carto::AttributedObject* parent, Tree* t, 
			     SubAdSvm & sad );
  };


  //	Fonctions inline
  inline SubAdSvm & SubAdSvm::operator = ( const SubAdSvm & sa )
  {
    if( this != &sa )
      {
	NonIncrementalSubAdaptive::operator = ( sa );

	_svm = svm_clone_model(sa._svm, sa._svm_nodes, &_svm_nodes);
        _svmmode = sa._svmmode;
	_netFileName = sa._netFileName;
	_svm_param = sa._svm_param;
      }
    return( *this );
  }


  inline SubAdaptive* SubAdSvm::clone() const
  {
    return( new SubAdSvm( *this ) );
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( svm_model * )
}

#endif // SI_NO_SVMLIB

#endif



