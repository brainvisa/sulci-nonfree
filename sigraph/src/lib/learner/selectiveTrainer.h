
#ifndef SI_LEARNER_SELECTIVETRAINER_H
#define SI_LEARNER_SELECTIVETRAINER_H


#include <si/learner/trainer.h>
#include <si/graph/mgWriter.h>
#include <sys/types.h>
#include <regex.h>


namespace sigraph
{
  class CGraph;
  class Adaptive;

  ///	Trainer sélectif: n'apprend que certains modèles
  class SelectiveTrainer : public Trainer
  {
  public:
    SelectiveTrainer(MGraph &mg, Learner *learner = NULL,
		      const std::string &pattern = ".*");
    virtual ~SelectiveTrainer();

    /**@name	Configuration */
    //@{
    ///	Filtre
    virtual void setPattern(const std::string &patt);
    /* If mixedatts is true, a single string will be built from all the
       given attributes found in a clique, and the regex will work on this
       string, so it can deal with combinations.
       The sting will be a concatenation of "attr=value;".
    */
    virtual void setFiltAttributes(const std::set<std::string> &atts, bool
        mixedatts = false );
    //@}
    virtual void init(TrainerMode mode, unsigned pass = 0);

    /**@name	Utilisation */
    //@{
    virtual Trainer::CliquesModelMap *
    dataBaseToCliquesModelMap(const std::set<CGraph *> &lrn);
        virtual void save(MGWriter &mg);
    bool checkAdap(carto::AttributedObject* ao, Adaptive* adap);
    //@}

    const std::set<Adaptive*> &usedAdap(void) const { return _usedAdap; }

  protected:
    virtual std::set<Model *> 
    *modelsFromCliquesModelMap(const CliquesModelMap *cllrn, 
                                const CliquesModelMap *cltst);


  private:
    std::set<Adaptive*>		_usedAdap;
    std::set<std::string>	_atts;
    bool                        _mixedAtts;
    regex_t			_pattern;
  };

}

#endif


