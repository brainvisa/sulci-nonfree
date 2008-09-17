


#ifndef SI_GLOBAL_GLOBAL_H
#define SI_GLOBAL_GLOBAL_H


#include <string>


namespace sigraph
{

  /**	classe fourre-tout, juste pour grouper des variables et des fonctions 
	globales
  */
  class Si
  {
  public:
    Si();
    ~Si();

    std::string basePath() const { return( _basePath ); }
    std::string labelsTranslPath() const { return( _labelsTranslPath ); }
    void setLabelsTranslPath( const std::string & filename )
    { _labelsTranslPath = filename; }
    static const std::string & version();
    static const std::string & shortVersion();

  private:
    static std::string getBasePath();
    std::string	_basePath;
    std::string	_labelsTranslPath;
  };


  ///	Accès au singleton Si
  Si & si();

}

#endif


