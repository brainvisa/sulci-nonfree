
#ifndef SI_FOLD_BUILDMODEL_H
#define SI_FOLD_BUILDMODEL_H


#include <si/fold/frgraph.h>
#include <si/model/model.h>
#include <si/domain/domain.h>


namespace sigraph
{

  /**	Classe labels de sillons. \\
	Lecture de liste de labels, création du modèle correspondant
  */
  class FoldLabels : public std::set<std::string>
  {
  public:
    FoldLabels();
    FoldLabels( const std::string & filename );
    ~FoldLabels();

    void readLabels( const std::string & filename );
    MGraph* createModel( const Model* mod, const Domain* dom, 
			 const Model* defMod = 0 ) const;

  protected:
    ///	Crée le noeud et les bons attributs, duplique le modèle et le domaine
    void makeVertex( MGraph* mg, const std::string & label, const Model* mod, 
		     const Domain* dom ) const;

  private:
  };

  //	inline

  inline FoldLabels::FoldLabels() : std::set<std::string>()
  {
  }


  inline FoldLabels::FoldLabels( const std::string & filename )
    : std::set<std::string>()
  {
    readLabels( filename );
  }


  inline FoldLabels::~FoldLabels()
  {
  }

}

#endif

