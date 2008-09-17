#ifndef SI_FOLD_INTERFOLDDESCR5_H
#define SI_FOLD_INTERFOLDDESCR5_H


#include <si/fold/interFoldDescr4.h>


namespace sigraph
{

  class InterFoldDescr5 : public InterFoldDescr4
  {
  public:
    ///	Liste des entrées
    enum Inputs
      {
	VEC_VALID, 

	SIZE_S1, 
	NCC_S1, 
	NCC_S1_REL, 

	SIZE_S2, 
	NCC_S2, 
	NCC_S2_REL, 

	SIZE_CORT, 
	DIST_MIN, 
	DIRX, 
	DIRY, 
	DIRZ, 
	DIST_EXTR_MIN1, 
	DIST_EXTR_MIN2, 
	GRELX, 
	GRELY, 
	GRELZ, 
	PSCAL_CORT, 

	NJONC, 
	MINDEPTH_JONC, 
	SIZE_JONC, 

	NPP, 
	MAXDEPTH_PP, 

        MOMENT_INV0, 
        MOMENT_INV1, 
        MOMENT_INV2, 
        MOMENT_INV3, 
        MOMENT_INV4, 
        MOMENT_INV5, 
        MOMENT_INV6, 
        MOMENT_INV7, 
        MOMENT_INV8, 
        MOMENT_INV9, 
        MOMENT_INV10, 
        MOMENT_INV11, 

	END
      };

    InterFoldDescr5();
    InterFoldDescr5( const InterFoldDescr5 & ifd );
    virtual ~InterFoldDescr5();
    virtual CliqueDescr* clone() const;

    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
  };

  //	inline

  inline InterFoldDescr5::InterFoldDescr5() 
    : InterFoldDescr4()
  {
  }


  inline InterFoldDescr5::InterFoldDescr5( const InterFoldDescr5 & ifd )
    : InterFoldDescr4( ifd )
  {
  }


  inline CliqueDescr* InterFoldDescr5::clone() const
  {
    return( new InterFoldDescr5( *this ) );
  }

}

#endif

