
namespace sigraph
{

class Anneal
{
%TypeHeaderCode
#include <si/graph/anneal.h>
%End

public:
  enum Mode
    {
      METROPOLIS, 
      GIBBS,
      ICM
    };

  enum IterType
    {
      VERTEX, 
      CLIQUE, 
      CUSTOM
    };

  enum VoidMode
    {
      VOIDMODE_NONE, 
      VOIDMODE_REGULAR, 
      VOIDMODE_STOCHASTIC
    };

  enum InitLabelsType
    {
      INITLABELS_NONE, 
      INITLABELS_VOID, 
      INITLABELS_RANDOM
    };

  Anneal( sigraph::CGraph &, sigraph::MGraph & );
  virtual ~Anneal();

  void init( sigraph::Anneal::Mode, double, double, double, 
             double, unsigned, bool = true, 
             sigraph::Anneal::IterType = sigraph::Anneal::VERTEX, 
             sigraph::Anneal::InitLabelsType 
               = sigraph::Anneal::INITLABELS_RANDOM, 
             const std::string & = "" /* , 
             std::ostream * = 0, 
             unsigned = 1 */ );
  void setVoidMode( sigraph::Anneal::VoidMode, unsigned = 0 );
  //void deleteExtensions();
  void addExtension(sigraph::AnnealExtension *ae, unsigned occurency=20);
  void clear();
  void reset();
  void checkStop();
  void fitStep();
  void fit();
  void stepMetropolis();
  void stepGibbs();
  void stepICM();
  void stepVoid();
  /* void processPotentials( const std::set<Vertex *> & vertices, 
                             std::vector<EnergyField> & en ); */
  double processAllPotentials();

  // const sigraph::CGraph & cGraph() const;
  sigraph::CGraph & cGraph();
  // const sigraph::MGraph & rGraph() const;
  sigraph::MGraph & rGraph();
  sigraph::Anneal::Mode modeI() const;
  sigraph::Anneal::Mode mode() const;
  double tempI() const;
  double temp() const;
  bool isFinished() const;
  double tMult() const;
  double tICM() const;
  double stopProp() const;
  unsigned nTrans() const;
  unsigned maxTrans() const;
  unsigned gibbsMaxTrans() const;
  double deltaE() const;
  double stepDeltaE() const;
  double initialEnergy() const;
  double energy() const;
  bool verbosity() const;
  sigraph::Anneal::IterType iterType() const;
  unsigned nIter() const;
  // bool doubleDrawingLots() const;
  // void setDoubleDrawingLots( bool );
  const std::string & voidLabel() const;
};


class AnnealConfigurator
{
%TypeHeaderCode
#include <si/graph/annealConfigurator.h>
%End

public:
  AnnealConfigurator();
  virtual ~AnnealConfigurator();

%If (SIPTHROW_BUG)
  virtual bool loadConfig( const std::string & filename )
    throw ( );
%End
%If (!SIPTHROW_BUG)
  virtual bool loadConfig( const std::string & filename )
    throw ( std::exception, carto::io_error );
%End
  virtual void saveConfig( const std::string & filename );
  virtual void init();
  virtual bool processParams();
  virtual void loadGraphs( sigraph::MGraph & rg, sigraph::CGraph & fg );

  /* remove virtual to compile this function (virtual impose to preserve
   * C++ signature in sip 4.5.2 */
  void initAnneal( sigraph::Anneal &ann, std::string &) const;
%MethodCode
  std::ofstream	*of = new std::ofstream(a1->c_str());
  sipCpp->initAnneal(*a0, of);
%End

public:
  std::string	plotFile;
  std::string	output;
};

class AnnealExtension
{
%TypeHeaderCode
#include <si/graph/annealExtension.h>
%End
public:
	AnnealExtension(sigraph::Anneal *ann);
	virtual ~AnnealExtension();
	virtual void specialStep(unsigned passnum=0) = 0;
	unsigned ntrans() const;
	unsigned maxTrans() const;
	double stepDeltaE() const;
	virtual std::string name() const = 0;
};


};

