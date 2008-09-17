
#ifndef SI_MIXER_MIXERPARSER_H
#define SI_MIXER_MIXERPARSER_H


#include <si/graph/treeParser.h>


namespace sigraph
{
  class Mixer;

  class MixerParser : public TreePostParser
  {
  public:
    MixerParser();
    virtual ~MixerParser();

    virtual FactorySet factories();
    static void buildMean( carto::AttributedObject* parent, Tree* t, 
			   const std::string & filename );

  protected:
    static void parseMix( carto::AttributedObject* parent, Tree* t, 
			  Mixer* mix );

  private:
  };


  //	inline

  inline MixerParser::MixerParser()
  {
  }


  inline MixerParser::~MixerParser()
  {
  }

}

#endif



