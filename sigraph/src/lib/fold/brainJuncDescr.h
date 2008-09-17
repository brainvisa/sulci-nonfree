
#ifndef SI_FOLD_BRAINJUNCDESCR_H
#define SI_FOLD_BRAINJUNCDESCR_H


#include <si/descr/adapDescr.h>


namespace sigraph
{

  class BrainJuncDescr : public AdapDescr
  {
  public:
    BrainJuncDescr();
    BrainJuncDescr( const BrainJuncDescr & h );
    virtual ~BrainJuncDescr();
    virtual CliqueDescr* clone() const;

    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model ) const;
    virtual void buildTree( Tree & t );

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );

  private:
  };


  //	inline


  inline CliqueDescr* BrainJuncDescr::clone() const
  {
    return( new BrainJuncDescr( *this ) );
  }

}

#endif


