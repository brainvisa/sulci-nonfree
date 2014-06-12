
#ifndef SI_FOLD_FOLDDESCR5_H
#define SI_FOLD_FOLDDESCR5_H


#include <si/fold/foldDescr4.h>


namespace sigraph
{

  /**	Fold descriptor, version 5 (07/2006).
     They add invariant moments information
  */
  class FoldDescr5 : public FoldDescr4
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
      MEANDEPTH,

      NCC,
      NCC_NOT_CORTICAL,
      NCORT,
      DISTMAX_CC,
      NPLIS,
      SIZE_HULLJUNC,

      GM_THICKNESS,
      FOLD_OPENING_HALF,
      FOLD_OPENING,

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

      INERTIA_0,
      INERTIA_1,
      INERTIA_2,
      INERTIA_3,
      INERTIA_4,
      INERTIA_5,

      INERTIA_EIGENVALUE_0,
      INERTIA_EIGENVALUE_1_RATIO,
      INERTIA_EIGENVALUE_2_RATIO,

      END
    };

    FoldDescr5();
    FoldDescr5( const FoldDescr5 & f );
    virtual ~FoldDescr5();
    virtual CliqueDescr* clone() const;

    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;
    bool outputInertia() const { return _outputInertia; }
    void setOutputInertia( bool x ) { _outputInertia = x; }

  protected:
    virtual bool makeVectorElements( const Clique* cl,
                                     std::vector<double> & vec,
                                     carto::GenericObject* model = 0 );

  private:
    bool _outputInertia;
  };


  //	inline


  inline CliqueDescr* FoldDescr5::clone() const
  {
    return( new FoldDescr5( *this ) );
  }

}

#endif

