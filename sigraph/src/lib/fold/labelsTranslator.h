
#ifndef SI_FOLD_LABELSTRANSLATOR_H
#define SI_FOLD_LABELSTRANSLATOR_H


#include <map>
#include <string>
#include <si/graph/cgraph.h>

class Tree;

namespace aims
{
  class SelectionSet;
}


namespace sigraph
{

  /**	Table de correspondance entre labels réels (des graphes de sillons) 
	et labels modèles (des graphes modèles). \\
	Elle permet de passer des graphes exemples dans la nomenclature du 
	modèle avant de travailler avec.
  */
  class FoldLabelsTranslator : public std::map<std::string, std::string>
  {
  public:
    FoldLabelsTranslator() : std::map<std::string, std::string>() {}
    FoldLabelsTranslator( const std::string & filename ) 
      : std::map<std::string, std::string>()
    { readLabels( filename ); }
    FoldLabelsTranslator(  const MGraph & mg, 
			   const std::string & filename = "" ) 
      : std::map<std::string, std::string>()
    { makeFromModel( mg, filename ); }
    ~FoldLabelsTranslator() {}

    /**	Initialise la table à partir d'un fichier.
	Ce fichier est soit une hiérarchie de noms (format Aims/Anatomist), 
	soit un fichier de traduction au format Jeff (JFM95) (en général, 
	\c "sillons_modele.def" ) */
    void readLabels( const std::string & filename );
    /**	Initializes table from a file AND a model graph. 
	Correspondances are read in the file and translations lead to labels 
	existing in the model. The labels file is either in Jeff's format 
	(JFM95) or in hierarchy format (Aims/Anatomist format)
    */
    void makeFromModel( const MGraph & mg, const std::string & filename );
    /**	Change les labels d'un graphe. 
	@param	ilabel	attribut contenant les labels initiaux
	@param	olabel	attribut contenant les labels changés
	@param	altilabel alternate input label (used if ilabel is absent)
    */
    void translate( CGraph & gr, const std::string & ilabel="name", 
		    const std::string & olabel="name", 
		    const std::string & altilabel = "" ) const;
    std::string lookupLabel( std::string label ) const;

  protected:
    bool readJFM95( const std::string & filename );
    bool readHierarchy( const std::string & filename );
    bool readSelection( const std::string & filename );
    bool makeFromModelJFM95( const MGraph & mg, const std::string & filename );
    bool makeFromModelHierarchy( const MGraph & mg, 
				 const std::string & filename );
    /* bool makeFromModelSelection( const MGraph & mg, 
       const std::string & filename ); */

  private:
    void insertTree( const Tree & t );
    void insertTree( const Tree & t, const std::set<std::string> & mnames );
    void insertSelection( const aims::SelectionSet & );
  };

}

#endif


