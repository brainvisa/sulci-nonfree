
#ifndef SI_FOLD_FOLDDESCR3_H
#define SI_FOLD_FOLDDESCR3_H

#include <si/fold/foldDescr2.h>

namespace sigraph
{

  /**	Cortical Fold Descriptor, version 3
  */
  class FoldDescr3 : public FoldDescr2
  {
  public:
    ///	Descriptors list
    enum Inputs
      {
	VEC_VALID,
	E1E2_VALID, 
	E1X, 
	E1Y, 
	E1Z, 
	E2X, 
	E2Y, 
	E2Z, 
	GX, 
	GY, 
	GZ, 

	NVALID, 
	NX, 
	NY, 
	NZ, 
	DIRX, 
	DIRY, 
	DIRZ, 

	SIZE, 
	DEPTH, 
	MINDEPTH, 

	NCC, 
	NCC_NOT_CORTICAL, 
	NCORT, 
	DISTMAX_CC,
	NPLIS, 
	SIZE_HULLJUNC, 

	SURFACE, 

	END
      };

    FoldDescr3();
    FoldDescr3( const FoldDescr3 & f );
    virtual ~FoldDescr3();
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


  inline CliqueDescr* FoldDescr3::clone() const
  {
    return( new FoldDescr3( *this ) );
  }

}

#endif

