
#ifndef SI_FOLD_INTERFOLDDESCR_H
#define SI_FOLD_INTERFOLDDESCR_H


#include <si/descr/adapDescr.h>


namespace sigraph
{

  class InterFoldDescr : public AdapDescr
  {
  public:
    ///	Liste des entrées
    enum Inputs
      {
	VEC_VALID, 
	SIZE1, 
	///	Composantes connexes / jonctions
	CONN1, 
	SIZE2, 
	///	Composantes connexes / jonctions
	CONN2, 
	DIST, 
	CORT_SIZE, 
	JUNC_SIZE, 
	DX, 
	DY, 
	DZ,
	CORT_VALID, 
	CX, 
	CY, 
	CZ, 
	JUNC_VALID, 
	JX, 
	JY, 
	JZ, 
	END
      };

    InterFoldDescr();
    InterFoldDescr( const InterFoldDescr & ifd );
    virtual ~InterFoldDescr();
    virtual CliqueDescr* clone() const;

    /**	Remplit le vecteur d'entrées. 
     */
    virtual bool makeVector( const Clique* cl, std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;
    virtual void buildTree( Tree & t );

    virtual bool makeLearnVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0 );

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );

  private:
  };

  //	inline

  inline InterFoldDescr::InterFoldDescr() : AdapDescr()
  {
  }


  inline InterFoldDescr::InterFoldDescr( const InterFoldDescr & ifd )
    : AdapDescr( ifd )
  {
  }


  inline CliqueDescr* InterFoldDescr::clone() const
  {
    return( new InterFoldDescr( *this ) );
  }

}

#endif

