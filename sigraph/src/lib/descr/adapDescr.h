
#ifndef SI_DESCR_ADAPDESCR_H
#define SI_DESCR_ADAPDESCR_H


#include <si/descr/cliqueDescr.h>
#include <si/learnable/dbLearnable.h>


namespace sigraph
{
  class AdaptiveLeaf;
  class LearnConstParam;

  class GeneratedVector
  {
	public:
	GeneratedVector(std::vector<double> vec, double outp,
					int class_id, int cycle) :
		_vec(vec), _outp(outp), _class_id(class_id), _cycle(cycle) {};

	inline const std::vector<double>& getVector() const { return _vec; }
	inline double getOutp() const { return _outp; }
	inline int getClassID() const { return _class_id; }
	inline int getCycle() const { return _cycle; }

	private:
	std::vector<double>	_vec;
	double	_outp;
	int	_class_id;
	int	_cycle;
  };

  /**	Clique descriptor with description for learning
   */
  class AdapDescr : public CliqueDescr
  {
  public:
    virtual ~AdapDescr();

   
    /** Convert (clique, outp, classid) to GeneratedVector and insert it in
        a list */
    virtual void addGeneratedVector(const LearnConstParam *lp);
    virtual const std::list<GeneratedVector *>& getGeneratedVectors(void) const
    {
	    return _generated_vectors;
    }
    /** read _generated_vectors and update _learnable, generated vectors
     *  are finally deleted */
    virtual void updateSiDBLearnable(void);

    virtual SiDBLearnable &getSiDBLearnable();

    /**	Build input vector for lerning (allow noising it with random)
	@param	cl	Clique to convert
	@param	vec	(RETURN) vector of description.
	@param	model	Generic Object of model
	@param	outp	learning output
	@return		{\tt true} if OK, {\tt false} if not.
    */
    virtual bool makeLearnVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0,
				  double outp = 0 );
    virtual bool makeStatsVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0,
				  double outp = 0 );
    ///	Handle stats learning stats
    virtual void handleStats( const Clique* cl, std::vector<double> & vec, 
			      carto::GenericObject* model = 0,
			      double outp = 0 );
    ///	Reset (set to zero) all internal stats
    virtual void reset() {}
    virtual void clearDB();

  protected:
    AdapDescr();
  private:
    std::list<GeneratedVector *> _generated_vectors;
    SiDBLearnable	*_learnable;
  };



  //	inline


  inline bool 
  AdapDescr::makeStatsVector( const Clique* cl, std::vector<double> & vec, 
			      carto::GenericObject* model, double outp )
  {
    if( makeLearnVector( cl, vec, model, outp ) )
      {
	handleStats( cl, vec, model, outp );
	return( true );
      }
    else return( false );
  }


  inline bool 
  AdapDescr::makeLearnVector( const Clique* cl, std::vector<double> & vec, 
			      carto::GenericObject* model, double )
  {
    return( makeVector( cl, vec, model ) );
  }


  inline void AdapDescr::handleStats( const Clique*, std::vector<double> &, 
				      carto::GenericObject*, double )
  {
  }

}

#endif

