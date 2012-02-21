
#ifndef SI_GRAPH_ANNEALCONFIGURATOR_H
#define SI_GRAPH_ANNEALCONFIGURATOR_H


#include <si/graph/anneal.h>
#include <fstream>


namespace sigraph
{

  /**	Lecture de fichiers de params de recuit (syntaxe siRelax.stx).
	Les données sont publiques parce que c'est un truc vite fait, et c'est 
	plus une structure qu'une classe
  */
  class AnnealConfigurator
  {
  public:
    AnnealConfigurator();
    virtual ~AnnealConfigurator();

    /// ne lit pas les graphes
    virtual bool loadConfig( const std::string & filename );
    virtual void saveConfig( const std::string & filename );
    /// ne s'occupe pas des "plotfiles" (il doit être ouvert avant)
    virtual void initAnneal( Anneal &ann, std::ofstream *plotf = 0 ) const;
    /// remet tout à zéro (config par défaut)
    virtual void init();
    /// appelée par loadConfig()
    virtual bool processParams();
    virtual void loadGraphs( MGraph & rg, CGraph & fg );

    std::string 		modelFile;
    std::string 		graphFile;
    std::vector<std::string>	graphFiles;
    std::string			output;
    std::vector<std::string>	outputs;
    std::string 		labelsMapFile;
    int				save;
    int				initMode;
    float			temp;
    std::string			mode;
    float			rate;
    float			tempICM;
    float			stopRate;
    int				gibbsChange;
    int				verbose;
    std::string			iterType;
    Anneal::Mode		bmode;
    Anneal::IterType		bItType;
    float			setWeights;
    int				removeVoid;
    std::string			plotFile;
    std::string			initLabelTypeString;
    Anneal::InitLabelsType	initLabelType;
    std::string			voidLabel;
    std::string			voidMode;
    int				voidOccurency;
    Anneal::VoidMode		bvoidmode;
    std::string			extensionMode;
    int				extPassOccurency;
    int				doubleDrawingLots;
    std::vector<std::string>	extModes;
    int				niterBelowStopProp;
    int                         allowThreads;
    int                         maxIterations;
    int                         mpmUnrecordedIterations;
    int                         forbidVoidLabel;
  protected:

  private:
  };

}

#endif

